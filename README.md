# oC Programming Language

oC is a small, self-hosted systems programming language with explicit regional memory management, compile-time lifetime verification, and a choice of code generation backends.

Instead of relying on a general-purpose heap allocator (`malloc`/`free`) or a runtime garbage collector, oC uses regional arenas (`sys_mmap` / `VirtualAlloc`) and compile-time escape analysis to prevent common temporal memory bugs (use-after-free, double-free, and dangling stack pointers).

Online documentation and language specification: [https://omerd2905.github.io/oC/](https://omerd2905.github.io/oC/)

---

## Example

```oc
import "std/mem.oc";

fn main() i32 {
    // 1. Regional arena allocated directly from the OS kernel
    let mut arena: Arena = arena_init(4096);

    // 2. Bump allocation within the arena
    let buf: *mut u8 = arena_alloc(&mut arena, 128);
    buf[0] = 79; // 'O'
    buf[1] = 67; // 'C'

    // 3. String slices are fat pointers { ptr: *u8, len: usize } with bounds checks
    let msg: string = "Hello from oC";
    print(msg[0..5]); // prints "Hello"

    // 4. Reset zeros the memory region before resetting offset
    arena_reset(&mut arena);

    // 5. Unmaps the region from the operating system
    arena_deinit(&mut arena);
    return 0;
}
```

---

## Design and Safety Model

- **Regional Arenas:** Dynamic allocations are performed against explicit arenas. There are no inline chunk headers, metadata footers, or global heap freelists.
- **Compile-Time Lifetime & Escape Checks:** An AST-level validator checks that references do not outlive their targets, stack addresses cannot be returned or leaked into outer scopes, and pointers into arenas cannot be accessed after the arena is reset or deinitialized.
- **Checked Arithmetic and Slices:** Integer overflow (`+`, `-`, `*`), division by zero, narrowing casts (`.as(T)`), and slice indexing (`s[i]`) are explicitly checked. In the C backend, these map to overflow intrinsics; in the native backend, they lower to hardware condition checks and trap instructions. Code that needs modular arithmetic (hashes, PRNGs) opts in explicitly with `wrapping_add`, `wrapping_sub` and `wrapping_mul`.
- **Zero Required Runtime:** Programs compiled with `--backend native` have no dependency on libc or an external runtime library.

---

## Toolchain & Backends

The compiler (`occ`) is written in oC and provides two backends:

| Backend | Output | Use Case |
|:---|:---|:---|
| **C Backend** (`--backend c`, default) | C11 source compiled via host C compiler (`cc`, `gcc`, `clang`) | Portability across Linux, macOS, BSDs, and Windows. Enables full optimizer passes (`-O1`, `-O2`, `-O3`). |
| **Native Backend** (`--backend native`) | Direct machine code and object format emission | Self-contained, dependency-free binaries without an external C compiler or linker. Emits static ELF64 on Linux and PE32+ on Windows. |

### Target Selection (`--target`)

The `--target` option controls the target platform (defaults to host):

| Target | Description | Supported Backends |
|:---|:---|:---|
| `linux-x86_64` | Linux x86_64 (static ELF64) | `--backend c`, `--backend native` |
| `windows-x86_64` | Windows x86_64 (PE32+ console executable) | `--backend c`, `--backend native` |
| `macos-x86_64` | macOS x86_64 | `--backend c` |
| `macos-aarch64` | macOS Apple Silicon (ARM64) | `--backend c` |

---

## Getting Started

### 1. Download the Repository
Clone with `git` or download and extract the repository ZIP:
```bash
git clone https://github.com/omerd2905/oC.git
cd oC
```
*(On Windows without git: download the ZIP from GitHub and open the extracted folder in Command Prompt or PowerShell).*

### 2. Prerequisites
- **Linux / macOS / BSD:** Standard C compiler (`cc`, `gcc`, or `clang`).
- **Windows:** GCC on `PATH` via [MSYS2](https://www.msys2.org/), [w64devkit](https://github.com/skeeto/w64devkit), or WinLibs (`winget install MSYS2.MSYS2`).

### 3. Bootstrap the Compiler
The portable C source (`bootstrap/occ_bootstrap.c`) is committed in the repository so no existing `oC` installation is required. The compiler binary itself is not committed; it is always built for your machine:

On Linux, macOS, or BSD:
```bash
make        # same as: cc -O2 bootstrap/occ_bootstrap.c -o bin/occ
```

On Windows:
```cmd
gcc -O2 bootstrap\occ_bootstrap.c -o bin\occ.exe
```

### 4. Run or Build
```bash
# Run directly
./bin/occ run examples/hello.oc

# On Windows:
bin\occ.exe run examples\hello.oc
```

### 5. Put `occ` on your PATH (optional)
```bash
./bin/occ install

# On Windows:
bin\occ.exe install
```
- **Linux / macOS / BSD:** creates the symlink `~/.local/bin/occ` (add `~/.local/bin` to `PATH` if it is not there).
- **Windows:** appends the `bin` directory to your user `PATH` (open a new terminal afterwards).
- **NixOS / Nix:** `nix profile install .` builds and installs the flake's default package.

After that, `occ build x.oc` works from any directory. `occ` locates `std/` relative to its own real location (`<prefix>/bin/occ` uses `<prefix>/std`), also when started through a symlink. To use a different standard library, set `OC_HOME` to the directory that contains `std/`.

### Updating oC
To update an existing installation to the latest commit (from any directory):
```bash
occ update
```
*(Runs `git pull` in the checkout the `occ` binary lives in and rebuilds the compiler from `bootstrap/occ_bootstrap.c`).*

---

## Building and Verification

### Self-Hosting Verification

To verify that the self-hosted compiler produces a bit-for-bit identical binary across bootstrap stages:
```bash
./showcase.sh
```

This runs:
1. Stage 2 and Stage 3 self-compilation and asserts binary equality (`cmp Stage 2 == Stage 3`).
2. Standalone static ELF generation and execution on `--backend native`.
3. Standard library functional test suite.
4. Compile-time exploit rejection suite and dual-backend runtime trap probes.

### Compiling Programs

```bash
# Build with optimizing C backend
./bin/occ build examples/hello.oc -o hello -O2

# Run directly
./bin/occ run examples/hello.oc

# Build a standalone static Linux ELF64 directly (no C compiler or linker needed)
./bin/occ build examples/hello.oc -o hello --backend native

# Cross-compile a standalone Windows PE32+ executable
./bin/occ build examples/hello.oc -o hello --backend native --target windows-x86_64
```

---

## Performance Characteristics

Measurements on Linux x86_64 comparing oC against GCC 9.x/13.x at `-O2`:

| Workload | oC | Native C (`-O2`) | Notes |
|:---|:---:|:---:|:---|
| **12.8M allocations + reset scrub** | 43 ms | 320 ms | Regional bump arena (`sys_mmap`) vs per-object `malloc` + `memset` + `free` |
| **25M wire packet parsing + FNV-1a** | 88 ms | 98 ms | Checked fat slices vs raw unchecked pointers |
| **78.1M recursive calls (`fib(38)`)** | 85 ms (`-O2`)<br>738 ms (`native`) | 78 ms | Near parity on C backend with overflow checks enabled; native backend uses single-pass register assignment |
| **Static binary size** | ~4.8 KB | 16 KB (dynamic)<br>~850 KB (static glibc) | Direct static ELF with raw system call instructions |

---

## Repository Structure

- `compiler/` - Compiler source code (lexer, parser, escape validator, C codegen, ELF/PE emitter, CLI driver).
- `std/` - Standard library modules (`mem.oc`, `fs.oc`, `io.oc`, `net.oc`, `math.oc`, `os.oc`).
- `examples/` - Example programs demonstrating arenas, slices, network sockets, and file I/O.
- `tests/` - Probe test suite and exploit rejection cases.
- `docs/` - HTML documentation and manual pages hosted on GitHub Pages.
- `bootstrap/` - Single-file portable C source for bootstrapping on new machines.

---

## License

Dual-licensed under either the [MIT License](LICENSE-MIT) or the [Apache License (Version 2.0)](LICENSE-APACHE) at your option.
