"""Random arithmetic programs for the C-vs-native differential.

Each program declares a few variables and prints a list of random, well-typed expressions:
integer arithmetic in every width (overflow, division by zero, MIN / -1 and failed casts must
panic the same way on both backends), shifts, ~ and unary minus, checked casts, wrapping_*
operations, f32/f64 arithmetic and conversions, comparisons, string literals with every escape,
and calls passing arrays by value ([3]T), by pointer (*[3]T) and as slices ([]T, with bounds
that are sometimes out of range), and casts to an enum (out of range: a panic). The first
panic ends the program; stdout, stderr and the exit status must match between backends.
"""

import random

INTS = {
    "i8": (-128, 127), "i16": (-32768, 32767), "i32": (-2**31, 2**31 - 1), "i64": (-2**63, 2**63 - 1),
    "u8": (0, 255), "u16": (0, 65535), "u32": (0, 2**32 - 1), "u64": (0, 2**64 - 1),
}
BITS = {"i8": 8, "i16": 16, "i32": 32, "i64": 64, "u8": 8, "u16": 16, "u32": 32, "u64": 64}
FLOATS = ["f64", "f32"]
FLOAT_LITS = ["0.0", "0.5", "1.0", "2.5", "3.0", "10.0", "0.1", "1000000.0", "1.0e10", "123.456", "1.0e300"]


def int_lit(rng, t):
    lo, hi = INTS[t]
    pool = [0, 1, 2, 3, 7, 10, 100, hi, hi - 1, lo, lo + 1, rng.randint(0, 50), rng.randint(lo, hi)]
    if lo < 0:
        pool += [-1, -1]   # with lo in the pool: MIN / -1 and MIN % -1
    v = rng.choice(pool)
    v = max(lo, min(hi, v))
    if v > 2**63 - 1:   # an untyped literal above the i64 maximum is a u64 only on its own
        return "(%d).as(u64)" % v
    if v < 0:
        if v == lo:   # -128 is written (-127 - 1): 128 alone does not fit i8
            return "(-%d - 1)" % (-v - 1)
        return "(-%d)" % -v
    return str(v)


