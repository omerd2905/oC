# Builds the oC compiler from the committed, portable C bootstrap source.
# Windows (MinGW): gcc -O2 bootstrap\occ_bootstrap.c -o bin\occ.exe
CC ?= cc

bin/occ: bootstrap/occ_bootstrap.c
	mkdir -p bin
	$(CC) -O2 $< -o $@
