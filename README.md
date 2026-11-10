# oC v1.0: Active-Defense Systems Programming Language

`oC` is a self-hosted low-level systems programming language designed for security-critical infrastructure. Its lexer, Pratt parser, compile-time `EscapeValidator`, C11 backend, and direct `x86_64` machine-code backend (Linux ELF64 and Windows PE32+) are written 100% in `oC`, neutralizing **17 CWE vulnerability classes** and blocking **13/13 exploit PoCs** plus **24 dual-backend negative probes** without garbage collection or inline heap allocator metadata.

📖 **Documentation & Spec:** [https://omerd2905.github.io/oC/](https://omerd2905.github.io/oC/)

---

## ⚡ Quickstart

```bash
# 1. Portable bootstrap (Linux, macOS, BSD; on Windows use MSYS2 MinGW64: gcc -O2 ... -o bin/occ.exe)
cc -O2 bootstrap/occ_bootstrap.c -o bin/occ

# 2. Verify self-hosting (Stage 2 == Stage 3 bit-for-bit) + 13/13 exploit PoCs + 91 probe & cross-target checks
./showcase.sh

# 3. Compile a standalone <5 KB zero-libc static ELF64 directly to x86_64 machine code (no gcc/ld required)
./bin/occ build examples/oc_tripwire.oc -o /tmp/oc_tripwire --backend native
/tmp/oc_tripwire

# 4. Cross-compile a ~1.5 KB Windows PE32+ from any host (no MinGW, no linker), then validate it
./bin/occ build examples/hello.oc -o /tmp/hello --backend native --target windows-x86_64
python3 tests/check_pe.py /tmp/hello.exe

# 5. Run the multi-workload empirical stress-test suite against Native C (-O2)
./benchmarks/run_benchmarks.sh
```

---

## 🏛 Architecture & Per-Backend Guarantee Matrix

The `occ` compiler shares a single 100% pure-`oC` frontend (`compiler/lexer.oc`, `compiler/parser.oc`) and offers two code generation backends:

| Guarantee / Capability | Optimizing C Backend (`--backend c`, default) | Direct Native ELF64 Backend (`--backend native`) |
|:---|:---|:---|
| **External Dependencies** | Emits C11 IR and invokes system `gcc`/`cc` (`-O1/-O2/-O3`) | **Zero external tools, zero `libc`** (emits raw `x86_64` machine code & ELF64 headers directly) |
| **Compile-Time Escape / Borrow / UAF / Linear `Arena` Validation** (`CWE-415`, `CWE-416`, `CWE-457`, `CWE-466`, `CWE-562`) | Enforced by `EscapeValidator` (`0.00 ns` runtime cost) | Enforced by `EscapeValidator` (`0.00 ns` runtime cost) |
| **Integer Overflow / Underflow** (`CWE-190`, `CWE-191`) | Checked via `__builtin_{add,sub,mul}_overflow` across widths -> `abort()` | Checked via hardware `jo`/`jc` + width range checks (`u8..u64`, `i8..i64`) -> `__trap_overflow` |
| **Division / Modulo by Zero** (`CWE-369`) | Checked via `__oc_div_check` / `__oc_mod_check` -> `abort()` | Checked via `test %rbx, %rbx; jz __trap_div_zero` before `div`/`idiv` |
| **Spatial Slice & Array Bounds** (`CWE-125`, `CWE-787`) | Fat slice `{ ptr, len }` + `oc_str_index` / `oc_str_slice` bounds trap | Fat slice `{ ptr, len }` + array index `cmp; jae __trap_slice_bounds` |
| **Narrowing Casts (`.as(T)`)** (`CWE-195`, `CWE-197`, `CWE-681`) | Range-checked for both signed (`i8..i64`) and unsigned (`u8..u64`, `usize`) targets | Range-checked for both signed (`i8..i64`) and unsigned (`u8..u64`, `usize`) targets |
| **Null Pointer Dereference (`*p`)** (`CWE-476`) | Checked via `__oc_notnull` on rvalue & lvalue dereferences | Checked via `test %rax, %rax; jz __trap_null_deref` |
| **Binary Hardening & Memory Layout** | Standard ELF / PE via system linker | Strict **W^X**: ELF64 (`PF_R` `.rodata`, `PF_R\|PF_X` `.text`, `PF_R\|PF_W` `PT_GNU_STACK`); PE32+ (RX `.text`, read-only `.rdata` incl. IAT, NX + ASLR) |

---

## 🌍 Cross-Platform Targets (`--target`)

`occ` builds and runs on Linux, macOS, the BSDs and Windows (MSYS2 MinGW64); CI exercises Linux, macOS and Windows on every push. `--target` selects the platform of the produced binary (default: the host). Output is deterministic: the same source and target yield byte-identical images on every host.

| `--target` | `--backend c` invokes | `--backend native` emits |
|:---|:---|:---|
| `linux-x86_64` | `cc` on Linux x86_64, else `x86_64-linux-gnu-gcc` | Static ELF64, W^X `PT_LOAD` segments, raw `syscall` |
| `windows-x86_64` (`windows`) | `gcc` on Windows, else `x86_64-w64-mingw32-gcc` | PE32+ console `.exe`: `.text` RX (`0x60000020`), `.rdata` R (`0x40000040`), ASLR + high-entropy VA + NX (`0x8160`), imports `KERNEL32!ExitProcess`/`WriteFile`/`GetStdHandle` |
| `macos-x86_64`, `macos-aarch64` | `cc -arch x86_64` / `cc -arch arm64` on macOS hosts | rejected (no Mach-O emitter) |

- Windows binaries get `.exe` appended automatically; module imports accept `/` and `\` separators; `occ run` uses `cmd.exe` conventions on Windows hosts.
- The PE32+ image is position independent (`%rip`-relative data), so ASLR needs no `.reloc` section, and no section is ever writable.
- `--backend native --target windows-x86_64` supports `print`, all runtime traps, `sys_exit` and `sys_host_target()`. Programs that reference Linux-only intrinsics (`sys_mmap`, `sys_open`, sockets, ...) are rejected at compile time; build them for Windows with `--backend c`. See [OC-OS(3)](docs/stdlib.html#std-os) and [OCC(1)](docs/man-occ.html#targets).

---

## 📊 Empirical Stress-Test Suite (`./benchmarks/run_benchmarks.sh`)

Measured across 3 non-trivial workloads designed to defeat dead-store elimination and constant folding (3-run averages on Linux x86_64, checksum-verified across targets):

| Stress Workload | oC v1.0 | Native C (`gcc -O2`) | Allocation / Execution Architecture Comparison |
|:---|:---:|:---:|:---|
| **12.8M Variable-Sized Allocs + CWE-14 Scrubbing**<br>*(50,000 waves × 256 objects, zero-scrubbed per wave)* | **43 ms**<br>*(Regional bump `Arena` + bulk reset scrub)* | 320 ms<br>*(Per-object `malloc` + `memset` + `free`)* | **7.44x faster** (Demonstrates regional bump allocation + linear bulk scrub vs per-object heap `ptmalloc` metadata overhead) |
| **25M Stateful Wire Packet Parses + FNV-1a**<br>*(Data-dependent 64B frames, 3 sub-slices + 3 indexed reads)* | **88 ms**<br>*(Checked fat slices)* | 98 ms<br>*(Unchecked `uint8_t*`)* | **1.0x parity** (Spatial bounds & CWE-197 checks proven in-bounds) |
| **78.1M Recursive Calls (`fib(38)`) + 200K Collatz**<br>*(Call-frame + unpredictable hailstone branch & div/mul stress)* | **85 ms** (`occ -O2`)<br>**738 ms** (`--backend native`) | 78 ms<br>*(`gcc -O2` unchecked)* | **Near-parity** on `-O2` with overflow/div-zero checks enabled; `--backend native` emits 1-pass x86_64 |
| **Standalone Static Binary Size**<br>*(`native_stress.oc` / `oc_tripwire.oc`)* | **4.7 KB – 5.2 KB**<br>*(Zero libc, static ELF64)* | 16.0 KB (dynamic)<br>~850 KB (static glibc) | **3.1x smaller** than dynamic C; **160x smaller** than static glibc |
