#!/usr/bin/env bash
# Builds each workload three ways (C reference, oC C backend, oC native backend), checks that all
# three print the same checksum, and prints a Markdown table of wall-clock times.
# The "Performance Characteristics" table in README.md is this script's output: to refresh it,
# run ./benchmarks/run_benchmarks.sh on an idle machine and paste the result.
set -euo pipefail

DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
OCC="$DIR/../bin/occ"
RUNS="${RUNS:-5}"
TMP="$(mktemp -d)"
trap 'rm -rf "$TMP"' EXIT

"$OCC" --version >/dev/null || { echo "error: $OCC does not run; build it with 'make'" >&2; exit 1; }

# Best wall-clock time of $RUNS runs, in milliseconds.
best_ms() {
    local best="" start end ms
    for _ in $(seq "$RUNS"); do
        start=$(date +%s%N)
        "$1" >/dev/null
        end=$(date +%s%N)
        ms=$(( (end - start) / 1000000 ))
        if [ -z "$best" ] || [ "$ms" -lt "$best" ]; then best=$ms; fi
    done
    echo "$best"
}

# row <benchmark file stem> <label> <notes>
row() {
    local name="$1" c oc_c oc_native
    gcc -O2 "$DIR/$name.c" -o "$TMP/${name}_c"
    "$OCC" build -O2 "$DIR/$name.oc" -o "$TMP/${name}_oc_c"
    "$OCC" build "$DIR/$name.oc" -o "$TMP/${name}_oc_native" --backend native
    c="$("$TMP/${name}_c")"
    oc_c="$("$TMP/${name}_oc_c")"
    oc_native="$("$TMP/${name}_oc_native")"
    if [ "$c" != "$oc_c" ] || [ "$c" != "$oc_native" ]; then
        echo "error: $name: checksum mismatch (C=$c, oC C backend=$oc_c, oC native=$oc_native)" >&2
        exit 1
    fi
    echo "| $2 | $(best_ms "$TMP/${name}_c") ms | $(best_ms "$TMP/${name}_oc_c") ms | $(best_ms "$TMP/${name}_oc_native") ms | $3 |"
}

CPU="$(grep -m1 'model name' /proc/cpuinfo 2>/dev/null | sed 's/.*: //' || true)"
echo "Measured $(date -u +%Y-%m-%d) with \`benchmarks/run_benchmarks.sh\`: ${CPU:-unknown CPU}, $(uname -sm), $(gcc --version | head -n 1). Best of $RUNS runs, wall-clock time; all three binaries of a row print the same checksum."
echo ""
echo "| Workload | C (\`gcc -O2\`) | oC, C backend (\`-O2\`) | oC, native backend | What is compared |"
echo "|:---|---:|---:|---:|:---|"
row arena_stress  "12.8M allocations + scrub on release" "Different allocators: oC bump arena (\`sys_mmap\`, one reset) vs C per-object \`malloc\` + \`memset\` + \`free\`"
row packet_stress "25M wire frames parsed + FNV-1a" "Same algorithm: bounds-checked fat slices vs unchecked C pointers"
row native_stress "\`fib(38)\` (78.1M calls) + 200K Collatz chains" "Same algorithm: overflow-checked arithmetic vs wrapping C arithmetic"
printf 'fn main() {\n    print("Hello World!");\n}\n' > "$TMP/hello.oc"
printf '#include <stdio.h>\nint main(void) { puts("Hello World!"); return 0; }\n' > "$TMP/hello.c"
"$OCC" build "$TMP/hello.oc" -o "$TMP/hello_native" --backend native
"$OCC" build -O2 "$TMP/hello.oc" -o "$TMP/hello_oc_c"
gcc -O2 "$TMP/hello.c" -o "$TMP/hello_c"
echo "| Hello-world binary size | $(stat -c %s "$TMP/hello_c") B | $(stat -c %s "$TMP/hello_oc_c") B | $(stat -c %s "$TMP/hello_native") B | Dynamically linked (libc) for the first two; the native backend emits a static ELF with no libc |"
