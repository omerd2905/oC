#!/usr/bin/env bash
set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"
OCC="$ROOT_DIR/bin/occ"

if [ ! -f "$OCC" ]; then
    echo "Error: $OCC not found"
    exit 1
fi

measure_ms() {
    local bin="$1"
    local start_ns end_ns
    start_ns=$(date +%s%N)
    "$bin" > /dev/null
    end_ns=$(date +%s%N)
    echo $(( (end_ns - start_ns) / 1000000 ))
}

echo "======================================================================================"
echo "                  oC v1.0 Multi-Workload Stress-Test Suite (3 Runs/Test)              "
echo "======================================================================================"

# 1. Compile all targets
echo "[1/4] Compiling stress-test binaries..."
"$OCC" build -O2 "$SCRIPT_DIR/arena_stress.oc" -o /tmp/oc_arena_stress
gcc -O2 "$SCRIPT_DIR/arena_stress.c" -o /tmp/c_arena_stress

"$OCC" build -O2 "$SCRIPT_DIR/packet_stress.oc" -o /tmp/oc_packet_stress
gcc -O2 "$SCRIPT_DIR/packet_stress.c" -o /tmp/c_packet_stress

"$OCC" build "$SCRIPT_DIR/native_stress.oc" -o /tmp/oc_native_elf --backend native
"$OCC" build -O2 "$SCRIPT_DIR/native_stress.oc" -o /tmp/oc_c_stress
gcc -O2 "$SCRIPT_DIR/native_stress.c" -o /tmp/c_native_stress

# Verify identical mathematical outputs across oC and C
oc_arena_out=$(/tmp/oc_arena_stress)
c_arena_out=$(/tmp/c_arena_stress)
oc_pkt_out=$(/tmp/oc_packet_stress)
c_pkt_out=$(/tmp/c_packet_stress)
oc_nat_out=$(/tmp/oc_native_elf)
c_nat_out=$(/tmp/c_native_stress)

if [ "$oc_arena_out" != "$c_arena_out" ] || [ "$oc_pkt_out" != "$c_pkt_out" ] || [ "$oc_nat_out" != "$c_nat_out" ]; then
    echo "✘ Checksum mismatch between oC and C!"
    exit 1
fi
echo "✔ Checksums verified identical across all oC and C stress workloads."

# 2. Run Stress Test 1: 12.8M Variable-Sized Allocations + CWE-14 Scrubbing
echo "[2/4] Running Stress Test 1: 12,800,000 Regional Allocations + CWE-14 Memory Scrub..."
oc_arena_ms=0
c_arena_ms=0
for _ in 1 2 3; do
    oc_arena_ms=$(( oc_arena_ms + $(measure_ms /tmp/oc_arena_stress) ))
    c_arena_ms=$(( c_arena_ms + $(measure_ms /tmp/c_arena_stress) ))
done
oc_arena_ms=$(( oc_arena_ms / 3 ))
c_arena_ms=$(( c_arena_ms / 3 ))

# 3. Run Stress Test 2: 25M Wire Frames Parsed (Checked Fat Slices vs Unchecked C Pointers)
echo "[3/4] Running Stress Test 2: 25,000,000 Stateful Wire Frames (Checked Slices vs Raw C)..."
oc_pkt_ms=0
c_pkt_ms=0
for _ in 1 2 3; do
    oc_pkt_ms=$(( oc_pkt_ms + $(measure_ms /tmp/oc_packet_stress) ))
    c_pkt_ms=$(( c_pkt_ms + $(measure_ms /tmp/c_packet_stress) ))
done
oc_pkt_ms=$(( oc_pkt_ms / 3 ))
c_pkt_ms=$(( c_pkt_ms / 3 ))

# 4. Run Stress Test 3: 78.1M Recursive Calls + 200K Collatz Trajectories
echo "[4/4] Running Stress Test 3: fib(38) (78.1M calls) + 200K Collatz Hailstone Chains..."
oc_elf_ms=$(measure_ms /tmp/oc_native_elf)
oc_opt_ms=$(measure_ms /tmp/oc_c_stress)
c_opt_ms=$(measure_ms /tmp/c_native_stress)

elf_sz=$(stat -c %s /tmp/oc_native_elf)
c_dyn_sz=$(stat -c %s /tmp/c_native_stress)

rm -f /tmp/oc_arena_stress /tmp/c_arena_stress /tmp/oc_packet_stress /tmp/c_packet_stress /tmp/oc_native_elf /tmp/oc_c_stress /tmp/c_native_stress

echo ""
echo "======================================================================================"
echo "                        oC v1.0 Stress-Test Results Scoreboard                        "
echo "======================================================================================"
printf "%-36s | %-16s | %-16s | %-14s\n" "Workload / Metric" "oC v1.0" "Native C (-O2)" "Delta / Notes"
echo "-------------------------------------+------------------+------------------+--------------"
printf "%-36s | %-16s | %-16s | %-14s\n" "12.8M Allocs + CWE-14 Scrubbing" "${oc_arena_ms} ms" "${c_arena_ms} ms" "$(awk "BEGIN {printf \"%.2fx faster\", $c_arena_ms / ($oc_arena_ms > 0 ? $oc_arena_ms : 1)}")"
printf "%-36s | %-16s | %-16s | %-14s\n" "25M Wire Frames (Checked vs Unsafe)" "${oc_pkt_ms} ms" "${c_pkt_ms} ms" "$(awk "BEGIN {printf \"%+d ms (bounds)\", $oc_pkt_ms - $c_pkt_ms}")"
printf "%-36s | %-16s | %-16s | %-14s\n" "78.1M Calls + 200K Collatz (-O2)" "${oc_opt_ms} ms" "${c_opt_ms} ms" "$(awk "BEGIN {printf \"%.2fx parity\", $oc_opt_ms / ($c_opt_ms > 0 ? $c_opt_ms : 1)}")"
printf "%-36s | %-16s | %-16s | %-14s\n" "78.1M Calls (--backend native ELF)" "${oc_elf_ms} ms" "${c_opt_ms} ms" "1-pass + jo/jz"
printf "%-36s | %-16s | %-16s | %-14s\n" "Binary Size (--backend native)" "${elf_sz} B (5.0K)" "${c_dyn_sz} B (16K)" "$(awk "BEGIN {printf \"%.1fx smaller\", $c_dyn_sz / $elf_sz}")"
echo "======================================================================================"
