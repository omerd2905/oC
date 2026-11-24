"""Random arena-lifetime programs for the oC validator, each labeled with its ground truth.

A program is a tree of statements (the classes below). The same tree is
  - rendered as oC source (render), and
  - executed abstractly (label) to find out what the validator should say:
      SAFE         no path uses a pointer or string after its arena was reset or freed;
      UNSAFE       the path that actually runs does (so running it must fault under
                   --arena-poison);
      PATH_UNSAFE  only a path that does not run does. A checker that does not evaluate
                   conditions must still reject it, because in general they depend on input.
    It also gives the exact stdout of a correct run, a second oracle for SAFE programs.

"Use" means any mention of the variable, a copy included: that is the validator's rule.
Conditions are bool variables with constant values and loops have constant trip counts, so
the actual path is known; for the static label, every if goes both ways and every loop runs
0, 1 or 2 times (two iterations expose a use in one iteration after a reset in the previous).
"""

import random

SAFE, UNSAFE, PATH_UNSAFE = "safe", "unsafe", "path-unsafe"


class Stmt:
    def children(self):
        """The statement lists nested in this statement (for the minimizer)."""
        return []


class NewArena(Stmt):
    def __init__(self, a): self.a = a
    def render(self, ind): return [ind + "let mut %s: Arena = arena_init(4096);" % self.a]


class LetCond(Stmt):
    def __init__(self, c, value): self.c, self.value = c, value
    def render(self, ind): return [ind + "let %s: bool = %s;" % (self.c, "true" if self.value else "false")]


class LetPtr(Stmt):
    """let mut v: *mut u8 = <alloc from arena a>; v[0] = k;  via: how it is allocated."""
    def __init__(self, v, a, k, via=None, cond=None):
        self.v, self.a, self.k, self.via, self.cond = v, a, k, via, cond

    def call(self):
        if self.via is None:
            return "arena_alloc(&mut %s, 16)" % self.a
        if self.via == "fz_fresh":
            return "fz_fresh(&mut %s, %s)" % (self.a, self.cond)
        return "%s(&mut %s)" % (self.via, self.a)

    def render(self, ind):
        return [ind + "let mut %s: *mut u8 = %s;" % (self.v, self.call()), ind + "%s[0] = %d;" % (self.v, self.k)]


class Realloc(LetPtr):
    """v = <alloc from arena a>; v[0] = k;  (v is declared already)"""
    def render(self, ind):
        return [ind + "%s = %s;" % (self.v, self.call()), ind + "%s[0] = %d;" % (self.v, self.k)]


class LetStr(Stmt):
    """let mut v: string = str_concat(&mut a, "lit", <src or "lit2">);  or through fz_str."""
    def __init__(self, v, a, lit, src=None, via=None):
        self.v, self.a, self.lit, self.src, self.via = v, a, lit, src, via

    def render(self, ind):
        second = self.src if self.src else '"%s"' % self.lit[::-1]
        if self.via == "fz_str":
            first = self.src if self.src else '"%s"' % self.lit
            return [ind + "let mut %s: string = fz_str(&mut %s, %s);" % (self.v, self.a, first)]
        return [ind + 'let mut %s: string = str_concat(&mut %s, "%s", %s);' % (self.v, self.a, self.lit, second)]


class LetHolder(Stmt):
    def __init__(self, h, p, s): self.h, self.p, self.s = h, p, s
    def render(self, ind): return [ind + "let mut %s: FzHolder = FzHolder { p: %s, s: %s };" % (self.h, self.p, self.s)]


class LetArr(Stmt):
    def __init__(self, r, cells): self.r, self.cells = r, cells
    def render(self, ind): return [ind + "let mut %s: [2]*mut u8 = [%s, %s];" % (self.r, self.cells[0], self.cells[1])]


class Reset(Stmt):
    """arena_reset(&mut a), or fz_reset_if(&mut a, cond) when cond is set."""
    def __init__(self, a, cond=None): self.a, self.cond = a, cond

    def render(self, ind):
        if self.cond:
            return [ind + "fz_reset_if(&mut %s, %s);" % (self.a, self.cond)]
        return [ind + "arena_reset(&mut %s);" % self.a]


class Deinit(Stmt):
    def __init__(self, a): self.a = a
    def render(self, ind): return [ind + "arena_deinit(&mut %s);" % self.a]


