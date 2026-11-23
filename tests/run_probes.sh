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
  "probe_helper_alloc_uaf.oc|use-after-free: variable 'p' used after arena 'a' was deinitialized"
  "probe_array_ptr_uaf.oc|use-after-free: variable 'ptrs' used after arena 'a' was reset"
  "probe_array_store_uaf.oc|use-after-free: variable 'q' used after arena 'a' was deinitialized"
  "probe_float_eq.oc|== and != are not allowed on floats"
  "probe_float_to_int_implicit.oc|implicit conversion from float to integer"
  "probe_undefined_fn.oc|probe_undefined_fn.oc:3:11: error: undefined function 'totally_undefined_fn'"
  "probe_misspelled_fn.oc|probe_misspelled_fn.oc:5:11: error: undefined function 'align_upp'"
  "probe_libc_no_extern.oc|probe_libc_no_extern.oc:4:18: error: undefined function 'getpid'"
  "probe_lossy_i64_to_f64.oc|probe_lossy_i64_to_f64.oc:4:18: error: implicit conversion from i64 to f64 in a variable initializer can lose precision; write .as(f64)"
  "probe_lossy_f64_to_f32.oc|probe_lossy_f64_to_f32.oc:4:18: error: implicit conversion from f64 to f32 in a variable initializer can lose precision; write .as(f32)"
  "probe_lossy_int_literal.oc|probe_lossy_int_literal.oc:3:18: error: integer literal 9007199254740993 in a variable initializer is not exactly representable as f64"
  "probe_strbuf_after_reset.oc|use-after-free: variable 'sb' used after arena 'a' was reset"
  "probe_arena_inner_escape.oc|cross-arena escape: pointer borrowed from arena 'inner' escapes into longer-lived target 'keep'"
  "probe_readline_after_reset.oc|use-after-free: variable 'keep' used after arena 'a' was reset"
  "probe_escape_unknown.oc|probe_escape_unknown.oc:4:13: error: unknown escape sequence '\\q'"
  "probe_escape_octal.oc|probe_escape_octal.oc:3:13: error: unknown escape sequence '\\1'"
  "probe_escape_hex_short.oc|probe_escape_hex_short.oc:3:12: error: \\x needs exactly two hex digits"
  "probe_escape_unterminated.oc|probe_escape_unterminated.oc:4:11: error: unterminated string literal"
  "probe_dup_std_name.oc|probe_dup_std_name.oc:5:4: error: 'find' is already defined at std/str.oc:"
  "probe_dup_function.oc|probe_dup_function.oc:4:4: error: 'f' is already defined at"
  "probe_dup_struct.oc|probe_dup_struct.oc:3:8: error: 'P' is already defined at"
  "probe_dup_const.oc|probe_dup_const.oc:4:5: error: 'limit' is already defined at"
  "probe_str_after_reset.oc|use-after-free: variable 'greeting' used after arena 'a' was reset"
  "probe_float_mixed_to_f32.oc|probe_float_mixed_to_f32.oc:6:20: error: implicit conversion from f64 to f32 in a variable initializer can lose precision"
  "probe_lossy_mixed_op.oc|probe_lossy_mixed_op.oc:3:12: error: implicit conversion from u64 to f64 in a float operation can lose precision"
)

