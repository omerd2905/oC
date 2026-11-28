#!/usr/bin/env python3
"""oC fuzzer (issue #68).

  python3 tools/fuzz/run.py [--budget N] [--seed S] [--mode lifetime|arith|both] [--jobs J]
                            [--out DIR] [--occ PATH]

lifetime: random arena programs (tools/fuzz/lifetime.py) with a known ground truth. The
  validator's verdict is compared with it, and every accepted SAFE program is run with
  --arena-poison on both backends: it must exit 0 and print exactly what the model predicts.
    accepted + unsafe       -> hole            (and running it must fault)
    accepted + path-unsafe  -> hole-path       (an unsafe path that this run does not take)
    rejected + safe         -> false-positive
    accepted + safe, but faults / prints something else -> fault-on-safe / wrong-output
arith: random expressions (tools/fuzz/arith.py); C and native must agree on stdout, stderr
  and exit status.

Each finding is minimized (statements removed while the same finding reproduces) and written
to DIR (default fuzz-findings/) with its seed and evidence. Exit status 1 if anything was
found. Seeds are deterministic: program i of a run uses seed S + i.
"""

import argparse
import copy
import multiprocessing
import os
import random
import subprocess
import sys
import tempfile
import time

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import arith       # noqa: E402
import lifetime    # noqa: E402

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
EXE = ".exe" if os.name == "nt" else ""
OCC = os.path.join(ROOT, "bin", "occ" + EXE)
# Words the validator uses for lifetime errors; any other compile error means the generator
# produced an invalid program.
LIFETIME_WORDS = ("use-after-free", "arena", "escape", "borrow", "live long enough", "dangling", "invalidated")
# Reported, but not a failure: the validator tracks a local array or struct as a whole, so
# after one arena's reset every element of an aggregate that also held its pointers counts as
# dangling (conservative, documented). The generator names aggregates h<N> and r<N>.
KNOWN = {"false-positive-aggregate"}


def build(src, path, backend):
    """(ok, compiler output). The source is written to path + '.oc'."""
    with open(path + ".oc", "w") as f:
        f.write(src)
    p = subprocess.run([OCC, "build", path + ".oc", "-o", path, "--backend", backend, "--arena-poison"],
                       capture_output=True, text=True, cwd=ROOT, timeout=60)
    return p.returncode == 0, (p.stdout + p.stderr).strip()


def execute(path):
    exe = path + EXE if EXE and os.path.exists(path + EXE) else path
    try:
        p = subprocess.run([exe], capture_output=True, timeout=10)
        out = p.stdout.decode("latin-1")
        err = p.stderr.decode("latin-1")
        if os.name == "nt":
            out = out.replace("\r\n", "\n")
            err = err.replace("\r\n", "\n")
        return p.returncode, out, err
    except subprocess.TimeoutExpired:
        return "timeout", "", ""


def first_error(msg):
    for line in msg.splitlines():
        if "error" in line:
            return line
    return msg.splitlines()[0] if msg else ""


# ---------------------------------------------------------------------------------------
# lifetime mode
# ---------------------------------------------------------------------------------------

def lifetime_verdict(body, work):
    """(category or None, details, label, compile_ok) for one statement tree."""
    lab, expected = lifetime.label(body)
    src = lifetime.render(body)
    path = os.path.join(work, "lt")
    ok, msg = build(src, path, "native")
    if not ok:
        if not any(w in msg for w in LIFETIME_WORDS):
            return "gen-error", first_error(msg), lab, ok
        if lab == lifetime.SAFE:
            err = first_error(msg)
            if "variable 'h" in err or "variable 'r" in err or "variable 'q" in err:
                return "false-positive-aggregate", err, lab, ok
            return "false-positive", err, lab, ok
        return None, "", lab, ok
    if lab == lifetime.UNSAFE:
        rc, out, err = execute(path)
        return "hole", "accepted; running it under --arena-poison: exit %s" % rc, lab, ok
    if lab == lifetime.PATH_UNSAFE:
        return "hole-path", "accepted, although a path not taken here uses freed memory", lab, ok
    want = "".join(line + "\n" for line in expected)
    rc, out, err = execute(path)
    if rc != 0:
        return "fault-on-safe", "native, --arena-poison: exit %s %s" % (rc, err.strip()[:200]), lab, ok
    if out != want:
        return "wrong-output", "native printed %r, expected %r" % (out[:200], want[:200]), lab, ok
    ok_c, msg_c = build(src, path + "_c", "c")
    if not ok_c:
        return "backend-mismatch", "native accepts, C backend: " + first_error(msg_c), lab, ok
    rc_c, out_c, err_c = execute(path + "_c")
    if (rc_c, out_c) != (0, want):
        return "backend-mismatch", "C backend: exit %s, printed %r" % (rc_c, out_c[:200]), lab, ok
    return None, "", lab, ok