class Use(Stmt):
    """print(<cell>), reading through it: p[0], s, h.p[0], r[i][0]; or through a helper."""
    def __init__(self, cell, kind, via=None): self.cell, self.kind, self.via = cell, kind, via

    def render(self, ind):
        if self.via == "fz_first":                    # cell is "h.p"
            return [ind + "print(fz_first(&%s));" % self.cell.split(".")[0]]
        if self.via == "fz_len":
            return [ind + "print(fz_len(%s));" % self.cell]
        if self.kind == "ptr":
            return [ind + "print(%s[0]);" % self.cell]
        return [ind + "print(%s);" % self.cell]


class Store(Stmt):
    """h.p = v; h.s = s; r[i] = v;"""
    def __init__(self, dst, src): self.dst, self.src = dst, src
    def render(self, ind): return [ind + "%s = %s;" % (self.dst, self.src)]


class If(Stmt):
    def __init__(self, c, then, els): self.c, self.then, self.els = c, then, els
    def children(self): return [self.then, self.els]

    def render(self, ind):
        out = [ind + "if %s {" % self.c]
        for s in self.then:
            out += s.render(ind + "    ")
        if self.els:
            out.append(ind + "} else {")
            for s in self.els:
                out += s.render(ind + "    ")
        out.append(ind + "}")
        return out


class While(Stmt):
    def __init__(self, i, n, body): self.i, self.n, self.body = i, n, body
    def children(self): return [self.body]

    def render(self, ind):
        out = [ind + "let mut %s: i32 = 0;" % self.i, ind + "while %s < %d {" % (self.i, self.n)]
        for s in self.body:
            out += s.render(ind + "    ")
        out += [ind + "    %s = %s + 1;" % (self.i, self.i), ind + "}"]
        return out


HELPERS = {
    "fz_alloc": "fn fz_alloc(a: *mut Arena) -> *mut u8 {\n    return arena_alloc(a, 16);\n}",
    "fz_alloc2": "fn fz_alloc2(a: *mut Arena) -> *mut u8 {\n    return fz_alloc(a);\n}",
    "fz_alloc3": "fn fz_alloc3(a: *mut Arena) -> *mut u8 {\n    let p: *mut u8 = fz_alloc2(a);\n    return p;\n}",
    "fz_fresh": "fn fz_fresh(a: *mut Arena, c: bool) -> *mut u8 {\n    if c {\n        arena_reset(a);\n    }\n    return arena_alloc(a, 16);\n}",
    "fz_reset_if": "fn fz_reset_if(a: *mut Arena, c: bool) {\n    if c {\n        arena_reset(a);\n    }\n}",
    "fz_str": 'fn fz_str(a: *mut Arena, x: string) -> string {\n    return str_concat(a, x, "+");\n}',
    "fz_first": "fn fz_first(h: *FzHolder) -> u8 {\n    return h.p[0];\n}",
    "fz_len": "fn fz_len(s: string) -> usize {\n    return s.len;\n}",
}
HELPER_DEPS = {"fz_alloc2": ["fz_alloc"], "fz_alloc3": ["fz_alloc2", "fz_alloc"]}


def walk(stmts):
    for s in stmts:
        yield s
        for block in s.children():
            yield from walk(block)


def render(body):
    """The whole program: imports, the helpers and struct it uses, then main."""
    used = set()
    holder = False
    for s in walk(body):
        for name in (getattr(s, "via", None), "fz_reset_if" if isinstance(s, Reset) and s.cond else None):
            if name:
                used.add(name)
                used.update(HELPER_DEPS.get(name, []))
        if isinstance(s, LetHolder):
            holder = True
    out = ['import "std/str.oc";', ""]
    if holder or "fz_first" in used:
        out += ["struct FzHolder {", "    p: *mut u8,", "    s: string,", "}", ""]
    for name in HELPERS:                       # fixed order: dependencies come first
        if name in used:
            out += [HELPERS[name], ""]
    out.append("fn main() {")
    for s in body:
        out += s.render("    ")
    out.append("}")
    return "\n".join(out) + "\n"


