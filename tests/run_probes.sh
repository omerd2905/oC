#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
OCC="$ROOT/bin/occ"

# A compiler that cannot even start must not be mistaken for a compiler that rejects programs.
if ! "$OCC" --version >/dev/null 2>&1; then
  echo "error: $OCC is missing or does not run on this machine." >&2
  echo "       Build it first:  make   (or: cc -O2 bootstrap/occ_bootstrap.c -o bin/occ)" >&2
  exit 1
fi

# probe file | text the compiler must print when it rejects the probe
COMPILE_REJECT=(
  "probe_immut_assign.oc|cannot assign to immutable variable"
  "probe_uninit_self.oc|variable used in its own initializer"
  "probe_cond_uaf.oc|use-after-free: variable 'p' used after arena 'a' was reset"
  "probe_loop_uaf.oc|use-after-free: variable 'p' used after arena 'a' was reset"
  "probe_param_escape.oc|cannot return reference to local stack variable 'x'"
  "probe_indirect_deinit_uaf.oc|use-after-free: variable 'p' used after arena 'a' was deinitialized"
  "probe_indirect_reset_uaf.oc|use-after-free: variable 'p' used after arena 'a' was reset"
  "probe_array_slice_escape.oc|cannot return reference to local stack variable 's'"
  "probe_struct_wrap_uaf.oc|use-after-free: variable 's' used after arena 'a' was reset"
  "probe_field_slice_escape.oc|cannot return reference to local stack variable 'm'"
)

RUNTIME_TRAP=(
  "probe_overflow_i32.oc"
  "probe_overflow_u8.oc"
  "probe_overflow_mul_u64.oc"
  "probe_overflow_mul_i32.oc"
  "probe_underflow_usize.oc"
  "probe_div_zero.oc"
  "probe_slice_bounds.oc"
  "probe_array_bounds.oc"
  "probe_array_slice_bounds.oc"
  "probe_cast_narrow_i8.oc"
  "probe_cast_neg_u64.oc"
)

PASS=0
TOTAL=0

echo "=== CLI: help and unknown options ==="
for flag in --help -h help; do
  TOTAL=$((TOTAL + 1))
  if "$OCC" "$flag" | grep -q "^Usage: occ"; then
    echo "  [PASS] occ $flag prints usage and exits 0"
    PASS=$((PASS + 1))
  else
    echo "  [FAIL] occ $flag did not print usage with exit code 0"
    exit 1
  fi
done
TOTAL=$((TOTAL + 1))
if "$OCC" build --no-such-option >/dev/null 2>&1; then
  echo "  [FAIL] occ accepted an unknown option"
  exit 1
else
  echo "  [PASS] unknown option rejected"
  PASS=$((PASS + 1))
fi

echo "=== Install layout: occ by name, through a symlink, from another directory ==="
TOTAL=$((TOTAL + 1))
WORK="$(mktemp -d)"
ln -s "$OCC" "$WORK/occ"
printf 'import "std/mem.oc";\nfn main() {\n    let mut a: Arena = arena_init(4096);\n    arena_deinit(&mut a);\n    print("std ok");\n}\n' > "$WORK/t.oc"
if [ "$(cd "$WORK" && PATH="$WORK:$PATH" occ run t.oc)" = "std ok" ]; then
  echo "  [PASS] std/ is found relative to the real executable, not argv[0]"
  PASS=$((PASS + 1))
else
  echo "  [FAIL] 'occ run' by name from another directory could not import std/mem.oc"
  exit 1
fi
rm -rf "$WORK"

echo "=== [1/4] Compile-Time Rejection Probes (C & Native Backends) ==="
for entry in "${COMPILE_REJECT[@]}"; do
  p="${entry%%|*}"
  want="${entry#*|}"
  for backend in c native; do
    TOTAL=$((TOTAL + 1))
    if out="$("$OCC" build "$ROOT/tests/probes/$p" -o /tmp/oc_probe_bin --backend "$backend" 2>&1)"; then
      echo "  [FAIL] $p (--backend $backend) unexpectedly compiled!"
      exit 1
    elif grep -qF -- "$want" <<<"$out"; then
      echo "  [PASS] $p (--backend $backend) rejected: $want"
      PASS=$((PASS + 1))
    else
      echo "  [FAIL] $p (--backend $backend) failed, but not with the expected error \"$want\":"
      echo "$out"
      exit 1
    fi
  done
done

