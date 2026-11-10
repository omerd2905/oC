#!/usr/bin/env python3
"""Validate PE32+ images emitted by `occ --backend native --target windows-x86_64`.

Checks the DOS/COFF/optional headers, strict W^X section flags (no writable or
writable+executable section), and that the import table binds KERNEL32.dll.
Usage: check_pe.py <image.exe> [...]
"""
import struct
import sys

SCN_CNT_CODE = 0x00000020
SCN_CNT_INITIALIZED_DATA = 0x00000040
SCN_MEM_EXECUTE = 0x20000000
SCN_MEM_READ = 0x40000000
SCN_MEM_WRITE = 0x80000000

EXPECTED_SECTIONS = {
    b".text": SCN_CNT_CODE | SCN_MEM_EXECUTE | SCN_MEM_READ,         # 0x60000020
    b".rdata": SCN_CNT_INITIALIZED_DATA | SCN_MEM_READ,              # 0x40000040
}


def cstr(data, off):
    end = data.index(b"\0", off)
    return data[off:end].decode("ascii")


def check(path):
    data = open(path, "rb").read()
    assert data[:2] == b"MZ", "missing MZ DOS signature"
    e_lfanew = struct.unpack_from("<I", data, 0x3C)[0]
    assert data[e_lfanew:e_lfanew + 4] == b"PE\0\0", "missing PE signature"

    coff = e_lfanew + 4
    machine, nsect, _ts, _symtab, _nsyms, opt_size, chars = struct.unpack_from("<HHIIIHH", data, coff)
    assert machine == 0x8664, "Machine is not AMD64: %#x" % machine
    assert nsect == 2, "expected 2 sections, got %d" % nsect
    assert opt_size == 240, "SizeOfOptionalHeader != 240"
    assert chars == 0x0022, "Characteristics != EXECUTABLE_IMAGE | LARGE_ADDRESS_AWARE: %#x" % chars

    opt = coff + 20
    magic = struct.unpack_from("<H", data, opt)[0]
    assert magic == 0x20B, "not a PE32+ optional header: %#x" % magic
    entry = struct.unpack_from("<I", data, opt + 16)[0]
    image_base = struct.unpack_from("<Q", data, opt + 24)[0]
    sect_align, file_align = struct.unpack_from("<II", data, opt + 32)
    size_of_image, size_of_headers = struct.unpack_from("<II", data, opt + 56)
    subsystem, dll_chars = struct.unpack_from("<HH", data, opt + 68)
    num_dirs = struct.unpack_from("<I", data, opt + 108)[0]
    assert entry == 0x1000, "AddressOfEntryPoint != 0x1000"
    assert image_base == 0x140000000, "ImageBase != 0x140000000"
    assert (sect_align, file_align) == (0x1000, 0x200), "unexpected alignment"
    assert size_of_image % sect_align == 0 and size_of_headers % file_align == 0
    assert subsystem == 3, "Subsystem is not WINDOWS_CUI"
    assert dll_chars == 0x8160, "DllCharacteristics != HIGH_ENTROPY_VA|DYNAMIC_BASE|NX_COMPAT|TS_AWARE: %#x" % dll_chars
    assert num_dirs == 16
    import_rva, import_size = struct.unpack_from("<II", data, opt + 112 + 1 * 8)
    iat_rva, iat_size = struct.unpack_from("<II", data, opt + 112 + 12 * 8)

    sections = []
    for i in range(nsect):
        off = opt + opt_size + i * 40
        name = data[off:off + 8].rstrip(b"\0")
        vsize, rva, raw_size, raw_ptr = struct.unpack_from("<IIII", data, off + 8)
        flags = struct.unpack_from("<I", data, off + 36)[0]
        assert not (flags & SCN_MEM_WRITE), "%s is writable (W^X violated)" % name
        assert not ((flags & SCN_MEM_WRITE) and (flags & SCN_MEM_EXECUTE)), "%s is W+X" % name
        assert EXPECTED_SECTIONS.get(name) == flags, "%s flags %#x" % (name, flags)
        assert rva % sect_align == 0 and raw_ptr % file_align == 0 and raw_ptr + raw_size <= len(data)
        sections.append((name, vsize, rva, raw_ptr))

    def rva_to_off(rva):
        for _name, vsize, srva, raw_ptr in sections:
            if srva <= rva < srva + vsize:
                return raw_ptr + (rva - srva)
        raise AssertionError("RVA %#x outside all sections" % rva)

    text = next(s for s in sections if s[0] == b".text")
    assert text[2] <= entry < text[2] + text[1], "entry point outside .text"

    rdata = next(s for s in sections if s[0] == b".rdata")
    for rva, size in ((import_rva, import_size), (iat_rva, iat_size)):
        assert rdata[2] <= rva and rva + size <= rdata[2] + rdata[1], "import data outside read-only .rdata"

    desc = rva_to_off(import_rva)
    ilt_rva, _ts, _fwd, name_rva, first_thunk = struct.unpack_from("<IIIII", data, desc)
    assert first_thunk == iat_rva, "FirstThunk does not point at the IAT directory"
    assert data[desc + 20:desc + 40] == b"\0" * 20, "missing null import descriptor"
    assert cstr(data, rva_to_off(name_rva)).upper() == "KERNEL32.DLL"
    imports = []
    thunk = rva_to_off(ilt_rva)
    while True:
        entry_rva = struct.unpack_from("<Q", data, thunk)[0]
        if entry_rva == 0:
            break
        assert not entry_rva >> 63, "unexpected import by ordinal"
        imports.append(cstr(data, rva_to_off(entry_rva) + 2))
        thunk += 8
    for required in ("ExitProcess", "WriteFile"):
        assert required in imports, "missing KERNEL32 import %s" % required

    return "PE32+ AMD64 OK, W^X sections %s, imports KERNEL32!%s" % (
        "/".join(s[0].decode() for s in sections), ",".join(imports))


def main(paths):
    if not paths:
        print(__doc__.strip().splitlines()[-1])
        return 2
    for path in paths:
        try:
            print("%s: %s" % (path, check(path)))
        except (AssertionError, ValueError, StopIteration, struct.error) as exc:
            print("%s: INVALID PE32+ image: %s" % (path, exc))
            return 1
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