# ---------------------------------------------------------------------------------------
# Ground truth: run the statement tree abstractly.
# A state is (arenas, cells, unsafe, out): arenas maps a name to [epoch, dead], cells maps a
# cell ("p0", "h0.p", "r0[1]", ...) to (arena, epoch at allocation, content).
# ---------------------------------------------------------------------------------------

class State:
    def __init__(self):
        self.arenas, self.cells, self.conds, self.unsafe, self.out = {}, {}, {}, False, []

    def copy(self):
        s = State()
        s.arenas = {k: list(v) for k, v in self.arenas.items()}
        s.cells, s.conds, s.unsafe, s.out = dict(self.cells), self.conds, self.unsafe, list(self.out)
        return s

    def key(self):
        return (tuple(sorted((k, tuple(v)) for k, v in self.arenas.items())), tuple(sorted(self.cells.items())), self.unsafe)

    def use(self, cell):
        a, epoch, content = self.cells[cell]
        if self.arenas[a][0] != epoch or self.arenas[a][1]:
            self.unsafe = True
        return content

    def emit(self, text):
        if not self.unsafe:            # after a dangling use the real run has faulted
            self.out.append(text)

    def fresh(self, cell, a, content):
        self.cells[cell] = (a, self.arenas[a][0], content)

    def reset(self, a):
        self.arenas[a][0] += 1


def step(s, st, static, trace):
    """Apply one statement to state st; returns the list of resulting states."""
    if isinstance(s, NewArena):
        st.arenas[s.a] = [0, False]
    elif isinstance(s, LetCond):
        st.conds = dict(st.conds)
        st.conds[s.c] = s.value
    elif isinstance(s, LetPtr):            # Realloc too
        if s.via == "fz_fresh":
            if static:
                other = st.copy()
                other.reset(s.a)
                other.fresh(s.v, s.a, s.k)
                st.fresh(s.v, s.a, s.k)
                return [st, other]
            if st.conds[s.cond]:
                st.reset(s.a)
        st.fresh(s.v, s.a, s.k)
    elif isinstance(s, LetStr):
        text = s.lit
        if s.via == "fz_str":
            text = (st.use(s.src) if s.src else s.lit) + "+"
        else:
            text += st.use(s.src) if s.src else s.lit[::-1]
        st.fresh(s.v, s.a, text)
    elif isinstance(s, LetHolder):
        st.use(s.p)
        st.use(s.s)
        st.cells[s.h + ".p"] = st.cells[s.p]
        st.cells[s.h + ".s"] = st.cells[s.s]
    elif isinstance(s, LetArr):
        for i, c in enumerate(s.cells):
            st.use(c)
            st.cells["%s[%d]" % (s.r, i)] = st.cells[c]
    elif isinstance(s, Reset):
        if s.cond and static:
            other = st.copy()
            other.reset(s.a)
            return [st, other]
        if not s.cond or st.conds[s.cond]:
            st.reset(s.a)
    elif isinstance(s, Deinit):
        st.arenas[s.a][1] = True
    elif isinstance(s, Use):
        if trace is not None and not static:
            trace[id(s)] = st.cells[s.cell][0]
        content = st.use(s.cell)
        st.emit(str(len(content)) if s.via == "fz_len" else str(content))
    elif isinstance(s, Store):
        st.use(s.src)
        st.cells[s.dst] = st.cells[s.src]
    elif isinstance(s, If):
        if static:
            other = st.copy()
            return run(s.then, [st], static, trace) + run(s.els, [other], static, trace)
        return run(s.then if st.conds[s.c] else s.els, [st], static, trace)
    elif isinstance(s, While):
        if static:
            out, cur = [st.copy()], [st]
            for _ in range(2):
                cur = run(s.body, cur, static, trace)
                out += [c.copy() for c in cur]
            return out
        cur = [st]
        for _ in range(s.n):
            cur = run(s.body, cur, static, trace)
        return cur
    return [st]


def run(stmts, states, static, trace=None):
    for s in stmts:
        nxt, seen = [], set()
        for st in states:
            for r in step(s, st, static, trace):
                k = r.key()
                if not static or k not in seen:
                    seen.add(k)
                    nxt.append(r)
        states = nxt
    return states