RUNTIME_TRAP=(
  "probe_str_substr_bounds.oc"
  "probe_str_arena_oom.oc"
  "probe_overflow_i32.oc"
  "probe_overflow_u8.oc"
  "probe_overflow_mul_u64.oc"
  "probe_overflow_mul_i32.oc"
  "probe_overflow_neg_i32.oc"
  "probe_underflow_usize.oc"
  "probe_div_zero.oc"
  "probe_slice_bounds.oc"
  "probe_array_bounds.oc"
  "probe_array_slice_bounds.oc"
  "probe_cast_narrow_i8.oc"
  "probe_cast_neg_u64.oc"
  "probe_math_abs_min.oc"
  "probe_math_pow_overflow.oc"
  "probe_math_ilog2_zero.oc"
  "probe_float_cast_nan.oc"
  "probe_float_cast_huge.oc"
  "probe_float_cast_neg_u32.oc"
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

echo "=== --fp-trap: NaN / infinity results are IEEE values by default, a panic on request ==="
for backend in c native; do
  TOTAL=$((TOTAL + 1))
  "$OCC" build "$ROOT/tests/probes/probe_fp_trap.oc" -o /tmp/oc_probe_bin --backend "$backend" >/dev/null
  plain="$(/tmp/oc_probe_bin 2>&1)"
  "$OCC" build "$ROOT/tests/probes/probe_fp_trap.oc" -o /tmp/oc_probe_bin --backend "$backend" --fp-trap >/dev/null
  set +e
  trapped="$(/tmp/oc_probe_bin 2>&1)"
  RC=$?
  set -e
  rm -f /tmp/oc_probe_bin
  if [ "$plain" = "inf" ] && [ "$RC" -ne 0 ] && [ "$trapped" = "panic: floating-point result is NaN or infinite at $ROOT/tests/probes/probe_fp_trap.oc:5:22" ]; then
    echo "  [PASS] probe_fp_trap.oc (--backend $backend): prints inf by default, panics with --fp-trap at the division"
    PASS=$((PASS + 1))
  else
    echo "  [FAIL] probe_fp_trap.oc (--backend $backend): default printed '$plain'; --fp-trap exit $RC, printed '$trapped'"
    exit 1
  fi
done

echo "=== Native backend limits: exceeding one is an error, not a wrong result ==="
WORK="$(mktemp -d)"
cp "$ROOT/tests/probes/probe_native_big_struct_return.oc" "$ROOT/tests/probes/probe_native_big_struct_copy.oc" "$WORK/"
{ echo 'fn main() {'; for i in $(seq 0 299); do echo "    let v$i: i64 = $i;"; done; echo '    print(v256);'; echo '}'; } > "$WORK/vars.oc"
{ for i in $(seq 0 139); do echo "let C$i: i64 = $i;"; done; echo 'fn main() { print(C139); }'; } > "$WORK/consts.oc"
for entry in "vars.oc|256|too many variables in one function (256)" "consts.oc|139|too many top-level constants (128)" \
             "probe_native_big_struct_return.oc|5|cannot return a struct larger than 32 bytes" \
             "probe_native_big_struct_copy.oc|5|struct larger than 32 bytes cannot be copied by value"; do
  IFS='|' read -r f want_c want_err <<<"$entry"
  TOTAL=$((TOTAL + 1))
  got_c="$("$OCC" run "$WORK/$f" --backend c 2>&1)"
  if out="$("$OCC" build "$WORK/$f" -o "$WORK/bin" --backend native 2>&1)"; then
    echo "  [FAIL] $f: the native backend compiled past its limit"
    exit 1
  elif [ "$got_c" = "$want_c" ] && grep -qF -- "$want_err" <<<"$out"; then
    echo "  [PASS] $f: C backend prints $want_c; native backend reports: $want_err"
    PASS=$((PASS + 1))
  else
    echo "  [FAIL] $f: C printed '$got_c', native said: $out"
    exit 1
  fi
done
rm -rf "$WORK"

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
  # A <name>.expected file next to the test holds independently computed results.
  want="${t%.oc}.expected"
  if [ -f "$want" ] && ! cmp -s /tmp/oc_parity_c.out "$want"; then
    echo "  [FAIL] $base: output differs from $(basename "$want"):"
    diff /tmp/oc_parity_c.out "$want" || true
    exit 1
  fi
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

echo "=== Standard input: read_line, read_i64, read_int, read_all ==="
WORK="$(mktemp -d)"
{ printf '  Ada \r\n-42\n7x\n-2147483649\nabc\r\n\n'; head -c 100000 /dev/zero | tr '\0' x; printf '\ntail1\ntail2'; } > "$WORK/input"
head -c 2000 /dev/zero | tr '\0' y > "$WORK/long"
for backend in c native; do
  TOTAL=$((TOTAL + 1))
  "$OCC" build "$ROOT/tests/stdin/read_input.oc" -o "$WORK/read_input" --backend "$backend" >/dev/null
  if [ "$("$WORK/read_input" < "$WORK/input")" = "$(cat "$ROOT/tests/stdin/read_input.expected")" ]; then
    echo "  [PASS] read_input.oc (--backend $backend): lines, \\r\\n, numbers, a 100000-byte line, EOF"
    PASS=$((PASS + 1))
  else
    echo "  [FAIL] read_input.oc (--backend $backend) printed:"
    "$WORK/read_input" < "$WORK/input" | head -20
    exit 1
  fi
  TOTAL=$((TOTAL + 1))
  "$OCC" build "$ROOT/tests/stdin/longest_line.oc" -o "$WORK/longest_line" --backend "$backend" >/dev/null
  if [ "$(printf 'x\nlonger\nmid\n' | "$WORK/longest_line")" = "$(cat "$ROOT/tests/stdin/longest_line.expected")" ]; then
    echo "  [PASS] longest_line.oc (--backend $backend): lengths and lines kept across loop iterations"
    PASS=$((PASS + 1))
  else
    echo "  [FAIL] longest_line.oc (--backend $backend)"
    exit 1
  fi
  TOTAL=$((TOTAL + 1))
  "$OCC" build "$ROOT/tests/stdin/read_line_oom.oc" -o "$WORK/read_line_oom" --backend "$backend" >/dev/null
  set +e
  out="$("$WORK/read_line_oom" < "$WORK/long" 2>&1)"
  RC=$?
  set -e
  if [ "$RC" -ne 0 ] && [ "$out" = "panic: arena out of memory" ]; then
    echo "  [PASS] read_line_oom.oc (--backend $backend): a line too long for the arena panics"
    PASS=$((PASS + 1))
  else
    echo "  [FAIL] read_line_oom.oc (--backend $backend): exit $RC, printed '$out'"
    exit 1
  fi
done
rm -rf "$WORK"

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