def lifetime_job(args):
    seed, = args
    phase = "ABC"[seed % 3]
    body, _, _ = lifetime.generate(seed, phase)
    with tempfile.TemporaryDirectory() as work:
        cat, details, lab, ok = lifetime_verdict(body, work)
    return {"mode": "lifetime", "seed": seed, "phase": phase, "label": lab, "accepted": ok,
            "category": cat, "details": details}


def positions(body, prefix=()):
    """Paths (block indices..., statement index) of every statement, outermost first."""
    out = []
    for i, s in enumerate(body):
        out.append(prefix + (i,))
        for j, block in enumerate(s.children()):
            out += positions(block, prefix + (i, j))
    return out


def locate(body, pos):
    """(block, index) for a path from positions()."""
    block = body
    for k in range(0, len(pos) - 1, 2):
        block = block[pos[k]].children()[pos[k + 1]]
    return block, pos[-1]


def minimize_lifetime(body, category, work):
    def same(candidate):
        try:
            cat, _, _, _ = lifetime_verdict(candidate, work)
        except (KeyError, IndexError):
            return False   # a removed declaration: the model cannot run it either
        return cat == category

    changed = True
    while changed:
        changed = False
        for pos in reversed(positions(body)):
            cand = copy.deepcopy(body)
            block, i = locate(cand, pos)
            replacements = [[]]
            if isinstance(block[i], lifetime.If):
                replacements += [block[i].then, block[i].els]
            elif isinstance(block[i], lifetime.While):
                replacements.append(block[i].body)
            for rep in replacements:
                trial = copy.deepcopy(cand)
                tb, ti = locate(trial, pos)
                tb[ti:ti + 1] = copy.deepcopy(rep)
                if same(trial):
                    body, changed = trial, True
                    break
            if changed:
                break
    return lifetime.render(body)


# ---------------------------------------------------------------------------------------
# arith mode
# ---------------------------------------------------------------------------------------

def arith_verdict(decls, stmts, work):
    src = arith.render(decls, stmts)
    path = os.path.join(work, "ar")
    ok_n, msg_n = build(src, path + "_n", "native")
    ok_c, msg_c = build(src, path + "_c", "c")
    if not ok_n and not ok_c:
        return "gen-error", first_error(msg_n)
    if ok_n != ok_c:
        which = "native" if not ok_n else "C"
        msg = msg_n if not ok_n else msg_c
        if "native codegen error" in msg:
            return "native-limit", first_error(msg)
        return "backend-mismatch", "only the %s backend rejects it: %s" % (which, first_error(msg))
    rn, rc = execute(path + "_n"), execute(path + "_c")
    if rn != rc:
        return "backend-mismatch", "native: exit %s out %r err %r | C: exit %s out %r err %r" % (
            rn[0], rn[1][-120:], rn[2][:120], rc[0], rc[1][-120:], rc[2][:120])
    return None, ""


def arith_job(args):
    seed, = args
    decls, stmts = arith.generate(seed)
    with tempfile.TemporaryDirectory() as work:
        cat, details = arith_verdict(decls, stmts, work)
    return {"mode": "arith", "seed": seed, "category": cat, "details": details}


