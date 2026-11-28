#!/usr/bin/env python3
"""Validate static PIE ELF64 images emitted by `occ --backend native --target linux-x86_64`.

Checks the ELF64 header (ET_DYN, EM_X86_64, 3 program headers), strict W^X PT_LOAD
segments plus non-executable PT_GNU_STACK, and verifies .text has no legacy fixed
0x400000-page movabs data immediates.
Usage: check_elf.py <image> [...]
"""
import struct
import sys

PT_LOAD = 1
PT_GNU_STACK = 0x6474E551
PF_X = 1
PF_W = 2
PF_R = 4


def check(path):
    data = open(path, "rb").read()
    assert data[:4] == b"\x7fELF", "missing ELF magic"
    assert data[4] == 2 and data[5] == 1, "not 64-bit little-endian ELF"
    e_type, e_machine, e_version = struct.unpack_from("<HHI", data, 16)
    assert e_type == 3, "e_type != ET_DYN (3, static PIE): got %d" % e_type
    assert e_machine == 62, "e_machine != EM_X86_64 (62): got %d" % e_machine
    assert e_version == 1, "e_version != 1"

    e_entry, e_phoff = struct.unpack_from("<QQ", data, 24)
    e_phentsize, e_phnum = struct.unpack_from("<HH", data, 54)
    assert e_phentsize == 56, "e_phentsize != 56"
    assert e_phnum == 3, "expected 3 program headers, got %d" % e_phnum

    phdrs = []
    for i in range(e_phnum):
        off = e_phoff + i * e_phentsize
        p_type, p_flags, p_offset, p_vaddr, p_paddr, p_filesz, p_memsz, p_align = struct.unpack_from(
            "<IIQQQQQQ", data, off
        )
        assert not ((p_flags & PF_W) and (p_flags & PF_X)), "W+X segment detected!"
        phdrs.append((p_type, p_flags, p_offset, p_vaddr, p_paddr, p_filesz, p_memsz, p_align))

    p0, p1, p2 = phdrs
    assert p0[0] == PT_LOAD and p0[1] == PF_R, "Phdr[0] must be read-only PT_LOAD"
    assert p0[3] == 0 and p0[4] == 0, "Phdr[0] vaddr/paddr must be 0 for PIE"
    assert p1[0] == PT_LOAD and p1[1] == (PF_R | PF_X), "Phdr[1] must be RX PT_LOAD"
    assert p1[3] >= 0x1000 and p1[3] == e_entry, "Phdr[1] vaddr must match e_entry >= 0x1000"
    assert p2[0] == PT_GNU_STACK and not (p2[1] & PF_X), "Phdr[2] PT_GNU_STACK must be NX"

    text = data[p1[2]:p1[2] + p1[5]]
    for idx in range(max(0, len(text) - 9)):
        if text[idx] == 0x48 and 0xB8 <= text[idx + 1] <= 0xBF:
            imm = struct.unpack_from("<Q", text, idx + 2)[0]
            assert not (0x400100 <= imm < 0x410000), "unpatched fixed 0x400000-page movabs at .text+%#x: %#x" % (idx, imm)

    return "ELF64 x86_64 static PIE (ET_DYN) OK, entry=%#x, W^X + NX stack" % e_entry


def main(paths):
    if not paths:
        print(__doc__.strip().splitlines()[-1])
        return 2
    for path in paths:
        try:
            print("%s: %s" % (path, check(path)))
        except (AssertionError, ValueError, struct.error) as exc:
            print("%s: INVALID ELF64 image: %s" % (path, exc))
            return 1
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
