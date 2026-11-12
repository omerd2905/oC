#!/usr/bin/env python3
"""Build every complete example program in the documentation.

A "complete example" is a <pre> block in docs/*.html (or an ```oc block in README.md) that
contains `fn main`. Each one must compile with bin/occ, so the docs cannot drift from the
compiler. A block that demonstrates a rejection is marked in the HTML:

    <pre data-occ-rejects="text the compiler must print">

and must then FAIL to compile with that text in the compiler output.
Usage: check_docs.py [path/to/occ]   (run from the repository root)
"""
import glob
import html
import os
import re
import subprocess
import sys
import tempfile

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
OCC = os.path.abspath(sys.argv[1]) if len(sys.argv) > 1 else os.path.join(ROOT, "bin", "occ")

PRE = re.compile(r"<pre([^>]*)>(.*?)</pre>", re.S)
REJECTS = re.compile(r'data-occ-rejects="([^"]*)"')
FENCE = re.compile(r"```oc\n(.*?)```", re.S)


def blocks():
    """Yield (location, source, expected_rejection_or_None)."""
    for path in sorted(glob.glob(os.path.join(ROOT, "docs", "*.html"))):
        text = open(path, encoding="utf-8").read()
        for m in PRE.finditer(text):
            src = html.unescape(re.sub(r"<[^>]+>", "", m.group(2)))
            if "fn main" not in src:
                continue
            line = text.count("\n", 0, m.start()) + 1
            rej = REJECTS.search(m.group(1))
            yield "%s:%d" % (os.path.relpath(path, ROOT), line), src, html.unescape(rej.group(1)) if rej else None
    readme = open(os.path.join(ROOT, "README.md"), encoding="utf-8").read()
    for m in FENCE.finditer(readme):
        if "fn main" in m.group(1):
            yield "README.md:%d" % (readme.count("\n", 0, m.start()) + 1), m.group(1), None


def main():
    failures = 0
    total = 0
    with tempfile.TemporaryDirectory() as tmp:
        for where, src, rejects in blocks():
            total += 1
            src_path = os.path.join(tmp, "doc_example.oc")
            with open(src_path, "w", encoding="utf-8") as f:
                f.write(src)
            # cwd = repo root, so `import "std/..."` and `import "examples/..."` resolve.
            run = subprocess.run([OCC, "build", src_path, "-o", os.path.join(tmp, "doc_example")],
                                 cwd=ROOT, capture_output=True, text=True)
            out = run.stdout + run.stderr
            if rejects is None:
                ok = run.returncode == 0
                why = "does not compile"
            else:
                ok = run.returncode != 0 and rejects in out
                why = 'must be rejected with "%s"' % rejects
            if ok:
                print("  [PASS] %s%s" % (where, " (rejected as documented)" if rejects else ""))
            else:
                failures += 1
                print("  [FAIL] %s %s:\n%s" % (where, why, out.strip()))
    print("%d / %d documentation examples OK" % (total - failures, total))
    return 1 if failures else 0


if __name__ == "__main__":
    sys.exit(main())