echo "=== [2/4] Runtime Hardware Trap Probes (C & Native Backends) ==="
for p in "${RUNTIME_TRAP[@]}"; do
  for backend in c native; do
    TOTAL=$((TOTAL + 1))
    if ! "$OCC" build "$ROOT/tests/probes/$p" -o /tmp/oc_probe_bin --backend "$backend" >/dev/null 2>&1; then
      echo "  [FAIL] $p (--backend $backend) did not compile!"
      exit 1
    fi
    set +e
    out="$(/tmp/oc_probe_bin 2>&1)"
    RC=$?
    set -e
    rm -f /tmp/oc_probe_bin
    # A controlled trap prints "panic: ..." and exits non-zero; a crash (e.g. SIGSEGV) is not a pass.
    if [ "$RC" -ne 0 ] && grep -q "^panic: " <<<"$out"; then
      echo "  [PASS] $p (--backend $backend) trapped at runtime (exit $RC, $(grep "^panic: " <<<"$out" | head -n 1))"
      PASS=$((PASS + 1))
    else
      echo "  [FAIL] $p (--backend $backend) did not trap (exit $RC):"
      echo "$out"
      exit 1
    fi
  done
done

echo "=== [3/4] Positive Examples Verification (C & Native Backends) ==="
for ex in "$ROOT"/examples/*.oc; do
  base="$(basename "$ex")"
  if [ "$base" = "math_utils.oc" ] || [ "$base" = "bad_stack_escape.oc" ]; then
    continue
  fi
  for backend in c native; do
    TOTAL=$((TOTAL + 1))
    "$OCC" build "$ex" -o /tmp/oc_ex_bin --backend "$backend" >/dev/null
    rm -f /tmp/oc_ex_bin
    PASS=$((PASS + 1))
  done
done

echo "=== Backend Parity: C and Native Output Must Match ==="
for t in "$ROOT"/tests/parity/*.oc; do
  base="$(basename "$t")"
  TOTAL=$((TOTAL + 1))
  for backend in c native; do
    "$OCC" build "$t" -o "/tmp/oc_parity_$backend" --backend "$backend" >/dev/null
    set +e
    "/tmp/oc_parity_$backend" >"/tmp/oc_parity_$backend.out" 2>&1
    echo "exit $?" >>"/tmp/oc_parity_$backend.out"
    set -e
  done
  if cmp -s /tmp/oc_parity_c.out /tmp/oc_parity_native.out; then
    echo "  [PASS] $base (identical output on both backends)"
    PASS=$((PASS + 1))
  else
    echo "  [FAIL] $base: C and native output differ:"
    diff /tmp/oc_parity_c.out /tmp/oc_parity_native.out || true
    exit 1
  fi
  rm -f /tmp/oc_parity_c /tmp/oc_parity_native /tmp/oc_parity_c.out /tmp/oc_parity_native.out
done

echo "=== [4/4] Cross-Target PE32+ Verification (--backend native --target windows-x86_64) ==="
PE_OUT=/tmp/oc_pe_bin
pe_check() {
  python3 "$ROOT/tests/check_pe.py" "$PE_OUT.exe" >/dev/null
}
# Trap probes use no OS intrinsics, so each must emit a valid W^X PE32+ image.
for p in "${RUNTIME_TRAP[@]}"; do
  TOTAL=$((TOTAL + 1))
  rm -f "$PE_OUT.exe"
  if "$OCC" build "$ROOT/tests/probes/$p" -o "$PE_OUT" --backend native --target windows-x86_64 >/dev/null 2>&1 && pe_check; then
    echo "  [PASS] $p -> valid PE32+ (AMD64, W^X .text/.rdata, KERNEL32 imports)"
    PASS=$((PASS + 1))
  else
    echo "  [FAIL] $p did not produce a valid PE32+ image"
    exit 1
  fi
done
# Every example must emit a valid PE32+: arenas and file I/O run on KERNEL32 (std/sys/windows.oc).
for ex in "$ROOT"/examples/*.oc; do
  base="$(basename "$ex")"
  if [ "$base" = "math_utils.oc" ] || [ "$base" = "bad_stack_escape.oc" ]; then
    continue
  fi
  TOTAL=$((TOTAL + 1))
  rm -f "$PE_OUT.exe"
  if out="$("$OCC" build "$ex" -o "$PE_OUT" --backend native --target windows-x86_64 2>&1)" && pe_check; then
    echo "  [PASS] $base -> valid PE32+"
    PASS=$((PASS + 1))
  else
    echo "  [FAIL] $base did not produce a valid PE32+ image:"
    echo "$out"
    exit 1
  fi
done
rm -f "$PE_OUT.exe"

echo "=== Documentation examples (docs/*.html, README.md) ==="
python3 "$ROOT/tests/check_docs.py" "$OCC"

echo "=== ALL $PASS / $TOTAL PROBE & EXAMPLE CHECKS PASSED ==="
