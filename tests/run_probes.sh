#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
OCC="$ROOT/bin/occ"

COMPILE_REJECT=(
  "probe_immut_assign.oc"
  "probe_uninit_self.oc"
  "probe_cond_uaf.oc"
  "probe_loop_uaf.oc"
  "probe_param_escape.oc"
)

RUNTIME_TRAP=(
  "probe_overflow_i32.oc"
  "probe_overflow_u8.oc"
  "probe_underflow_usize.oc"
  "probe_div_zero.oc"
  "probe_slice_bounds.oc"
  "probe_cast_narrow_i8.oc"
  "probe_cast_neg_u64.oc"
)

PASS=0
TOTAL=0

echo "=== [1/4] Compile-Time Rejection Probes (C & Native Backends) ==="
for p in "${COMPILE_REJECT[@]}"; do
  for backend in c native; do
    TOTAL=$((TOTAL + 1))
    if "$OCC" build "$ROOT/tests/probes/$p" -o /tmp/oc_probe_bin --backend "$backend" >/dev/null 2>&1; then
      echo "  [FAIL] $p (--backend $backend) unexpectedly compiled!"
      exit 1
    else
      echo "  [PASS] $p (--backend $backend) rejected at compile time"
      PASS=$((PASS + 1))
    fi
  done
done

echo "=== [2/4] Runtime Hardware Trap Probes (C & Native Backends) ==="
for p in "${RUNTIME_TRAP[@]}"; do
  for backend in c native; do
    TOTAL=$((TOTAL + 1))
    "$OCC" build "$ROOT/tests/probes/$p" -o /tmp/oc_probe_bin --backend "$backend" >/dev/null 2>&1
    set +e
    /tmp/oc_probe_bin >/dev/null 2>&1
    RC=$?
    set -e
    rm -f /tmp/oc_probe_bin
    if [ "$RC" -ne 0 ]; then
      echo "  [PASS] $p (--backend $backend) trapped at runtime (exit $RC)"
      PASS=$((PASS + 1))
    else
      echo "  [FAIL] $p (--backend $backend) exited 0 without trapping!"
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
# Examples either emit a valid PE32+ or are rejected for Linux-only syscall intrinsics (fail closed).
for ex in "$ROOT"/examples/*.oc; do
  base="$(basename "$ex")"
  if [ "$base" = "math_utils.oc" ] || [ "$base" = "bad_stack_escape.oc" ]; then
    continue
  fi
  TOTAL=$((TOTAL + 1))
  rm -f "$PE_OUT.exe"
  if out="$("$OCC" build "$ex" -o "$PE_OUT" --backend native --target windows-x86_64 2>&1)"; then
    if pe_check; then
      echo "  [PASS] $base -> valid PE32+"
      PASS=$((PASS + 1))
    else
      echo "  [FAIL] $base produced an invalid PE32+ image"
      exit 1
    fi
  elif grep -q "unavailable on target windows-x86_64" <<<"$out"; then
    echo "  [PASS] $base rejected at compile time (Linux syscall intrinsic: $(grep -A1 "unavailable on target" <<<"$out" | tail -n 1))"
    PASS=$((PASS + 1))
  else
    echo "  [FAIL] $base: unexpected windows-x86_64 build failure:"
    echo "$out"
    exit 1
  fi
done
rm -f "$PE_OUT.exe"

echo "=== ALL $PASS / $TOTAL PROBE & EXAMPLE CHECKS PASSED ==="