class Arith:
    def __init__(self, rng):
        self.rng = rng
        self.vars = {}          # name -> type
        self.arrays = {}        # name -> element type of a [3]T variable

    def var_of(self, t):
        names = [n for n, vt in self.vars.items() if vt == t]
        return self.rng.choice(names) if names else None

    def int_expr(self, t, d):
        r = self.rng
        if d == 0 or r.random() < 0.25:
            v = self.var_of(t)
            return v if v and r.random() < 0.6 else int_lit(r, t)
        k = r.randint(0, 9)
        if k <= 2:
            op = r.choice(["+", "-", "*", "/", "%"])
            return "(%s %s %s)" % (self.int_expr(t, d - 1), op, self.int_expr(t, d - 1))
        if k == 3:
            op = r.choice(["&", "|", "^"])
            return "(%s %s %s)" % (self.int_expr(t, d - 1), op, self.int_expr(t, d - 1))
        if k == 4:
            amount = str(r.choice([0, 1, 3, BITS[t] - 1, BITS[t], BITS[t] + 1, r.randint(0, BITS[t] - 1)]))
            return "(%s %s %s)" % (self.int_expr(t, d - 1), r.choice(["<<", ">>"]), amount)
        if k == 5:
            if t.startswith("i") and r.random() < 0.5:
                return "(-%s)" % self.int_expr(t, d - 1)
            return "(~%s)" % self.int_expr(t, d - 1)
        if k == 6:
            src = r.choice(list(INTS))
            return "(%s).as(%s)" % (self.int_expr(src, d - 1), t)
        if k == 7:
            src = r.choice(FLOATS)
            return "(%s).as(%s)" % (self.float_expr(src, d - 1), t)
        if k == 8:
            fn = r.choice(["wrapping_add", "wrapping_sub", "wrapping_mul"])
            return "%s(%s, %s)" % (fn, self.int_expr(t, d - 1), self.int_expr(t, d - 1))
        return self.int_expr(t, d - 1)

    def float_expr(self, t, d):
        r = self.rng
        if d == 0 or r.random() < 0.3:
            v = self.var_of(t)
            if v and r.random() < 0.6:
                return v
            lit = r.choice(FLOAT_LITS)
            if t == "f32" and "e300" in lit:
                lit = "1.0e30"
            return lit if r.random() < 0.8 else "(-%s)" % lit
        k = r.randint(0, 4)
        if k <= 2:
            return "(%s %s %s)" % (self.float_expr(t, d - 1), r.choice(["+", "-", "*", "/"]), self.float_expr(t, d - 1))
        if k == 3:
            src = r.choice(["i32", "u8", "i16", "u16", "f32", "f64"])   # exact conversions only
            if src in FLOATS:
                if src == t:
                    return self.float_expr(t, d - 1)
                if t == "f32":   # f64 -> f32 can round: written with .as
                    return "(%s).as(f32)" % self.float_expr("f64", d - 1)
                return "(%s).as(f64)" % self.float_expr("f32", d - 1)
            return "(%s).as(%s)" % (self.int_expr(src, d - 1), t)
        return "(-%s)" % self.float_expr(t, d - 1)

    def string_lit(self):
        r = self.rng
        parts = []
        for _ in range(r.randint(1, 8)):
            parts.append(r.choice(["a", "Z", "0", " ", "~", "\\n", "\\t", "\\r", "\\\\", '\\"', "\\x41",
                                   "\\x7e", "\\x00", "\\0", "\\x0a", "\\xff", "ok"]))
        return '"%s"' % "".join(parts)

    def statement(self):
        r = self.rng
        k = r.randint(0, 9)
        if k <= 4:
            t = r.choice(list(INTS))
            return "print(%s);" % self.int_expr(t, r.randint(1, 4))
        if k <= 6:
            t = r.choice(FLOATS)
            return "print(%s);" % self.float_expr(t, r.randint(1, 3))
        if k == 7:
            t = r.choice(list(INTS))
            return "print(%s %s %s);" % (self.int_expr(t, 2), r.choice(["<", "<=", ">", ">=", "==", "!="]), self.int_expr(t, 2))
        if k == 8:
            t = r.choice(FLOATS)
            return "print(%s %s %s);" % (self.float_expr(t, 2), r.choice(["<", "<=", ">", ">="]), self.float_expr(t, 2))
        if k == 9 and self.arrays and r.random() < 0.6:
            return self.array_call()
        if k == 9 and r.random() < 0.4:   # a checked enum cast, in range or not
            return "print(fz_enum_name((%s).as(FzE)));" % self.int_expr("u32", 1)
        return "print(%s);" % self.string_lit()

    def array_call(self):
        """A call passing a [3]T variable as a slice, by value or by pointer (see HELPERS)."""
        r = self.rng
        a = r.choice(sorted(self.arrays))
        t = self.arrays[a]
        k = r.randint(0, 2)
        if k == 0:   # a[lo..hi], sometimes out of range
            lo = r.choice([0, 0, 1, 2, 3])
            hi = r.choice([lo, 3, 3, min(lo + 1, 3), 4])
            return "print(fz_sum_%s(%s[%d..%d]));" % (t, a, lo, hi)
        if k == 1:
            return "print(fz_first_%s(%s));" % (t, a)
        return "print(fz_at_%s(&%s, %d));" % (t, a, r.choice([0, 1, 2, 2, 3]))

    def program(self):
        """(declaration lines, statement lines)."""
        r = self.rng
        decls = []
        for i in range(r.randint(2, 6)):
            t = r.choice(list(INTS) + FLOATS)
            name = "v%d" % i
            value = int_lit(r, t) if t in INTS else r.choice(FLOAT_LITS[:9])
            if t == "f32" and "e" in value:
                value = "2.0"
            decls.append("let %s: %s = %s;" % (name, t, value))
            self.vars[name] = t
        for i in range(r.randint(0, 2)):
            t = r.choice(list(INTS))
            name = "arr%d" % i
            decls.append("let %s: [3]%s = [%s, %s, %s];" % (name, t, int_lit(r, t), int_lit(r, t), int_lit(r, t)))
            self.arrays[name] = t
        stmts = [self.statement() for _ in range(r.randint(4, 10))]
        return decls, stmts


# Functions taking arrays, emitted for the element types a program uses.
HELPERS = {
    "fz_sum_%s": "fn fz_sum_%s(a: []%s) -> %s {\n    let mut s: %s = 0;\n    let mut i: usize = 0;\n"
                 "    while i < a.len {\n        s = s + a[i];\n        i = i + 1;\n    }\n    return s;\n}\n",
    "fz_first_%s": "fn fz_first_%s(a: [3]%s) -> %s {\n    return a[0];\n}\n",
    "fz_at_%s": "fn fz_at_%s(p: *[3]%s, i: usize) -> %s {\n    return p[i];\n}\n",
}


ENUM_HELPER = ("enum FzE { A, B, C }\n\nfn fz_enum_name(e: FzE) -> u32 {\n    match e {\n"
               "        FzE.A => { return 10; }\n        FzE.B => { return 20; }\n        FzE.C => { return 30; }\n"
               "    }\n}\n\n")


def render(decls, stmts):
    body = "".join("    %s\n" % line for line in decls + stmts)
    helpers = ENUM_HELPER if "fz_enum_name(" in body else ""
    for t in INTS:
        for pattern, text in HELPERS.items():
            if (pattern % t) + "(" in body:
                helpers += text.replace("%s", t) + "\n"
    return helpers + "fn main() {\n" + body + "}\n"


def generate(seed):
    return Arith(random.Random(seed)).program()
