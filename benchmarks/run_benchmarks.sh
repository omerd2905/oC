#!/usr/bin/env bash
set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"
OCC="$ROOT_DIR/zig-out/bin/occ"

if [ ! -f "$OCC" ]; then
    echo "Building occ..."
    (cd "$ROOT_DIR" && zig build)
fi

OCC_SRC="$SCRIPT_DIR/throughput.oc"
C_SRC="$SCRIPT_DIR/throughput.c"
SAFETY_SRC="$SCRIPT_DIR/safety_test.oc"

OCC_OUT="$SCRIPT_DIR/occ_bench"
GCC_OUT="$SCRIPT_DIR/gcc_bench"
SAFETY_OUT="$SCRIPT_DIR/safety_test"

# 1. Measure Build Latency
echo "=== 1. Measuring Compilation Latency ==="
start_occ_build=$(date +%s%N)
"$OCC" -O2 "$OCC_SRC" -o "$OCC_OUT"
end_occ_build=$(date +%s%N)
occ_build_ms=$(( (end_occ_build - start_occ_build) / 1000000 ))

start_gcc_build=$(date +%s%N)
gcc -O2 "$C_SRC" -o "$GCC_OUT"
end_gcc_build=$(date +%s%N)
gcc_build_ms=$(( (end_gcc_build - start_gcc_build) / 1000000 ))

echo "occ build latency: ${occ_build_ms} ms"
echo "gcc build latency: ${gcc_build_ms} ms"

# 2. Measure Binary Footprint
echo "=== 2. Measuring Binary Footprint ==="
occ_size_bytes=$(stat -c %s "$OCC_OUT")
gcc_size_bytes=$(stat -c %s "$GCC_OUT")
occ_size_kb=$(awk "BEGIN {printf \"%.1f\", $occ_size_bytes / 1024}")
gcc_size_kb=$(awk "BEGIN {printf \"%.1f\", $gcc_size_bytes / 1024}")

echo "occ binary size: ${occ_size_kb} KB (${occ_size_bytes} bytes)"
echo "gcc binary size: ${gcc_size_kb} KB (${gcc_size_bytes} bytes)"

# 3. Measure Execution Speed (Average of 3 runs for 10M iterations)
echo "=== 3. Measuring Execution Runtime (10,000,000 iterations) ==="
run_iters=3
occ_total_ms=0
gcc_total_ms=0

for i in $(seq 1 $run_iters); do
    start_run=$(date +%s%N)
    "$OCC_OUT" > /dev/null
    end_run=$(date +%s%N)
    occ_total_ms=$(( occ_total_ms + (end_run - start_run) / 1000000 ))

    start_run=$(date +%s%N)
    "$GCC_OUT" > /dev/null
    end_run=$(date +%s%N)
    gcc_total_ms=$(( gcc_total_ms + (end_run - start_run) / 1000000 ))
done

occ_avg_ms=$(( occ_total_ms / run_iters ))
gcc_avg_ms=$(( gcc_total_ms / run_iters ))
occ_avg_s=$(awk "BEGIN {printf \"%.3f\", $occ_avg_ms / 1000}")
gcc_avg_s=$(awk "BEGIN {printf \"%.3f\", $gcc_avg_ms / 1000}")

echo "occ runtime avg: ${occ_avg_s} s (${occ_avg_ms} ms)"
echo "gcc runtime avg: ${gcc_avg_s} s (${gcc_avg_ms} ms)"

# 4. Safety Bounds Verification
echo "=== 4. Executing Spatial Bounds Safety Test ==="
"$OCC" "$SAFETY_SRC" -o "$SAFETY_OUT"
safety_passed="FAIL"
set +e
safety_err=$("$SAFETY_OUT" 2>&1)
safety_status=$?
set -e

if [ $safety_status -ne 0 ] && echo "$safety_err" | grep -q "panic: index out of bounds"; then
    safety_passed="PASS (trapped OOB)"
    echo "Safety check verified: caught expected out-of-bounds panic."
else
    echo "Safety check failed! Output: $safety_err"
fi

# Clean Scoreboard
echo ""
echo "=================================================================================="
echo "                        oC vs Native C Benchmark Scoreboard                       "
echo "=================================================================================="
printf "%-28s | %-20s | %-20s | %-10s\n" "Metric" "oC (occ -O2)" "Native C (gcc -O2)" "Comparison"
echo "-----------------------------+----------------------+----------------------+------------"
printf "%-28s | %-20s | %-20s | %-10s\n" "Build Latency" "${occ_build_ms} ms" "${gcc_build_ms} ms" "$(awk "BEGIN {printf \"~%.2fx\", $occ_build_ms / ($gcc_build_ms > 0 ? $gcc_build_ms : 1)}")"
printf "%-28s | %-20s | %-20s | %-10s\n" "Binary Footprint" "${occ_size_kb} KB" "${gcc_size_kb} KB" "$(awk "BEGIN {printf \"%.2fx\", $occ_size_bytes / $gcc_size_bytes}")"
printf "%-28s | %-20s | %-20s | %-10s\n" "Runtime (10M packets)" "${occ_avg_s} s" "${gcc_avg_s} s" "$(awk "BEGIN {printf \"~%.2fx\", $occ_avg_ms / ($gcc_avg_ms > 0 ? $gcc_avg_ms : 1)}")"
printf "%-28s | %-20s | %-20s | %-10s\n" "Spatial Bounds Safety" "$safety_passed" "N/A (unsafe)" "Enforced"
echo "=================================================================================="