def label(body, trace=None):
    """(label, expected stdout lines of the real run)."""
    dyn = run(body, [State()], False, trace)[0]
    if dyn.unsafe:
        return UNSAFE, dyn.out
    if any(st.unsafe for st in run(body, [State()], True)):
        return PATH_UNSAFE, dyn.out
    return SAFE, dyn.out


# ---------------------------------------------------------------------------------------
# Generation. phase "A": straight line, one or two arenas; "B": also helpers, structs and
# arrays; "C": also if / while and conditional resets in callees.
# ---------------------------------------------------------------------------------------

WORDS = ["ab", "cd", "oc", "xy", "hi", "zz"]


class Gen:
    def __init__(self, rng, phase):
        self.rng, self.phase = rng, phase
        self.n = {}
        self.arena_of = {}        # ptr/str variable -> its arena (fixed for the variable's life)
        self.valid = {}           # cell -> kind, for cells believed usable here (a guide only)
        self.ptr_vars, self.holders, self.arrays, self.conds = [], [], [], []

    def name(self, prefix):
        k = self.n.get(prefix, 0)
        self.n[prefix] = k + 1
        return "%s%d" % (prefix, k)

    def program(self):
        r = self.rng
        body = []
        self.arenas = [self.name("a") for _ in range(r.randint(1, 2 if self.phase == "A" else 3))]
        body += [NewArena(a) for a in self.arenas]
        if self.phase == "C":
            for _ in range(r.randint(1, 2)):
                c = self.name("c")
                self.conds.append(c)
                body.append(LetCond(c, r.random() < 0.5))
        for _ in range(r.randint(3, 6)):
            body.append(self.new_ptr())
        body += self.block(r.randint(4, 12), top=True)
        for cell, kind in sorted(self.valid.items()):
            if r.random() < 0.5:
                body.append(Use(cell, kind))
        body += [Deinit(a) for a in self.arenas]
        return body

    def new_ptr(self):
        r = self.rng
        v, a = self.name("p"), r.choice(self.arenas)
        via, cond = None, None
        if self.phase != "A" and r.random() < 0.4:
            via = r.choice(["fz_alloc", "fz_alloc2", "fz_alloc3"] + (["fz_fresh"] if self.conds else []))
            if via == "fz_fresh":
                cond = r.choice(self.conds)
                self.drop_arena(a)
        self.arena_of[v] = a
        self.ptr_vars.append(v)
        self.valid[v] = "ptr"
        return LetPtr(v, a, r.randint(1, 250), via, cond)

    def new_str(self):
        r = self.rng
        v, a = self.name("s"), r.choice(self.arenas)
        strs = [c for c, k in self.valid.items() if k == "str" and "." not in c]
        src = r.choice(strs) if strs and r.random() < 0.5 else None
        via = "fz_str" if self.phase != "A" and r.random() < 0.3 else None
        self.arena_of[v] = a
        self.valid[v] = "str"
        return LetStr(v, a, r.choice(WORDS), src, via)

    def drop_arena(self, a):
        for cell in list(self.valid):
            if self.cell_arena(cell) == a:
                del self.valid[cell]

    def cell_arena(self, cell):
        return self.arena_of.get(cell)

    def pick_cell(self, kind=None):
        r = self.rng
        cells = [c for c, k in self.valid.items() if kind in (None, k)]
        if r.random() < 0.06 or not cells:   # now and then a cell that may be dangling
            cells = [c for c in self.all_cells() if kind in (None, self.kind_of(c))]
        return r.choice(cells) if cells else None

    def all_cells(self):
        out = list(self.arena_of)
        for h in self.holders:
            out += [h + ".p", h + ".s"]
        for a in self.arrays:
            out += [a + "[0]", a + "[1]"]
        return out

    def kind_of(self, cell):
        if cell.endswith(".s") or cell.startswith("s"):
            return "str"
        return "ptr"

    def block(self, steps, top=False, depth=0):
        r = self.rng
        out = []
        for _ in range(steps):
            choices = ["use", "use", "reset", "realloc"]
            if top:
                choices += ["ptr", "str"]
                if self.phase != "A":
                    choices += ["holder", "array"]
            if self.phase != "A":
                choices += ["store"]
            if self.phase == "C" and depth < 2:
                choices += ["if", "while"]
            what = r.choice(choices)
            s = getattr(self, "gen_" + what)(depth)
            if s is not None:
                out.append(s)
        return out

    def gen_ptr(self, depth): return self.new_ptr()
    def gen_str(self, depth): return self.new_str()

    def gen_use(self, depth):
        cell = self.pick_cell()
        if cell is None:
            return None
        kind = self.kind_of(cell)
        via = None
        if self.phase != "A":
            if kind == "str" and self.rng.random() < 0.3:
                via = "fz_len"
            elif cell.endswith(".p") and self.rng.random() < 0.4:
                via = "fz_first"
        return Use(cell, kind, via)

    def gen_reset(self, depth):
        a = self.rng.choice(self.arenas)
        cond = self.rng.choice(self.conds) if self.conds and self.rng.random() < 0.4 else None
        self.drop_arena(a)
        return Reset(a, cond)

    def gen_realloc(self, depth):
        if not self.ptr_vars:
            return None
        v = self.rng.choice(self.ptr_vars)
        self.valid[v] = "ptr"
        return Realloc(v, self.arena_of[v], self.rng.randint(1, 250))

    def gen_holder(self, depth):
        p, s = self.pick_cell("ptr"), self.pick_cell("str")
        if p is None or s is None or "." in p or "[" in p or "." in s:
            return None
        h = self.name("h")
        self.holders.append(h)
        self.arena_of[h + ".p"], self.arena_of[h + ".s"] = self.arena_of.get(p), self.arena_of.get(s)
        self.valid[h + ".p"], self.valid[h + ".s"] = "ptr", "str"
        return LetHolder(h, p, s)

    def gen_array(self, depth):
        a, b = self.pick_cell("ptr"), self.pick_cell("ptr")
        if a is None or b is None or "." in a + b or "[" in a + b:
            return None
        rr = self.name("r")
        self.arrays.append(rr)
        self.arena_of[rr + "[0]"], self.arena_of[rr + "[1]"] = self.arena_of[a], self.arena_of[b]
        self.valid[rr + "[0]"], self.valid[rr + "[1]"] = "ptr", "ptr"
        return LetArr(rr, [a, b])

    def gen_store(self, depth):
        dsts = [h + ".p" for h in self.holders] + [h + ".s" for h in self.holders] + \
               ["%s[%d]" % (a, i) for a in self.arrays for i in (0, 1)]
        if not dsts:
            return None
        dst = self.rng.choice(dsts)
        src = self.pick_cell(self.kind_of(dst))
        if src is None or "." in src or "[" in src:
            return None
        self.arena_of[dst] = self.arena_of[src]
        if src in self.valid:
            self.valid[dst] = self.kind_of(dst)
        else:
            self.valid.pop(dst, None)
        return Store(dst, src)

    def gen_if(self, depth):
        if not self.conds:
            return None
        before = dict(self.valid)
        then = self.block(self.rng.randint(1, 3), depth=depth + 1)
        after_then = self.valid
        self.valid = dict(before)
        els = self.block(self.rng.randint(0, 2), depth=depth + 1)
        self.valid = {c: k for c, k in self.valid.items() if c in after_then}
        return If(self.rng.choice(self.conds), then, els)

    def gen_while(self, depth):
        before = dict(self.valid)
        body = self.block(self.rng.randint(1, 3), depth=depth + 1)
        self.valid = {c: k for c, k in self.valid.items() if c in before}
        return While(self.name("i"), self.rng.randint(1, 3), body)


def find_uses(body, trace):
    """(block, index) of every Use that the real run executes."""
    out = []

    def visit(block):
        for i, s in enumerate(block):
            if isinstance(s, Use) and id(s) in trace:
                out.append((block, i))
            for b in s.children():
                visit(b)
    visit(body)
    return out


def generate(seed, phase):
    """(body, label, expected stdout) for this seed. Half the SAFE programs get a reset or
    deinit inserted just before a use that runs, which makes them UNSAFE."""
    rng = random.Random(seed)
    body = Gen(rng, phase).program()
    trace = {}
    lab, out = label(body, trace)
    if lab == SAFE and rng.random() < 0.5:
        uses = find_uses(body, trace)
        if uses:
            block, i = rng.choice(uses)
            a = trace[id(block[i])]
            block.insert(i, Deinit(a) if rng.random() < 0.2 else Reset(a))
            lab, out = label(body)
    return body, lab, out
