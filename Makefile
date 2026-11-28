# Builds the oC compiler from the committed, portable C bootstrap source.
# Windows (MinGW): gcc -O2 bootstrap\occ_bootstrap.c -o bin\occ.exe
CC ?= cc

bin/occ: bootstrap/occ_bootstrap.c
	mkdir -p bin
	$(CC) -O2 $< -o $@

# Fuzzer (tools/fuzz, issue #68): FUZZ_BUDGET random programs per mode (arena lifetimes and
# C-vs-native arithmetic), from FUZZ_SEED (random if unset; it is printed). Findings are
# minimized into fuzz-findings/. About a minute on 4 cores at the default budget.
FUZZ_BUDGET ?= 1000
fuzz: bin/occ
	python3 tools/fuzz/run.py --budget $(FUZZ_BUDGET) $(if $(FUZZ_SEED),--seed $(FUZZ_SEED))

.PHONY: fuzz

# The full test suite: self-hosting fixed point, every probe, parity test and fuzz regression
# on both backends (./showcase.sh).
test: bin/occ
	./showcase.sh

.PHONY: test
