# oC v1.0: Active-Defense Systems Programming Language

`oC` is a 100% self-hosted low-level systems programming language designed to replace C for security-critical infrastructure. It delivers C-parity execution speed and `<5 KB` zero-libc static ELF binaries while neutralizing **17 CWE vulnerability classes** and blocking **13/13 exploit PoCs** at compile time with zero garbage collection or heap allocator metadata overhead.

📖 **Documentation & Spec:** [https://omerd2905.github.io/oC/](https://omerd2905.github.io/oC/)

---

## ⚡ Quickstart

```bash
# 1. Verify self-hosting (Stage 2 == Stage 3 bit-for-bit) + 13/13 exploit PoC rejection
./showcase.sh

# 2. Compile a standalone <5 KB zero-libc static ELF directly to x86_64 machine code
./bin/occ build examples/oc_tripwire.oc -o /tmp/oc_tripwire --backend native
/tmp/oc_tripwire

# 3. Run the multi-workload empirical stress-test suite against Native C (-O2)
./benchmarks/run_benchmarks.sh
```

---

## 🏛 Architecture (`100% Pure Self-Hosted oC`)

```
                  ┌──────────────────────┐
                  │    oC Source Code    │
                  └──────────┬───────────┘
                             │
                  ┌──────────▼───────────┐
                  │ Zero-Alloc Lexer     │ (compiler/lexer.oc)
                  └──────────┬───────────┘
                             │
                  ┌──────────▼───────────┐
                  │ Pratt AST Parser     │ (compiler/parser.oc)
                  │ + EscapeValidator    │ (Blocks UAF, Cross-Arena & Stack Escapes)
                  └──────────┬───────────┘
                             │
            ┌────────────────┴────────────────┐
            │                                 │
 ┌──────────▼──────────┐           ┌──────────▼──────────┐
 │ Direct Native ELF   │           │ Optimizing C99 IR   │
 │ (compiler/elf.oc)   │           │ (compiler/codegen.oc│
 ├─────────────────────┤           ├─────────────────────┤
 │ • Raw x86_64 bytes  │           │ • -O2 / -O3 SIMD    │
 │ • Direct syscalls   │           │ • Checked casts     │
 │ • 4.7 KB Static ELF │           │ • Bounds-checked    │
 └──────────┬──────────┘           └──────────┬──────────┘
            │                                 │
            └────────────────┬────────────────┘
                             │
                  ┌──────────▼───────────┐
                  │  Standalone Binary   │
                  └──────────────────────┘
```

---

## 📊 Empirical Stress-Test Suite (`./benchmarks/run_benchmarks.sh`)

Measured across 3 non-trivial workloads designed to defeat dead-store elimination and constant folding (3-run averages on Linux x86_64, checksum-verified across targets):

| Stress Workload | oC v1.0 | Native C (`gcc -O2`) | Result / Engineering Notes |
|:---|:---:|:---:|:---|
| **12.8M Variable-Sized Allocs + CWE-14 Scrubbing**<br>*(50,000 waves × 256 objects, zero-scrubbed per wave)* | **43 ms**<br>*(297.6M allocs/s)* | 320 ms<br>*(`malloc`+`memset`+`free`)* | **7.44x faster** (`sys_mmap` bump arena + linear scrub vs `ptmalloc`) |
| **25M Stateful Wire Packet Parses + FNV-1a**<br>*(Data-dependent 64B frames, 3 sub-slices + 3 indexed reads)* | **88 ms**<br>*(Checked fat slices)* | 98 ms<br>*(Unchecked `uint8_t*`)* | **1.0x parity** (Spatial bounds & CWE-197 checks proven in-bounds) |
| **78.1M Recursive Calls (`fib(38)`) + 200K Collatz**<br>*(Call-frame + unpredictable hailstone branch & div/mul stress)* | **79 ms** (`occ -O2`)<br>**738 ms** (`--backend native`) | 78 ms<br>*(`gcc -O2`)* | **1.01x parity** on `-O2`; `--backend native` emits 1-pass x86_64 with hardware `jo`/`jz` traps |
| **Standalone Static Binary Size**<br>*(`native_stress.oc` / `oc_tripwire.oc`)* | **4.7 KB – 5.2 KB**<br>*(Zero libc, static ELF64)* | 16.0 KB (dynamic)<br>~850 KB (static glibc) | **3.1x smaller** than dynamic C; **160x smaller** than static glibc |

---

## 🛡 17 Neutralized CWE Vulnerability Classes

- **Compile-Time `EscapeValidator` (`0.00 ns` runtime cost):**
  - `CWE-416` (Use-After-Free on `arena_reset`, `arena_deinit`, and container mutation invalidation)
  - `CWE-415` (Double Free)
  - `CWE-562` (Return of Stack Variable Address — direct, struct-wrapped, or cast-laundered)
  - `CWE-466` (Out-of-Scope Pointer Escape & Cross-Arena Escape)
  - `CWE-457` (Uninitialized Variable Use)
- **Hardware & Kernel Enforcement:**
  - `CWE-122` (Heap Metadata Corruption — eliminated via headerless `sys_mmap` bump arenas)
  - `CWE-125 / CWE-787` (Spatial Out-of-Bounds Read/Write — fat slices `(ptr, len)` with `jae __trap_slice_bounds`)
  - `CWE-190 / CWE-191` (Integer Overflow/Underflow — hardware `jo __trap_overflow`)
  - `CWE-195 / CWE-197` (Signed-to-Unsigned & Narrowing Cast Truncation — checked `.as(T)`)
  - `CWE-369` (Divide by Zero — `jz __trap_div_zero`)
  - `CWE-476` (NULL Pointer Dereference — `jz __trap_null_deref`)
  - `CWE-14` (Compiler Removal of Secret Scrubbing — mandatory byte scrubbing on `arena_reset` & `arena_deinit`)
  - `CWE-200` (Struct Alignment Padding Information Leak)
  - `CWE-59 / CWE-367 / CWE-403 / CWE-170 / CWE-78` (`O_NOFOLLOW | O_CLOEXEC`, bounded `\0` paths, shell metacharacter guards)