def minimize_arith(decls, stmts, category, work):
    lines = decls + stmts
    nd = len(decls)
    changed = True
    while changed:
        changed = False
        for i in reversed(range(len(lines))):
            trial = lines[:i] + lines[i + 1:]
            d = nd - (1 if i < nd else 0)
            if arith_verdict(trial[:d], trial[d:], work)[0] == category:
                lines, nd, changed = trial, d, True
                break
    return arith.render(lines[:nd], lines[nd:])


# ---------------------------------------------------------------------------------------

def main():
    global OCC
    ap = argparse.ArgumentParser(description=__doc__.split("\n\n")[0])
    ap.add_argument("--budget", type=int, default=2000, help="programs per mode")
    ap.add_argument("--seed", type=int, default=None, help="first seed (default: random)")
    ap.add_argument("--mode", choices=["lifetime", "arith", "both"], default="both")
    ap.add_argument("--jobs", type=int, default=min(2 if os.name == "nt" else 4, os.cpu_count() or 2))
    ap.add_argument("--out", default=os.path.join(ROOT, "fuzz-findings"))
    ap.add_argument("--occ", default=OCC)
    ap.add_argument("--minimize", type=int, default=3, help="findings to minimize per category")
    a = ap.parse_args()
    OCC = os.path.abspath(a.occ)
    seed0 = a.seed if a.seed is not None else random.randrange(1, 2**31)
    print("oC fuzz: seed %d, budget %d per mode, %d jobs" % (seed0, a.budget, a.jobs), flush=True)

    jobs = []
    if a.mode in ("lifetime", "both"):
        jobs += [(lifetime_job, (seed0 + i,)) for i in range(a.budget)]
    if a.mode in ("arith", "both"):
        jobs += [(arith_job, (seed0 + i,)) for i in range(a.budget)]
    start = time.time()
    with multiprocessing.Pool(a.jobs) as pool:
        results = pool.map(run_job, jobs, chunksize=8)

    stats = {}
    findings = [r for r in results if r["category"]]
    for r in results:
        key = (r["mode"], r.get("label", "-"), "accepted" if r.get("accepted") else ("-" if r["mode"] == "arith" else "rejected"))
        stats[key] = stats.get(key, 0) + 1
    print("%d programs in %.1f s" % (len(results), time.time() - start))
    for key in sorted(stats):
        print("  %-9s %-12s %-9s %6d" % (key + (stats[key],)))
    by_cat = {}
    for r in findings:
        by_cat.setdefault((r["mode"], r["category"]), []).append(r)
    if not findings:
        print("no findings")
        return 0
    failed = [r for r in findings if r["category"] not in KNOWN]
    os.makedirs(a.out, exist_ok=True)
    for (mode, cat), rs in sorted(by_cat.items()):
        print("%s %s: %d (seeds %s)" % (mode, cat, len(rs), ", ".join(str(r["seed"]) for r in rs[:10])))
        for r in rs[:0 if cat in KNOWN else a.minimize]:
            with tempfile.TemporaryDirectory() as work:
                if mode == "lifetime":
                    body, _, _ = lifetime.generate(r["seed"], r["phase"])
                    src = minimize_lifetime(body, cat, work)
                else:
                    decls, stmts = arith.generate(r["seed"])
                    src = minimize_arith(decls, stmts, cat, work)
            name = os.path.join(a.out, "%s-%s-%d.oc" % (mode, cat, r["seed"]))
            with open(name, "w") as f:
                f.write("// fuzz finding: %s %s, seed %d\n// %s\n%s" % (mode, cat, r["seed"], r["details"], src))
            print("  minimized: %s" % os.path.relpath(name, ROOT))
    if not failed:
        print("only known limitations (%s): not a failure" % ", ".join(sorted(KNOWN)))
        return 0
    return 1


def run_job(job):
    fn, args = job
    return fn(args)


if __name__ == "__main__":
    sys.exit(main())
