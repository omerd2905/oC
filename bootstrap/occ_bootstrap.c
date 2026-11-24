#pragma GCC diagnostic ignored "-Wmain"
#pragma GCC diagnostic ignored "-Wdiscarded-qualifiers"
#pragma GCC diagnostic ignored "-Wpointer-sign"
#pragma GCC diagnostic ignored "-Wincompatible-pointer-types"
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#ifdef _WIN32
#include <io.h>
__declspec(dllimport) void* __stdcall VirtualAlloc(void*, size_t, unsigned long, unsigned long);
__declspec(dllimport) int __stdcall VirtualFree(void*, size_t, unsigned long);
__declspec(dllimport) int __stdcall VirtualProtect(void*, size_t, unsigned long, unsigned long*);
#define O_CLOEXEC_HOST _O_NOINHERIT
#else
#include <unistd.h>
#include <sys/mman.h>
#include <sys/socket.h>
#ifndef MAP_ANONYMOUS
#define MAP_ANONYMOUS MAP_ANON
#endif
#define O_CLOEXEC_HOST O_CLOEXEC
#endif
#ifndef O_NOFOLLOW
#define O_NOFOLLOW 0
#endif
#ifndef O_BINARY
#define O_BINARY 0
#endif
#ifdef htons
#undef htons
#endif
#ifdef ntohs
#undef ntohs
#endif
#ifdef htonl
#undef htonl
#endif
#ifdef ntohl
#undef ntohl
#endif

static inline int __oc_open_flags(int32_t f) { int r = O_BINARY; int acc = f & 3; if (acc == 1) r |= O_WRONLY; else if (acc == 2) r |= O_RDWR; else r |= O_RDONLY; if (f & 64) r |= O_CREAT; if (f & 512) r |= O_TRUNC; if (f & 1024) r |= O_APPEND; if (f & 131072) r |= O_NOFOLLOW; if (f & 524288) r |= O_CLOEXEC_HOST; return r; }
#ifdef _WIN32
static inline int64_t sys_read(int32_t fd, char* buf, size_t count) { return (int64_t)_read(fd, buf, (unsigned)count); }
static inline int64_t sys_write(int32_t fd, const char* buf, size_t count) { return (int64_t)_write(fd, buf, (unsigned)count); }
static inline int32_t sys_open(const char* path, int32_t flags, uint32_t mode) { (void)mode; return (int32_t)_open(path, __oc_open_flags(flags), _S_IREAD | _S_IWRITE); }
static inline int32_t sys_close(int32_t fd) { return (int32_t)_close(fd); }
static inline void* sys_mmap(size_t addr, size_t len, int32_t prot, int32_t flags, int32_t fd, size_t off) { (void)addr; (void)prot; (void)flags; (void)fd; (void)off; void* p = VirtualAlloc(0, len, 0x3000, 0x04); return p ? p : (void*)-1; }
static inline int32_t sys_munmap(void* addr, size_t len) { (void)len; return VirtualFree(addr, 0, 0x8000) ? 0 : -1; }
static inline int32_t sys_mprotect(void* addr, size_t len, int32_t prot) { unsigned long old; return VirtualProtect(addr, len, prot == 0 ? 0x01 : 0x04, &old) ? 0 : -1; }
static inline int32_t sys_chmod(const char* path, uint32_t mode) { return (int32_t)_chmod(path, _S_IREAD | ((mode & 128) ? _S_IWRITE : 0)); }
static inline int32_t sys_unlink(const char* path) { return (int32_t)_unlink(path); }
static inline int32_t sys_socket(int32_t domain, int32_t type, int32_t protocol) { (void)domain; (void)type; (void)protocol; return -1; }
static inline int32_t sys_connect(int32_t fd, const void* addr, uint32_t len) { (void)fd; (void)addr; (void)len; return -1; }
#else
static inline int64_t sys_read(int32_t fd, char* buf, size_t count) { return (int64_t)read(fd, buf, count); }
static inline int64_t sys_write(int32_t fd, const char* buf, size_t count) { return (int64_t)write(fd, buf, count); }
static inline int32_t sys_open(const char* path, int32_t flags, uint32_t mode) { return (int32_t)open(path, __oc_open_flags(flags), (mode_t)mode); }
static inline int32_t sys_close(int32_t fd) { return (int32_t)close(fd); }
static inline void* sys_mmap(size_t addr, size_t len, int32_t prot, int32_t flags, int32_t fd, size_t off) { int p = 0; if (prot & 1) p |= PROT_READ; if (prot & 2) p |= PROT_WRITE; if (prot & 4) p |= PROT_EXEC; int m = 0; if (flags & 1) m |= MAP_SHARED; if (flags & 2) m |= MAP_PRIVATE; if (flags & 32) m |= MAP_ANONYMOUS; return mmap((void*)addr, len, p, m, fd, (off_t)off); }
static inline int32_t sys_munmap(void* addr, size_t len) { return (int32_t)munmap(addr, len); }
static inline int32_t sys_mprotect(void* addr, size_t len, int32_t prot) { int p = 0; if (prot & 1) p |= PROT_READ; if (prot & 2) p |= PROT_WRITE; return (int32_t)mprotect(addr, len, p); }
static inline int32_t sys_chmod(const char* path, uint32_t mode) { return (int32_t)chmod(path, (mode_t)mode); }
static inline int32_t sys_unlink(const char* path) { return (int32_t)unlink(path); }
static inline int32_t sys_socket(int32_t domain, int32_t type, int32_t protocol) { return (int32_t)socket(domain, type, protocol); }
#if defined(__APPLE__) || defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__) || defined(__DragonFly__)
static inline int32_t sys_connect(int32_t fd, const void* addr, uint32_t len) { uint8_t sa[128]; if (len > sizeof sa) return -1; memcpy(sa, addr, len); sa[1] = sa[0]; sa[0] = (uint8_t)len; return (int32_t)connect(fd, (const struct sockaddr*)sa, (socklen_t)len); }
#else
static inline int32_t sys_connect(int32_t fd, const void* addr, uint32_t len) { return (int32_t)connect(fd, (const struct sockaddr*)addr, (socklen_t)len); }
#endif
#endif
#ifdef _WIN32
#include <direct.h>
static inline int32_t sys_mkdtemp(char* tmpl) { return (_mktemp(tmpl) != 0 && _mkdir(tmpl) == 0) ? 0 : -1; }
static inline int32_t sys_rmdir(const char* path) { return (int32_t)_rmdir(path); }
#else
static inline int32_t sys_mkdtemp(char* tmpl) { return mkdtemp(tmpl) ? 0 : -1; }
static inline int32_t sys_rmdir(const char* path) { return (int32_t)rmdir(path); }
#endif
#if defined(_WIN32)
__declspec(dllimport) unsigned long __stdcall GetModuleFileNameA(void*, char*, unsigned long);
static inline int64_t sys_exe_path(char* buf, size_t cap) { unsigned long n = GetModuleFileNameA(0, buf, (unsigned long)cap); return (n == 0 || n >= cap) ? -1 : (int64_t)n; }
#elif defined(__APPLE__)
#include <limits.h>
#include <mach-o/dyld.h>
static inline int64_t sys_exe_path(char* buf, size_t cap) { char raw[PATH_MAX]; char real[PATH_MAX]; uint32_t sz = sizeof raw; if (_NSGetExecutablePath(raw, &sz) != 0 || !realpath(raw, real)) return -1; size_t n = strlen(real); if (n >= cap) return -1; memcpy(buf, real, n + 1); return (int64_t)n; }
#else
static inline int64_t sys_exe_path(char* buf, size_t cap) { ssize_t n = cap ? readlink("/proc/self/exe", buf, cap - 1) : -1; if (n <= 0) return -1; buf[n] = 0; return (int64_t)n; }
#endif
static inline uint64_t sys_f64_bits(const char* text) { double d = strtod(text, 0); uint64_t u; memcpy(&u, &d, sizeof u); return u; }
static inline uint64_t sys_f32_bits(const char* text) { float f = strtof(text, 0); uint32_t u; memcpy(&u, &f, sizeof u); return u; }
static inline void sys_exit(int32_t code) { _exit(code); }
static inline bool sys_arena_poison(void) { return 0; }
#if defined(_WIN32)
static inline int32_t sys_host_target(void) { return 2; }
#elif defined(__APPLE__) && (defined(__aarch64__) || defined(__arm64__))
static inline int32_t sys_host_target(void) { return 4; }
#elif defined(__APPLE__) && defined(__x86_64__)
static inline int32_t sys_host_target(void) { return 3; }
#elif defined(__linux__) && defined(__x86_64__)
static inline int32_t sys_host_target(void) { return 1; }
#else
static inline int32_t sys_host_target(void) { return 0; }
#endif

#ifdef _WIN32
#define __OC_STDOUT_BUF _IONBF
#else
#define __OC_STDOUT_BUF _IOLBF
#endif

typedef struct {
    const uint8_t* ptr;
    size_t len;
} oc_string;

static inline void __oc_print_str(const char* s) { printf("%s\n", s); }
static inline void __oc_print_u8_ptr(uint8_t* s) { printf("%s\n", (const char*)s); }
static inline void __oc_print_const_u8_ptr(const uint8_t* s) { printf("%s\n", (const char*)s); }
static inline void __oc_print_oc_string(oc_string s) { printf("%.*s\n", (int)s.len, (const char*)s.ptr); }
static inline void __oc_print_bool(bool b) { printf("%s\n", b ? "true" : "false"); }
static inline void __oc_print_u64(uint64_t u) { printf("%llu\n", (unsigned long long)u); }
static inline void __oc_print_i64(int64_t i) { printf("%lld\n", (long long)i); }

#define __oc_print(x) _Generic((x), \
    char*: __oc_print_str, \
    const char*: __oc_print_str, \
    uint8_t*: __oc_print_u8_ptr, \
    const uint8_t*: __oc_print_const_u8_ptr, \
    oc_string: __oc_print_oc_string, \
    bool: __oc_print_bool, \
    signed char: __oc_print_i64, \
    short: __oc_print_i64, \
    int: __oc_print_i64, \
    long: __oc_print_i64, \
    long long: __oc_print_i64, \
    default: __oc_print_u64 \
)(x)

static inline oc_string __oc_slice_str(oc_string s, size_t start, size_t end) {
    if (start > end || end > s.len) { fprintf(stderr, "panic: slice index out of bounds\n"); exit(1); }
    return (oc_string){ .ptr = s.ptr + start, .len = end - start };
}

static inline uint64_t f64_to_bits(double x) { uint64_t u; memcpy(&u, &x, 8); return u; }
#define __OC_FCHK(x, loc) ({ __auto_type _r = (x); if (!(_r - _r == 0)) { fprintf(stderr, "panic: floating-point result is NaN or infinite at %s\n", loc); exit(1); } _r; })

#define __OC_ARR(a) ((oc_string){ .ptr = (const uint8_t*)(a), .len = __builtin_choose_expr(__builtin_types_compatible_p(__typeof__(a), __typeof__(&(a)[0])), (void)0, sizeof(a)) })

static inline uint8_t __oc_index_str(oc_string s, size_t idx) {
    if (idx >= s.len) { fprintf(stderr, "panic: slice index out of bounds\n"); exit(1); }
    return s.ptr[idx];
}

#define __OC_AT(a, i) (*__builtin_choose_expr(__builtin_types_compatible_p(__typeof__(a), __typeof__(&(a)[0])), &(a)[i], ({ size_t _i = (size_t)(i); if (_i >= (sizeof(a) + 0) / sizeof((a)[0])) { fprintf(stderr, "panic: slice index out of bounds\n"); exit(1); } &(a)[_i]; })))

static inline int64_t __oc_div_check(int64_t a, int64_t b) {
    if (b == 0) { fprintf(stderr, "panic: division by zero\n"); exit(1); }
    return a / b;
}

static inline int64_t __oc_mod_check(int64_t a, int64_t b) {
    if (b == 0) { fprintf(stderr, "panic: division by zero\n"); exit(1); }
    return a % b;
}

#define __oc_notnull(p) ({ __auto_type _p = (p); if (!_p) { fprintf(stderr, "panic: null pointer dereference\n"); exit(1); } _p; })
#define __OC_DIV(a, b) ({ __auto_type _a = (a); __auto_type _b = (b); if (_b == 0) { fprintf(stderr, "panic: division by zero\n"); exit(1); } _a / _b; })
#define __OC_MOD(a, b) ({ __auto_type _a = (a); __auto_type _b = (b); if (_b == 0) { fprintf(stderr, "panic: division by zero\n"); exit(1); } _a % _b; })
#define __OC_ADD(a, b) ({ __auto_type _a = (a); __auto_type _b = (b); __typeof__(__builtin_choose_expr(sizeof(_a) < 4, _a, __builtin_choose_expr(sizeof(_b) < 4, _b, _a + _b))) _r; if (__builtin_add_overflow(_a, _b, &_r)) { fprintf(stderr, "panic: integer overflow\n"); exit(1); } _r; })
#define __OC_SUB(a, b) ({ __auto_type _a = (a); __auto_type _b = (b); __typeof__(__builtin_choose_expr(sizeof(_a) < 4, _a, __builtin_choose_expr(sizeof(_b) < 4, _b, _a - _b))) _r; if (__builtin_sub_overflow(_a, _b, &_r)) { fprintf(stderr, "panic: integer overflow\n"); exit(1); } _r; })
#define __OC_MUL(a, b) ({ __auto_type _a = (a); __auto_type _b = (b); __typeof__(__builtin_choose_expr(sizeof(_a) < 4, _a, __builtin_choose_expr(sizeof(_b) < 4, _b, _a * _b))) _r; if (__builtin_mul_overflow(_a, _b, &_r)) { fprintf(stderr, "panic: integer overflow\n"); exit(1); } _r; })
#define __OC_WRAP(op, a, b) ({ __auto_type _a = (a); __auto_type _b = (b); __typeof__(__builtin_choose_expr(sizeof(_a) < 4, _a, __builtin_choose_expr(sizeof(_b) < 4, _b, _a + _b))) _r; (void)__builtin_##op##_overflow(_a, _b, &_r); _r; })

static const int32_t OC_PROT_READ = 1;
static const int32_t OC_PROT_WRITE = 2;
static const int32_t OC_PROT_RW = 3;
static const int32_t OC_MAP_PRIVATE_ANON = 34;
static const int32_t OC_O_RDONLY = 0;
static const int32_t OC_O_SAFE_READ = 655360;
static const int32_t OC_O_SAFE_WRITE_TRUNC = 655937;
static const uint32_t OC_MODE_FILE = 420;
static const uint32_t OC_MODE_EXEC = 493;
static const int32_t OC_AF_INET = 2;
static const int32_t OC_SOCK_STREAM = 1;
static const int32_t OC_TARGET_OTHER = 0;
static const int32_t OC_TARGET_LINUX_X86_64 = 1;
static const int32_t OC_TARGET_WINDOWS_X86_64 = 2;
static const int32_t OC_TARGET_MACOS_X86_64 = 3;
static const int32_t OC_TARGET_MACOS_AARCH64 = 4;
static const uint32_t TOK_EOF = 0;
static const uint32_t TOK_IDENT = 1;
static const uint32_t TOK_INT = 2;
static const uint32_t TOK_STRING = 3;
static const uint32_t TOK_FLOAT = 4;
static const uint32_t TOK_FN = 10;
static const uint32_t TOK_LET = 11;
static const uint32_t TOK_MUT = 12;
static const uint32_t TOK_RETURN = 13;
static const uint32_t TOK_IF = 14;
static const uint32_t TOK_ELSE = 15;
static const uint32_t TOK_WHILE = 16;
static const uint32_t TOK_STRUCT = 17;
static const uint32_t TOK_PACKED = 18;
static const uint32_t TOK_EXTERN = 19;
static const uint32_t TOK_IMPORT = 20;
static const uint32_t TOK_PRINT = 21;
static const uint32_t TOK_TRUE = 22;
static const uint32_t TOK_FALSE = 23;
static const uint32_t TOK_AS = 24;
static const uint32_t TOK_LPAREN = 30;
static const uint32_t TOK_RPAREN = 31;
static const uint32_t TOK_LBRACE = 32;
static const uint32_t TOK_RBRACE = 33;
static const uint32_t TOK_LBRACKET = 34;
static const uint32_t TOK_RBRACKET = 35;
static const uint32_t TOK_COLON = 36;
static const uint32_t TOK_COMMA = 37;
static const uint32_t TOK_DOT = 38;
static const uint32_t TOK_ARROW = 39;
static const uint32_t TOK_EQUAL = 40;
static const uint32_t TOK_PLUS = 41;
static const uint32_t TOK_MINUS = 42;
static const uint32_t TOK_STAR = 43;
static const uint32_t TOK_SLASH = 44;
static const uint32_t TOK_PERCENT = 45;
static const uint32_t TOK_AMP = 46;
static const uint32_t TOK_BANG = 47;
static const uint32_t TOK_EQEQ = 48;
static const uint32_t TOK_NEQ = 49;
static const uint32_t TOK_LT = 50;
static const uint32_t TOK_LTE = 51;
static const uint32_t TOK_GT = 52;
static const uint32_t TOK_GTE = 53;
static const uint32_t TOK_SEMICOLON = 54;
static const uint32_t TOK_DOTDOT = 55;
static const uint32_t TOK_PIPE = 56;
static const uint32_t TOK_CARET = 57;
static const uint32_t TOK_SHL = 58;
static const uint32_t TOK_SHR = 59;
static const uint32_t TOK_BREAK = 25;
static const uint32_t TOK_CONTINUE = 26;
static const uint32_t TOK_TILDE = 85;
static const uint32_t TOK_ENUM = 86;
static const uint32_t TOK_MATCH = 87;
static const uint32_t TOK_TRY = 88;
static const uint32_t TOK_CATCH = 89;
static const uint32_t TOK_FATARROW = 90;
static const uint32_t TOK_ERROR = 99;
static const uint32_t NODE_PROGRAM = 1;
static const uint32_t NODE_FN = 2;
static const uint32_t NODE_PARAM = 3;
static const uint32_t NODE_LET = 4;
static const uint32_t NODE_RETURN = 5;
static const uint32_t NODE_IF = 6;
static const uint32_t NODE_WHILE = 7;
static const uint32_t NODE_PRINT = 8;
static const uint32_t NODE_ASSIGN = 9;
static const uint32_t NODE_CALL = 10;
static const uint32_t NODE_BINARY = 11;
static const uint32_t NODE_IDENT = 12;
static const uint32_t NODE_INT = 13;
static const uint32_t NODE_STRING = 14;
static const uint32_t NODE_BLOCK = 15;
static const uint32_t NODE_BOOL = 16;
static const uint32_t NODE_ADDRESS_OF = 17;
static const uint32_t NODE_DEREF = 18;
static const uint32_t NODE_CAST = 19;
static const uint32_t NODE_INDEX = 20;
static const uint32_t NODE_SLICE = 21;
static const uint32_t NODE_FIELD_ACCESS = 22;
static const uint32_t NODE_STRUCT_INIT = 23;
static const uint32_t NODE_ARRAY_INIT = 24;
static const uint32_t NODE_STRUCT_DECL = 25;
static const uint32_t NODE_UNARY = 26;
static const uint32_t NODE_FLOAT = 29;
static const uint32_t NODE_GENERIC = 30;
static const uint32_t NODE_ENUM = 31;
static const uint32_t NODE_MATCH = 32;
static const uint32_t NODE_ARM = 33;
static const uint32_t NODE_TRY = 34;
static const uint32_t NODE_CATCH = 35;
static const uint32_t NODE_BREAK = 27;
static const uint32_t NODE_CONTINUE = 28;
static const size_t NG_IMP_EXIT_PROCESS = 0;
static const size_t NG_IMP_GET_STD_HANDLE = 1;
static const size_t NG_IMP_WRITE_FILE = 2;
static const size_t NG_IMP_COUNT = 10;

typedef struct Arena Arena;
typedef struct Split Split;
typedef struct StrBuf StrBuf;
typedef struct ParsedU64 ParsedU64;
typedef struct ParsedI64 ParsedI64;
typedef struct ReadLine ReadLine;
typedef struct ReadAll ReadAll;
typedef struct Token Token;
typedef struct Lexer Lexer;
typedef struct AstNode AstNode;
typedef struct SourceMap SourceMap;
typedef struct Parser Parser;
typedef struct VarInfo VarInfo;
typedef struct FnEffects FnEffects;
typedef struct Validator Validator;
typedef struct Generics Generics;
typedef struct GArgs GArgs;
typedef struct GBind GBind;
typedef struct Errs Errs;
typedef struct CodeBuffer CodeBuffer;
typedef struct LitByte LitByte;
typedef struct NativeGen NativeGen;
typedef struct ModuleLoader ModuleLoader;
typedef struct __ErrorUnion__u64__u32 __ErrorUnion__u64__u32;
typedef struct __ErrorUnion__i64__u32 __ErrorUnion__i64__u32;
typedef struct __ErrorUnion__string__u32 __ErrorUnion__string__u32;
typedef struct __ErrorUnion__usize__u32 __ErrorUnion__usize__u32;

struct Arena {
    size_t offset;
    size_t capacity;
    uint8_t* buffer;
};

struct Split {
    oc_string head;
    oc_string tail;
};

struct StrBuf {
    Arena* arena;
    uint8_t* buf;
    size_t len;
    size_t cap;
};

struct ParsedU64 {
    bool ok;
    uint64_t value;
};

struct ParsedI64 {
    bool ok;
    int64_t value;
};

struct ReadLine {
    bool ok;
    oc_string line;
};

struct ReadAll {
    bool ok;
    oc_string data;
};

struct Token {
    uint32_t tag;
    oc_string slice;
    uint32_t line;
    uint32_t col;
};

struct Lexer {
    oc_string source;
    size_t index;
    uint32_t line;
    uint32_t col;
};

struct AstNode {
    uint32_t tag;
    Token tok;
    oc_string type_name;
    AstNode* left;
    AstNode* right;
    AstNode* next;
};

struct SourceMap {
    size_t count;
    uint32_t first_line[128];
    oc_string path[128];
};

struct Parser {
    Lexer lex;
    Token current;
    Arena* arena;
    bool allow_struct_init;
    bool has_error;
    int32_t loop_depth;
    SourceMap* srcmap;
};

struct VarInfo {
    oc_string name;
    oc_string type_name;
    uint32_t scope_depth;
    bool is_stack_local;
    bool points_to_stack_local;
    oc_string borrowed_from_arena;
    oc_string borrowed_from_var;
    bool is_invalidated;
    bool is_mut;
};

struct FnEffects {
    oc_string names[256];
    uint8_t effects[4096];
    bool borrows[4096];
    size_t count;
};

struct Validator {
    FnEffects* fx;
    AstNode* program;
    oc_string ret_type;
    VarInfo vars[512];
    size_t vars_count;
    oc_string deinit_arenas[128];
    size_t deinit_count;
    uint32_t scope_depth;
    bool has_error;
    oc_string initializing_var;
    SourceMap* srcmap;
    oc_string fn_name;
    Generics* gen;
};

struct Generics {
    AstNode* tmpl[256];
    size_t tmpl_count;
    oc_string inst_name[1024];
    oc_string inst_tmpl[1024];
    oc_string inst_args[1024];
    Token inst_site[1024];
    size_t inst_count;
    Arena* arena;
    SourceMap* m;
    AstNode* root;
    AstNode* last;
    bool ok;
};

struct GArgs {
    size_t n;
    oc_string items[8];
};

struct GBind {
    size_t n;
    oc_string b_name[8];
    oc_string b_val[8];
    oc_string conflict;
};

struct Errs {
    AstNode* enums[64];
    size_t enum_count;
    Arena* arena;
    SourceMap* m;
    AstNode* root;
    bool ok;
    uint32_t tmp_n;
    oc_string fn_ret;
    oc_string syms[512];
    oc_string sym_types[512];
    size_t sym_count;
    AstNode* pre;
};

struct CodeBuffer {
    uint8_t* buffer;
    size_t len;
    size_t capacity;
    oc_string ptr_vars[512];
    size_t ptr_vars_count;
    oc_string str_vars[512];
    size_t str_vars_count;
    oc_string ptr_fields[256];
    size_t ptr_fields_count;
    oc_string str_fields[256];
    size_t str_fields_count;
    oc_string ptr_fns[256];
    size_t ptr_fns_count;
    oc_string str_fns[256];
    size_t str_fns_count;
    oc_string var_names[512];
    oc_string var_types[512];
    size_t var_types_len;
    oc_string flt_names[256];
    oc_string flt_types[256];
    size_t flt_count;
    bool fp_trap;
    bool arena_poison;
    SourceMap* srcmap;
    AstNode* program;
};

struct LitByte {
    uint8_t b;
    size_t next;
};

struct NativeGen {
    oc_string fn_names[1024];
    size_t fn_offsets[1024];
    oc_string fn_ret_types[1024];
    size_t fn_count;
    size_t fixup_offsets[1024];
    oc_string fixup_names[1024];
    bool fixup_is_6byte[1024];
    size_t fixup_count;
    size_t data_reloc_code[4096];
    size_t data_reloc_data[4096];
    size_t data_reloc_count;
    oc_string var_names[256];
    int32_t var_offsets[256];
    bool var_is_slice[256];
    oc_string var_types[256];
    bool var_active[256];
    int32_t var_scope[256];
    size_t var_arr_len[256];
    size_t var_count;
    int32_t curr_off;
    int32_t scope_depth;
    oc_string struct_names[256];
    int32_t struct_sizes[256];
    size_t struct_count;
    oc_string field_struct[1024];
    oc_string field_names[1024];
    int32_t field_offsets[1024];
    oc_string field_types[1024];
    bool field_packed[1024];
    size_t field_count;
    AstNode* root;
    bool fp_trap;
    bool arena_poison;
    SourceMap* srcmap;
    oc_string cur_ret;
    oc_string const_types[128];
    oc_string const_names[128];
    uint64_t const_vals[128];
    size_t const_count;
    size_t loop_start;
    int32_t loop_depth;
    size_t break_patches[128];
    int32_t break_depths[128];
    size_t break_count;
    int32_t target;
    bool has_error;
    size_t iat_fix_code[64];
    size_t iat_fix_slot[64];
    size_t iat_fix_count;
};

struct ModuleLoader {
    oc_string visited[128];
    size_t visited_count;
    SourceMap map;
    uint32_t lines;
};

struct __ErrorUnion__u64__u32 {
    uint64_t code;
    uint64_t value;
};

struct __ErrorUnion__i64__u32 {
    uint64_t code;
    int64_t value;
};

struct __ErrorUnion__string__u32 {
    uint64_t code;
    oc_string value;
};

struct __ErrorUnion__usize__u32 {
    uint64_t code;
    size_t value;
};

void arena_poison_region(Arena* arena);
Arena arena_init(size_t capacity);
uint8_t* arena_alloc(Arena* arena, size_t size);
void arena_reset(Arena* arena);
void arena_deinit(Arena* arena);
bool str_eq(oc_string a, oc_string b);
bool starts_with(oc_string s, oc_string prefix);
bool ends_with(oc_string s, oc_string suffix);
int64_t find(oc_string s, oc_string needle);
bool contains(oc_string s, oc_string needle);
oc_string trim_start(oc_string s);
oc_string trim_end(oc_string s);
oc_string trim(oc_string s);
bool split_once(oc_string s, oc_string sep, Split* out);
oc_string substr(oc_string s, size_t start, size_t len);
oc_string cstr_to_string(uint8_t* cstr);
bool is_digit(uint8_t c);
bool is_upper(uint8_t c);
bool is_lower(uint8_t c);
bool is_alpha(uint8_t c);
bool is_space(uint8_t c);
uint8_t to_upper(uint8_t c);
uint8_t to_lower(uint8_t c);
uint8_t* str_alloc(Arena* arena, size_t n);
uint8_t* str_grow(Arena* arena, uint8_t* buf, size_t len, size_t cap);
oc_string str_concat(Arena* arena, oc_string a, oc_string b);
oc_string str_repeat(Arena* arena, oc_string s, size_t n);
oc_string str_join(Arena* arena, oc_string* parts, size_t count, oc_string sep);
oc_string str_replace(Arena* arena, oc_string s, oc_string from, oc_string to);
oc_string str_to_upper(Arena* arena, oc_string s);
oc_string str_to_lower(Arena* arena, oc_string s);
StrBuf strbuf_init(Arena* arena, size_t cap);
void strbuf_push(StrBuf* b, uint8_t c);
void strbuf_push_str(StrBuf* b, oc_string s);
oc_string strbuf_to_string(StrBuf* b);
uint64_t digit_value(uint8_t c);
void num_check_radix(uint32_t radix);
__ErrorUnion__u64__u32 num_digits(oc_string s, uint64_t radix);
__ErrorUnion__u64__u32 str_to_u64_radix(oc_string s, uint32_t radix);
__ErrorUnion__i64__u32 str_to_i64_radix(oc_string s, uint32_t radix);
__ErrorUnion__u64__u32 str_to_u64(oc_string s);
__ErrorUnion__i64__u32 str_to_i64(oc_string s);
ParsedU64 parsed_u64(__ErrorUnion__u64__u32 r);
ParsedI64 parsed_i64(__ErrorUnion__i64__u32 r);
bool radix_ok(uint64_t radix);
ParsedU64 parse_digits_u64(oc_string s, uint64_t radix);
ParsedU64 parse_u64_radix(oc_string s, uint32_t radix);
ParsedI64 parse_i64_radix(oc_string s, uint32_t radix);
ParsedU64 parse_u64(oc_string s);
ParsedI64 parse_i64(oc_string s);
size_t fmt_digits_back(uint8_t* buf, size_t end, uint64_t v, uint64_t radix);
oc_string int_text(Arena* arena, bool neg, uint64_t mag, uint64_t radix);
uint64_t i64_magnitude(int64_t n);
oc_string u64_to_str(Arena* arena, uint64_t n);
oc_string i64_to_str(Arena* arena, int64_t n);
oc_string u64_to_hex(Arena* arena, uint64_t n);
void print_str(oc_string s);
void print_int_text(bool neg, uint64_t mag, uint64_t radix);
void print_u64(uint64_t n);
void print_i64(int64_t n);
void print_hex(uint64_t n);
__ErrorUnion__string__u32 stdin_line(Arena* arena);
ReadLine read_line(Arena* arena);
__ErrorUnion__string__u32 stdin_all(Arena* arena);
ReadAll read_all(Arena* arena);
__ErrorUnion__i64__u32 stdin_i64(void);
ParsedI64 read_i64(void);
int32_t read_int(void);
uint8_t* cstr_path(oc_string path, uint8_t* out_buf);
__ErrorUnion__usize__u32 file_read(oc_string path, uint8_t* out_buf, size_t max_len);
__ErrorUnion__usize__u32 file_write(oc_string path, oc_string content);
int64_t read_file(oc_string path, uint8_t* out_buf, size_t max_len);
bool write_file(oc_string path, oc_string content);
bool chmod_exec(oc_string path);
bool unlink_file(oc_string path);
bool is_keyword(uint32_t tag);
Lexer lexer_init(oc_string source);
bool is_hex_digit(uint8_t c);
bool is_ident_start(uint8_t c);
bool is_ident_char(uint8_t c);
uint32_t check_keyword(oc_string s);
Token lexer_next(Lexer* lex);
int32_t srcmap_file(SourceMap* m, uint32_t line);
AstNode* ast_alloc_node(Arena* arena, uint32_t tag, Token tok);
void parser_report_lex_error(Parser* p, Token tok);
void parser_error(Parser* p, Token tok, oc_string msg);
Parser parser_init(oc_string source, Arena* arena);
void parser_advance(Parser* p);
Token parser_expect(Parser* p, uint32_t expected_tag);
uint32_t get_precedence(uint32_t tag);
oc_string parse_type(Parser* p);
AstNode* parse_expr(Parser* p, uint32_t min_prec);
AstNode* parse_generic_params(Parser* p);
oc_string parser_try_type_args(Parser* p);
AstNode* parse_primary(Parser* p);
AstNode* parse_postfix(Parser* p);
AstNode* parse_unary(Parser* p);
AstNode* parse_expr(Parser* p, uint32_t min_prec);
AstNode* parse_stmt(Parser* p);
AstNode* parse_enum_decl(Parser* p);
AstNode* parse_match_stmt(Parser* p);
AstNode* parse_block(Parser* p);
AstNode* parse_print_stmt(Parser* p);
AstNode* parse_let_stmt(Parser* p);
AstNode* parse_return_stmt(Parser* p);
AstNode* parse_if_stmt(Parser* p);
AstNode* parse_while_stmt(Parser* p);
AstNode* parse_stmt(Parser* p);
AstNode* parse_struct_decl(Parser* p, bool is_packed);
AstNode* parse_fn_decl(Parser* p);
void print_str_raw(oc_string s);
void print_u32_raw(uint32_t n);
void print_src_pos(SourceMap* m, Token tok);
void print_src_loc(SourceMap* m, Token tok);
void print_err_ref_invalidated(oc_string name, oc_string bv);
void print_err_uaf_reset(oc_string name, oc_string arena);
void print_err_uaf_deinit(oc_string name, oc_string arena);
void print_err_borrow_escape(oc_string src, oc_string tgt);
void print_err_cross_arena(oc_string arena, oc_string tgt);
void print_err_ret_local(oc_string src);
void print_err_ret_arena(oc_string arena);
bool is_scalar_value_type(oc_string t);
int32_t validator_find_var(Validator* v, oc_string name);
void validator_set_points_to_stack_local(Validator* v, oc_string name);
bool validator_is_arena_deinit(Validator* v, oc_string name);
bool validator_mark_deinit(Validator* v, oc_string name);
void validator_invalidate_arena(Validator* v, oc_string arena_name);
void validator_invalidate_var(Validator* v, oc_string var_name);
oc_string get_ident_name(AstNode* expr);
oc_string get_underlying_address_of(AstNode* expr);
uint8_t fx_param_effect(FnEffects* fx, oc_string fn_name, size_t idx);
bool fx_param_borrows(FnEffects* fx, oc_string fn_name, size_t idx);
oc_string get_arena_arg_name(AstNode* arg);
bool fx_scan(FnEffects* fx, size_t slot, AstNode* params, bool ret_ptr, AstNode* n);
oc_string get_arena_source(FnEffects* fx, AstNode* expr);
oc_string get_expr_arena_borrow(Validator* v, AstNode* expr);
oc_string validator_field_type(Validator* v, oc_string s_name, oc_string f_name);
oc_string validator_stack_place_type(Validator* v, AstNode* expr);
bool is_float_type_name(oc_string t);
bool is_int_type_name(oc_string t);
oc_string strip_ptr_type(oc_string t);
AstNode* program_find_fn(AstNode* program, oc_string name);
AstNode* validator_find_fn(Validator* v, oc_string name);
bool is_builtin_fn(oc_string name);
bool is_hex_char(uint8_t c);
bool check_string_escapes(SourceMap* m, AstNode* node);
bool check_unique_names(AstNode* root, SourceMap* m);
bool check_calls_resolved(AstNode* program, SourceMap* m, AstNode* node);
oc_string validator_expr_type(Validator* v, AstNode* expr);
bool is_int_literal(AstNode* expr);
bool is_untyped_float(AstNode* expr);
void bind_float_literals(AstNode* expr, oc_string t);
void bind_to_target(oc_string target, AstNode* value);
bool is_scalar_load(Validator* v, AstNode* expr);
bool validator_is_struct_type(Validator* v, oc_string t);
void validator_print_instance(Validator* v);
bool validator_check_operands(Validator* v, AstNode* expr);
bool validator_is_float(Validator* v, AstNode* expr);
bool validator_check_no_float_to_int(Validator* v, oc_string target, AstNode* value, oc_string place);
bool int_converts_exactly(oc_string int_t, oc_string float_t);
bool int_literal_converts_exactly(oc_string text, uint64_t bits);
bool validator_check_lossless_to_float(Validator* v, oc_string target, AstNode* value, oc_string place);
oc_string array_elem_type(oc_string t);
bool validator_check_conversion(Validator* v, oc_string target, AstNode* value, oc_string place);
oc_string get_local_array_slice_source(Validator* v, AstNode* expr);
oc_string contains_stack_ref(Validator* v, AstNode* expr);
bool check_expr_use(Validator* v, AstNode* expr);
void validator_save_invalidated(Validator* v, bool* out);
void validator_restore_invalidated(Validator* v, bool* saved);
bool block_always_returns(AstNode* block);
bool validate_stmt(Validator* v, AstNode* stmt);
bool validate_function(AstNode* f, FnEffects* fx, AstNode* program, SourceMap* m, Generics* g);
bool validate_program(AstNode* root, SourceMap* m, Generics* g);
AstNode* parse_program(Parser* p);
oc_string gx_done(StrBuf sb);
void gx_err(Generics* g, Token tok, oc_string msg, oc_string name);
bool gx_is_ident_char(uint8_t c);
oc_string gx_canon(Generics* g, oc_string t);
GArgs gx_split(Generics* g, oc_string text);
void gx_mangle_into(StrBuf* sb, oc_string t);
oc_string gx_mangled(Generics* g, oc_string name, GArgs args);
oc_string gx_join(Generics* g, GArgs args);
AstNode* gx_template(Generics* g, oc_string name);
size_t gx_count(AstNode* list);
oc_string gx_subst(Generics* g, oc_string t, AstNode* params, GArgs args);
AstNode* gx_clone(Generics* g, AstNode* n);
void gx_fill(Generics* g, AstNode* n, AstNode* params, GArgs args);
void gx_append(Generics* g, AstNode* d);
oc_string gx_instantiate(Generics* g, AstNode* t, GArgs args, Token site);
oc_string gx_resolve(Generics* g, oc_string t, Token site);
int32_t gx_param_index(AstNode* params, oc_string word);
bool gx_bind(GBind* b, oc_string name, oc_string val);
bool gx_unify(Generics* g, AstNode* params, oc_string pat, oc_string con, GBind* b);
oc_string gx_arg_type(Generics* g, Validator* v, AstNode* arg);
GArgs gx_infer(Generics* g, Validator* v, AstNode* t, AstNode* call);
void gx_add_var(Validator* v, oc_string name, oc_string t);
void gx_walk(Generics* g, Validator* v, AstNode* n);
bool expand_generics(AstNode* root, Arena* arena, SourceMap* m, Generics* g);
void dz_err(Errs* e, Token tok, oc_string msg, oc_string name, oc_string tail);
AstNode* dz_enum(Errs* e, oc_string name);
int32_t dz_variant(AstNode* en, oc_string name);
int64_t dz_bang(oc_string t);
oc_string dz_ok_type(oc_string t);
oc_string dz_err_type(oc_string t);
void dz_sym_add(Errs* e, oc_string name, oc_string t);
oc_string dz_sym(Errs* e, oc_string name);
oc_string dz_type_of(Errs* e, AstNode* x);
Token dz_tok(Token at, uint32_t tag, oc_string text);
AstNode* dz_node(Errs* e, uint32_t tag, Token at, oc_string text);
AstNode* dz_ident(Errs* e, oc_string name, Token at);
AstNode* dz_field(Errs* e, AstNode* obj, oc_string field, Token at);
AstNode* dz_bin(Errs* e, uint32_t tag, oc_string op, AstNode* l, AstNode* r, Token at);
AstNode* dz_int(Errs* e, oc_string text, Token at);
AstNode* dz_let(Errs* e, oc_string name, oc_string t, AstNode* init, bool mutable, Token at);
AstNode* dz_assign(Errs* e, AstNode* lhs, AstNode* rhs, Token at);
AstNode* dz_block(Errs* e, AstNode* stmts, Token at);
AstNode* dz_if(Errs* e, AstNode* cond, AstNode* then_stmts, AstNode* else_node, Token at);
oc_string dz_tmp(Errs* e);
AstNode* dz_union_lit(Errs* e, oc_string ut, AstNode* code, AstNode* value, Token at);
void dz_expr(Errs* e, AstNode* x);
AstNode* dz_stmts(Errs* e, AstNode* first);
AstNode* dz_stmt(Errs* e, AstNode* s);
void dz_try(Errs* e, AstNode* t);
void dz_catch(Errs* e, AstNode* c);
void dz_expect(Errs* e, AstNode* x, oc_string expected);
void dz_expr(Errs* e, AstNode* x);
AstNode* dz_arm_body(Errs* e, AstNode* body);
AstNode* dz_append(AstNode* first, AstNode* more);
AstNode* dz_match(Errs* e, AstNode* s);
void dz_try(Errs* e, AstNode* t);
void dz_catch(Errs* e, AstNode* c);
AstNode* dz_stmt_in(Errs* e, AstNode* s);
AstNode* dz_stmt(Errs* e, AstNode* s);
AstNode* dz_stmts(Errs* e, AstNode* first);
oc_string dz_type(Errs* e, oc_string t);
void dz_types(Errs* e, AstNode* n);
void dz_fn(Errs* e, AstNode* f);
bool desugar_errors(AstNode* root, Arena* arena, SourceMap* m);
CodeBuffer buf_init(size_t capacity);
void buf_append_u8(CodeBuffer* buf, uint8_t byte);
void buf_append(CodeBuffer* buf, oc_string text);
void buf_append_u32(CodeBuffer* buf, uint32_t v);
void buf_append_src_loc(CodeBuffer* buf, SourceMap* m, Token tok, bool c_escape);
void cg_fchk_close(CodeBuffer* out, Token tok);
bool is_c_reserved(oc_string name);
void buf_append_ident(CodeBuffer* buf, oc_string name);
uint8_t hex_digit_value(uint8_t c);
LitByte lit_next_byte(oc_string raw, size_t k);
void buf_append_c_literal(CodeBuffer* out, oc_string tok_text);
oc_string buf_to_string(CodeBuffer* buf);
void buf_deinit(CodeBuffer* buf);
oc_string to_c_type(oc_string type_name);
void emit_c_type(oc_string type_name, CodeBuffer* out);
void buf_append_c_type(CodeBuffer* out, oc_string t);
bool is_type_ptr(oc_string t);
bool is_type_str(oc_string t);
oc_string cg_var_type(CodeBuffer* out, oc_string name);
oc_string cg_expr_type(AstNode* expr, CodeBuffer* out);
bool is_expr_string(AstNode* expr, CodeBuffer* out);
bool is_expr_pointer(AstNode* expr, CodeBuffer* out);
bool is_float_type(oc_string t);
void cg_flt_add(CodeBuffer* out, oc_string name, oc_string t);
oc_string cg_flt_lookup(CodeBuffer* out, oc_string name);
oc_string cg_elem_type(oc_string t);
oc_string cg_float_type(AstNode* expr, CodeBuffer* out);
oc_string float_cast_lo(oc_string t);
oc_string float_cast_hi(oc_string t);
oc_string wrapping_op(oc_string fn_name);
void codegen_expr(AstNode* expr, CodeBuffer* out);
bool is_comparison_tok(uint32_t tag);
bool is_bool_expr(AstNode* expr);
oc_string cg_var_type(CodeBuffer* out, oc_string name);
bool is_u8_array_type(oc_string t);
void codegen_print(AstNode* node, CodeBuffer* out);
void codegen_stmt(AstNode* stmt, CodeBuffer* out);
bool is_builtin_c_fn(oc_string fn_name);
void codegen_program(AstNode* root, CodeBuffer* out);
void elf_append_u16(CodeBuffer* buf, uint32_t v);
void elf_append_u32(CodeBuffer* buf, uint32_t v);
void elf_append_u64(CodeBuffer* buf, uint64_t v);
void elf_patch_u32(CodeBuffer* buf, size_t offset, uint32_t v);
void elf_patch_u64(CodeBuffer* buf, size_t offset, uint64_t v);
void elf_append_i32(CodeBuffer* buf, int32_t v);
void elf_patch_i32(CodeBuffer* buf, size_t offset, int32_t v);
oc_string ng_imp_name(size_t slot);
int32_t ng_imp_slot(oc_string name);
void ng_fail(NativeGen* ng, oc_string msg);
void ng_add_fixup(NativeGen* ng, size_t code_off, oc_string fn_name, bool is_6byte);
void ng_add_data_reloc(NativeGen* ng, size_t code_off, size_t data_off);
void ng_add_iat_fixup(NativeGen* ng, size_t code_off, size_t slot);
void ng_emit_call(NativeGen* ng, CodeBuffer* code, oc_string fn_name);
void ng_emit_iat_call(NativeGen* ng, CodeBuffer* code, size_t slot);
uint64_t parse_literal_u64(oc_string s);
int32_t ng_find_var_idx(NativeGen* ng, oc_string name);
int32_t ng_find_var_offset(NativeGen* ng, oc_string name);
oc_string ng_find_var_type(NativeGen* ng, oc_string name);
bool ng_is_var_slice(NativeGen* ng, oc_string name);
int32_t ng_struct_raw_size(NativeGen* ng, oc_string name);
int32_t ng_find_struct_size(NativeGen* ng, oc_string name);
int32_t ng_find_field_idx(NativeGen* ng, oc_string s_name, oc_string f_name);
int32_t ng_find_field_offset(NativeGen* ng, oc_string s_name, oc_string f_name);
oc_string ng_find_field_type(NativeGen* ng, oc_string s_name, oc_string f_name);
int32_t ng_scalar_width(oc_string t);
int32_t ng_field_width(NativeGen* ng, oc_string s_name, oc_string f_name);
oc_string ng_find_fn_ret_type(NativeGen* ng, oc_string fn_name);
oc_string ng_strip_ptr_type(oc_string t);
size_t ng_parse_array_len(oc_string t);
oc_string ng_parse_array_elem(oc_string t);
int32_t ng_type_stride(oc_string t);
bool ng_is_byte_type(oc_string t);
bool is_unsigned_type_name(oc_string t);
oc_string ng_float_result_type(oc_string lt, oc_string rt);
int32_t ng_int_class(oc_string t);
oc_string ng_expr_type(NativeGen* ng, AstNode* expr);
bool ng_is_unsigned_expr(NativeGen* ng, AstNode* expr);
bool ng_binary_unsigned(NativeGen* ng, AstNode* left, AstNode* right);
bool ng_is_expr_slice(NativeGen* ng, AstNode* expr);
void ng_emit_disp_store_rax(CodeBuffer* code, int32_t off);
void ng_emit_disp_store_rdx(CodeBuffer* code, int32_t off);
void ng_emit_disp_store_rcx(CodeBuffer* code, int32_t off);
void ng_emit_disp_store_r8(CodeBuffer* code, int32_t off);
void ng_emit_disp_load_rax(CodeBuffer* code, int32_t off);
void ng_emit_disp_load_rdx(CodeBuffer* code, int32_t off);
void ng_emit_disp_load_rcx(CodeBuffer* code, int32_t off);
void ng_emit_disp_load_r8(CodeBuffer* code, int32_t off);
void ng_emit_disp_lea_rax(CodeBuffer* code, int32_t off);
size_t ng_append_unescaped_str(CodeBuffer* data, oc_string raw);
void ng_emit_range_check(NativeGen* ng, CodeBuffer* code, oc_string t, oc_string trap);
void ng_emit_load_at_rax(CodeBuffer* code, oc_string t, int32_t width);
int32_t ng_value_words(NativeGen* ng, oc_string t);
bool ng_emit_load_struct_at_rax(NativeGen* ng, CodeBuffer* code, oc_string s_name);
void ng_emit_truncate(CodeBuffer* code, oc_string t);
void ng_emit_store_at_rcx(CodeBuffer* code, int32_t width);
uint64_t ng_literal_bits(NativeGen* ng, oc_string text, bool single);
void ng_emit_rax_to_xmm0(CodeBuffer* code, bool wide);
void ng_emit_rbx_to_xmm1(CodeBuffer* code, bool wide);
void ng_emit_xmm0_to_rax(CodeBuffer* code, bool wide);
uint8_t ng_sse_prefix(bool wide);
void ng_emit_convert(NativeGen* ng, CodeBuffer* code, oc_string from_t, oc_string to_t);
uint64_t ng_float_cast_lo(oc_string t);
uint64_t ng_float_cast_hi(oc_string t);
void ng_emit_xmm1_const(CodeBuffer* code, uint64_t bits);
void ng_emit_float_to_int(NativeGen* ng, CodeBuffer* code, oc_string src, oc_string dst);
void ng_emit_fp_check(NativeGen* ng, CodeBuffer* code, CodeBuffer* data, bool wide, Token tok);
void ng_emit_stderr_write(NativeGen* ng, CodeBuffer* code, bool is_win);
void ng_emit_store_disp(CodeBuffer* code, uint8_t reg, int32_t width, int32_t disp, bool at_rsp);
void ng_emit_array_init_at(NativeGen* ng, CodeBuffer* code, CodeBuffer* data, int32_t off, oc_string arr_t, AstNode* init, bool at_rsp);
oc_string ng_fn_param_type(NativeGen* ng, oc_string fn_name, size_t idx);
bool ng_is_place(NativeGen* ng, AstNode* e);
void ng_emit_expr(NativeGen* ng, CodeBuffer* code, CodeBuffer* data, AstNode* expr);
void ng_emit_expr_as(NativeGen* ng, CodeBuffer* code, CodeBuffer* data, AstNode* expr, oc_string to_t);
void ng_emit_lvalue_addr(NativeGen* ng, CodeBuffer* code, CodeBuffer* data, AstNode* expr);
void ng_emit_expr(NativeGen* ng, CodeBuffer* code, CodeBuffer* data, AstNode* expr);
void ng_emit_print_lit(NativeGen* ng, CodeBuffer* code, CodeBuffer* data, oc_string raw);
void ng_emit_stmt(NativeGen* ng, CodeBuffer* code, CodeBuffer* data, AstNode* stmt);
int32_t calc_locals_bytes_stmt(NativeGen* ng, AstNode* stmt);
void ng_emit_store_param_reg(CodeBuffer* code, size_t reg_idx, int32_t off);
void ng_emit_load_arg_reg(CodeBuffer* code, size_t reg_idx, uint32_t disp);
void ng_emit_store_param(CodeBuffer* code, size_t reg_idx, size_t total, int32_t off);
void ng_define_helper(NativeGen* ng, bool* has_offset, oc_string name, size_t off);
void ng_emit_write_fd(NativeGen* ng, CodeBuffer* code);
void ng_emit_win_runtime(NativeGen* ng, CodeBuffer* code, bool* has_offset);
size_t pe_align(size_t v, size_t a);
void pe_append_zeros(CodeBuffer* buf, size_t n);
void pe_append_section_header(CodeBuffer* pe, oc_string name, size_t vsize, size_t rva, size_t raw_size, size_t raw_off, uint32_t flags);
bool ng_write_pe(NativeGen* ng, CodeBuffer* code, CodeBuffer* data, oc_string out_file_str);
bool emit_native_image(AstNode* root, oc_string out_file_str, int32_t target, bool fp_trap, bool arena_poison, SourceMap* m);
bool emit_native_elf(AstNode* root, oc_string out_file_str, bool fp_trap, bool arena_poison, SourceMap* m);
bool emit_native_pe(AstNode* root, oc_string out_file_str, bool fp_trap, bool arena_poison, SourceMap* m);
bool ng_write_elf(NativeGen* ng, CodeBuffer* code, CodeBuffer* data, oc_string out_file_str);
bool is_safe_path(oc_string s, bool allow_backslash);
oc_string arena_dup_str(Arena* arena, oc_string s);
bool is_path_sep(uint8_t c);
bool is_abs_path(oc_string p);
oc_string dir_of_path(Arena* arena, oc_string path);
bool ends_with_exe(oc_string p);
int32_t parse_target_name(oc_string name);
bool is_macos_target(int32_t t);
oc_string c_compiler_for(int32_t target, int32_t host);
bool load_module_recursive(oc_string path, oc_string base_dir, oc_string exe_dir, CodeBuffer* out, ModuleLoader* loader, Arena* arena);
oc_string env_str(oc_string name);
oc_string make_private_tmp_dir(Arena* arena, bool allow_backslash);
void remove_private_tmp_dir(Arena* arena, oc_string dir);
bool source_uses_floats(oc_string src);
void print_usage(void);
int32_t run_sys_cmd(oc_string cmd);
oc_string exe_path(Arena* arena, oc_string argv0);
int32_t handle_update(int32_t host, oc_string repo);
int32_t handle_install(int32_t host, oc_string exe, oc_string exe_dir);
int32_t handle_self_cmd(bool is_install, oc_string argv0);
int main(int argc, char** argv);
size_t fmt_put_u64(uint8_t* buf, size_t n, uint64_t v);
size_t fmt_put_fixed6(uint8_t* buf, size_t n, double v);
void fmt_print_f64(double x);
size_t fmt_put_sci(uint8_t* buf, size_t n, double v);

void arena_poison_region(Arena* arena) {
    if ((((arena)->capacity) > 0)) {
    sys_mprotect(((void*)(((arena)->buffer))), ((arena)->capacity), 0);
    }
}

Arena arena_init(size_t capacity) {
    void* raw = sys_mmap(0, capacity, OC_PROT_RW, OC_MAP_PRIVATE_ANON, (-1), 0);
    uint8_t* buf = ((uint8_t*)(raw));
    if ((((size_t)(buf)) == 0)) {
    sys_exit(1);
    }
    if ((((size_t)(buf)) >= 0xfffffffffffff000)) {
    sys_exit(1);
    }
    return (Arena){ .offset = 0, .capacity = capacity, .buffer = buf };
}

uint8_t* arena_alloc(Arena* arena, size_t size) {
    if ((size > ((arena)->capacity))) {
    return ((uint8_t*)(0));
    }
    if ((size > __OC_SUB(((arena)->capacity), ((arena)->offset)))) {
    return ((uint8_t*)(0));
    }
    size_t rem = __OC_MOD(size, 8);
    size_t aligned = size;
    if ((rem != 0)) {
    if ((__OC_ADD(size, __OC_SUB(8, rem)) <= __OC_SUB(((arena)->capacity), ((arena)->offset)))) {
    aligned = __OC_ADD(size, __OC_SUB(8, rem));
    }
    }
    uint8_t* ptr = ((uint8_t*)((((arena)->buffer) + ((arena)->offset))));
    ((arena)->offset) = __OC_ADD(((arena)->offset), aligned);
    return ptr;
}

void arena_reset(Arena* arena) {
    if (sys_arena_poison()) {
    arena_poison_region(arena);
    Arena fresh = arena_init(((arena)->capacity));
    ((arena)->buffer) = ((fresh).buffer);
    ((arena)->offset) = 0;
    return;
    }
    size_t i = 0;
    while ((i < ((arena)->offset))) {
    __OC_AT((((arena)->buffer)), (i)) = 0;
    i = __OC_ADD(i, 1);
    }
    ((arena)->offset) = 0;
}

void arena_deinit(Arena* arena) {
    if (sys_arena_poison()) {
    arena_poison_region(arena);
    ((arena)->buffer) = ((uint8_t*)(0));
    ((arena)->capacity) = 0;
    ((arena)->offset) = 0;
    return;
    }
    size_t i = 0;
    while ((i < ((arena)->offset))) {
    __OC_AT((((arena)->buffer)), (i)) = 0;
    i = __OC_ADD(i, 1);
    }
    sys_munmap(((void*)(((arena)->buffer))), ((arena)->capacity));
    ((arena)->buffer) = ((uint8_t*)(0));
    ((arena)->capacity) = 0;
    ((arena)->offset) = 0;
}

bool str_eq(oc_string a, oc_string b) {
    if ((((a).len) != ((b).len))) {
    return false;
    }
    size_t i = 0;
    while ((i < ((a).len))) {
    if ((__oc_index_str(a, i) != __oc_index_str(b, i))) {
    return false;
    }
    i = __OC_ADD(i, 1);
    }
    return true;
}

bool starts_with(oc_string s, oc_string prefix) {
    if ((((s).len) < ((prefix).len))) {
    return false;
    }
    return str_eq(__oc_slice_str(s, 0, ((prefix).len)), prefix);
}

bool ends_with(oc_string s, oc_string suffix) {
    if ((((s).len) < ((suffix).len))) {
    return false;
    }
    return str_eq(__oc_slice_str(s, __OC_SUB(((s).len), ((suffix).len)), ((s).len)), suffix);
}

int64_t find(oc_string s, oc_string needle) {
    if ((((needle).len) > ((s).len))) {
    return (-1);
    }
    size_t i = 0;
    while ((__OC_ADD(i, ((needle).len)) <= ((s).len))) {
    if (str_eq(__oc_slice_str(s, i, __OC_ADD(i, ((needle).len))), needle)) {
    return ({ __auto_type _v = (i); if (!(_v < 0) && (uint64_t)_v > 9223372036854775807ULL) { fprintf(stderr, "panic: invalid signed cast\n"); exit(1); } (int64_t)_v; });
    }
    i = __OC_ADD(i, 1);
    }
    return (-1);
}

bool contains(oc_string s, oc_string needle) {
    return (find(s, needle) >= 0);
}

oc_string trim_start(oc_string s) {
    size_t i = 0;
    while ((i < ((s).len))) {
    if ((!is_space(__oc_index_str(s, i)))) {
    break;
    }
    i = __OC_ADD(i, 1);
    }
    return __oc_slice_str(s, i, ((s).len));
}

oc_string trim_end(oc_string s) {
    size_t n = ((s).len);
    while ((n > 0)) {
    if ((!is_space(__oc_index_str(s, __OC_SUB(n, 1))))) {
    break;
    }
    n = __OC_SUB(n, 1);
    }
    return __oc_slice_str(s, 0, n);
}

oc_string trim(oc_string s) {
    return trim_end(trim_start(s));
}

bool split_once(oc_string s, oc_string sep, Split* out) {
    int64_t at = find(s, sep);
    if ((at < 0)) {
    ((out)->head) = s;
    ((out)->tail) = __oc_slice_str(s, ((s).len), ((s).len));
    return false;
    }
    size_t i = ({ __auto_type _v = (at); if (_v < 0) { fprintf(stderr, "panic: invalid unsigned cast\n"); exit(1); } (size_t)_v; });
    ((out)->head) = __oc_slice_str(s, 0, i);
    ((out)->tail) = __oc_slice_str(s, __OC_ADD(i, ((sep).len)), ((s).len));
    return true;
}

oc_string substr(oc_string s, size_t start, size_t len) {
    return __oc_slice_str(s, start, __OC_ADD(start, len));
}

oc_string cstr_to_string(uint8_t* cstr) {
    size_t len = 0;
    while ((__OC_AT((cstr), (len)) != 0)) {
    len = __OC_ADD(len, 1);
    }
    return (oc_string){ .ptr = cstr, .len = len };
}

bool is_digit(uint8_t c) {
    if ((c < 48)) {
    return false;
    }
    return (c <= 57);
}

bool is_upper(uint8_t c) {
    if ((c < 65)) {
    return false;
    }
    return (c <= 90);
}

bool is_lower(uint8_t c) {
    if ((c < 97)) {
    return false;
    }
    return (c <= 122);
}

bool is_alpha(uint8_t c) {
    if (is_upper(c)) {
    return true;
    }
    return is_lower(c);
}

bool is_space(uint8_t c) {
    if ((c == 32)) {
    return true;
    }
    if ((c < 9)) {
    return false;
    }
    return (c <= 13);
}

uint8_t to_upper(uint8_t c) {
    if (is_lower(c)) {
    return __OC_SUB(c, 32);
    }
    return c;
}

uint8_t to_lower(uint8_t c) {
    if (is_upper(c)) {
    return __OC_ADD(c, 32);
    }
    return c;
}

uint8_t* str_alloc(Arena* arena, size_t n) {
    uint8_t* p = arena_alloc(arena, n);
    if ((((size_t)(p)) == 0)) {
    sys_write(2, (((oc_string){ .ptr = (const uint8_t*)("panic: arena out of memory\012"), .len = sizeof("panic: arena out of memory\012") - 1 }).ptr), 27);
    sys_exit(1);
    }
    return p;
}

uint8_t* str_grow(Arena* arena, uint8_t* buf, size_t len, size_t cap) {
    if ((__OC_ADD(((size_t)(buf)), cap) == __OC_ADD(((size_t)(((arena)->buffer))), ((arena)->offset)))) {
    str_alloc(arena, cap);
    return buf;
    }
    uint8_t* bigger = str_alloc(arena, __OC_MUL(cap, 2));
    size_t i = 0;
    while ((i < len)) {
    __OC_AT((bigger), (i)) = __OC_AT((buf), (i));
    i = __OC_ADD(i, 1);
    }
    return bigger;
}

oc_string str_concat(Arena* arena, oc_string a, oc_string b) {
    size_t total = __OC_ADD(((a).len), ((b).len));
    if ((total == 0)) {
    return (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 };
    }
    uint8_t* buf = str_alloc(arena, total);
    size_t i = 0;
    while ((i < ((a).len))) {
    __OC_AT((buf), (i)) = __oc_index_str(a, i);
    i = __OC_ADD(i, 1);
    }
    size_t j = 0;
    while ((j < ((b).len))) {
    __OC_AT((buf), (__OC_ADD(((a).len), j))) = __oc_index_str(b, j);
    j = __OC_ADD(j, 1);
    }
    return (oc_string){ .ptr = ((uint8_t*)(buf)), .len = total };
}

oc_string str_repeat(Arena* arena, oc_string s, size_t n) {
    size_t total = __OC_MUL(((s).len), n);
    if ((total == 0)) {
    return (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 };
    }
    uint8_t* buf = str_alloc(arena, total);
    size_t pos = 0;
    while ((pos < total)) {
    __OC_AT((buf), (pos)) = __oc_index_str(s, __OC_MOD(pos, ((s).len)));
    pos = __OC_ADD(pos, 1);
    }
    return (oc_string){ .ptr = ((uint8_t*)(buf)), .len = total };
}

oc_string str_join(Arena* arena, oc_string* parts, size_t count, oc_string sep) {
    if ((count == 0)) {
    return (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 };
    }
    size_t total = __OC_MUL(((sep).len), __OC_SUB(count, 1));
    size_t i = 0;
    while ((i < count)) {
    total = __OC_ADD(total, ((__OC_AT((parts), (i))).len));
    i = __OC_ADD(i, 1);
    }
    if ((total == 0)) {
    return (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 };
    }
    uint8_t* buf = str_alloc(arena, total);
    size_t pos = 0;
    i = 0;
    while ((i < count)) {
    if ((i > 0)) {
    size_t k = 0;
    while ((k < ((sep).len))) {
    __OC_AT((buf), (pos)) = __oc_index_str(sep, k);
    pos = __OC_ADD(pos, 1);
    k = __OC_ADD(k, 1);
    }
    }
    oc_string part = __OC_AT((parts), (i));
    size_t k = 0;
    while ((k < ((part).len))) {
    __OC_AT((buf), (pos)) = __oc_index_str(part, k);
    pos = __OC_ADD(pos, 1);
    k = __OC_ADD(k, 1);
    }
    i = __OC_ADD(i, 1);
    }
    return (oc_string){ .ptr = ((uint8_t*)(buf)), .len = total };
}

oc_string str_replace(Arena* arena, oc_string s, oc_string from, oc_string to) {
    size_t count = 0;
    if ((((from).len) > 0)) {
    size_t i = 0;
    while ((__OC_ADD(i, ((from).len)) <= ((s).len))) {
    if (str_eq(__oc_slice_str(s, i, __OC_ADD(i, ((from).len))), from)) {
    count = __OC_ADD(count, 1);
    i = __OC_ADD(i, ((from).len));
    }
    else {
    i = __OC_ADD(i, 1);
    }
    }
    }
    size_t total = __OC_ADD(__OC_SUB(((s).len), __OC_MUL(count, ((from).len))), __OC_MUL(count, ((to).len)));
    if ((total == 0)) {
    return (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 };
    }
    uint8_t* buf = str_alloc(arena, total);
    size_t pos = 0;
    size_t i = 0;
    while ((i < ((s).len))) {
    bool hit = false;
    if ((count > 0)) {
    if ((__OC_ADD(i, ((from).len)) <= ((s).len))) {
    hit = str_eq(__oc_slice_str(s, i, __OC_ADD(i, ((from).len))), from);
    }
    }
    if (hit) {
    size_t k = 0;
    while ((k < ((to).len))) {
    __OC_AT((buf), (pos)) = __oc_index_str(to, k);
    pos = __OC_ADD(pos, 1);
    k = __OC_ADD(k, 1);
    }
    i = __OC_ADD(i, ((from).len));
    }
    else {
    __OC_AT((buf), (pos)) = __oc_index_str(s, i);
    pos = __OC_ADD(pos, 1);
    i = __OC_ADD(i, 1);
    }
    }
    return (oc_string){ .ptr = ((uint8_t*)(buf)), .len = total };
}

oc_string str_to_upper(Arena* arena, oc_string s) {
    if ((((s).len) == 0)) {
    return (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 };
    }
    uint8_t* buf = str_alloc(arena, ((s).len));
    size_t i = 0;
    while ((i < ((s).len))) {
    __OC_AT((buf), (i)) = to_upper(__oc_index_str(s, i));
    i = __OC_ADD(i, 1);
    }
    return (oc_string){ .ptr = ((uint8_t*)(buf)), .len = ((s).len) };
}

oc_string str_to_lower(Arena* arena, oc_string s) {
    if ((((s).len) == 0)) {
    return (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 };
    }
    uint8_t* buf = str_alloc(arena, ((s).len));
    size_t i = 0;
    while ((i < ((s).len))) {
    __OC_AT((buf), (i)) = to_lower(__oc_index_str(s, i));
    i = __OC_ADD(i, 1);
    }
    return (oc_string){ .ptr = ((uint8_t*)(buf)), .len = ((s).len) };
}

StrBuf strbuf_init(Arena* arena, size_t cap) {
    size_t c = cap;
    if ((c < 16)) {
    c = 16;
    }
    return (StrBuf){ .arena = arena, .buf = str_alloc(arena, c), .len = 0, .cap = c };
}

void strbuf_push(StrBuf* b, uint8_t c) {
    if ((((b)->len) == ((b)->cap))) {
    ((b)->buf) = str_grow(((b)->arena), ((b)->buf), ((b)->len), ((b)->cap));
    ((b)->cap) = __OC_MUL(((b)->cap), 2);
    }
    __OC_AT((((b)->buf)), (((b)->len))) = c;
    ((b)->len) = __OC_ADD(((b)->len), 1);
}

void strbuf_push_str(StrBuf* b, oc_string s) {
    while ((__OC_ADD(((b)->len), ((s).len)) > ((b)->cap))) {
    ((b)->buf) = str_grow(((b)->arena), ((b)->buf), ((b)->len), ((b)->cap));
    ((b)->cap) = __OC_MUL(((b)->cap), 2);
    }
    size_t i = 0;
    while ((i < ((s).len))) {
    __OC_AT((((b)->buf)), (__OC_ADD(((b)->len), i))) = __oc_index_str(s, i);
    i = __OC_ADD(i, 1);
    }
    ((b)->len) = __OC_ADD(((b)->len), ((s).len));
}

oc_string strbuf_to_string(StrBuf* b) {
    return (oc_string){ .ptr = ((uint8_t*)(((b)->buf))), .len = ((b)->len) };
}

uint64_t digit_value(uint8_t c) {
    if (is_digit(c)) {
    return ({ __auto_type _v = (__OC_SUB(c, 48)); if (_v < 0) { fprintf(stderr, "panic: invalid unsigned cast\n"); exit(1); } (uint64_t)_v; });
    }
    if ((c >= 97)) {
    if ((c <= 102)) {
    return ({ __auto_type _v = (__OC_SUB(c, 87)); if (_v < 0) { fprintf(stderr, "panic: invalid unsigned cast\n"); exit(1); } (uint64_t)_v; });
    }
    }
    if ((c >= 65)) {
    if ((c <= 70)) {
    return ({ __auto_type _v = (__OC_SUB(c, 55)); if (_v < 0) { fprintf(stderr, "panic: invalid unsigned cast\n"); exit(1); } (uint64_t)_v; });
    }
    }
    return 99;
}

void num_check_radix(uint32_t radix) {
    bool bad = (radix < 2);
    if ((radix > 16)) {
    bad = true;
    }
    if (bad) {
    sys_write(2, (((oc_string){ .ptr = (const uint8_t*)("panic: radix must be 2 to 16\012"), .len = sizeof("panic: radix must be 2 to 16\012") - 1 }).ptr), 29);
    sys_exit(1);
    }
}

__ErrorUnion__u64__u32 num_digits(oc_string s, uint64_t radix) {
    if ((((s).len) == 0)) {
    return (__ErrorUnion__u64__u32){ .code = __OC_ADD(({ __auto_type _v = (1); if (_v < 0) { fprintf(stderr, "panic: invalid unsigned cast\n"); exit(1); } (uint64_t)_v; }), 1) };
    }
    uint64_t v = 0;
    size_t i = 0;
    while ((i < ((s).len))) {
    uint64_t d = digit_value(__oc_index_str(s, i));
    if ((d >= radix)) {
    return (__ErrorUnion__u64__u32){ .code = __OC_ADD(({ __auto_type _v = (1); if (_v < 0) { fprintf(stderr, "panic: invalid unsigned cast\n"); exit(1); } (uint64_t)_v; }), 1) };
    }
    if ((v > __OC_DIV(__OC_SUB(18446744073709551615ULL, d), radix))) {
    return (__ErrorUnion__u64__u32){ .code = __OC_ADD(({ __auto_type _v = (2); if (_v < 0) { fprintf(stderr, "panic: invalid unsigned cast\n"); exit(1); } (uint64_t)_v; }), 1) };
    }
    v = __OC_ADD(__OC_MUL(v, radix), d);
    i = __OC_ADD(i, 1);
    }
    return (__ErrorUnion__u64__u32){ .code = 0, .value = v };
}

__ErrorUnion__u64__u32 str_to_u64_radix(oc_string s, uint32_t radix) {
    num_check_radix(radix);
    if ((((s).len) == 0)) {
    return (__ErrorUnion__u64__u32){ .code = __OC_ADD(({ __auto_type _v = (0); if (_v < 0) { fprintf(stderr, "panic: invalid unsigned cast\n"); exit(1); } (uint64_t)_v; }), 1) };
    }
    oc_string digits = s;
    if ((__oc_index_str(s, 0) == 43)) {
    digits = __oc_slice_str(s, 1, ((s).len));
    }
    return num_digits(digits, ({ __auto_type _v = (radix); if (_v < 0) { fprintf(stderr, "panic: invalid unsigned cast\n"); exit(1); } (uint64_t)_v; }));
}

__ErrorUnion__i64__u32 str_to_i64_radix(oc_string s, uint32_t radix) {
    num_check_radix(radix);
    if ((((s).len) == 0)) {
    return (__ErrorUnion__i64__u32){ .code = __OC_ADD(({ __auto_type _v = (0); if (_v < 0) { fprintf(stderr, "panic: invalid unsigned cast\n"); exit(1); } (uint64_t)_v; }), 1) };
    }
    bool neg = false;
    oc_string digits = s;
    if ((__oc_index_str(s, 0) == 45)) {
    neg = true;
    digits = __oc_slice_str(s, 1, ((s).len));
    }
    else {
    if ((__oc_index_str(s, 0) == 43)) {
    digits = __oc_slice_str(s, 1, ((s).len));
    }
    }
    __ErrorUnion__u64__u32 __err1 = num_digits(digits, ({ __auto_type _v = (radix); if (_v < 0) { fprintf(stderr, "panic: invalid unsigned cast\n"); exit(1); } (uint64_t)_v; }));
    if ((((__err1).code) != 0)) {
    return (__ErrorUnion__i64__u32){ .code = ((__err1).code) };
    }
    uint64_t mag = ((__err1).value);
    if (neg) {
    if ((mag > 9223372036854775808ULL)) {
    return (__ErrorUnion__i64__u32){ .code = __OC_ADD(({ __auto_type _v = (2); if (_v < 0) { fprintf(stderr, "panic: invalid unsigned cast\n"); exit(1); } (uint64_t)_v; }), 1) };
    }
    if ((mag == 9223372036854775808ULL)) {
    return (__ErrorUnion__i64__u32){ .code = 0, .value = (-9223372036854775807LL - 1) };
    }
    return (__ErrorUnion__i64__u32){ .code = 0, .value = __OC_SUB(0, ({ __auto_type _v = (mag); if (!(_v < 0) && (uint64_t)_v > 9223372036854775807ULL) { fprintf(stderr, "panic: invalid signed cast\n"); exit(1); } (int64_t)_v; })) };
    }
    if ((mag > 9223372036854775807)) {
    return (__ErrorUnion__i64__u32){ .code = __OC_ADD(({ __auto_type _v = (2); if (_v < 0) { fprintf(stderr, "panic: invalid unsigned cast\n"); exit(1); } (uint64_t)_v; }), 1) };
    }
    return (__ErrorUnion__i64__u32){ .code = 0, .value = ({ __auto_type _v = (mag); if (!(_v < 0) && (uint64_t)_v > 9223372036854775807ULL) { fprintf(stderr, "panic: invalid signed cast\n"); exit(1); } (int64_t)_v; }) };
}

__ErrorUnion__u64__u32 str_to_u64(oc_string s) {
    return str_to_u64_radix(s, 10);
}

__ErrorUnion__i64__u32 str_to_i64(oc_string s) {
    return str_to_i64_radix(s, 10);
}

ParsedU64 parsed_u64(__ErrorUnion__u64__u32 r) {
    ParsedU64 fail = (ParsedU64){ .ok = false, .value = 0 };
    __ErrorUnion__u64__u32 __err2 = r;
    if ((((__err2).code) == 0)) {
    uint64_t v = ((__err2).value);
    return (ParsedU64){ .ok = true, .value = v };
    }
    else {
    return fail;
    }
    return fail;
}

ParsedI64 parsed_i64(__ErrorUnion__i64__u32 r) {
    ParsedI64 fail = (ParsedI64){ .ok = false, .value = 0 };
    __ErrorUnion__i64__u32 __err3 = r;
    if ((((__err3).code) == 0)) {
    int64_t v = ((__err3).value);
    return (ParsedI64){ .ok = true, .value = v };
    }
    else {
    return fail;
    }
    return fail;
}

bool radix_ok(uint64_t radix) {
    if ((radix < 2)) {
    return false;
    }
    return (radix <= 16);
}

ParsedU64 parse_digits_u64(oc_string s, uint64_t radix) {
    if ((!radix_ok(radix))) {
    return (ParsedU64){ .ok = false, .value = 0 };
    }
    return parsed_u64(num_digits(s, radix));
}

ParsedU64 parse_u64_radix(oc_string s, uint32_t radix) {
    if ((!radix_ok(({ __auto_type _v = (radix); if (_v < 0) { fprintf(stderr, "panic: invalid unsigned cast\n"); exit(1); } (uint64_t)_v; })))) {
    return (ParsedU64){ .ok = false, .value = 0 };
    }
    return parsed_u64(str_to_u64_radix(s, radix));
}

ParsedI64 parse_i64_radix(oc_string s, uint32_t radix) {
    if ((!radix_ok(({ __auto_type _v = (radix); if (_v < 0) { fprintf(stderr, "panic: invalid unsigned cast\n"); exit(1); } (uint64_t)_v; })))) {
    return (ParsedI64){ .ok = false, .value = 0 };
    }
    return parsed_i64(str_to_i64_radix(s, radix));
}

ParsedU64 parse_u64(oc_string s) {
    return parse_u64_radix(s, 10);
}

ParsedI64 parse_i64(oc_string s) {
    return parse_i64_radix(s, 10);
}

size_t fmt_digits_back(uint8_t* buf, size_t end, uint64_t v, uint64_t radix) {
    size_t pos = end;
    uint64_t x = v;
    while ((pos > 0)) {
    pos = __OC_SUB(pos, 1);
    uint64_t d = __OC_MOD(x, radix);
    if ((d < 10)) {
    __OC_AT((buf), (pos)) = ({ __auto_type _v = (__OC_ADD(48, d)); if (_v < 0 || (uint64_t)_v > 255ULL) { fprintf(stderr, "panic: invalid unsigned cast\n"); exit(1); } (uint8_t)_v; });
    }
    else {
    __OC_AT((buf), (pos)) = ({ __auto_type _v = (__OC_ADD(87, d)); if (_v < 0 || (uint64_t)_v > 255ULL) { fprintf(stderr, "panic: invalid unsigned cast\n"); exit(1); } (uint8_t)_v; });
    }
    x = __OC_DIV(x, radix);
    if ((x == 0)) {
    break;
    }
    }
    return pos;
}

oc_string int_text(Arena* arena, bool neg, uint64_t mag, uint64_t radix) {
    uint8_t tmp[24] = {0};
    size_t start = fmt_digits_back(((uint8_t*)((&(tmp)))), 24, mag, radix);
    size_t len = __OC_SUB(24, start);
    if (neg) {
    len = __OC_ADD(len, 1);
    }
    uint8_t* buf = str_alloc(arena, len);
    size_t pos = 0;
    if (neg) {
    __OC_AT((buf), (0)) = 45;
    pos = 1;
    }
    size_t i = start;
    while ((i < 24)) {
    __OC_AT((buf), (pos)) = __OC_AT((tmp), (i));
    pos = __OC_ADD(pos, 1);
    i = __OC_ADD(i, 1);
    }
    return (oc_string){ .ptr = ((uint8_t*)(buf)), .len = len };
}

uint64_t i64_magnitude(int64_t n) {
    if ((n >= 0)) {
    return ({ __auto_type _v = (n); if (_v < 0) { fprintf(stderr, "panic: invalid unsigned cast\n"); exit(1); } (uint64_t)_v; });
    }
    return __OC_ADD(({ __auto_type _v = (__OC_SUB(0, __OC_ADD(n, 1))); if (_v < 0) { fprintf(stderr, "panic: invalid unsigned cast\n"); exit(1); } (uint64_t)_v; }), 1);
}

oc_string u64_to_str(Arena* arena, uint64_t n) {
    return int_text(arena, false, n, 10);
}

oc_string i64_to_str(Arena* arena, int64_t n) {
    return int_text(arena, (n < 0), i64_magnitude(n), 10);
}

oc_string u64_to_hex(Arena* arena, uint64_t n) {
    return int_text(arena, false, n, 16);
}

void print_str(oc_string s) {
    sys_write(1, ((s).ptr), ((s).len));
}

void print_int_text(bool neg, uint64_t mag, uint64_t radix) {
    uint8_t tmp[24] = {0};
    size_t start = fmt_digits_back(((uint8_t*)((&(tmp)))), 24, mag, radix);
    if (neg) {
    sys_write(1, (((oc_string){ .ptr = (const uint8_t*)("-"), .len = sizeof("-") - 1 }).ptr), 1);
    }
    oc_string all = (oc_string){ .ptr = ((uint8_t*)((&(tmp)))), .len = 24 };
    print_str(__oc_slice_str(all, start, 24));
}

void print_u64(uint64_t n) {
    print_int_text(false, n, 10);
}

void print_i64(int64_t n) {
    print_int_text((n < 0), i64_magnitude(n), 10);
}

void print_hex(uint64_t n) {
    print_int_text(false, n, 16);
}

__ErrorUnion__string__u32 stdin_line(Arena* arena) {
    size_t cap = 128;
    uint8_t* buf = str_alloc(arena, cap);
    size_t len = 0;
    bool got = false;
    uint8_t c[1] = {0};
    bool reading = true;
    while (reading) {
    int64_t n = sys_read(0, ((uint8_t*)((&(c)))), 1);
    if ((n <= 0)) {
    reading = false;
    }
    else {
    got = true;
    if ((__OC_AT((c), (0)) == 10)) {
    reading = false;
    }
    else {
    if ((len == cap)) {
    buf = str_grow(arena, buf, len, cap);
    cap = __OC_MUL(cap, 2);
    }
    __OC_AT((buf), (len)) = __OC_AT((c), (0));
    len = __OC_ADD(len, 1);
    }
    }
    }
    if ((!got)) {
    return (__ErrorUnion__string__u32){ .code = __OC_ADD(({ __auto_type _v = (0); if (_v < 0) { fprintf(stderr, "panic: invalid unsigned cast\n"); exit(1); } (uint64_t)_v; }), 1) };
    }
    if ((len > 0)) {
    if ((__OC_AT((buf), (__OC_SUB(len, 1))) == 13)) {
    len = __OC_SUB(len, 1);
    }
    }
    return (__ErrorUnion__string__u32){ .code = 0, .value = (oc_string){ .ptr = ((uint8_t*)(buf)), .len = len } };
}

ReadLine read_line(Arena* arena) {
    __ErrorUnion__string__u32 __err4 = stdin_line(arena);
    if ((((__err4).code) == 0)) {
    oc_string line = ((__err4).value);
    return (ReadLine){ .ok = true, .line = line };
    }
    else {
    return (ReadLine){ .ok = false, .line = (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 } };
    }
    return (ReadLine){ .ok = false, .line = (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 } };
}

__ErrorUnion__string__u32 stdin_all(Arena* arena) {
    size_t cap = 4096;
    uint8_t* buf = str_alloc(arena, cap);
    size_t len = 0;
    bool reading = true;
    while (reading) {
    if ((len == cap)) {
    buf = str_grow(arena, buf, len, cap);
    cap = __OC_MUL(cap, 2);
    }
    int64_t n = sys_read(0, ((uint8_t*)((buf + len))), __OC_SUB(cap, len));
    if ((n < 0)) {
    return (__ErrorUnion__string__u32){ .code = __OC_ADD(({ __auto_type _v = (1); if (_v < 0) { fprintf(stderr, "panic: invalid unsigned cast\n"); exit(1); } (uint64_t)_v; }), 1) };
    }
    if ((n == 0)) {
    reading = false;
    }
    else {
    len = __OC_ADD(len, ({ __auto_type _v = (n); if (_v < 0) { fprintf(stderr, "panic: invalid unsigned cast\n"); exit(1); } (size_t)_v; }));
    }
    }
    return (__ErrorUnion__string__u32){ .code = 0, .value = (oc_string){ .ptr = ((uint8_t*)(buf)), .len = len } };
}

ReadAll read_all(Arena* arena) {
    __ErrorUnion__string__u32 __err5 = stdin_all(arena);
    if ((((__err5).code) == 0)) {
    oc_string data = ((__err5).value);
    return (ReadAll){ .ok = true, .data = data };
    }
    else {
    return (ReadAll){ .ok = false, .data = (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 } };
    }
    return (ReadAll){ .ok = false, .data = (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 } };
}

__ErrorUnion__i64__u32 stdin_i64(void) {
    uint8_t buf[64] = {0};
    size_t len = 0;
    bool got = false;
    bool too_long = false;
    uint8_t c[1] = {0};
    bool reading = true;
    while (reading) {
    int64_t n = sys_read(0, ((uint8_t*)((&(c)))), 1);
    if ((n <= 0)) {
    reading = false;
    }
    else {
    got = true;
    if ((__OC_AT((c), (0)) == 10)) {
    reading = false;
    }
    else {
    if ((len < 64)) {
    __OC_AT((buf), (len)) = __OC_AT((c), (0));
    len = __OC_ADD(len, 1);
    }
    else {
    too_long = true;
    }
    }
    }
    }
    if ((!got)) {
    return (__ErrorUnion__i64__u32){ .code = __OC_ADD(({ __auto_type _v = (0); if (_v < 0) { fprintf(stderr, "panic: invalid unsigned cast\n"); exit(1); } (uint64_t)_v; }), 1) };
    }
    if (too_long) {
    return (__ErrorUnion__i64__u32){ .code = __OC_ADD(({ __auto_type _v = (2); if (_v < 0) { fprintf(stderr, "panic: invalid unsigned cast\n"); exit(1); } (uint64_t)_v; }), 1) };
    }
    oc_string all = (oc_string){ .ptr = ((uint8_t*)((&(buf)))), .len = 64 };
    __ErrorUnion__i64__u32 __err6 = str_to_i64(trim(__oc_slice_str(all, 0, len)));
    if ((((__err6).code) == 0)) {
    int64_t v = ((__err6).value);
    return (__ErrorUnion__i64__u32){ .code = 0, .value = v };
    }
    else {
    return (__ErrorUnion__i64__u32){ .code = __OC_ADD(({ __auto_type _v = (3); if (_v < 0) { fprintf(stderr, "panic: invalid unsigned cast\n"); exit(1); } (uint64_t)_v; }), 1) };
    }
    return (__ErrorUnion__i64__u32){ .code = __OC_ADD(({ __auto_type _v = (3); if (_v < 0) { fprintf(stderr, "panic: invalid unsigned cast\n"); exit(1); } (uint64_t)_v; }), 1) };
}

ParsedI64 read_i64(void) {
    __ErrorUnion__i64__u32 __err7 = stdin_i64();
    if ((((__err7).code) == 0)) {
    int64_t v = ((__err7).value);
    return (ParsedI64){ .ok = true, .value = v };
    }
    else {
    return (ParsedI64){ .ok = false, .value = 0 };
    }
    return (ParsedI64){ .ok = false, .value = 0 };
}

int32_t read_int(void) {
    ParsedI64 r = read_i64();
    if ((!((r).ok))) {
    return 0;
    }
    if ((((r).value) < (-2147483648))) {
    return 0;
    }
    if ((((r).value) > 2147483647)) {
    return 0;
    }
    return ({ __auto_type _v = (((r).value)); if ((_v < 0 && (int64_t)_v < -2147483648LL) || (!(_v < 0) && (uint64_t)_v > 2147483647ULL)) { fprintf(stderr, "panic: invalid signed cast\n"); exit(1); } (int32_t)_v; });
}

uint8_t* cstr_path(oc_string path, uint8_t* out_buf) {
    size_t len = ((path).len);
    if ((len > 4095)) {
    len = 4095;
    }
    size_t i = 0;
    while ((i < len)) {
    __OC_AT((out_buf), (i)) = __oc_index_str(path, i);
    i = __OC_ADD(i, 1);
    }
    __OC_AT((out_buf), (len)) = 0;
    return ((uint8_t*)(out_buf));
}

__ErrorUnion__usize__u32 file_read(oc_string path, uint8_t* out_buf, size_t max_len) {
    uint8_t pbuf[4096] = {0};
    uint8_t* p = cstr_path(path, ((uint8_t*)((&(pbuf)))));
    int32_t fd = sys_open(p, OC_O_SAFE_READ, 0);
    if ((fd < 0)) {
    return (__ErrorUnion__usize__u32){ .code = __OC_ADD(({ __auto_type _v = (0); if (_v < 0) { fprintf(stderr, "panic: invalid unsigned cast\n"); exit(1); } (uint64_t)_v; }), 1) };
    }
    int64_t n = sys_read(fd, out_buf, max_len);
    sys_close(fd);
    if ((n < 0)) {
    return (__ErrorUnion__usize__u32){ .code = __OC_ADD(({ __auto_type _v = (1); if (_v < 0) { fprintf(stderr, "panic: invalid unsigned cast\n"); exit(1); } (uint64_t)_v; }), 1) };
    }
    return (__ErrorUnion__usize__u32){ .code = 0, .value = ({ __auto_type _v = (n); if (_v < 0) { fprintf(stderr, "panic: invalid unsigned cast\n"); exit(1); } (size_t)_v; }) };
}

__ErrorUnion__usize__u32 file_write(oc_string path, oc_string content) {
    uint8_t pbuf[4096] = {0};
    uint8_t* p = cstr_path(path, ((uint8_t*)((&(pbuf)))));
    int32_t fd = sys_open(p, OC_O_SAFE_WRITE_TRUNC, OC_MODE_FILE);
    if ((fd < 0)) {
    return (__ErrorUnion__usize__u32){ .code = __OC_ADD(({ __auto_type _v = (0); if (_v < 0) { fprintf(stderr, "panic: invalid unsigned cast\n"); exit(1); } (uint64_t)_v; }), 1) };
    }
    int64_t n = sys_write(fd, ((uint8_t*)(((content).ptr))), ((content).len));
    sys_close(fd);
    if ((n != ({ __auto_type _v = (((content).len)); if (!(_v < 0) && (uint64_t)_v > 9223372036854775807ULL) { fprintf(stderr, "panic: invalid signed cast\n"); exit(1); } (int64_t)_v; }))) {
    return (__ErrorUnion__usize__u32){ .code = __OC_ADD(({ __auto_type _v = (2); if (_v < 0) { fprintf(stderr, "panic: invalid unsigned cast\n"); exit(1); } (uint64_t)_v; }), 1) };
    }
    return (__ErrorUnion__usize__u32){ .code = 0, .value = ((content).len) };
}

int64_t read_file(oc_string path, uint8_t* out_buf, size_t max_len) {
    __ErrorUnion__usize__u32 __err8 = file_read(path, out_buf, max_len);
    if ((((__err8).code) == 0)) {
    size_t n = ((__err8).value);
    return ({ __auto_type _v = (n); if (!(_v < 0) && (uint64_t)_v > 9223372036854775807ULL) { fprintf(stderr, "panic: invalid signed cast\n"); exit(1); } (int64_t)_v; });
    }
    else {
    return (-1);
    }
    return (-1);
}

bool write_file(oc_string path, oc_string content) {
    __ErrorUnion__usize__u32 __err9 = file_write(path, content);
    if ((((__err9).code) == 0)) {
    return true;
    }
    else {
    return false;
    }
    return false;
}

bool chmod_exec(oc_string path) {
    uint8_t pbuf[4096] = {0};
    uint8_t* p = cstr_path(path, ((uint8_t*)((&(pbuf)))));
    return (sys_chmod(p, OC_MODE_EXEC) == 0);
}

bool unlink_file(oc_string path) {
    uint8_t pbuf[4096] = {0};
    uint8_t* p = cstr_path(path, ((uint8_t*)((&(pbuf)))));
    return (sys_unlink(p) == 0);
}

bool is_keyword(uint32_t tag) {
    if ((tag >= 10)) {
    if ((tag <= 24)) {
    return true;
    }
    }
    return false;
}

Lexer lexer_init(oc_string source) {
    return (Lexer){ .source = source, .index = 0, .line = 1, .col = 1 };
}

bool is_hex_digit(uint8_t c) {
    if ((c >= 48)) {
    if ((c <= 57)) {
    return true;
    }
    }
    if ((c >= 97)) {
    if ((c <= 102)) {
    return true;
    }
    }
    if ((c >= 65)) {
    if ((c <= 70)) {
    return true;
    }
    }
    if ((c == 95)) {
    return true;
    }
    return false;
}

bool is_ident_start(uint8_t c) {
    if ((c >= 97)) {
    if ((c <= 122)) {
    return true;
    }
    }
    if ((c >= 65)) {
    if ((c <= 90)) {
    return true;
    }
    }
    if ((c == 95)) {
    return true;
    }
    return false;
}

bool is_ident_char(uint8_t c) {
    if (is_ident_start(c)) {
    return true;
    }
    if (is_digit(c)) {
    return true;
    }
    return false;
}

uint32_t check_keyword(oc_string s) {
    if (str_eq(s, (oc_string){ .ptr = (const uint8_t*)("fn"), .len = sizeof("fn") - 1 })) {
    return TOK_FN;
    }
    if (str_eq(s, (oc_string){ .ptr = (const uint8_t*)("let"), .len = sizeof("let") - 1 })) {
    return TOK_LET;
    }
    if (str_eq(s, (oc_string){ .ptr = (const uint8_t*)("mut"), .len = sizeof("mut") - 1 })) {
    return TOK_MUT;
    }
    if (str_eq(s, (oc_string){ .ptr = (const uint8_t*)("return"), .len = sizeof("return") - 1 })) {
    return TOK_RETURN;
    }
    if (str_eq(s, (oc_string){ .ptr = (const uint8_t*)("if"), .len = sizeof("if") - 1 })) {
    return TOK_IF;
    }
    if (str_eq(s, (oc_string){ .ptr = (const uint8_t*)("else"), .len = sizeof("else") - 1 })) {
    return TOK_ELSE;
    }
    if (str_eq(s, (oc_string){ .ptr = (const uint8_t*)("while"), .len = sizeof("while") - 1 })) {
    return TOK_WHILE;
    }
    if (str_eq(s, (oc_string){ .ptr = (const uint8_t*)("struct"), .len = sizeof("struct") - 1 })) {
    return TOK_STRUCT;
    }
    if (str_eq(s, (oc_string){ .ptr = (const uint8_t*)("packed"), .len = sizeof("packed") - 1 })) {
    return TOK_PACKED;
    }
    if (str_eq(s, (oc_string){ .ptr = (const uint8_t*)("extern"), .len = sizeof("extern") - 1 })) {
    return TOK_EXTERN;
    }
    if (str_eq(s, (oc_string){ .ptr = (const uint8_t*)("import"), .len = sizeof("import") - 1 })) {
    return TOK_IMPORT;
    }
    if (str_eq(s, (oc_string){ .ptr = (const uint8_t*)("print"), .len = sizeof("print") - 1 })) {
    return TOK_PRINT;
    }
    if (str_eq(s, (oc_string){ .ptr = (const uint8_t*)("true"), .len = sizeof("true") - 1 })) {
    return TOK_TRUE;
    }
    if (str_eq(s, (oc_string){ .ptr = (const uint8_t*)("false"), .len = sizeof("false") - 1 })) {
    return TOK_FALSE;
    }
    if (str_eq(s, (oc_string){ .ptr = (const uint8_t*)("as"), .len = sizeof("as") - 1 })) {
    return TOK_AS;
    }
    if (str_eq(s, (oc_string){ .ptr = (const uint8_t*)("break"), .len = sizeof("break") - 1 })) {
    return TOK_BREAK;
    }
    if (str_eq(s, (oc_string){ .ptr = (const uint8_t*)("continue"), .len = sizeof("continue") - 1 })) {
    return TOK_CONTINUE;
    }
    if (str_eq(s, (oc_string){ .ptr = (const uint8_t*)("enum"), .len = sizeof("enum") - 1 })) {
    return TOK_ENUM;
    }
    if (str_eq(s, (oc_string){ .ptr = (const uint8_t*)("match"), .len = sizeof("match") - 1 })) {
    return TOK_MATCH;
    }
    if (str_eq(s, (oc_string){ .ptr = (const uint8_t*)("try"), .len = sizeof("try") - 1 })) {
    return TOK_TRY;
    }
    if (str_eq(s, (oc_string){ .ptr = (const uint8_t*)("catch"), .len = sizeof("catch") - 1 })) {
    return TOK_CATCH;
    }
    return TOK_IDENT;
}

Token lexer_next(Lexer* lex) {
    bool skipping = true;
    while (skipping) {
    if ((((lex)->index) >= ((((lex)->source)).len))) {
    skipping = false;
    }
    else {
    uint8_t c = __oc_index_str(((lex)->source), ((lex)->index));
    if ((c == 10)) {
    ((lex)->index) = __OC_ADD(((lex)->index), 1);
    ((lex)->line) = __OC_ADD(((lex)->line), 1);
    ((lex)->col) = 1;
    }
    else {
    if (is_space(c)) {
    ((lex)->index) = __OC_ADD(((lex)->index), 1);
    ((lex)->col) = __OC_ADD(((lex)->col), 1);
    }
    else {
    if ((c == 47)) {
    if ((__OC_ADD(((lex)->index), 1) < ((((lex)->source)).len))) {
    if ((__oc_index_str(((lex)->source), __OC_ADD(((lex)->index), 1)) == 47)) {
    ((lex)->index) = __OC_ADD(((lex)->index), 2);
    ((lex)->col) = __OC_ADD(((lex)->col), 2);
    while ((((lex)->index) < ((((lex)->source)).len))) {
    if ((__oc_index_str(((lex)->source), ((lex)->index)) == 10)) {
    break;
    }
    ((lex)->index) = __OC_ADD(((lex)->index), 1);
    ((lex)->col) = __OC_ADD(((lex)->col), 1);
    }
    }
    else {
    skipping = false;
    }
    }
    else {
    skipping = false;
    }
    }
    else {
    skipping = false;
    }
    }
    }
    }
    }
    if ((((lex)->index) >= ((((lex)->source)).len))) {
    return (Token){ .tag = TOK_EOF, .slice = (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 }, .line = ((lex)->line), .col = ((lex)->col) };
    }
    size_t start_idx = ((lex)->index);
    uint32_t tok_line = ((lex)->line);
    uint32_t tok_col = ((lex)->col);
    uint8_t c = __oc_index_str(((lex)->source), ((lex)->index));
    if (is_ident_start(c)) {
    ((lex)->index) = __OC_ADD(((lex)->index), 1);
    ((lex)->col) = __OC_ADD(((lex)->col), 1);
    while ((((lex)->index) < ((((lex)->source)).len))) {
    if (is_ident_char(__oc_index_str(((lex)->source), ((lex)->index)))) {
    ((lex)->index) = __OC_ADD(((lex)->index), 1);
    ((lex)->col) = __OC_ADD(((lex)->col), 1);
    }
    else {
    break;
    }
    }
    oc_string slice = __oc_slice_str(((lex)->source), start_idx, ((lex)->index));
    uint32_t tag = check_keyword(slice);
    return (Token){ .tag = tag, .slice = slice, .line = tok_line, .col = tok_col };
    }
    if (is_digit(c)) {
    if ((c == 48)) {
    if ((__OC_ADD(((lex)->index), 1) < ((((lex)->source)).len))) {
    uint8_t nc = __oc_index_str(((lex)->source), __OC_ADD(((lex)->index), 1));
    if ((nc == 120)) {
    ((lex)->index) = __OC_ADD(((lex)->index), 2);
    ((lex)->col) = __OC_ADD(((lex)->col), 2);
    while ((((lex)->index) < ((((lex)->source)).len))) {
    uint8_t hc = __oc_index_str(((lex)->source), ((lex)->index));
    if (is_hex_digit(hc)) {
    ((lex)->index) = __OC_ADD(((lex)->index), 1);
    ((lex)->col) = __OC_ADD(((lex)->col), 1);
    }
    else {
    break;
    }
    }
    oc_string slice = __oc_slice_str(((lex)->source), start_idx, ((lex)->index));
    return (Token){ .tag = TOK_INT, .slice = slice, .line = tok_line, .col = tok_col };
    }
    else {
    if ((nc == 88)) {
    ((lex)->index) = __OC_ADD(((lex)->index), 2);
    ((lex)->col) = __OC_ADD(((lex)->col), 2);
    while ((((lex)->index) < ((((lex)->source)).len))) {
    uint8_t hc = __oc_index_str(((lex)->source), ((lex)->index));
    if (is_hex_digit(hc)) {
    ((lex)->index) = __OC_ADD(((lex)->index), 1);
    ((lex)->col) = __OC_ADD(((lex)->col), 1);
    }
    else {
    break;
    }
    }
    oc_string slice = __oc_slice_str(((lex)->source), start_idx, ((lex)->index));
    return (Token){ .tag = TOK_INT, .slice = slice, .line = tok_line, .col = tok_col };
    }
    else {
    if ((nc == 98)) {
    ((lex)->index) = __OC_ADD(((lex)->index), 2);
    ((lex)->col) = __OC_ADD(((lex)->col), 2);
    while ((((lex)->index) < ((((lex)->source)).len))) {
    uint8_t bc = __oc_index_str(((lex)->source), ((lex)->index));
    if ((bc == 48)) {
    ((lex)->index) = __OC_ADD(((lex)->index), 1);
    ((lex)->col) = __OC_ADD(((lex)->col), 1);
    }
    else {
    if ((bc == 49)) {
    ((lex)->index) = __OC_ADD(((lex)->index), 1);
    ((lex)->col) = __OC_ADD(((lex)->col), 1);
    }
    else {
    if ((bc == 95)) {
    ((lex)->index) = __OC_ADD(((lex)->index), 1);
    ((lex)->col) = __OC_ADD(((lex)->col), 1);
    }
    else {
    break;
    }
    }
    }
    }
    oc_string slice = __oc_slice_str(((lex)->source), start_idx, ((lex)->index));
    return (Token){ .tag = TOK_INT, .slice = slice, .line = tok_line, .col = tok_col };
    }
    }
    }
    }
    }
    ((lex)->index) = __OC_ADD(((lex)->index), 1);
    ((lex)->col) = __OC_ADD(((lex)->col), 1);
    while ((((lex)->index) < ((((lex)->source)).len))) {
    uint8_t dc = __oc_index_str(((lex)->source), ((lex)->index));
    if (is_digit(dc)) {
    ((lex)->index) = __OC_ADD(((lex)->index), 1);
    ((lex)->col) = __OC_ADD(((lex)->col), 1);
    }
    else {
    if ((dc == 95)) {
    ((lex)->index) = __OC_ADD(((lex)->index), 1);
    ((lex)->col) = __OC_ADD(((lex)->col), 1);
    }
    else {
    break;
    }
    }
    }
    bool is_float = false;
    if ((__OC_ADD(((lex)->index), 1) < ((((lex)->source)).len))) {
    if ((__oc_index_str(((lex)->source), ((lex)->index)) == 46)) {
    if (is_digit(__oc_index_str(((lex)->source), __OC_ADD(((lex)->index), 1)))) {
    is_float = true;
    ((lex)->index) = __OC_ADD(((lex)->index), 1);
    ((lex)->col) = __OC_ADD(((lex)->col), 1);
    while ((((lex)->index) < ((((lex)->source)).len))) {
    if ((!is_digit(__oc_index_str(((lex)->source), ((lex)->index))))) {
    break;
    }
    ((lex)->index) = __OC_ADD(((lex)->index), 1);
    ((lex)->col) = __OC_ADD(((lex)->col), 1);
    }
    }
    }
    }
    if ((__OC_ADD(((lex)->index), 1) < ((((lex)->source)).len))) {
    if (((__oc_index_str(((lex)->source), ((lex)->index)) | 32) == 101)) {
    size_t exp_at = __OC_ADD(((lex)->index), 1);
    uint8_t sign = __oc_index_str(((lex)->source), exp_at);
    if ((sign == 43)) {
    exp_at = __OC_ADD(exp_at, 1);
    }
    if ((sign == 45)) {
    exp_at = __OC_ADD(exp_at, 1);
    }
    if ((exp_at < ((((lex)->source)).len))) {
    if (is_digit(__oc_index_str(((lex)->source), exp_at))) {
    is_float = true;
    ((lex)->col) = __OC_ADD(((lex)->col), ({ __auto_type _v = (__OC_SUB(exp_at, ((lex)->index))); if (_v < 0 || (uint64_t)_v > 4294967295ULL) { fprintf(stderr, "panic: invalid unsigned cast\n"); exit(1); } (uint32_t)_v; }));
    ((lex)->index) = exp_at;
    while ((((lex)->index) < ((((lex)->source)).len))) {
    if ((!is_digit(__oc_index_str(((lex)->source), ((lex)->index))))) {
    break;
    }
    ((lex)->index) = __OC_ADD(((lex)->index), 1);
    ((lex)->col) = __OC_ADD(((lex)->col), 1);
    }
    }
    }
    }
    }
    oc_string slice = __oc_slice_str(((lex)->source), start_idx, ((lex)->index));
    uint32_t num_tag = TOK_INT;
    if (is_float) {
    num_tag = TOK_FLOAT;
    }
    return (Token){ .tag = num_tag, .slice = slice, .line = tok_line, .col = tok_col };
    }
    if ((c == 34)) {
    ((lex)->index) = __OC_ADD(((lex)->index), 1);
    ((lex)->col) = __OC_ADD(((lex)->col), 1);
    bool closed = false;
    while ((((lex)->index) < ((((lex)->source)).len))) {
    uint8_t sc = __oc_index_str(((lex)->source), ((lex)->index));
    if ((sc == 92)) {
    bool escaped_newline = false;
    if ((__OC_ADD(((lex)->index), 1) < ((((lex)->source)).len))) {
    escaped_newline = (__oc_index_str(((lex)->source), __OC_ADD(((lex)->index), 1)) == 10);
    }
    ((lex)->index) = __OC_ADD(((lex)->index), 2);
    if (escaped_newline) {
    ((lex)->line) = __OC_ADD(((lex)->line), 1);
    ((lex)->col) = 1;
    }
    else {
    ((lex)->col) = __OC_ADD(((lex)->col), 2);
    }
    if ((((lex)->index) > ((((lex)->source)).len))) {
    ((lex)->index) = ((((lex)->source)).len);
    }
    }
    else {
    if ((sc == 34)) {
    ((lex)->index) = __OC_ADD(((lex)->index), 1);
    ((lex)->col) = __OC_ADD(((lex)->col), 1);
    closed = true;
    break;
    }
    else {
    if ((sc == 10)) {
    ((lex)->line) = __OC_ADD(((lex)->line), 1);
    ((lex)->col) = 1;
    }
    else {
    ((lex)->col) = __OC_ADD(((lex)->col), 1);
    }
    ((lex)->index) = __OC_ADD(((lex)->index), 1);
    }
    }
    }
    oc_string slice = __oc_slice_str(((lex)->source), start_idx, ((lex)->index));
    uint32_t str_tag = TOK_STRING;
    if ((!closed)) {
    str_tag = TOK_ERROR;
    }
    return (Token){ .tag = str_tag, .slice = slice, .line = tok_line, .col = tok_col };
    }
    if ((__OC_ADD(((lex)->index), 1) < ((((lex)->source)).len))) {
    uint8_t next_c = __oc_index_str(((lex)->source), __OC_ADD(((lex)->index), 1));
    if ((c == 45)) {
    if ((next_c == 62)) {
    ((lex)->index) = __OC_ADD(((lex)->index), 2);
    ((lex)->col) = __OC_ADD(((lex)->col), 2);
    return (Token){ .tag = TOK_ARROW, .slice = __oc_slice_str(((lex)->source), start_idx, ((lex)->index)), .line = tok_line, .col = tok_col };
    }
    }
    if ((c == 61)) {
    if ((next_c == 61)) {
    ((lex)->index) = __OC_ADD(((lex)->index), 2);
    ((lex)->col) = __OC_ADD(((lex)->col), 2);
    return (Token){ .tag = TOK_EQEQ, .slice = __oc_slice_str(((lex)->source), start_idx, ((lex)->index)), .line = tok_line, .col = tok_col };
    }
    if ((next_c == 62)) {
    ((lex)->index) = __OC_ADD(((lex)->index), 2);
    ((lex)->col) = __OC_ADD(((lex)->col), 2);
    return (Token){ .tag = TOK_FATARROW, .slice = __oc_slice_str(((lex)->source), start_idx, ((lex)->index)), .line = tok_line, .col = tok_col };
    }
    }
    if ((c == 33)) {
    if ((next_c == 61)) {
    ((lex)->index) = __OC_ADD(((lex)->index), 2);
    ((lex)->col) = __OC_ADD(((lex)->col), 2);
    return (Token){ .tag = TOK_NEQ, .slice = __oc_slice_str(((lex)->source), start_idx, ((lex)->index)), .line = tok_line, .col = tok_col };
    }
    }
    if ((c == 46)) {
    if ((next_c == 46)) {
    ((lex)->index) = __OC_ADD(((lex)->index), 2);
    ((lex)->col) = __OC_ADD(((lex)->col), 2);
    return (Token){ .tag = TOK_DOTDOT, .slice = __oc_slice_str(((lex)->source), start_idx, ((lex)->index)), .line = tok_line, .col = tok_col };
    }
    }
    if ((c == 60)) {
    if ((next_c == 60)) {
    ((lex)->index) = __OC_ADD(((lex)->index), 2);
    ((lex)->col) = __OC_ADD(((lex)->col), 2);
    return (Token){ .tag = TOK_SHL, .slice = __oc_slice_str(((lex)->source), start_idx, ((lex)->index)), .line = tok_line, .col = tok_col };
    }
    if ((next_c == 61)) {
    ((lex)->index) = __OC_ADD(((lex)->index), 2);
    ((lex)->col) = __OC_ADD(((lex)->col), 2);
    return (Token){ .tag = TOK_LTE, .slice = __oc_slice_str(((lex)->source), start_idx, ((lex)->index)), .line = tok_line, .col = tok_col };
    }
    }
    if ((c == 62)) {
    if ((next_c == 62)) {
    ((lex)->index) = __OC_ADD(((lex)->index), 2);
    ((lex)->col) = __OC_ADD(((lex)->col), 2);
    return (Token){ .tag = TOK_SHR, .slice = __oc_slice_str(((lex)->source), start_idx, ((lex)->index)), .line = tok_line, .col = tok_col };
    }
    if ((next_c == 61)) {
    ((lex)->index) = __OC_ADD(((lex)->index), 2);
    ((lex)->col) = __OC_ADD(((lex)->col), 2);
    return (Token){ .tag = TOK_GTE, .slice = __oc_slice_str(((lex)->source), start_idx, ((lex)->index)), .line = tok_line, .col = tok_col };
    }
    }
    }
    ((lex)->index) = __OC_ADD(((lex)->index), 1);
    ((lex)->col) = __OC_ADD(((lex)->col), 1);
    uint32_t tag = 0;
    if ((c == 40)) {
    tag = TOK_LPAREN;
    }
    else {
    if ((c == 41)) {
    tag = TOK_RPAREN;
    }
    else {
    if ((c == 123)) {
    tag = TOK_LBRACE;
    }
    else {
    if ((c == 125)) {
    tag = TOK_RBRACE;
    }
    else {
    if ((c == 91)) {
    tag = TOK_LBRACKET;
    }
    else {
    if ((c == 93)) {
    tag = TOK_RBRACKET;
    }
    else {
    if ((c == 58)) {
    tag = TOK_COLON;
    }
    else {
    if ((c == 59)) {
    tag = TOK_SEMICOLON;
    }
    else {
    if ((c == 44)) {
    tag = TOK_COMMA;
    }
    else {
    if ((c == 46)) {
    tag = TOK_DOT;
    }
    else {
    if ((c == 61)) {
    tag = TOK_EQUAL;
    }
    else {
    if ((c == 43)) {
    tag = TOK_PLUS;
    }
    else {
    if ((c == 45)) {
    tag = TOK_MINUS;
    }
    else {
    if ((c == 42)) {
    tag = TOK_STAR;
    }
    else {
    if ((c == 47)) {
    tag = TOK_SLASH;
    }
    else {
    if ((c == 37)) {
    tag = TOK_PERCENT;
    }
    else {
    if ((c == 38)) {
    tag = TOK_AMP;
    }
    else {
    if ((c == 33)) {
    tag = TOK_BANG;
    }
    else {
    if ((c == 60)) {
    tag = TOK_LT;
    }
    else {
    if ((c == 62)) {
    tag = TOK_GT;
    }
    else {
    if ((c == 124)) {
    tag = TOK_PIPE;
    }
    else {
    if ((c == 94)) {
    tag = TOK_CARET;
    }
    else {
    if ((c == 126)) {
    tag = TOK_TILDE;
    }
    else {
    tag = TOK_ERROR;
    }
    }
    }
    }
    }
    }
    }
    }
    }
    }
    }
    }
    }
    }
    }
    }
    }
    }
    }
    }
    }
    }
    }
    return (Token){ .tag = tag, .slice = __oc_slice_str(((lex)->source), start_idx, ((lex)->index)), .line = tok_line, .col = tok_col };
}

int32_t srcmap_file(SourceMap* m, uint32_t line) {
    if ((m == ((SourceMap*)(0)))) {
    return (-1);
    }
    size_t i = ((m)->count);
    while ((i > 0)) {
    i = __OC_SUB(i, 1);
    if ((__OC_AT((((m)->first_line)), (i)) <= line)) {
    return ({ __auto_type _v = (i); if ((_v < 0 && (int64_t)_v < -2147483648LL) || (!(_v < 0) && (uint64_t)_v > 2147483647ULL)) { fprintf(stderr, "panic: invalid signed cast\n"); exit(1); } (int32_t)_v; });
    }
    }
    return (-1);
}

AstNode* ast_alloc_node(Arena* arena, uint32_t tag, Token tok) {
    uint8_t* raw = arena_alloc(arena, 128);
    AstNode* node = ((AstNode*)(raw));
    ((node)->tag) = tag;
    ((node)->tok) = tok;
    ((node)->type_name) = (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 };
    ((node)->left) = ((AstNode*)(0));
    ((node)->right) = ((AstNode*)(0));
    ((node)->next) = ((AstNode*)(0));
    return node;
}

void parser_report_lex_error(Parser* p, Token tok) {
    print_src_loc(((p)->srcmap), tok);
    if ((((((tok).slice)).len) > 0)) {
    if ((__oc_index_str(((tok).slice), 0) == 34)) {
    print_str_raw((oc_string){ .ptr = (const uint8_t*)("error: unterminated string literal (a backslash escapes the next character, so \134\042 does not end a string)\012"), .len = sizeof("error: unterminated string literal (a backslash escapes the next character, so \134\042 does not end a string)\012") - 1 });
    return;
    }
    }
    print_str_raw((oc_string){ .ptr = (const uint8_t*)("error: unexpected character '"), .len = sizeof("error: unexpected character '") - 1 });
    print_str_raw(((tok).slice));
    print_str_raw((oc_string){ .ptr = (const uint8_t*)("'\012"), .len = sizeof("'\012") - 1 });
}

void parser_error(Parser* p, Token tok, oc_string msg) {
    if ((((tok).tag) == TOK_ERROR)) {
    return;
    }
    if ((((tok).tag) == TOK_EOF)) {
    if (((p)->has_error)) {
    return;
    }
    }
    print_src_loc(((p)->srcmap), tok);
    print_str_raw(msg);
    print_str_raw((oc_string){ .ptr = (const uint8_t*)(" '"), .len = sizeof(" '") - 1 });
    if ((((tok).tag) == TOK_EOF)) {
    print_str_raw((oc_string){ .ptr = (const uint8_t*)("end of input"), .len = sizeof("end of input") - 1 });
    }
    else {
    print_str_raw(((tok).slice));
    }
    print_str_raw((oc_string){ .ptr = (const uint8_t*)("'\012"), .len = sizeof("'\012") - 1 });
}

Parser parser_init(oc_string source, Arena* arena) {
    Lexer lex = lexer_init(source);
    Token cur = lexer_next((&(lex)));
    bool err = false;
    if ((((cur).tag) == TOK_ERROR)) {
    err = true;
    }
    return (Parser){ .lex = lex, .current = cur, .arena = arena, .allow_struct_init = true, .has_error = err, .loop_depth = 0, .srcmap = ((SourceMap*)(0)) };
}

void parser_advance(Parser* p) {
    ((p)->current) = lexer_next((&(((p)->lex))));
    if ((((((p)->current)).tag) == TOK_ERROR)) {
    ((p)->has_error) = true;
    parser_report_lex_error(p, ((p)->current));
    }
}

Token parser_expect(Parser* p, uint32_t expected_tag) {
    Token tok = ((p)->current);
    if ((((tok).tag) == expected_tag)) {
    parser_advance(p);
    }
    else {
    parser_error(p, tok, (oc_string){ .ptr = (const uint8_t*)("parse error: unexpected"), .len = sizeof("parse error: unexpected") - 1 });
    ((p)->has_error) = true;
    }
    return tok;
}

uint32_t get_precedence(uint32_t tag) {
    if ((tag == TOK_PIPE)) {
    return 1;
    }
    if ((tag == TOK_CARET)) {
    return 2;
    }
    if ((tag == TOK_AMP)) {
    return 3;
    }
    if ((tag == TOK_EQEQ)) {
    return 4;
    }
    if ((tag == TOK_NEQ)) {
    return 4;
    }
    if ((tag == TOK_LT)) {
    return 5;
    }
    if ((tag == TOK_LTE)) {
    return 5;
    }
    if ((tag == TOK_GT)) {
    return 5;
    }
    if ((tag == TOK_GTE)) {
    return 5;
    }
    if ((tag == TOK_SHL)) {
    return 6;
    }
    if ((tag == TOK_SHR)) {
    return 6;
    }
    if ((tag == TOK_PLUS)) {
    return 7;
    }
    if ((tag == TOK_MINUS)) {
    return 7;
    }
    if ((tag == TOK_STAR)) {
    return 8;
    }
    if ((tag == TOK_SLASH)) {
    return 8;
    }
    if ((tag == TOK_PERCENT)) {
    return 8;
    }
    return 0;
}

oc_string parse_type(Parser* p) {
    uint8_t* start_ptr = ((((((p)->current)).slice)).ptr);
    uint8_t* end_ptr = (start_ptr + ((((((p)->current)).slice)).len));
    bool cont = true;
    while (cont) {
    if ((((((p)->current)).tag) == TOK_STAR)) {
    end_ptr = (((((((p)->current)).slice)).ptr) + ((((((p)->current)).slice)).len));
    parser_advance(p);
    if ((((((p)->current)).tag) == TOK_MUT)) {
    end_ptr = (((((((p)->current)).slice)).ptr) + ((((((p)->current)).slice)).len));
    parser_advance(p);
    }
    }
    else {
    if ((((((p)->current)).tag) == TOK_AMP)) {
    end_ptr = (((((((p)->current)).slice)).ptr) + ((((((p)->current)).slice)).len));
    parser_advance(p);
    if ((((((p)->current)).tag) == TOK_MUT)) {
    end_ptr = (((((((p)->current)).slice)).ptr) + ((((((p)->current)).slice)).len));
    parser_advance(p);
    }
    }
    else {
    if ((((((p)->current)).tag) == TOK_LBRACKET)) {
    end_ptr = (((((((p)->current)).slice)).ptr) + ((((((p)->current)).slice)).len));
    parser_advance(p);
    while ((((((p)->current)).tag) != TOK_RBRACKET)) {
    if ((((((p)->current)).tag) == TOK_EOF)) {
    break;
    }
    end_ptr = (((((((p)->current)).slice)).ptr) + ((((((p)->current)).slice)).len));
    parser_advance(p);
    }
    if ((((((p)->current)).tag) == TOK_RBRACKET)) {
    end_ptr = (((((((p)->current)).slice)).ptr) + ((((((p)->current)).slice)).len));
    parser_advance(p);
    }
    }
    else {
    cont = false;
    }
    }
    }
    }
    if ((((((p)->current)).tag) != TOK_EOF)) {
    end_ptr = (((((((p)->current)).slice)).ptr) + ((((((p)->current)).slice)).len));
    parser_advance(p);
    }
    if ((((((p)->current)).tag) == TOK_LT)) {
    int32_t depth = 0;
    bool scanning = true;
    while (scanning) {
    uint32_t t = ((((p)->current)).tag);
    if ((t == TOK_LT)) {
    depth = __OC_ADD(depth, 1);
    }
    if ((t == TOK_GT)) {
    depth = __OC_SUB(depth, 1);
    }
    if ((t == TOK_SHR)) {
    depth = __OC_SUB(depth, 2);
    }
    if ((t == TOK_EOF)) {
    scanning = false;
    }
    if (scanning) {
    end_ptr = (((((((p)->current)).slice)).ptr) + ((((((p)->current)).slice)).len));
    parser_advance(p);
    if ((depth <= 0)) {
    scanning = false;
    }
    }
    }
    }
    if ((((((p)->current)).tag) == TOK_BANG)) {
    parser_advance(p);
    oc_string err_t = parse_type(p);
    end_ptr = (((err_t).ptr) + ((err_t).len));
    }
    size_t len = __OC_SUB(((size_t)(end_ptr)), ((size_t)(start_ptr)));
    return (oc_string){ .ptr = start_ptr, .len = len };
}

AstNode* parse_generic_params(Parser* p) {
    parser_expect(p, TOK_LT);
    AstNode* first = ((AstNode*)(0));
    AstNode* last = ((AstNode*)(0));
    while ((((((p)->current)).tag) != TOK_GT)) {
    if ((((((p)->current)).tag) == TOK_EOF)) {
    break;
    }
    bool is_const = false;
    if ((((((p)->current)).tag) == TOK_IDENT)) {
    if (str_eq(((((p)->current)).slice), (oc_string){ .ptr = (const uint8_t*)("const"), .len = sizeof("const") - 1 })) {
    is_const = true;
    parser_advance(p);
    }
    }
    Token name = parser_expect(p, TOK_IDENT);
    AstNode* node = ast_alloc_node(((p)->arena), NODE_PARAM, name);
    if (is_const) {
    parser_expect(p, TOK_COLON);
    ((node)->type_name) = parse_type(p);
    }
    if ((first == ((AstNode*)(0)))) {
    first = node;
    }
    else {
    ((last)->next) = node;
    }
    last = node;
    if ((((((p)->current)).tag) == TOK_COMMA)) {
    parser_advance(p);
    }
    else {
    break;
    }
    }
    parser_expect(p, TOK_GT);
    return first;
}

oc_string parser_try_type_args(Parser* p) {
    Lexer saved_lex = ((p)->lex);
    Token saved_cur = ((p)->current);
    uint8_t* start = (((((((p)->current)).slice)).ptr) + 1);
    uint8_t* end = start;
    int32_t depth = 0;
    bool ok = true;
    bool scanning = true;
    while (scanning) {
    uint32_t t = ((((p)->current)).tag);
    bool allowed = false;
    if ((t == TOK_LT)) {
    depth = __OC_ADD(depth, 1);
    allowed = true;
    }
    if ((t == TOK_GT)) {
    depth = __OC_SUB(depth, 1);
    allowed = true;
    if ((depth == 0)) {
    end = ((((((p)->current)).slice)).ptr);
    }
    }
    if ((t == TOK_SHR)) {
    depth = __OC_SUB(depth, 2);
    allowed = true;
    if ((depth == 0)) {
    end = (((((((p)->current)).slice)).ptr) + 1);
    }
    }
    if ((t == TOK_IDENT)) {
    allowed = true;
    }
    if ((t == TOK_INT)) {
    allowed = true;
    }
    if ((t == TOK_STAR)) {
    allowed = true;
    }
    if ((t == TOK_AMP)) {
    allowed = true;
    }
    if ((t == TOK_MUT)) {
    allowed = true;
    }
    if ((t == TOK_LBRACKET)) {
    allowed = true;
    }
    if ((t == TOK_RBRACKET)) {
    allowed = true;
    }
    if ((t == TOK_COMMA)) {
    allowed = true;
    }
    if ((!allowed)) {
    ok = false;
    }
    if ((depth < 0)) {
    ok = false;
    }
    if ((!ok)) {
    scanning = false;
    }
    if (scanning) {
    ((p)->current) = lexer_next((&(((p)->lex))));
    if ((depth == 0)) {
    scanning = false;
    }
    }
    }
    if (ok) {
    ok = (((((p)->current)).tag) == TOK_LPAREN);
    if ((((((p)->current)).tag) == TOK_LBRACE)) {
    ok = ((p)->allow_struct_init);
    }
    }
    if (ok) {
    if ((((size_t)(end)) <= ((size_t)(start)))) {
    ok = false;
    }
    }
    if ((!ok)) {
    ((p)->lex) = saved_lex;
    ((p)->current) = saved_cur;
    return (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 };
    }
    return (oc_string){ .ptr = start, .len = __OC_SUB(((size_t)(end)), ((size_t)(start))) };
}

AstNode* parse_primary(Parser* p) {
    Token tok = ((p)->current);
    if ((((tok).tag) == TOK_INT)) {
    parser_advance(p);
    return ast_alloc_node(((p)->arena), NODE_INT, tok);
    }
    if ((((tok).tag) == TOK_STRING)) {
    parser_advance(p);
    return ast_alloc_node(((p)->arena), NODE_STRING, tok);
    }
    if ((((tok).tag) == TOK_FLOAT)) {
    parser_advance(p);
    return ast_alloc_node(((p)->arena), NODE_FLOAT, tok);
    }
    if ((((tok).tag) == TOK_TRUE)) {
    parser_advance(p);
    return ast_alloc_node(((p)->arena), NODE_BOOL, tok);
    }
    if ((((tok).tag) == TOK_FALSE)) {
    parser_advance(p);
    return ast_alloc_node(((p)->arena), NODE_BOOL, tok);
    }
    if ((((tok).tag) == TOK_LBRACKET)) {
    parser_advance(p);
    AstNode* arr_node = ast_alloc_node(((p)->arena), NODE_ARRAY_INIT, tok);
    AstNode* first_elem = ((AstNode*)(0));
    AstNode* last_elem = ((AstNode*)(0));
    while ((((((p)->current)).tag) != TOK_RBRACKET)) {
    if ((((((p)->current)).tag) == TOK_EOF)) {
    break;
    }
    AstNode* elem = parse_expr(p, 0);
    if ((first_elem == ((AstNode*)(0)))) {
    first_elem = elem;
    last_elem = elem;
    }
    else {
    ((last_elem)->next) = elem;
    last_elem = elem;
    }
    if ((((((p)->current)).tag) == TOK_COMMA)) {
    parser_advance(p);
    }
    else {
    break;
    }
    }
    parser_expect(p, TOK_RBRACKET);
    ((arr_node)->left) = first_elem;
    return arr_node;
    }
    if ((((tok).tag) == TOK_IDENT)) {
    parser_advance(p);
    oc_string type_args = (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 };
    if ((((((p)->current)).tag) == TOK_LT)) {
    type_args = parser_try_type_args(p);
    }
    if (((p)->allow_struct_init)) {
    if ((((((p)->current)).tag) == TOK_LBRACE)) {
    parser_advance(p);
    AstNode* s_node = ast_alloc_node(((p)->arena), NODE_STRUCT_INIT, tok);
    ((s_node)->type_name) = type_args;
    AstNode* first_f = ((AstNode*)(0));
    AstNode* last_f = ((AstNode*)(0));
    while ((((((p)->current)).tag) != TOK_RBRACE)) {
    if ((((((p)->current)).tag) == TOK_EOF)) {
    break;
    }
    Token f_name = parser_expect(p, TOK_IDENT);
    parser_expect(p, TOK_COLON);
    bool prev_allow = ((p)->allow_struct_init);
    ((p)->allow_struct_init) = true;
    AstNode* f_val = parse_expr(p, 0);
    ((p)->allow_struct_init) = prev_allow;
    AstNode* f_node = ast_alloc_node(((p)->arena), NODE_PARAM, f_name);
    ((f_node)->left) = f_val;
    if ((first_f == ((AstNode*)(0)))) {
    first_f = f_node;
    last_f = f_node;
    }
    else {
    ((last_f)->next) = f_node;
    last_f = f_node;
    }
    if ((((((p)->current)).tag) == TOK_COMMA)) {
    parser_advance(p);
    }
    else {
    break;
    }
    }
    parser_expect(p, TOK_RBRACE);
    ((s_node)->left) = first_f;
    return s_node;
    }
    }
    AstNode* id_node = ast_alloc_node(((p)->arena), NODE_IDENT, tok);
    ((id_node)->type_name) = type_args;
    return id_node;
    }
    if ((((tok).tag) == TOK_LPAREN)) {
    parser_advance(p);
    AstNode* inner = parse_expr(p, 0);
    parser_expect(p, TOK_RPAREN);
    return inner;
    }
    parser_error(p, tok, (oc_string){ .ptr = (const uint8_t*)("parse error: unexpected in an expression:"), .len = sizeof("parse error: unexpected in an expression:") - 1 });
    ((p)->has_error) = true;
    parser_advance(p);
    return ((AstNode*)(0));
}

AstNode* parse_postfix(Parser* p) {
    AstNode* expr = parse_primary(p);
    bool running = true;
    while (running) {
    if ((((((p)->current)).tag) == TOK_LPAREN)) {
    Token call_tok = ((p)->current);
    if ((expr != ((AstNode*)(0)))) {
    call_tok = ((expr)->tok);
    }
    parser_advance(p);
    AstNode* call_node = ast_alloc_node(((p)->arena), NODE_CALL, call_tok);
    if ((expr != ((AstNode*)(0)))) {
    if ((((expr)->tag) == NODE_IDENT)) {
    ((call_node)->type_name) = ((expr)->type_name);
    }
    }
    AstNode* first_arg = ((AstNode*)(0));
    AstNode* last_arg = ((AstNode*)(0));
    while ((((((p)->current)).tag) != TOK_RPAREN)) {
    if ((((((p)->current)).tag) == TOK_EOF)) {
    break;
    }
    AstNode* arg = parse_expr(p, 0);
    if ((first_arg == ((AstNode*)(0)))) {
    first_arg = arg;
    last_arg = arg;
    }
    else {
    ((last_arg)->next) = arg;
    last_arg = arg;
    }
    if ((((((p)->current)).tag) == TOK_COMMA)) {
    parser_advance(p);
    }
    else {
    break;
    }
    }
    parser_expect(p, TOK_RPAREN);
    ((call_node)->left) = first_arg;
    expr = call_node;
    }
    else {
    if ((((((p)->current)).tag) == TOK_DOT)) {
    parser_advance(p);
    if ((((((p)->current)).tag) == TOK_AS)) {
    parser_advance(p);
    parser_expect(p, TOK_LPAREN);
    oc_string t_name = parse_type(p);
    parser_expect(p, TOK_RPAREN);
    AstNode* cast_node = ast_alloc_node(((p)->arena), NODE_CAST, ((p)->current));
    ((cast_node)->left) = expr;
    ((cast_node)->type_name) = t_name;
    expr = cast_node;
    }
    else {
    if ((((((p)->current)).tag) == TOK_IDENT)) {
    if (str_eq(((((p)->current)).slice), (oc_string){ .ptr = (const uint8_t*)("as"), .len = sizeof("as") - 1 })) {
    parser_advance(p);
    parser_expect(p, TOK_LPAREN);
    oc_string t_name = parse_type(p);
    parser_expect(p, TOK_RPAREN);
    AstNode* cast_node = ast_alloc_node(((p)->arena), NODE_CAST, ((p)->current));
    ((cast_node)->left) = expr;
    ((cast_node)->type_name) = t_name;
    expr = cast_node;
    }
    else {
    Token f_tok = parser_expect(p, TOK_IDENT);
    AstNode* fa_node = ast_alloc_node(((p)->arena), NODE_FIELD_ACCESS, f_tok);
    ((fa_node)->left) = expr;
    expr = fa_node;
    }
    }
    else {
    running = false;
    }
    }
    }
    else {
    if ((((((p)->current)).tag) == TOK_AS)) {
    parser_advance(p);
    oc_string t_name = parse_type(p);
    AstNode* cast_node = ast_alloc_node(((p)->arena), NODE_CAST, ((p)->current));
    ((cast_node)->left) = expr;
    ((cast_node)->type_name) = t_name;
    expr = cast_node;
    }
    else {
    if ((((((p)->current)).tag) == TOK_LBRACKET)) {
    parser_advance(p);
    if ((((((p)->current)).tag) == TOK_DOTDOT)) {
    parser_advance(p);
    AstNode* end_expr = ((AstNode*)(0));
    if ((((((p)->current)).tag) != TOK_RBRACKET)) {
    if ((((((p)->current)).tag) != TOK_EOF)) {
    end_expr = parse_expr(p, 0);
    }
    }
    parser_expect(p, TOK_RBRACKET);
    AstNode* sl_node = ast_alloc_node(((p)->arena), NODE_SLICE, ((p)->current));
    ((sl_node)->left) = expr;
    AstNode* range_node = ast_alloc_node(((p)->arena), NODE_PARAM, ((p)->current));
    ((range_node)->left) = ((AstNode*)(0));
    ((range_node)->right) = end_expr;
    ((sl_node)->right) = range_node;
    expr = sl_node;
    }
    else {
    AstNode* first_expr = parse_expr(p, 0);
    if ((((((p)->current)).tag) == TOK_DOTDOT)) {
    parser_advance(p);
    AstNode* end_expr = ((AstNode*)(0));
    if ((((((p)->current)).tag) != TOK_RBRACKET)) {
    if ((((((p)->current)).tag) != TOK_EOF)) {
    end_expr = parse_expr(p, 0);
    }
    }
    parser_expect(p, TOK_RBRACKET);
    AstNode* sl_node = ast_alloc_node(((p)->arena), NODE_SLICE, ((p)->current));
    ((sl_node)->left) = expr;
    AstNode* range_node = ast_alloc_node(((p)->arena), NODE_PARAM, ((p)->current));
    ((range_node)->left) = first_expr;
    ((range_node)->right) = end_expr;
    ((sl_node)->right) = range_node;
    expr = sl_node;
    }
    else {
    parser_expect(p, TOK_RBRACKET);
    AstNode* idx_node = ast_alloc_node(((p)->arena), NODE_INDEX, ((p)->current));
    ((idx_node)->left) = expr;
    ((idx_node)->right) = first_expr;
    expr = idx_node;
    }
    }
    }
    else {
    running = false;
    }
    }
    }
    }
    }
    return expr;
}

AstNode* parse_unary(Parser* p) {
    Token tok = ((p)->current);
    if ((((tok).tag) == TOK_TRY)) {
    parser_advance(p);
    AstNode* t = ast_alloc_node(((p)->arena), NODE_TRY, tok);
    ((t)->left) = parse_unary(p);
    return t;
    }
    if ((((tok).tag) == TOK_MINUS)) {
    parser_advance(p);
    AstNode* target = parse_unary(p);
    AstNode* un = ast_alloc_node(((p)->arena), NODE_UNARY, tok);
    ((un)->left) = target;
    return un;
    }
    if ((((tok).tag) == TOK_BANG)) {
    parser_advance(p);
    AstNode* target = parse_unary(p);
    AstNode* un = ast_alloc_node(((p)->arena), NODE_UNARY, tok);
    ((un)->left) = target;
    return un;
    }
    if ((((tok).tag) == TOK_TILDE)) {
    parser_advance(p);
    AstNode* target = parse_unary(p);
    AstNode* un = ast_alloc_node(((p)->arena), NODE_UNARY, tok);
    ((un)->left) = target;
    return un;
    }
    if ((((tok).tag) == TOK_STAR)) {
    parser_advance(p);
    AstNode* target = parse_unary(p);
    AstNode* deref = ast_alloc_node(((p)->arena), NODE_DEREF, tok);
    ((deref)->left) = target;
    return deref;
    }
    if ((((tok).tag) == TOK_AMP)) {
    parser_advance(p);
    bool is_mut = false;
    if ((((((p)->current)).tag) == TOK_MUT)) {
    parser_advance(p);
    is_mut = true;
    }
    AstNode* target = parse_unary(p);
    AstNode* addr = ast_alloc_node(((p)->arena), NODE_ADDRESS_OF, tok);
    ((addr)->left) = target;
    if (is_mut) {
    ((addr)->type_name) = (oc_string){ .ptr = (const uint8_t*)("mut"), .len = sizeof("mut") - 1 };
    }
    return addr;
    }
    return parse_postfix(p);
}

AstNode* parse_expr(Parser* p, uint32_t min_prec) {
    AstNode* left = parse_unary(p);
    bool running = true;
    while (running) {
    uint32_t prec = get_precedence(((((p)->current)).tag));
    if ((prec == 0)) {
    running = false;
    }
    else {
    if ((prec < min_prec)) {
    running = false;
    }
    else {
    Token op_tok = ((p)->current);
    parser_advance(p);
    AstNode* right = parse_expr(p, __OC_ADD(prec, 1));
    AstNode* bin = ast_alloc_node(((p)->arena), NODE_BINARY, op_tok);
    ((bin)->left) = left;
    ((bin)->right) = right;
    left = bin;
    }
    }
    }
    if ((min_prec == 0)) {
    if ((((((p)->current)).tag) == TOK_CATCH)) {
    AstNode* c = ast_alloc_node(((p)->arena), NODE_CATCH, ((p)->current));
    parser_advance(p);
    ((c)->left) = left;
    ((c)->right) = parse_expr(p, 0);
    return c;
    }
    }
    return left;
}

AstNode* parse_enum_decl(Parser* p) {
    parser_expect(p, TOK_ENUM);
    Token name = parser_expect(p, TOK_IDENT);
    AstNode* node = ast_alloc_node(((p)->arena), NODE_ENUM, name);
    parser_expect(p, TOK_LBRACE);
    AstNode* last = ((AstNode*)(0));
    while ((((((p)->current)).tag) != TOK_RBRACE)) {
    if ((((((p)->current)).tag) == TOK_EOF)) {
    break;
    }
    AstNode* v = ast_alloc_node(((p)->arena), NODE_PARAM, parser_expect(p, TOK_IDENT));
    if ((last == ((AstNode*)(0)))) {
    ((node)->left) = v;
    }
    else {
    ((last)->next) = v;
    }
    last = v;
    if ((((((p)->current)).tag) == TOK_COMMA)) {
    parser_advance(p);
    }
    else {
    break;
    }
    }
    parser_expect(p, TOK_RBRACE);
    return node;
}

AstNode* parse_match_stmt(Parser* p) {
    AstNode* node = ast_alloc_node(((p)->arena), NODE_MATCH, parser_expect(p, TOK_MATCH));
    bool prev_allow = ((p)->allow_struct_init);
    ((p)->allow_struct_init) = false;
    ((node)->left) = parse_expr(p, 0);
    ((p)->allow_struct_init) = prev_allow;
    parser_expect(p, TOK_LBRACE);
    AstNode* last = ((AstNode*)(0));
    while ((((((p)->current)).tag) != TOK_RBRACE)) {
    if ((((((p)->current)).tag) == TOK_EOF)) {
    break;
    }
    Token first = parser_expect(p, TOK_IDENT);
    AstNode* arm = ast_alloc_node(((p)->arena), NODE_ARM, first);
    if ((((((p)->current)).tag) == TOK_DOT)) {
    parser_advance(p);
    ((arm)->right) = ast_alloc_node(((p)->arena), NODE_IDENT, first);
    ((arm)->tok) = parser_expect(p, TOK_IDENT);
    }
    if ((((((p)->current)).tag) == TOK_LPAREN)) {
    parser_advance(p);
    ((arm)->type_name) = ((parser_expect(p, TOK_IDENT)).slice);
    parser_expect(p, TOK_RPAREN);
    }
    parser_expect(p, TOK_FATARROW);
    if ((((((p)->current)).tag) == TOK_LBRACE)) {
    ((arm)->left) = parse_block(p);
    }
    else {
    ((arm)->left) = parse_stmt(p);
    }
    if ((last == ((AstNode*)(0)))) {
    ((node)->right) = arm;
    }
    else {
    ((last)->next) = arm;
    }
    last = arm;
    if ((((((p)->current)).tag) == TOK_COMMA)) {
    parser_advance(p);
    }
    }
    parser_expect(p, TOK_RBRACE);
    return node;
}

AstNode* parse_block(Parser* p) {
    Token tok = parser_expect(p, TOK_LBRACE);
    AstNode* block_node = ast_alloc_node(((p)->arena), NODE_BLOCK, tok);
    AstNode* first_stmt = ((AstNode*)(0));
    AstNode* last_stmt = ((AstNode*)(0));
    while ((((((p)->current)).tag) != TOK_RBRACE)) {
    if ((((((p)->current)).tag) == TOK_EOF)) {
    break;
    }
    uint32_t prev_tag = ((((p)->current)).tag);
    size_t prev_idx = ((((p)->lex)).index);
    AstNode* stmt = parse_stmt(p);
    if ((stmt != ((AstNode*)(0)))) {
    if ((first_stmt == ((AstNode*)(0)))) {
    first_stmt = stmt;
    last_stmt = stmt;
    }
    else {
    ((last_stmt)->next) = stmt;
    last_stmt = stmt;
    }
    }
    if ((((((p)->current)).tag) == prev_tag)) {
    if ((((((p)->lex)).index) == prev_idx)) {
    parser_advance(p);
    }
    }
    }
    parser_expect(p, TOK_RBRACE);
    ((block_node)->left) = first_stmt;
    return block_node;
}

AstNode* parse_print_stmt(Parser* p) {
    Token tok = ((p)->current);
    parser_advance(p);
    parser_expect(p, TOK_LPAREN);
    AstNode* expr = parse_expr(p, 0);
    parser_expect(p, TOK_RPAREN);
    if ((((((p)->current)).tag) == TOK_SEMICOLON)) {
    parser_advance(p);
    }
    AstNode* node = ast_alloc_node(((p)->arena), NODE_PRINT, tok);
    ((node)->left) = expr;
    return node;
}

AstNode* parse_let_stmt(Parser* p) {
    Token tok = ((p)->current);
    parser_advance(p);
    bool is_mut = false;
    if ((((((p)->current)).tag) == TOK_MUT)) {
    parser_advance(p);
    is_mut = true;
    }
    Token name_tok = parser_expect(p, TOK_IDENT);
    if (is_mut) {
    ((name_tok).tag) = TOK_MUT;
    }
    AstNode* node = ast_alloc_node(((p)->arena), NODE_LET, name_tok);
    if ((((((p)->current)).tag) == TOK_COLON)) {
    parser_advance(p);
    ((node)->type_name) = parse_type(p);
    }
    parser_expect(p, TOK_EQUAL);
    AstNode* init_expr = parse_expr(p, 0);
    if ((((((p)->current)).tag) == TOK_SEMICOLON)) {
    parser_advance(p);
    }
    ((node)->left) = init_expr;
    return node;
}

AstNode* parse_return_stmt(Parser* p) {
    Token tok = ((p)->current);
    parser_advance(p);
    AstNode* node = ast_alloc_node(((p)->arena), NODE_RETURN, tok);
    if ((((((p)->current)).tag) != TOK_SEMICOLON)) {
    if ((((((p)->current)).tag) != TOK_RBRACE)) {
    if ((((((p)->current)).tag) != TOK_EOF)) {
    ((node)->left) = parse_expr(p, 0);
    }
    }
    }
    if ((((((p)->current)).tag) == TOK_SEMICOLON)) {
    parser_advance(p);
    }
    return node;
}

AstNode* parse_if_stmt(Parser* p) {
    Token tok = ((p)->current);
    parser_advance(p);
    bool prev_allow = ((p)->allow_struct_init);
    ((p)->allow_struct_init) = false;
    AstNode* cond = parse_expr(p, 0);
    ((p)->allow_struct_init) = prev_allow;
    AstNode* then_block = parse_block(p);
    AstNode* node = ast_alloc_node(((p)->arena), NODE_IF, tok);
    ((node)->left) = cond;
    ((node)->right) = then_block;
    if ((((((p)->current)).tag) == TOK_ELSE)) {
    parser_advance(p);
    if ((((((p)->current)).tag) == TOK_IF)) {
    ((then_block)->next) = parse_if_stmt(p);
    }
    else {
    ((then_block)->next) = parse_block(p);
    }
    }
    return node;
}

AstNode* parse_while_stmt(Parser* p) {
    Token tok = ((p)->current);
    parser_advance(p);
    bool prev_allow = ((p)->allow_struct_init);
    ((p)->allow_struct_init) = false;
    AstNode* cond = parse_expr(p, 0);
    ((p)->allow_struct_init) = prev_allow;
    ((p)->loop_depth) = __OC_ADD(((p)->loop_depth), 1);
    AstNode* body = parse_block(p);
    ((p)->loop_depth) = __OC_SUB(((p)->loop_depth), 1);
    AstNode* node = ast_alloc_node(((p)->arena), NODE_WHILE, tok);
    ((node)->left) = cond;
    ((node)->right) = body;
    return node;
}

AstNode* parse_stmt(Parser* p) {
    if ((((((p)->current)).tag) == TOK_PRINT)) {
    return parse_print_stmt(p);
    }
    if ((((((p)->current)).tag) == TOK_LET)) {
    return parse_let_stmt(p);
    }
    if ((((((p)->current)).tag) == TOK_RETURN)) {
    return parse_return_stmt(p);
    }
    if ((((((p)->current)).tag) == TOK_IF)) {
    return parse_if_stmt(p);
    }
    if ((((((p)->current)).tag) == TOK_WHILE)) {
    return parse_while_stmt(p);
    }
    if ((((((p)->current)).tag) == TOK_BREAK)) {
    Token tok = ((p)->current);
    parser_advance(p);
    parser_expect(p, TOK_SEMICOLON);
    if ((((p)->loop_depth) <= 0)) {
    fwrite("parse error: 'break' outside of loop", 1, sizeof("parse error: 'break' outside of loop") - 1, stdout); fputc('\n', stdout);
    ((p)->has_error) = true;
    }
    return ast_alloc_node(((p)->arena), NODE_BREAK, tok);
    }
    if ((((((p)->current)).tag) == TOK_CONTINUE)) {
    Token tok = ((p)->current);
    parser_advance(p);
    parser_expect(p, TOK_SEMICOLON);
    if ((((p)->loop_depth) <= 0)) {
    fwrite("parse error: 'continue' outside of loop", 1, sizeof("parse error: 'continue' outside of loop") - 1, stdout); fputc('\n', stdout);
    ((p)->has_error) = true;
    }
    return ast_alloc_node(((p)->arena), NODE_CONTINUE, tok);
    }
    if ((((((p)->current)).tag) == TOK_LBRACE)) {
    return parse_block(p);
    }
    if ((((((p)->current)).tag) == TOK_MATCH)) {
    return parse_match_stmt(p);
    }
    if ((((((p)->current)).tag) == TOK_SEMICOLON)) {
    parser_advance(p);
    return ((AstNode*)(0));
    }
    AstNode* expr = parse_expr(p, 0);
    if ((((((p)->current)).tag) == TOK_EQUAL)) {
    Token eq_tok = ((p)->current);
    parser_advance(p);
    AstNode* val = parse_expr(p, 0);
    if ((((((p)->current)).tag) == TOK_SEMICOLON)) {
    parser_advance(p);
    }
    AstNode* assign_node = ast_alloc_node(((p)->arena), NODE_ASSIGN, eq_tok);
    ((assign_node)->left) = expr;
    ((assign_node)->right) = val;
    return assign_node;
    }
    if ((((((p)->current)).tag) == TOK_SEMICOLON)) {
    parser_advance(p);
    }
    return expr;
}

AstNode* parse_struct_decl(Parser* p, bool is_packed) {
    Token name_tok = parser_expect(p, TOK_IDENT);
    AstNode* gparams = ((AstNode*)(0));
    if ((((((p)->current)).tag) == TOK_LT)) {
    gparams = parse_generic_params(p);
    }
    parser_expect(p, TOK_LBRACE);
    AstNode* node = ast_alloc_node(((p)->arena), NODE_STRUCT_DECL, name_tok);
    if (is_packed) {
    ((node)->type_name) = (oc_string){ .ptr = (const uint8_t*)("packed"), .len = sizeof("packed") - 1 };
    }
    AstNode* first_f = ((AstNode*)(0));
    AstNode* last_f = ((AstNode*)(0));
    while ((((((p)->current)).tag) != TOK_RBRACE)) {
    if ((((((p)->current)).tag) == TOK_EOF)) {
    break;
    }
    Token f_name = parser_expect(p, TOK_IDENT);
    parser_expect(p, TOK_COLON);
    oc_string f_type = parse_type(p);
    AstNode* f_node = ast_alloc_node(((p)->arena), NODE_PARAM, f_name);
    ((f_node)->type_name) = f_type;
    if ((first_f == ((AstNode*)(0)))) {
    first_f = f_node;
    last_f = f_node;
    }
    else {
    ((last_f)->next) = f_node;
    last_f = f_node;
    }
    if ((((((p)->current)).tag) == TOK_COMMA)) {
    parser_advance(p);
    }
    }
    parser_expect(p, TOK_RBRACE);
    ((node)->left) = first_f;
    if ((gparams != ((AstNode*)(0)))) {
    AstNode* g = ast_alloc_node(((p)->arena), NODE_GENERIC, name_tok);
    ((g)->left) = gparams;
    ((g)->right) = node;
    return g;
    }
    return node;
}

AstNode* parse_fn_decl(Parser* p) {
    Token fn_tok = parser_expect(p, TOK_FN);
    Token name_tok = parser_expect(p, TOK_IDENT);
    AstNode* fn_node = ast_alloc_node(((p)->arena), NODE_FN, name_tok);
    AstNode* gparams = ((AstNode*)(0));
    if ((((((p)->current)).tag) == TOK_LT)) {
    gparams = parse_generic_params(p);
    }
    parser_expect(p, TOK_LPAREN);
    AstNode* first_param = ((AstNode*)(0));
    AstNode* last_param = ((AstNode*)(0));
    while ((((((p)->current)).tag) != TOK_RPAREN)) {
    if ((((((p)->current)).tag) == TOK_EOF)) {
    break;
    }
    Token p_name = parser_expect(p, TOK_IDENT);
    AstNode* param_node = ast_alloc_node(((p)->arena), NODE_PARAM, p_name);
    parser_expect(p, TOK_COLON);
    ((param_node)->type_name) = parse_type(p);
    if ((first_param == ((AstNode*)(0)))) {
    first_param = param_node;
    last_param = param_node;
    }
    else {
    ((last_param)->next) = param_node;
    last_param = param_node;
    }
    if ((((((p)->current)).tag) == TOK_COMMA)) {
    parser_advance(p);
    }
    else {
    break;
    }
    }
    parser_expect(p, TOK_RPAREN);
    if ((((((p)->current)).tag) == TOK_ARROW)) {
    parser_advance(p);
    ((fn_node)->type_name) = parse_type(p);
    }
    if ((((((p)->current)).tag) != TOK_LBRACE)) {
    if ((((((p)->current)).tag) == TOK_SEMICOLON)) {
    parser_advance(p);
    }
    ((fn_node)->left) = first_param;
    ((fn_node)->right) = ((AstNode*)(0));
    return fn_node;
    }
    AstNode* body = parse_block(p);
    ((fn_node)->left) = first_param;
    ((fn_node)->right) = body;
    if ((gparams != ((AstNode*)(0)))) {
    AstNode* g = ast_alloc_node(((p)->arena), NODE_GENERIC, name_tok);
    ((g)->left) = gparams;
    ((g)->right) = fn_node;
    return g;
    }
    return fn_node;
}

void print_str_raw(oc_string s) {
    sys_write(1, ((s).ptr), ((s).len));
}

void print_u32_raw(uint32_t n) {
    uint8_t digits[10] = {0};
    size_t pos = 10;
    uint32_t v = n;
    while ((pos > 0)) {
    pos = __OC_SUB(pos, 1);
    __OC_AT((digits), (pos)) = ({ __auto_type _v = (__OC_ADD(48, __OC_MOD(v, 10))); if (_v < 0 || (uint64_t)_v > 255ULL) { fprintf(stderr, "panic: invalid unsigned cast\n"); exit(1); } (uint8_t)_v; });
    v = __OC_DIV(v, 10);
    if ((v == 0)) {
    break;
    }
    }
    oc_string all = (oc_string){ .ptr = ((uint8_t*)((&(digits)))), .len = 10 };
    print_str_raw(__oc_slice_str(all, pos, 10));
}

void print_src_pos(SourceMap* m, Token tok) {
    int32_t f = srcmap_file(m, ((tok).line));
    if ((f >= 0)) {
    print_str_raw(__OC_AT((((m)->path)), (({ __auto_type _v = (f); if (_v < 0) { fprintf(stderr, "panic: invalid unsigned cast\n"); exit(1); } (size_t)_v; }))));
    print_str_raw((oc_string){ .ptr = (const uint8_t*)(":"), .len = sizeof(":") - 1 });
    print_u32_raw(__OC_ADD(__OC_SUB(((tok).line), __OC_AT((((m)->first_line)), (({ __auto_type _v = (f); if (_v < 0) { fprintf(stderr, "panic: invalid unsigned cast\n"); exit(1); } (size_t)_v; })))), 1));
    }
    else {
    print_u32_raw(((tok).line));
    }
    print_str_raw((oc_string){ .ptr = (const uint8_t*)(":"), .len = sizeof(":") - 1 });
    print_u32_raw(((tok).col));
}

void print_src_loc(SourceMap* m, Token tok) {
    int32_t f = srcmap_file(m, ((tok).line));
    if ((f >= 0)) {
    print_str_raw(__OC_AT((((m)->path)), (({ __auto_type _v = (f); if (_v < 0) { fprintf(stderr, "panic: invalid unsigned cast\n"); exit(1); } (size_t)_v; }))));
    print_str_raw((oc_string){ .ptr = (const uint8_t*)(":"), .len = sizeof(":") - 1 });
    print_u32_raw(__OC_ADD(__OC_SUB(((tok).line), __OC_AT((((m)->first_line)), (({ __auto_type _v = (f); if (_v < 0) { fprintf(stderr, "panic: invalid unsigned cast\n"); exit(1); } (size_t)_v; })))), 1));
    }
    else {
    print_u32_raw(((tok).line));
    }
    print_str_raw((oc_string){ .ptr = (const uint8_t*)(":"), .len = sizeof(":") - 1 });
    print_u32_raw(((tok).col));
    print_str_raw((oc_string){ .ptr = (const uint8_t*)(": "), .len = sizeof(": ") - 1 });
}

void print_err_ref_invalidated(oc_string name, oc_string bv) {
    print_str_raw((oc_string){ .ptr = (const uint8_t*)("error: reference '"), .len = sizeof("error: reference '") - 1 });
    print_str_raw(name);
    print_str_raw((oc_string){ .ptr = (const uint8_t*)("' invalidated by mutable operation on '"), .len = sizeof("' invalidated by mutable operation on '") - 1 });
    print_str_raw(bv);
    print_str_raw((oc_string){ .ptr = (const uint8_t*)("'\012"), .len = sizeof("'\012") - 1 });
}

void print_err_uaf_reset(oc_string name, oc_string arena) {
    print_str_raw((oc_string){ .ptr = (const uint8_t*)("error: use-after-free: variable '"), .len = sizeof("error: use-after-free: variable '") - 1 });
    print_str_raw(name);
    print_str_raw((oc_string){ .ptr = (const uint8_t*)("' used after arena '"), .len = sizeof("' used after arena '") - 1 });
    print_str_raw(arena);
    print_str_raw((oc_string){ .ptr = (const uint8_t*)("' was reset\012"), .len = sizeof("' was reset\012") - 1 });
}

void print_err_uaf_deinit(oc_string name, oc_string arena) {
    print_str_raw((oc_string){ .ptr = (const uint8_t*)("error: use-after-free: variable '"), .len = sizeof("error: use-after-free: variable '") - 1 });
    print_str_raw(name);
    print_str_raw((oc_string){ .ptr = (const uint8_t*)("' used after arena '"), .len = sizeof("' used after arena '") - 1 });
    print_str_raw(arena);
    print_str_raw((oc_string){ .ptr = (const uint8_t*)("' was deinitialized\012"), .len = sizeof("' was deinitialized\012") - 1 });
}

void print_err_borrow_escape(oc_string src, oc_string tgt) {
    print_str_raw((oc_string){ .ptr = (const uint8_t*)("error: borrowed argument '"), .len = sizeof("error: borrowed argument '") - 1 });
    print_str_raw(src);
    print_str_raw((oc_string){ .ptr = (const uint8_t*)("' does not live long enough (escapes into '"), .len = sizeof("' does not live long enough (escapes into '") - 1 });
    print_str_raw(tgt);
    print_str_raw((oc_string){ .ptr = (const uint8_t*)("')\012"), .len = sizeof("')\012") - 1 });
}

void print_err_cross_arena(oc_string arena, oc_string tgt) {
    print_str_raw((oc_string){ .ptr = (const uint8_t*)("error: cross-arena escape: pointer borrowed from arena '"), .len = sizeof("error: cross-arena escape: pointer borrowed from arena '") - 1 });
    print_str_raw(arena);
    print_str_raw((oc_string){ .ptr = (const uint8_t*)("' escapes into longer-lived target '"), .len = sizeof("' escapes into longer-lived target '") - 1 });
    print_str_raw(tgt);
    print_str_raw((oc_string){ .ptr = (const uint8_t*)("'\012"), .len = sizeof("'\012") - 1 });
}

void print_err_ret_local(oc_string src) {
    print_str_raw((oc_string){ .ptr = (const uint8_t*)("error: cannot return reference to local stack variable '"), .len = sizeof("error: cannot return reference to local stack variable '") - 1 });
    print_str_raw(src);
    print_str_raw((oc_string){ .ptr = (const uint8_t*)("'\012"), .len = sizeof("'\012") - 1 });
}

void print_err_ret_arena(oc_string arena) {
    print_str_raw((oc_string){ .ptr = (const uint8_t*)("error: cannot return reference to locally allocated arena '"), .len = sizeof("error: cannot return reference to locally allocated arena '") - 1 });
    print_str_raw(arena);
    print_str_raw((oc_string){ .ptr = (const uint8_t*)("'\012"), .len = sizeof("'\012") - 1 });
}

bool is_scalar_value_type(oc_string t) {
    if (str_eq(t, (oc_string){ .ptr = (const uint8_t*)("i32"), .len = sizeof("i32") - 1 })) {
    return true;
    }
    if (str_eq(t, (oc_string){ .ptr = (const uint8_t*)("i64"), .len = sizeof("i64") - 1 })) {
    return true;
    }
    if (str_eq(t, (oc_string){ .ptr = (const uint8_t*)("u64"), .len = sizeof("u64") - 1 })) {
    return true;
    }
    if (str_eq(t, (oc_string){ .ptr = (const uint8_t*)("usize"), .len = sizeof("usize") - 1 })) {
    return true;
    }
    if (str_eq(t, (oc_string){ .ptr = (const uint8_t*)("u32"), .len = sizeof("u32") - 1 })) {
    return true;
    }
    if (str_eq(t, (oc_string){ .ptr = (const uint8_t*)("u16"), .len = sizeof("u16") - 1 })) {
    return true;
    }
    if (str_eq(t, (oc_string){ .ptr = (const uint8_t*)("u8"), .len = sizeof("u8") - 1 })) {
    return true;
    }
    if (str_eq(t, (oc_string){ .ptr = (const uint8_t*)("i16"), .len = sizeof("i16") - 1 })) {
    return true;
    }
    if (str_eq(t, (oc_string){ .ptr = (const uint8_t*)("i8"), .len = sizeof("i8") - 1 })) {
    return true;
    }
    if (str_eq(t, (oc_string){ .ptr = (const uint8_t*)("bool"), .len = sizeof("bool") - 1 })) {
    return true;
    }
    return false;
}

int32_t validator_find_var(Validator* v, oc_string name) {
    size_t i = ((v)->vars_count);
    while ((i > 0)) {
    i = __OC_SUB(i, 1);
    if (str_eq(((__OC_AT((((v)->vars)), (i))).name), name)) {
    return ({ __auto_type _v = (i); if ((_v < 0 && (int64_t)_v < -2147483648LL) || (!(_v < 0) && (uint64_t)_v > 2147483647ULL)) { fprintf(stderr, "panic: invalid signed cast\n"); exit(1); } (int32_t)_v; });
    }
    }
    return (-1);
}

void validator_set_points_to_stack_local(Validator* v, oc_string name) {
    size_t i = ((v)->vars_count);
    while ((i > 0)) {
    i = __OC_SUB(i, 1);
    if (str_eq(((__OC_AT((((v)->vars)), (i))).name), name)) {
    ((__OC_AT((((v)->vars)), (i))).points_to_stack_local) = true;
    return;
    }
    }
}

bool validator_is_arena_deinit(Validator* v, oc_string name) {
    size_t i = 0;
    while ((i < ((v)->deinit_count))) {
    if (str_eq(__OC_AT((((v)->deinit_arenas)), (i)), name)) {
    return true;
    }
    i = __OC_ADD(i, 1);
    }
    return false;
}

bool validator_mark_deinit(Validator* v, oc_string name) {
    if (validator_is_arena_deinit(v, name)) {
    return true;
    }
    if ((((v)->deinit_count) >= 128)) {
    fwrite("security error: validator deinit table capacity exceeded", 1, sizeof("security error: validator deinit table capacity exceeded") - 1, stdout); fputc('\n', stdout);
    ((v)->has_error) = true;
    return false;
    }
    __OC_AT((((v)->deinit_arenas)), (((v)->deinit_count))) = name;
    ((v)->deinit_count) = __OC_ADD(((v)->deinit_count), 1);
    return true;
}

void validator_invalidate_arena(Validator* v, oc_string arena_name) {
    size_t i = 0;
    while ((i < ((v)->vars_count))) {
    if (str_eq(((__OC_AT((((v)->vars)), (i))).borrowed_from_arena), arena_name)) {
    ((__OC_AT((((v)->vars)), (i))).is_invalidated) = true;
    }
    i = __OC_ADD(i, 1);
    }
}

void validator_invalidate_var(Validator* v, oc_string var_name) {
    size_t i = 0;
    while ((i < ((v)->vars_count))) {
    if (str_eq(((__OC_AT((((v)->vars)), (i))).borrowed_from_var), var_name)) {
    ((__OC_AT((((v)->vars)), (i))).is_invalidated) = true;
    }
    i = __OC_ADD(i, 1);
    }
}

oc_string get_ident_name(AstNode* expr) {
    if ((expr == ((AstNode*)(0)))) {
    return (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 };
    }
    if ((((expr)->tag) == NODE_IDENT)) {
    return ((((expr)->tok)).slice);
    }
    if ((((expr)->tag) == NODE_CAST)) {
    return get_ident_name(((expr)->left));
    }
    if ((((expr)->tag) == NODE_INDEX)) {
    return get_ident_name(((expr)->left));
    }
    if ((((expr)->tag) == NODE_FIELD_ACCESS)) {
    return get_ident_name(((expr)->left));
    }
    if ((((expr)->tag) == NODE_DEREF)) {
    return get_ident_name(((expr)->left));
    }
    return (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 };
}

oc_string get_underlying_address_of(AstNode* expr) {
    if ((expr == ((AstNode*)(0)))) {
    return (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 };
    }
    if ((((expr)->tag) == NODE_ADDRESS_OF)) {
    return get_ident_name(((expr)->left));
    }
    if ((((expr)->tag) == NODE_CAST)) {
    return get_underlying_address_of(((expr)->left));
    }
    return (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 };
}

uint8_t fx_param_effect(FnEffects* fx, oc_string fn_name, size_t idx) {
    if ((idx == 0)) {
    if (str_eq(fn_name, (oc_string){ .ptr = (const uint8_t*)("arena_deinit"), .len = sizeof("arena_deinit") - 1 })) {
    return 2;
    }
    if (str_eq(fn_name, (oc_string){ .ptr = (const uint8_t*)("arena_reset"), .len = sizeof("arena_reset") - 1 })) {
    return 1;
    }
    }
    if ((idx >= 16)) {
    return 0;
    }
    size_t i = 0;
    while ((i < ((fx)->count))) {
    if (str_eq(__OC_AT((((fx)->names)), (i)), fn_name)) {
    return __OC_AT((((fx)->effects)), (__OC_ADD(__OC_MUL(i, 16), idx)));
    }
    i = __OC_ADD(i, 1);
    }
    return 0;
}

bool fx_param_borrows(FnEffects* fx, oc_string fn_name, size_t idx) {
    if ((idx == 0)) {
    if (str_eq(fn_name, (oc_string){ .ptr = (const uint8_t*)("arena_alloc"), .len = sizeof("arena_alloc") - 1 })) {
    return true;
    }
    }
    if ((idx >= 16)) {
    return false;
    }
    size_t i = 0;
    while ((i < ((fx)->count))) {
    if (str_eq(__OC_AT((((fx)->names)), (i)), fn_name)) {
    return __OC_AT((((fx)->borrows)), (__OC_ADD(__OC_MUL(i, 16), idx)));
    }
    i = __OC_ADD(i, 1);
    }
    return false;
}

oc_string get_arena_arg_name(AstNode* arg) {
    oc_string u = get_underlying_address_of(arg);
    if ((((u).len) > 0)) {
    return u;
    }
    return get_ident_name(arg);
}

bool fx_scan(FnEffects* fx, size_t slot, AstNode* params, bool ret_ptr, AstNode* n) {
    bool changed = false;
    AstNode* cur = n;
    while ((cur != ((AstNode*)(0)))) {
    if ((((cur)->tag) == NODE_CALL)) {
    AstNode* arg = ((cur)->left);
    size_t ai = 0;
    while ((arg != ((AstNode*)(0)))) {
    uint8_t eff = fx_param_effect(fx, ((((cur)->tok)).slice), ai);
    bool allocs = false;
    if (ret_ptr) {
    allocs = fx_param_borrows(fx, ((((cur)->tok)).slice), ai);
    }
    bool relevant = (eff > 0);
    if (allocs) {
    relevant = true;
    }
    if (relevant) {
    oc_string name = get_arena_arg_name(arg);
    AstNode* prm = params;
    size_t pi = 0;
    while ((prm != ((AstNode*)(0)))) {
    if ((pi < 16)) {
    if (str_eq(((((prm)->tok)).slice), name)) {
    if ((__OC_AT((((fx)->effects)), (__OC_ADD(__OC_MUL(slot, 16), pi))) < eff)) {
    __OC_AT((((fx)->effects)), (__OC_ADD(__OC_MUL(slot, 16), pi))) = eff;
    changed = true;
    }
    if (allocs) {
    if ((!__OC_AT((((fx)->borrows)), (__OC_ADD(__OC_MUL(slot, 16), pi))))) {
    __OC_AT((((fx)->borrows)), (__OC_ADD(__OC_MUL(slot, 16), pi))) = true;
    changed = true;
    }
    }
    }
    }
    prm = ((prm)->next);
    pi = __OC_ADD(pi, 1);
    }
    }
    arg = ((arg)->next);
    ai = __OC_ADD(ai, 1);
    }
    }
    if (fx_scan(fx, slot, params, ret_ptr, ((cur)->left))) {
    changed = true;
    }
    if (fx_scan(fx, slot, params, ret_ptr, ((cur)->right))) {
    changed = true;
    }
    cur = ((cur)->next);
    }
    return changed;
}

oc_string get_arena_source(FnEffects* fx, AstNode* expr) {
    if ((expr == ((AstNode*)(0)))) {
    return (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 };
    }
    if ((((expr)->tag) == NODE_CALL)) {
    AstNode* arg = ((expr)->left);
    size_t ai = 0;
    while ((arg != ((AstNode*)(0)))) {
    if (fx_param_borrows(fx, ((((expr)->tok)).slice), ai)) {
    oc_string name = get_arena_arg_name(arg);
    if ((((name).len) > 0)) {
    return name;
    }
    }
    arg = ((arg)->next);
    ai = __OC_ADD(ai, 1);
    }
    return (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 };
    }
    if ((((expr)->tag) == NODE_CAST)) {
    return get_arena_source(fx, ((expr)->left));
    }
    return (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 };
}

oc_string get_expr_arena_borrow(Validator* v, AstNode* expr) {
    if ((expr == ((AstNode*)(0)))) {
    return (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 };
    }
    oc_string direct = get_arena_source(((v)->fx), expr);
    if ((((direct).len) > 0)) {
    return direct;
    }
    if ((((expr)->tag) == NODE_IDENT)) {
    int32_t idx = validator_find_var(v, ((((expr)->tok)).slice));
    if ((idx >= 0)) {
    return ((__OC_AT((((v)->vars)), (({ __auto_type _v = (idx); if (_v < 0) { fprintf(stderr, "panic: invalid unsigned cast\n"); exit(1); } (size_t)_v; })))).borrowed_from_arena);
    }
    return (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 };
    }
    if ((((expr)->tag) == NODE_CAST)) {
    return get_expr_arena_borrow(v, ((expr)->left));
    }
    if ((((expr)->tag) == NODE_SLICE)) {
    return get_expr_arena_borrow(v, ((expr)->left));
    }
    if ((((expr)->tag) == NODE_STRUCT_INIT)) {
    AstNode* f = ((expr)->left);
    while ((f != ((AstNode*)(0)))) {
    oc_string wrapped = get_expr_arena_borrow(v, ((f)->left));
    if ((((wrapped).len) > 0)) {
    return wrapped;
    }
    f = ((f)->next);
    }
    return (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 };
    }
    if ((((expr)->tag) == NODE_ARRAY_INIT)) {
    AstNode* el = ((expr)->left);
    while ((el != ((AstNode*)(0)))) {
    oc_string held = get_expr_arena_borrow(v, el);
    if ((((held).len) > 0)) {
    return held;
    }
    el = ((el)->next);
    }
    return (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 };
    }
    bool is_load = (((expr)->tag) == NODE_INDEX);
    if ((((expr)->tag) == NODE_FIELD_ACCESS)) {
    is_load = true;
    }
    if (is_load) {
    oc_string loaded_t = validator_stack_place_type(v, expr);
    if ((((loaded_t).len) == 0)) {
    return (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 };
    }
    if (is_scalar_value_type(loaded_t)) {
    return (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 };
    }
    int32_t root_idx = validator_find_var(v, get_ident_name(expr));
    if ((root_idx < 0)) {
    return (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 };
    }
    return ((__OC_AT((((v)->vars)), (({ __auto_type _v = (root_idx); if (_v < 0) { fprintf(stderr, "panic: invalid unsigned cast\n"); exit(1); } (size_t)_v; })))).borrowed_from_arena);
    }
    if ((((expr)->tag) == NODE_ADDRESS_OF)) {
    oc_string id = get_ident_name(((expr)->left));
    if ((((id).len) > 0)) {
    int32_t idx = validator_find_var(v, id);
    if ((idx >= 0)) {
    return ((__OC_AT((((v)->vars)), (({ __auto_type _v = (idx); if (_v < 0) { fprintf(stderr, "panic: invalid unsigned cast\n"); exit(1); } (size_t)_v; })))).borrowed_from_arena);
    }
    }
    return get_expr_arena_borrow(v, ((expr)->left));
    }
    if ((((expr)->tag) == NODE_BINARY)) {
    oc_string l = get_expr_arena_borrow(v, ((expr)->left));
    if ((((l).len) > 0)) {
    return l;
    }
    return get_expr_arena_borrow(v, ((expr)->right));
    }
    return (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 };
}

oc_string validator_field_type(Validator* v, oc_string s_name, oc_string f_name) {
    if (str_eq(s_name, (oc_string){ .ptr = (const uint8_t*)("string"), .len = sizeof("string") - 1 })) {
    if (str_eq(f_name, (oc_string){ .ptr = (const uint8_t*)("len"), .len = sizeof("len") - 1 })) {
    return (oc_string){ .ptr = (const uint8_t*)("usize"), .len = sizeof("usize") - 1 };
    }
    if (str_eq(f_name, (oc_string){ .ptr = (const uint8_t*)("ptr"), .len = sizeof("ptr") - 1 })) {
    return (oc_string){ .ptr = (const uint8_t*)("*u8"), .len = sizeof("*u8") - 1 };
    }
    return (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 };
    }
    AstNode* decl = ((((v)->program))->left);
    while ((decl != ((AstNode*)(0)))) {
    if ((((decl)->tag) == NODE_STRUCT_DECL)) {
    if (str_eq(((((decl)->tok)).slice), s_name)) {
    AstNode* f = ((decl)->left);
    while ((f != ((AstNode*)(0)))) {
    if (str_eq(((((f)->tok)).slice), f_name)) {
    return ((f)->type_name);
    }
    f = ((f)->next);
    }
    }
    }
    decl = ((decl)->next);
    }
    return (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 };
}

oc_string validator_stack_place_type(Validator* v, AstNode* expr) {
    if ((expr == ((AstNode*)(0)))) {
    return (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 };
    }
    if ((((expr)->tag) == NODE_IDENT)) {
    int32_t idx = validator_find_var(v, ((((expr)->tok)).slice));
    if ((idx < 0)) {
    return (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 };
    }
    if ((!((__OC_AT((((v)->vars)), (({ __auto_type _v = (idx); if (_v < 0) { fprintf(stderr, "panic: invalid unsigned cast\n"); exit(1); } (size_t)_v; })))).is_stack_local))) {
    return (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 };
    }
    return ((__OC_AT((((v)->vars)), (({ __auto_type _v = (idx); if (_v < 0) { fprintf(stderr, "panic: invalid unsigned cast\n"); exit(1); } (size_t)_v; })))).type_name);
    }
    if ((((expr)->tag) == NODE_FIELD_ACCESS)) {
    oc_string base = validator_stack_place_type(v, ((expr)->left));
    if ((((base).len) == 0)) {
    return (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 };
    }
    if (starts_with(base, (oc_string){ .ptr = (const uint8_t*)("*"), .len = sizeof("*") - 1 })) {
    return (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 };
    }
    if (starts_with(base, (oc_string){ .ptr = (const uint8_t*)("&"), .len = sizeof("&") - 1 })) {
    return (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 };
    }
    return validator_field_type(v, base, ((((expr)->tok)).slice));
    }
    if ((((expr)->tag) == NODE_INDEX)) {
    oc_string base = validator_stack_place_type(v, ((expr)->left));
    if ((!starts_with(base, (oc_string){ .ptr = (const uint8_t*)("["), .len = sizeof("[") - 1 }))) {
    return (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 };
    }
    size_t close = 1;
    while ((close < ((base).len))) {
    if ((__oc_index_str(base, close) == 93)) {
    break;
    }
    close = __OC_ADD(close, 1);
    }
    if ((__OC_ADD(close, 1) >= ((base).len))) {
    return (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 };
    }
    return __oc_slice_str(base, __OC_ADD(close, 1), ((base).len));
    }
    return (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 };
}

bool is_float_type_name(oc_string t) {
    if (str_eq(t, (oc_string){ .ptr = (const uint8_t*)("f64"), .len = sizeof("f64") - 1 })) {
    return true;
    }
    return str_eq(t, (oc_string){ .ptr = (const uint8_t*)("f32"), .len = sizeof("f32") - 1 });
}

bool is_int_type_name(oc_string t) {
    if ((!is_scalar_value_type(t))) {
    return false;
    }
    return (!str_eq(t, (oc_string){ .ptr = (const uint8_t*)("bool"), .len = sizeof("bool") - 1 }));
}

oc_string strip_ptr_type(oc_string t) {
    if ((((t).len) < 2)) {
    return t;
    }
    bool is_ptr = (__oc_index_str(t, 0) == 42);
    if ((__oc_index_str(t, 0) == 38)) {
    is_ptr = true;
    }
    if ((!is_ptr)) {
    return t;
    }
    if ((((t).len) > 5)) {
    if (str_eq(__oc_slice_str(t, 1, 5), (oc_string){ .ptr = (const uint8_t*)("mut "), .len = sizeof("mut ") - 1 })) {
    return __oc_slice_str(t, 5, ((t).len));
    }
    }
    return __oc_slice_str(t, 1, ((t).len));
}

AstNode* program_find_fn(AstNode* program, oc_string name) {
    AstNode* decl = ((program)->left);
    while ((decl != ((AstNode*)(0)))) {
    if ((((decl)->tag) == NODE_FN)) {
    if (str_eq(((((decl)->tok)).slice), name)) {
    return decl;
    }
    }
    decl = ((decl)->next);
    }
    return ((AstNode*)(0));
}

AstNode* validator_find_fn(Validator* v, oc_string name) {
    return program_find_fn(((v)->program), name);
}

bool is_builtin_fn(oc_string name) {
    if (str_eq(name, (oc_string){ .ptr = (const uint8_t*)("sqrt_f64"), .len = sizeof("sqrt_f64") - 1 })) {
    return true;
    }
    if (str_eq(name, (oc_string){ .ptr = (const uint8_t*)("f64_to_bits"), .len = sizeof("f64_to_bits") - 1 })) {
    return true;
    }
    if (str_eq(name, (oc_string){ .ptr = (const uint8_t*)("wrapping_add"), .len = sizeof("wrapping_add") - 1 })) {
    return true;
    }
    if (str_eq(name, (oc_string){ .ptr = (const uint8_t*)("wrapping_sub"), .len = sizeof("wrapping_sub") - 1 })) {
    return true;
    }
    if (str_eq(name, (oc_string){ .ptr = (const uint8_t*)("wrapping_mul"), .len = sizeof("wrapping_mul") - 1 })) {
    return true;
    }
    return false;
}

bool is_hex_char(uint8_t c) {
    if (is_digit(c)) {
    return true;
    }
    if ((c >= 97)) {
    if ((c <= 102)) {
    return true;
    }
    }
    if ((c >= 65)) {
    return (c <= 70);
    }
    return false;
}

bool check_string_escapes(SourceMap* m, AstNode* node) {
    bool ok = true;
    AstNode* n = node;
    while ((n != ((AstNode*)(0)))) {
    if ((((n)->tag) == NODE_STRING)) {
    oc_string text = ((((n)->tok)).slice);
    Token at = ((n)->tok);
    ((at).col) = __OC_ADD(((at).col), 1);
    size_t k = 1;
    while ((__OC_ADD(k, 1) < ((text).len))) {
    uint8_t c = __oc_index_str(text, k);
    if ((c == 10)) {
    ((at).line) = __OC_ADD(((at).line), 1);
    ((at).col) = 1;
    k = __OC_ADD(k, 1);
    }
    else {
    if ((c == 92)) {
    uint8_t e = __oc_index_str(text, __OC_ADD(k, 1));
    bool good = false;
    size_t width = 2;
    if ((e == 110)) {
    good = true;
    }
    if ((e == 116)) {
    good = true;
    }
    if ((e == 114)) {
    good = true;
    }
    if ((e == 48)) {
    good = true;
    }
    if ((e == 92)) {
    good = true;
    }
    if ((e == 34)) {
    good = true;
    }
    if ((e == 120)) {
    width = 4;
    if ((__OC_ADD(k, 3) < __OC_SUB(((text).len), 1))) {
    if (is_hex_char(__oc_index_str(text, __OC_ADD(k, 2)))) {
    if (is_hex_char(__oc_index_str(text, __OC_ADD(k, 3)))) {
    good = true;
    }
    }
    }
    if ((!good)) {
    print_src_loc(m, at);
    print_str_raw((oc_string){ .ptr = (const uint8_t*)("error: \134x needs exactly two hex digits, as in \134x41\012"), .len = sizeof("error: \134x needs exactly two hex digits, as in \134x41\012") - 1 });
    ok = false;
    good = true;
    }
    }
    if ((!good)) {
    print_src_loc(m, at);
    print_str_raw((oc_string){ .ptr = (const uint8_t*)("error: unknown escape sequence '"), .len = sizeof("error: unknown escape sequence '") - 1 });
    print_str_raw(__oc_slice_str(text, k, __OC_ADD(k, 2)));
    print_str_raw((oc_string){ .ptr = (const uint8_t*)("' (oC has \134n \134t \134r \1340 \134\134 \134\042 and \134xNN)\012"), .len = sizeof("' (oC has \134n \134t \134r \1340 \134\134 \134\042 and \134xNN)\012") - 1 });
    ok = false;
    }
    k = __OC_ADD(k, width);
    ((at).col) = __OC_ADD(((at).col), ({ __auto_type _v = (width); if (_v < 0 || (uint64_t)_v > 4294967295ULL) { fprintf(stderr, "panic: invalid unsigned cast\n"); exit(1); } (uint32_t)_v; }));
    }
    else {
    k = __OC_ADD(k, 1);
    ((at).col) = __OC_ADD(((at).col), 1);
    }
    }
    }
    }
    if ((!check_string_escapes(m, ((n)->left)))) {
    ok = false;
    }
    if ((!check_string_escapes(m, ((n)->right)))) {
    ok = false;
    }
    n = ((n)->next);
    }
    return ok;
}

bool check_unique_names(AstNode* root, SourceMap* m) {
    bool ok = true;
    AstNode* d = ((root)->left);
    while ((d != ((AstNode*)(0)))) {
    bool named = (((d)->tag) == NODE_FN);
    if ((((d)->tag) == NODE_STRUCT_DECL)) {
    named = true;
    }
    if ((((d)->tag) == NODE_LET)) {
    named = true;
    }
    bool is_decl_only = false;
    if ((((d)->tag) == NODE_FN)) {
    is_decl_only = (((d)->right) == ((AstNode*)(0)));
    }
    if (named) {
    if ((!is_decl_only)) {
    AstNode* e = ((root)->left);
    while ((e != d)) {
    bool e_named = (((e)->tag) == NODE_FN);
    if ((((e)->tag) == NODE_STRUCT_DECL)) {
    e_named = true;
    }
    if ((((e)->tag) == NODE_LET)) {
    e_named = true;
    }
    if ((((e)->tag) == NODE_FN)) {
    if ((((e)->right) == ((AstNode*)(0)))) {
    e_named = false;
    }
    }
    if (e_named) {
    if (str_eq(((((e)->tok)).slice), ((((d)->tok)).slice))) {
    print_src_loc(m, ((d)->tok));
    print_str_raw((oc_string){ .ptr = (const uint8_t*)("error: '"), .len = sizeof("error: '") - 1 });
    print_str_raw(((((d)->tok)).slice));
    print_str_raw((oc_string){ .ptr = (const uint8_t*)("' is already defined at "), .len = sizeof("' is already defined at ") - 1 });
    print_src_pos(m, ((e)->tok));
    print_str_raw((oc_string){ .ptr = (const uint8_t*)("\012"), .len = sizeof("\012") - 1 });
    ok = false;
    break;
    }
    }
    e = ((e)->next);
    }
    }
    }
    d = ((d)->next);
    }
    return ok;
}

bool check_calls_resolved(AstNode* program, SourceMap* m, AstNode* node) {
    bool ok = true;
    AstNode* n = node;
    while ((n != ((AstNode*)(0)))) {
    if ((((n)->tag) == NODE_CALL)) {
    if ((!is_builtin_fn(((((n)->tok)).slice)))) {
    if ((program_find_fn(program, ((((n)->tok)).slice)) == ((AstNode*)(0)))) {
    print_src_loc(m, ((n)->tok));
    print_str_raw((oc_string){ .ptr = (const uint8_t*)("error: undefined function '"), .len = sizeof("error: undefined function '") - 1 });
    print_str_raw(((((n)->tok)).slice));
    print_str_raw((oc_string){ .ptr = (const uint8_t*)("' (declare it with `fn`, or with `extern fn` if it lives outside oC)\012"), .len = sizeof("' (declare it with `fn`, or with `extern fn` if it lives outside oC)\012") - 1 });
    ok = false;
    }
    }
    }
    if ((!check_calls_resolved(program, m, ((n)->left)))) {
    ok = false;
    }
    if ((!check_calls_resolved(program, m, ((n)->right)))) {
    ok = false;
    }
    n = ((n)->next);
    }
    return ok;
}

oc_string validator_expr_type(Validator* v, AstNode* expr) {
    if ((expr == ((AstNode*)(0)))) {
    return (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 };
    }
    if ((((expr)->tag) == NODE_FLOAT)) {
    if ((((((expr)->type_name)).len) > 0)) {
    return ((expr)->type_name);
    }
    return (oc_string){ .ptr = (const uint8_t*)("f64"), .len = sizeof("f64") - 1 };
    }
    if ((((expr)->tag) == NODE_BOOL)) {
    return (oc_string){ .ptr = (const uint8_t*)("bool"), .len = sizeof("bool") - 1 };
    }
    if ((((expr)->tag) == NODE_STRING)) {
    return (oc_string){ .ptr = (const uint8_t*)("string"), .len = sizeof("string") - 1 };
    }
    if ((((expr)->tag) == NODE_CAST)) {
    return ((expr)->type_name);
    }
    if ((((expr)->tag) == NODE_STRUCT_INIT)) {
    return ((((expr)->tok)).slice);
    }
    if ((((expr)->tag) == NODE_IDENT)) {
    int32_t idx = validator_find_var(v, ((((expr)->tok)).slice));
    if ((idx >= 0)) {
    return ((__OC_AT((((v)->vars)), (({ __auto_type _v = (idx); if (_v < 0) { fprintf(stderr, "panic: invalid unsigned cast\n"); exit(1); } (size_t)_v; })))).type_name);
    }
    AstNode* decl = ((((v)->program))->left);
    while ((decl != ((AstNode*)(0)))) {
    if ((((decl)->tag) == NODE_LET)) {
    if (str_eq(((((decl)->tok)).slice), ((((expr)->tok)).slice))) {
    return ((decl)->type_name);
    }
    }
    decl = ((decl)->next);
    }
    return (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 };
    }
    if ((((expr)->tag) == NODE_CALL)) {
    if (str_eq(((((expr)->tok)).slice), (oc_string){ .ptr = (const uint8_t*)("sqrt_f64"), .len = sizeof("sqrt_f64") - 1 })) {
    return (oc_string){ .ptr = (const uint8_t*)("f64"), .len = sizeof("f64") - 1 };
    }
    if (str_eq(((((expr)->tok)).slice), (oc_string){ .ptr = (const uint8_t*)("f64_to_bits"), .len = sizeof("f64_to_bits") - 1 })) {
    return (oc_string){ .ptr = (const uint8_t*)("u64"), .len = sizeof("u64") - 1 };
    }
    AstNode* callee = validator_find_fn(v, ((((expr)->tok)).slice));
    if ((callee == ((AstNode*)(0)))) {
    return (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 };
    }
    return ((callee)->type_name);
    }
    if ((((expr)->tag) == NODE_UNARY)) {
    if ((((((expr)->tok)).tag) == TOK_BANG)) {
    return (oc_string){ .ptr = (const uint8_t*)("bool"), .len = sizeof("bool") - 1 };
    }
    return validator_expr_type(v, ((expr)->left));
    }
    if ((((expr)->tag) == NODE_DEREF)) {
    return strip_ptr_type(validator_expr_type(v, ((expr)->left)));
    }
    if ((((expr)->tag) == NODE_FIELD_ACCESS)) {
    return validator_field_type(v, strip_ptr_type(validator_expr_type(v, ((expr)->left))), ((((expr)->tok)).slice));
    }
    if ((((expr)->tag) == NODE_INDEX)) {
    oc_string base = validator_expr_type(v, ((expr)->left));
    if (str_eq(base, (oc_string){ .ptr = (const uint8_t*)("string"), .len = sizeof("string") - 1 })) {
    return (oc_string){ .ptr = (const uint8_t*)("u8"), .len = sizeof("u8") - 1 };
    }
    if (starts_with(base, (oc_string){ .ptr = (const uint8_t*)("["), .len = sizeof("[") - 1 })) {
    size_t close = 1;
    while ((close < ((base).len))) {
    if ((__oc_index_str(base, close) == 93)) {
    break;
    }
    close = __OC_ADD(close, 1);
    }
    if ((__OC_ADD(close, 1) >= ((base).len))) {
    return (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 };
    }
    return __oc_slice_str(base, __OC_ADD(close, 1), ((base).len));
    }
    if (starts_with(base, (oc_string){ .ptr = (const uint8_t*)("*"), .len = sizeof("*") - 1 })) {
    return strip_ptr_type(base);
    }
    return (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 };
    }
    if ((((expr)->tag) == NODE_BINARY)) {
    uint32_t op = ((((expr)->tok)).tag);
    bool is_cmp = (op == TOK_EQEQ);
    if ((op == TOK_NEQ)) {
    is_cmp = true;
    }
    if ((op == TOK_LT)) {
    is_cmp = true;
    }
    if ((op == TOK_LTE)) {
    is_cmp = true;
    }
    if ((op == TOK_GT)) {
    is_cmp = true;
    }
    if ((op == TOK_GTE)) {
    is_cmp = true;
    }
    if (is_cmp) {
    return (oc_string){ .ptr = (const uint8_t*)("bool"), .len = sizeof("bool") - 1 };
    }
    oc_string lt = validator_expr_type(v, ((expr)->left));
    oc_string rt = validator_expr_type(v, ((expr)->right));
    if (is_untyped_float(((expr)->left))) {
    if (is_float_type_name(rt)) {
    return rt;
    }
    }
    if (is_untyped_float(((expr)->right))) {
    if (is_float_type_name(lt)) {
    return lt;
    }
    }
    if (str_eq(lt, (oc_string){ .ptr = (const uint8_t*)("f64"), .len = sizeof("f64") - 1 })) {
    return lt;
    }
    if (str_eq(rt, (oc_string){ .ptr = (const uint8_t*)("f64"), .len = sizeof("f64") - 1 })) {
    return rt;
    }
    if (is_float_type_name(lt)) {
    return lt;
    }
    if (is_float_type_name(rt)) {
    return rt;
    }
    if ((((lt).len) > 0)) {
    return lt;
    }
    return rt;
    }
    return (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 };
}

bool is_int_literal(AstNode* expr) {
    if ((expr == ((AstNode*)(0)))) {
    return false;
    }
    if ((((expr)->tag) == NODE_UNARY)) {
    if ((((((expr)->tok)).tag) == TOK_MINUS)) {
    return is_int_literal(((expr)->left));
    }
    return false;
    }
    return (((expr)->tag) == NODE_INT);
}

bool is_untyped_float(AstNode* expr) {
    if ((expr == ((AstNode*)(0)))) {
    return false;
    }
    if ((((expr)->tag) == NODE_FLOAT)) {
    return (((((expr)->type_name)).len) == 0);
    }
    if ((((expr)->tag) == NODE_UNARY)) {
    if ((((((expr)->tok)).tag) == TOK_MINUS)) {
    return is_untyped_float(((expr)->left));
    }
    return false;
    }
    if ((((expr)->tag) != NODE_BINARY)) {
    return false;
    }
    bool arith = (((((expr)->tok)).tag) == TOK_PLUS);
    if ((((((expr)->tok)).tag) == TOK_MINUS)) {
    arith = true;
    }
    if ((((((expr)->tok)).tag) == TOK_STAR)) {
    arith = true;
    }
    if ((((((expr)->tok)).tag) == TOK_SLASH)) {
    arith = true;
    }
    if ((!arith)) {
    return false;
    }
    if (is_untyped_float(((expr)->left))) {
    if (is_untyped_float(((expr)->right))) {
    return true;
    }
    return is_int_literal(((expr)->right));
    }
    if (is_untyped_float(((expr)->right))) {
    return is_int_literal(((expr)->left));
    }
    return false;
}

void bind_float_literals(AstNode* expr, oc_string t) {
    if ((expr == ((AstNode*)(0)))) {
    return;
    }
    if ((((expr)->tag) == NODE_FLOAT)) {
    if ((((((expr)->type_name)).len) == 0)) {
    ((expr)->type_name) = t;
    }
    return;
    }
    if ((((expr)->tag) == NODE_UNARY)) {
    bind_float_literals(((expr)->left), t);
    return;
    }
    if ((((expr)->tag) == NODE_BINARY)) {
    bind_float_literals(((expr)->left), t);
    bind_float_literals(((expr)->right), t);
    }
}

void bind_to_target(oc_string target, AstNode* value) {
    if ((!str_eq(target, (oc_string){ .ptr = (const uint8_t*)("f32"), .len = sizeof("f32") - 1 }))) {
    return;
    }
    if (is_untyped_float(value)) {
    bind_float_literals(value, (oc_string){ .ptr = (const uint8_t*)("f32"), .len = sizeof("f32") - 1 });
    }
}

bool is_scalar_load(Validator* v, AstNode* expr) {
    if ((expr == ((AstNode*)(0)))) {
    return false;
    }
    bool load = (((expr)->tag) == NODE_INDEX);
    if ((((expr)->tag) == NODE_FIELD_ACCESS)) {
    load = true;
    }
    if ((!load)) {
    return false;
    }
    return is_scalar_value_type(validator_expr_type(v, expr));
}

bool validator_is_struct_type(Validator* v, oc_string t) {
    if ((((t).len) == 0)) {
    return false;
    }
    AstNode* decl = ((((v)->program))->left);
    while ((decl != ((AstNode*)(0)))) {
    if ((((decl)->tag) == NODE_STRUCT_DECL)) {
    if (str_eq(((((decl)->tok)).slice), t)) {
    return true;
    }
    }
    decl = ((decl)->next);
    }
    return false;
}

void validator_print_instance(Validator* v) {
    if ((((v)->gen) == ((Generics*)(0)))) {
    return;
    }
    size_t i = 0;
    while ((i < ((((v)->gen))->inst_count))) {
    if (str_eq(__OC_AT((((((v)->gen))->inst_name)), (i)), ((v)->fn_name))) {
    print_str_raw((oc_string){ .ptr = (const uint8_t*)(" (in "), .len = sizeof(" (in ") - 1 });
    print_str_raw(__OC_AT((((((v)->gen))->inst_tmpl)), (i)));
    print_str_raw((oc_string){ .ptr = (const uint8_t*)("<"), .len = sizeof("<") - 1 });
    print_str_raw(__OC_AT((((((v)->gen))->inst_args)), (i)));
    print_str_raw((oc_string){ .ptr = (const uint8_t*)(">, instantiated at "), .len = sizeof(">, instantiated at ") - 1 });
    print_src_pos(((v)->srcmap), __OC_AT((((((v)->gen))->inst_site)), (i)));
    print_str_raw((oc_string){ .ptr = (const uint8_t*)(")"), .len = sizeof(")") - 1 });
    return;
    }
    i = __OC_ADD(i, 1);
    }
}

bool validator_check_operands(Validator* v, AstNode* expr) {
    oc_string lt = validator_expr_type(v, ((expr)->left));
    oc_string rt = validator_expr_type(v, ((expr)->right));
    oc_string bad = (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 };
    if (str_eq(rt, (oc_string){ .ptr = (const uint8_t*)("string"), .len = sizeof("string") - 1 })) {
    bad = rt;
    }
    if (validator_is_struct_type(v, rt)) {
    bad = rt;
    }
    if (str_eq(lt, (oc_string){ .ptr = (const uint8_t*)("string"), .len = sizeof("string") - 1 })) {
    bad = lt;
    }
    if (validator_is_struct_type(v, lt)) {
    bad = lt;
    }
    if ((((bad).len) == 0)) {
    return true;
    }
    print_src_loc(((v)->srcmap), ((expr)->tok));
    print_str_raw((oc_string){ .ptr = (const uint8_t*)("error: '"), .len = sizeof("error: '") - 1 });
    print_str_raw(((((expr)->tok)).slice));
    print_str_raw((oc_string){ .ptr = (const uint8_t*)("' is not defined for "), .len = sizeof("' is not defined for ") - 1 });
    print_str_raw(bad);
    if (str_eq(bad, (oc_string){ .ptr = (const uint8_t*)("string"), .len = sizeof("string") - 1 })) {
    print_str_raw((oc_string){ .ptr = (const uint8_t*)(" (compare strings with str_eq from std/str.oc)"), .len = sizeof(" (compare strings with str_eq from std/str.oc)") - 1 });
    }
    validator_print_instance(v);
    print_str_raw((oc_string){ .ptr = (const uint8_t*)("\012"), .len = sizeof("\012") - 1 });
    ((v)->has_error) = true;
    return false;
}

bool validator_is_float(Validator* v, AstNode* expr) {
    return is_float_type_name(validator_expr_type(v, expr));
}

bool validator_check_no_float_to_int(Validator* v, oc_string target, AstNode* value, oc_string place) {
    if ((!is_int_type_name(target))) {
    return true;
    }
    if ((!validator_is_float(v, value))) {
    return true;
    }
    print_str_raw((oc_string){ .ptr = (const uint8_t*)("error: implicit conversion from float to integer in "), .len = sizeof("error: implicit conversion from float to integer in ") - 1 });
    print_str_raw(place);
    print_str_raw((oc_string){ .ptr = (const uint8_t*)("; write .as("), .len = sizeof("; write .as(") - 1 });
    print_str_raw(target);
    print_str_raw((oc_string){ .ptr = (const uint8_t*)("), which checks that the value fits\012"), .len = sizeof("), which checks that the value fits\012") - 1 });
    ((v)->has_error) = true;
    return false;
}

bool int_converts_exactly(oc_string int_t, oc_string float_t) {
    if (str_eq(int_t, (oc_string){ .ptr = (const uint8_t*)("i8"), .len = sizeof("i8") - 1 })) {
    return true;
    }
    if (str_eq(int_t, (oc_string){ .ptr = (const uint8_t*)("u8"), .len = sizeof("u8") - 1 })) {
    return true;
    }
    if (str_eq(int_t, (oc_string){ .ptr = (const uint8_t*)("i16"), .len = sizeof("i16") - 1 })) {
    return true;
    }
    if (str_eq(int_t, (oc_string){ .ptr = (const uint8_t*)("u16"), .len = sizeof("u16") - 1 })) {
    return true;
    }
    if (str_eq(float_t, (oc_string){ .ptr = (const uint8_t*)("f32"), .len = sizeof("f32") - 1 })) {
    return false;
    }
    if (str_eq(int_t, (oc_string){ .ptr = (const uint8_t*)("i32"), .len = sizeof("i32") - 1 })) {
    return true;
    }
    return str_eq(int_t, (oc_string){ .ptr = (const uint8_t*)("u32"), .len = sizeof("u32") - 1 });
}

bool int_literal_converts_exactly(oc_string text, uint64_t bits) {
    uint64_t base = 10;
    size_t i = 0;
    if ((((text).len) > 2)) {
    if ((__oc_index_str(text, 0) == 48)) {
    if ((__oc_index_str(text, 1) == 120)) {
    base = 16;
    i = 2;
    }
    if ((__oc_index_str(text, 1) == 88)) {
    base = 16;
    i = 2;
    }
    if ((__oc_index_str(text, 1) == 98)) {
    base = 2;
    i = 2;
    }
    }
    }
    uint64_t val = 0;
    while ((i < ((text).len))) {
    uint8_t c = __oc_index_str(text, i);
    uint64_t d = 99;
    if ((c >= 48)) {
    if ((c <= 57)) {
    d = ({ __auto_type _v = (__OC_SUB(c, 48)); if (_v < 0) { fprintf(stderr, "panic: invalid unsigned cast\n"); exit(1); } (uint64_t)_v; });
    }
    }
    if ((c >= 97)) {
    if ((c <= 102)) {
    d = ({ __auto_type _v = (__OC_SUB(c, 87)); if (_v < 0) { fprintf(stderr, "panic: invalid unsigned cast\n"); exit(1); } (uint64_t)_v; });
    }
    }
    if ((c >= 65)) {
    if ((c <= 70)) {
    d = ({ __auto_type _v = (__OC_SUB(c, 55)); if (_v < 0) { fprintf(stderr, "panic: invalid unsigned cast\n"); exit(1); } (uint64_t)_v; });
    }
    }
    if ((d < base)) {
    if ((val > __OC_DIV(__OC_SUB(18446744073709551615ULL, d), base))) {
    return false;
    }
    val = __OC_ADD(__OC_MUL(val, base), d);
    }
    i = __OC_ADD(i, 1);
    }
    while ((val > 0)) {
    if (((val & 1) == 1)) {
    break;
    }
    val = (val >> 1);
    }
    return ((val >> bits) == 0);
}

bool validator_check_lossless_to_float(Validator* v, oc_string target, AstNode* value, oc_string place) {
    if ((!is_float_type_name(target))) {
    return true;
    }
    if ((value == ((AstNode*)(0)))) {
    return true;
    }
    AstNode* lit = value;
    if ((((lit)->tag) == NODE_UNARY)) {
    if ((((((lit)->tok)).tag) == TOK_MINUS)) {
    lit = ((lit)->left);
    }
    }
    if ((((lit)->tag) == NODE_FLOAT)) {
    return true;
    }
    if ((((lit)->tag) == NODE_INT)) {
    uint64_t bits = 53;
    if (str_eq(target, (oc_string){ .ptr = (const uint8_t*)("f32"), .len = sizeof("f32") - 1 })) {
    bits = 24;
    }
    if (int_literal_converts_exactly(((((lit)->tok)).slice), bits)) {
    return true;
    }
    print_src_loc(((v)->srcmap), ((lit)->tok));
    print_str_raw((oc_string){ .ptr = (const uint8_t*)("error: integer literal "), .len = sizeof("error: integer literal ") - 1 });
    print_str_raw(((((lit)->tok)).slice));
    print_str_raw((oc_string){ .ptr = (const uint8_t*)(" in "), .len = sizeof(" in ") - 1 });
    print_str_raw(place);
    print_str_raw((oc_string){ .ptr = (const uint8_t*)(" is not exactly representable as "), .len = sizeof(" is not exactly representable as ") - 1 });
    print_str_raw(target);
    print_str_raw((oc_string){ .ptr = (const uint8_t*)("; write ("), .len = sizeof("; write (") - 1 });
    print_str_raw(((((lit)->tok)).slice));
    print_str_raw((oc_string){ .ptr = (const uint8_t*)(").as("), .len = sizeof(").as(") - 1 });
    print_str_raw(target);
    print_str_raw((oc_string){ .ptr = (const uint8_t*)(") to accept the rounding\012"), .len = sizeof(") to accept the rounding\012") - 1 });
    ((v)->has_error) = true;
    return false;
    }
    oc_string from = validator_expr_type(v, value);
    if (is_int_type_name(from)) {
    if (int_converts_exactly(from, target)) {
    return true;
    }
    }
    else {
    if (str_eq(from, (oc_string){ .ptr = (const uint8_t*)("f64"), .len = sizeof("f64") - 1 })) {
    if ((!str_eq(target, (oc_string){ .ptr = (const uint8_t*)("f32"), .len = sizeof("f32") - 1 }))) {
    return true;
    }
    }
    else {
    return true;
    }
    }
    print_src_loc(((v)->srcmap), ((value)->tok));
    print_str_raw((oc_string){ .ptr = (const uint8_t*)("error: implicit conversion from "), .len = sizeof("error: implicit conversion from ") - 1 });
    print_str_raw(from);
    print_str_raw((oc_string){ .ptr = (const uint8_t*)(" to "), .len = sizeof(" to ") - 1 });
    print_str_raw(target);
    print_str_raw((oc_string){ .ptr = (const uint8_t*)(" in "), .len = sizeof(" in ") - 1 });
    print_str_raw(place);
    print_str_raw((oc_string){ .ptr = (const uint8_t*)(" can lose precision; write .as("), .len = sizeof(" can lose precision; write .as(") - 1 });
    print_str_raw(target);
    print_str_raw((oc_string){ .ptr = (const uint8_t*)("), which rounds to the nearest "), .len = sizeof("), which rounds to the nearest ") - 1 });
    print_str_raw(target);
    print_str_raw((oc_string){ .ptr = (const uint8_t*)("\012"), .len = sizeof("\012") - 1 });
    ((v)->has_error) = true;
    return false;
}

oc_string array_elem_type(oc_string t) {
    if ((!starts_with(t, (oc_string){ .ptr = (const uint8_t*)("["), .len = sizeof("[") - 1 }))) {
    return (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 };
    }
    size_t i = 1;
    while ((i < ((t).len))) {
    if ((__oc_index_str(t, i) == 93)) {
    return __oc_slice_str(t, __OC_ADD(i, 1), ((t).len));
    }
    i = __OC_ADD(i, 1);
    }
    return (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 };
}

bool validator_check_conversion(Validator* v, oc_string target, AstNode* value, oc_string place) {
    if ((value == ((AstNode*)(0)))) {
    return true;
    }
    bind_to_target(target, value);
    if ((((value)->tag) == NODE_ARRAY_INIT)) {
    oc_string elem = array_elem_type(target);
    if ((((elem).len) > 0)) {
    AstNode* el = ((value)->left);
    while ((el != ((AstNode*)(0)))) {
    if ((!validator_check_conversion(v, elem, el, (oc_string){ .ptr = (const uint8_t*)("an array element"), .len = sizeof("an array element") - 1 }))) {
    return false;
    }
    el = ((el)->next);
    }
    }
    return true;
    }
    if ((!validator_check_no_float_to_int(v, target, value, place))) {
    return false;
    }
    return validator_check_lossless_to_float(v, target, value, place);
}

oc_string get_local_array_slice_source(Validator* v, AstNode* expr) {
    if ((expr == ((AstNode*)(0)))) {
    return (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 };
    }
    if ((((expr)->tag) != NODE_SLICE)) {
    return (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 };
    }
    if ((!starts_with(validator_stack_place_type(v, ((expr)->left)), (oc_string){ .ptr = (const uint8_t*)("["), .len = sizeof("[") - 1 }))) {
    return (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 };
    }
    return get_ident_name(((expr)->left));
}

oc_string contains_stack_ref(Validator* v, AstNode* expr) {
    if ((expr == ((AstNode*)(0)))) {
    return (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 };
    }
    if ((((expr)->tag) == NODE_SLICE)) {
    oc_string arr = get_local_array_slice_source(v, expr);
    if ((((arr).len) > 0)) {
    return arr;
    }
    return contains_stack_ref(v, ((expr)->left));
    }
    if ((((expr)->tag) == NODE_ADDRESS_OF)) {
    oc_string id = get_ident_name(((expr)->left));
    if ((((id).len) > 0)) {
    int32_t idx = validator_find_var(v, id);
    if ((idx >= 0)) {
    if (((__OC_AT((((v)->vars)), (({ __auto_type _v = (idx); if (_v < 0) { fprintf(stderr, "panic: invalid unsigned cast\n"); exit(1); } (size_t)_v; })))).is_stack_local)) {
    return id;
    }
    }
    }
    return (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 };
    }
    if ((((expr)->tag) == NODE_IDENT)) {
    oc_string id = ((((expr)->tok)).slice);
    int32_t idx = validator_find_var(v, id);
    if ((idx >= 0)) {
    if (((__OC_AT((((v)->vars)), (({ __auto_type _v = (idx); if (_v < 0) { fprintf(stderr, "panic: invalid unsigned cast\n"); exit(1); } (size_t)_v; })))).points_to_stack_local)) {
    return id;
    }
    }
    return (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 };
    }
    if ((((expr)->tag) == NODE_CAST)) {
    return contains_stack_ref(v, ((expr)->left));
    }
    if ((((expr)->tag) == NODE_STRUCT_INIT)) {
    AstNode* f = ((expr)->left);
    while ((f != ((AstNode*)(0)))) {
    oc_string s = contains_stack_ref(v, ((f)->left));
    if ((((s).len) > 0)) {
    return s;
    }
    f = ((f)->next);
    }
    return (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 };
    }
    if ((((expr)->tag) == NODE_ARRAY_INIT)) {
    AstNode* el = ((expr)->left);
    while ((el != ((AstNode*)(0)))) {
    oc_string s = contains_stack_ref(v, el);
    if ((((s).len) > 0)) {
    return s;
    }
    el = ((el)->next);
    }
    return (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 };
    }
    if ((((expr)->tag) == NODE_CALL)) {
    if (str_eq(((((expr)->tok)).slice), (oc_string){ .ptr = (const uint8_t*)("arena_alloc"), .len = sizeof("arena_alloc") - 1 })) {
    return (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 };
    }
    if (str_eq(((((expr)->tok)).slice), (oc_string){ .ptr = (const uint8_t*)("lexer_next"), .len = sizeof("lexer_next") - 1 })) {
    return (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 };
    }
    if (str_eq(((((expr)->tok)).slice), (oc_string){ .ptr = (const uint8_t*)("read_file"), .len = sizeof("read_file") - 1 })) {
    return (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 };
    }
    if (str_eq(((((expr)->tok)).slice), (oc_string){ .ptr = (const uint8_t*)("write_file"), .len = sizeof("write_file") - 1 })) {
    return (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 };
    }
    if (str_eq(((((expr)->tok)).slice), (oc_string){ .ptr = (const uint8_t*)("sys_read"), .len = sizeof("sys_read") - 1 })) {
    return (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 };
    }
    if (str_eq(((((expr)->tok)).slice), (oc_string){ .ptr = (const uint8_t*)("sys_write"), .len = sizeof("sys_write") - 1 })) {
    return (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 };
    }
    if (str_eq(((((expr)->tok)).slice), (oc_string){ .ptr = (const uint8_t*)("sys_open"), .len = sizeof("sys_open") - 1 })) {
    return (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 };
    }
    if (str_eq(((((expr)->tok)).slice), (oc_string){ .ptr = (const uint8_t*)("sys_close"), .len = sizeof("sys_close") - 1 })) {
    return (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 };
    }
    if (str_eq(((((expr)->tok)).slice), (oc_string){ .ptr = (const uint8_t*)("sys_chmod"), .len = sizeof("sys_chmod") - 1 })) {
    return (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 };
    }
    if (str_eq(((((expr)->tok)).slice), (oc_string){ .ptr = (const uint8_t*)("sys_unlink"), .len = sizeof("sys_unlink") - 1 })) {
    return (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 };
    }
    if (str_eq(((((expr)->tok)).slice), (oc_string){ .ptr = (const uint8_t*)("sys_connect"), .len = sizeof("sys_connect") - 1 })) {
    return (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 };
    }
    AstNode* a = ((expr)->left);
    size_t ai = 0;
    while ((a != ((AstNode*)(0)))) {
    if ((!fx_param_borrows(((v)->fx), ((((expr)->tok)).slice), ai))) {
    oc_string s = contains_stack_ref(v, a);
    if ((((s).len) > 0)) {
    return s;
    }
    }
    a = ((a)->next);
    ai = __OC_ADD(ai, 1);
    }
    return (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 };
    }
    if ((((expr)->tag) == NODE_FIELD_ACCESS)) {
    return contains_stack_ref(v, ((expr)->left));
    }
    if ((((expr)->tag) == NODE_INDEX)) {
    return contains_stack_ref(v, ((expr)->left));
    }
    if ((((expr)->tag) == NODE_BINARY)) {
    if ((((((expr)->tok)).tag) == TOK_EQEQ)) {
    return (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 };
    }
    if ((((((expr)->tok)).tag) == TOK_NEQ)) {
    return (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 };
    }
    if ((((((expr)->tok)).tag) == TOK_LT)) {
    return (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 };
    }
    if ((((((expr)->tok)).tag) == TOK_LTE)) {
    return (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 };
    }
    if ((((((expr)->tok)).tag) == TOK_GT)) {
    return (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 };
    }
    if ((((((expr)->tok)).tag) == TOK_GTE)) {
    return (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 };
    }
    oc_string s1 = contains_stack_ref(v, ((expr)->left));
    if ((((s1).len) > 0)) {
    return s1;
    }
    return contains_stack_ref(v, ((expr)->right));
    }
    if ((((expr)->tag) == NODE_UNARY)) {
    return contains_stack_ref(v, ((expr)->left));
    }
    if ((((expr)->tag) == NODE_DEREF)) {
    return contains_stack_ref(v, ((expr)->left));
    }
    return (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 };
}

bool check_expr_use(Validator* v, AstNode* expr) {
    if ((expr == ((AstNode*)(0)))) {
    return true;
    }
    if ((((expr)->tag) == NODE_IDENT)) {
    if ((((((v)->initializing_var)).len) > 0)) {
    if (str_eq(((((expr)->tok)).slice), ((v)->initializing_var))) {
    fwrite("security error: variable used in its own initializer (CWE-457)", 1, sizeof("security error: variable used in its own initializer (CWE-457)") - 1, stdout); fputc('\n', stdout);
    ((v)->has_error) = true;
    return false;
    }
    }
    int32_t idx = validator_find_var(v, ((((expr)->tok)).slice));
    if ((idx >= 0)) {
    size_t i = ({ __auto_type _v = (idx); if (_v < 0) { fprintf(stderr, "panic: invalid unsigned cast\n"); exit(1); } (size_t)_v; });
    if (((__OC_AT((((v)->vars)), (i))).is_invalidated)) {
    if ((((((__OC_AT((((v)->vars)), (i))).borrowed_from_var)).len) > 0)) {
    print_err_ref_invalidated(((((expr)->tok)).slice), ((__OC_AT((((v)->vars)), (i))).borrowed_from_var));
    ((v)->has_error) = true;
    return false;
    }
    print_err_uaf_reset(((((expr)->tok)).slice), ((__OC_AT((((v)->vars)), (i))).borrowed_from_arena));
    ((v)->has_error) = true;
    return false;
    }
    if ((((((__OC_AT((((v)->vars)), (i))).borrowed_from_arena)).len) > 0)) {
    if (validator_is_arena_deinit(v, ((__OC_AT((((v)->vars)), (i))).borrowed_from_arena))) {
    print_err_uaf_deinit(((((expr)->tok)).slice), ((__OC_AT((((v)->vars)), (i))).borrowed_from_arena));
    ((v)->has_error) = true;
    return false;
    }
    }
    }
    return true;
    }
    if ((((expr)->tag) == NODE_CALL)) {
    AstNode* mut_arg = ((expr)->left);
    while ((mut_arg != ((AstNode*)(0)))) {
    if ((((mut_arg)->tag) == NODE_ADDRESS_OF)) {
    if (str_eq(((mut_arg)->type_name), (oc_string){ .ptr = (const uint8_t*)("mut"), .len = sizeof("mut") - 1 })) {
    oc_string tgt_name = get_ident_name(((mut_arg)->left));
    if ((((tgt_name).len) > 0)) {
    validator_invalidate_var(v, tgt_name);
    int32_t tgt_idx = validator_find_var(v, tgt_name);
    if ((tgt_idx >= 0)) {
    AstNode* borrow_arg = ((expr)->left);
    while ((borrow_arg != ((AstNode*)(0)))) {
    oc_string src_name = (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 };
    bool is_ptr_arg = false;
    if ((((borrow_arg)->tag) == NODE_ADDRESS_OF)) {
    src_name = get_ident_name(((borrow_arg)->left));
    is_ptr_arg = true;
    }
    else {
    src_name = get_ident_name(borrow_arg);
    }
    if ((((src_name).len) > 0)) {
    int32_t src_idx = validator_find_var(v, src_name);
    if ((src_idx >= 0)) {
    if (((__OC_AT((((v)->vars)), (({ __auto_type _v = (src_idx); if (_v < 0) { fprintf(stderr, "panic: invalid unsigned cast\n"); exit(1); } (size_t)_v; })))).points_to_stack_local)) {
    is_ptr_arg = true;
    }
    if ((((((__OC_AT((((v)->vars)), (({ __auto_type _v = (src_idx); if (_v < 0) { fprintf(stderr, "panic: invalid unsigned cast\n"); exit(1); } (size_t)_v; })))).borrowed_from_arena)).len) > 0)) {
    is_ptr_arg = true;
    }
    if (is_ptr_arg) {
    if ((((__OC_AT((((v)->vars)), (({ __auto_type _v = (src_idx); if (_v < 0) { fprintf(stderr, "panic: invalid unsigned cast\n"); exit(1); } (size_t)_v; })))).scope_depth) > ((__OC_AT((((v)->vars)), (({ __auto_type _v = (tgt_idx); if (_v < 0) { fprintf(stderr, "panic: invalid unsigned cast\n"); exit(1); } (size_t)_v; })))).scope_depth))) {
    print_err_borrow_escape(src_name, tgt_name);
    ((v)->has_error) = true;
    return false;
    }
    }
    }
    }
    borrow_arg = ((borrow_arg)->next);
    }
    }
    }
    }
    }
    mut_arg = ((mut_arg)->next);
    }
    AstNode* a = ((expr)->left);
    while ((a != ((AstNode*)(0)))) {
    if ((!check_expr_use(v, a))) {
    return false;
    }
    a = ((a)->next);
    }
    AstNode* callee = validator_find_fn(v, ((((expr)->tok)).slice));
    if ((callee != ((AstNode*)(0)))) {
    AstNode* prm = ((callee)->left);
    AstNode* arg_v = ((expr)->left);
    while ((prm != ((AstNode*)(0)))) {
    if ((arg_v == ((AstNode*)(0)))) {
    break;
    }
    if ((!validator_check_conversion(v, ((prm)->type_name), arg_v, (oc_string){ .ptr = (const uint8_t*)("a function argument"), .len = sizeof("a function argument") - 1 }))) {
    return false;
    }
    prm = ((prm)->next);
    arg_v = ((arg_v)->next);
    }
    }
    AstNode* ea = ((expr)->left);
    size_t ei = 0;
    while ((ea != ((AstNode*)(0)))) {
    uint8_t eff = fx_param_effect(((v)->fx), ((((expr)->tok)).slice), ei);
    if ((eff > 0)) {
    oc_string a_name = get_arena_arg_name(ea);
    if ((((a_name).len) > 0)) {
    if ((eff == 2)) {
    if ((!validator_mark_deinit(v, a_name))) {
    return false;
    }
    }
    else {
    validator_invalidate_arena(v, a_name);
    }
    }
    }
    ea = ((ea)->next);
    ei = __OC_ADD(ei, 1);
    }
    return true;
    }
    if ((((expr)->tag) == NODE_FIELD_ACCESS)) {
    return check_expr_use(v, ((expr)->left));
    }
    if ((((expr)->tag) == NODE_SLICE)) {
    if ((((expr)->left) != ((AstNode*)(0)))) {
    if ((((((expr)->left))->tag) == NODE_IDENT)) {
    int32_t s_idx = validator_find_var(v, ((((((expr)->left))->tok)).slice));
    if ((s_idx >= 0)) {
    oc_string s_type = ((__OC_AT((((v)->vars)), (({ __auto_type _v = (s_idx); if (_v < 0) { fprintf(stderr, "panic: invalid unsigned cast\n"); exit(1); } (size_t)_v; })))).type_name);
    bool is_raw_ptr = starts_with(s_type, (oc_string){ .ptr = (const uint8_t*)("*"), .len = sizeof("*") - 1 });
    if (starts_with(s_type, (oc_string){ .ptr = (const uint8_t*)("&"), .len = sizeof("&") - 1 })) {
    is_raw_ptr = true;
    }
    if (is_raw_ptr) {
    print_str_raw((oc_string){ .ptr = (const uint8_t*)("error: cannot slice raw pointer '"), .len = sizeof("error: cannot slice raw pointer '") - 1 });
    print_str_raw(((((((expr)->left))->tok)).slice));
    print_str_raw((oc_string){ .ptr = (const uint8_t*)("': it has no length (slice a string or a [N]T array)\012"), .len = sizeof("': it has no length (slice a string or a [N]T array)\012") - 1 });
    ((v)->has_error) = true;
    return false;
    }
    }
    }
    }
    if ((!check_expr_use(v, ((expr)->left)))) {
    return false;
    }
    if ((((expr)->right) != ((AstNode*)(0)))) {
    if ((!check_expr_use(v, ((((expr)->right))->left)))) {
    return false;
    }
    return check_expr_use(v, ((((expr)->right))->right));
    }
    return true;
    }
    if ((((expr)->tag) == NODE_INDEX)) {
    if ((!check_expr_use(v, ((expr)->left)))) {
    return false;
    }
    if (validator_is_float(v, ((expr)->right))) {
    fwrite("error: an index must be an integer; convert the float with .as(usize)", 1, sizeof("error: an index must be an integer; convert the float with .as(usize)") - 1, stdout); fputc('\n', stdout);
    ((v)->has_error) = true;
    return false;
    }
    return check_expr_use(v, ((expr)->right));
    }
    if ((((expr)->tag) == NODE_BINARY)) {
    if ((!validator_check_operands(v, expr))) {
    return false;
    }
    bool float_operand = validator_is_float(v, ((expr)->left));
    if (validator_is_float(v, ((expr)->right))) {
    float_operand = true;
    }
    if (float_operand) {
    bool is_equality = (((((expr)->tok)).tag) == TOK_EQEQ);
    if ((((((expr)->tok)).tag) == TOK_NEQ)) {
    is_equality = true;
    }
    if (is_equality) {
    fwrite("error: == and != are not allowed on floats: rounding makes exact equality unreliable (0.1 + 0.2 is not 0.3). Compare with approx_eq(a, b, eps), or use is_nan(x) to test for NaN", 1, sizeof("error: == and != are not allowed on floats: rounding makes exact equality unreliable (0.1 + 0.2 is not 0.3). Compare with approx_eq(a, b, eps), or use is_nan(x) to test for NaN") - 1, stdout); fputc('\n', stdout);
    ((v)->has_error) = true;
    return false;
    }
    bool int_only = (((((expr)->tok)).tag) == TOK_PERCENT);
    if ((((((expr)->tok)).tag) == TOK_AMP)) {
    int_only = true;
    }
    if ((((((expr)->tok)).tag) == TOK_PIPE)) {
    int_only = true;
    }
    if ((((((expr)->tok)).tag) == TOK_CARET)) {
    int_only = true;
    }
    if ((((((expr)->tok)).tag) == TOK_SHL)) {
    int_only = true;
    }
    if ((((((expr)->tok)).tag) == TOK_SHR)) {
    int_only = true;
    }
    if (int_only) {
    fwrite("error: the operators % & | ^ << >> are defined for integers only, not for floats", 1, sizeof("error: the operators % & | ^ << >> are defined for integers only, not for floats") - 1, stdout); fputc('\n', stdout);
    ((v)->has_error) = true;
    return false;
    }
    if (is_untyped_float(((expr)->right))) {
    if ((!is_untyped_float(((expr)->left)))) {
    bind_to_target(validator_expr_type(v, ((expr)->left)), ((expr)->right));
    }
    }
    if (is_untyped_float(((expr)->left))) {
    if ((!is_untyped_float(((expr)->right)))) {
    bind_to_target(validator_expr_type(v, ((expr)->right)), ((expr)->left));
    }
    }
    oc_string lt = validator_expr_type(v, ((expr)->left));
    oc_string rt = validator_expr_type(v, ((expr)->right));
    if (is_float_type_name(lt)) {
    if ((!is_float_type_name(rt))) {
    if ((!validator_check_lossless_to_float(v, lt, ((expr)->right), (oc_string){ .ptr = (const uint8_t*)("a float operation"), .len = sizeof("a float operation") - 1 }))) {
    return false;
    }
    }
    }
    if (is_float_type_name(rt)) {
    if ((!is_float_type_name(lt))) {
    if ((!validator_check_lossless_to_float(v, rt, ((expr)->left), (oc_string){ .ptr = (const uint8_t*)("a float operation"), .len = sizeof("a float operation") - 1 }))) {
    return false;
    }
    }
    }
    }
    if ((!check_expr_use(v, ((expr)->left)))) {
    return false;
    }
    return check_expr_use(v, ((expr)->right));
    }
    if ((((expr)->tag) == NODE_UNARY)) {
    return check_expr_use(v, ((expr)->left));
    }
    if ((((expr)->tag) == NODE_DEREF)) {
    return check_expr_use(v, ((expr)->left));
    }
    if ((((expr)->tag) == NODE_CAST)) {
    bind_to_target(((expr)->type_name), ((expr)->left));
    return check_expr_use(v, ((expr)->left));
    }
    if ((((expr)->tag) == NODE_ADDRESS_OF)) {
    return check_expr_use(v, ((expr)->left));
    }
    if ((((expr)->tag) == NODE_STRUCT_INIT)) {
    AstNode* f = ((expr)->left);
    while ((f != ((AstNode*)(0)))) {
    if ((!check_expr_use(v, ((f)->left)))) {
    return false;
    }
    oc_string f_type = validator_field_type(v, ((((expr)->tok)).slice), ((((f)->tok)).slice));
    if ((!validator_check_conversion(v, f_type, ((f)->left), (oc_string){ .ptr = (const uint8_t*)("a struct field"), .len = sizeof("a struct field") - 1 }))) {
    return false;
    }
    f = ((f)->next);
    }
    return true;
    }
    if ((((expr)->tag) == NODE_ARRAY_INIT)) {
    AstNode* el = ((expr)->left);
    while ((el != ((AstNode*)(0)))) {
    if ((!check_expr_use(v, el))) {
    return false;
    }
    el = ((el)->next);
    }
    return true;
    }
    return true;
}

void validator_save_invalidated(Validator* v, bool* out) {
    size_t i = 0;
    while ((i < ((v)->vars_count))) {
    __OC_AT((out), (i)) = ((__OC_AT((((v)->vars)), (i))).is_invalidated);
    i = __OC_ADD(i, 1);
    }
}

void validator_restore_invalidated(Validator* v, bool* saved) {
    size_t i = 0;
    while ((i < ((v)->vars_count))) {
    ((__OC_AT((((v)->vars)), (i))).is_invalidated) = __OC_AT((saved), (i));
    i = __OC_ADD(i, 1);
    }
}

bool block_always_returns(AstNode* block) {
    if ((block == ((AstNode*)(0)))) {
    return false;
    }
    if ((((block)->tag) != NODE_BLOCK)) {
    return false;
    }
    AstNode* last = ((block)->left);
    if ((last == ((AstNode*)(0)))) {
    return false;
    }
    while ((((last)->next) != ((AstNode*)(0)))) {
    last = ((last)->next);
    }
    return (((last)->tag) == NODE_RETURN);
}

bool validate_stmt(Validator* v, AstNode* stmt) {
    if ((stmt == ((AstNode*)(0)))) {
    return true;
    }
    if ((((stmt)->tag) == NODE_BREAK)) {
    return true;
    }
    if ((((stmt)->tag) == NODE_CONTINUE)) {
    return true;
    }
    if ((((stmt)->tag) == NODE_LET)) {
    ((v)->initializing_var) = ((((stmt)->tok)).slice);
    bool ok_init = check_expr_use(v, ((stmt)->left));
    ((v)->initializing_var) = (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 };
    if ((!ok_init)) {
    return false;
    }
    oc_string inferred_type = ((stmt)->type_name);
    if ((((inferred_type).len) == 0)) {
    if ((((stmt)->left) != ((AstNode*)(0)))) {
    if ((((((stmt)->left))->tag) == NODE_CALL)) {
    if (str_eq(((((((stmt)->left))->tok)).slice), (oc_string){ .ptr = (const uint8_t*)("arena_init"), .len = sizeof("arena_init") - 1 })) {
    inferred_type = (oc_string){ .ptr = (const uint8_t*)("Arena"), .len = sizeof("Arena") - 1 };
    }
    }
    if ((((((stmt)->left))->tag) == NODE_STRUCT_INIT)) {
    inferred_type = ((((((stmt)->left))->tok)).slice);
    }
    if (validator_is_float(v, ((stmt)->left))) {
    inferred_type = validator_expr_type(v, ((stmt)->left));
    }
    }
    }
    if ((!validator_check_conversion(v, inferred_type, ((stmt)->left), (oc_string){ .ptr = (const uint8_t*)("a variable initializer"), .len = sizeof("a variable initializer") - 1 }))) {
    return false;
    }
    if ((((stmt)->left) != ((AstNode*)(0)))) {
    if ((((((stmt)->left))->tag) == NODE_IDENT)) {
    bool is_arena_copy = false;
    if (str_eq(inferred_type, (oc_string){ .ptr = (const uint8_t*)("Arena"), .len = sizeof("Arena") - 1 })) {
    is_arena_copy = true;
    }
    else {
    int32_t rhs_idx = validator_find_var(v, ((((((stmt)->left))->tok)).slice));
    if ((rhs_idx >= 0)) {
    if (str_eq(((__OC_AT((((v)->vars)), (({ __auto_type _v = (rhs_idx); if (_v < 0) { fprintf(stderr, "panic: invalid unsigned cast\n"); exit(1); } (size_t)_v; })))).type_name), (oc_string){ .ptr = (const uint8_t*)("Arena"), .len = sizeof("Arena") - 1 })) {
    is_arena_copy = true;
    }
    }
    }
    if (is_arena_copy) {
    fwrite("security error: Arena cannot be copied by value (linear resource)", 1, sizeof("security error: Arena cannot be copied by value (linear resource)") - 1, stdout); fputc('\n', stdout);
    ((v)->has_error) = true;
    return false;
    }
    }
    }
    bool points_to_local = false;
    oc_string arena_source = get_expr_arena_borrow(v, ((stmt)->left));
    oc_string src_name = get_underlying_address_of(((stmt)->left));
    if ((((src_name).len) > 0)) {
    int32_t idx = validator_find_var(v, src_name);
    if ((idx >= 0)) {
    if (((__OC_AT((((v)->vars)), (({ __auto_type _v = (idx); if (_v < 0) { fprintf(stderr, "panic: invalid unsigned cast\n"); exit(1); } (size_t)_v; })))).is_stack_local)) {
    points_to_local = true;
    }
    }
    }
    else {
    oc_string id = get_ident_name(((stmt)->left));
    if (is_scalar_load(v, ((stmt)->left))) {
    id = (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 };
    }
    if ((((id).len) > 0)) {
    int32_t idx = validator_find_var(v, id);
    if ((idx >= 0)) {
    if (((__OC_AT((((v)->vars)), (({ __auto_type _v = (idx); if (_v < 0) { fprintf(stderr, "panic: invalid unsigned cast\n"); exit(1); } (size_t)_v; })))).points_to_stack_local)) {
    points_to_local = true;
    }
    }
    }
    else {
    if ((((contains_stack_ref(v, ((stmt)->left))).len) > 0)) {
    if ((!is_scalar_value_type(inferred_type))) {
    points_to_local = true;
    }
    }
    }
    }
    oc_string var_source = (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 };
    bool is_addr_of = false;
    if ((((stmt)->left) != ((AstNode*)(0)))) {
    if ((((((stmt)->left))->tag) == NODE_ADDRESS_OF)) {
    is_addr_of = true;
    }
    }
    if (is_addr_of) {
    var_source = get_ident_name(((((stmt)->left))->left));
    }
    else {
    oc_string id = get_ident_name(((stmt)->left));
    if ((((id).len) > 0)) {
    int32_t idx = validator_find_var(v, id);
    if ((idx >= 0)) {
    if ((((((__OC_AT((((v)->vars)), (({ __auto_type _v = (idx); if (_v < 0) { fprintf(stderr, "panic: invalid unsigned cast\n"); exit(1); } (size_t)_v; })))).borrowed_from_var)).len) > 0)) {
    var_source = ((__OC_AT((((v)->vars)), (({ __auto_type _v = (idx); if (_v < 0) { fprintf(stderr, "panic: invalid unsigned cast\n"); exit(1); } (size_t)_v; })))).borrowed_from_var);
    }
    }
    }
    }
    if ((((v)->vars_count) >= 512)) {
    fwrite("security error: validator symbol table capacity exceeded", 1, sizeof("security error: validator symbol table capacity exceeded") - 1, stdout); fputc('\n', stdout);
    ((v)->has_error) = true;
    return false;
    }
    __OC_AT((((v)->vars)), (((v)->vars_count))) = (VarInfo){ .name = ((((stmt)->tok)).slice), .type_name = inferred_type, .scope_depth = ((v)->scope_depth), .is_stack_local = true, .points_to_stack_local = points_to_local, .borrowed_from_arena = arena_source, .borrowed_from_var = var_source, .is_invalidated = false, .is_mut = (((((stmt)->tok)).tag) == TOK_MUT) };
    ((v)->vars_count) = __OC_ADD(((v)->vars_count), 1);
    return true;
    }
    if ((((stmt)->tag) == NODE_ASSIGN)) {
    if ((!check_expr_use(v, ((stmt)->right)))) {
    return false;
    }
    if ((!validator_check_conversion(v, validator_expr_type(v, ((stmt)->left)), ((stmt)->right), (oc_string){ .ptr = (const uint8_t*)("an assignment"), .len = sizeof("an assignment") - 1 }))) {
    return false;
    }
    bool is_direct_ident = false;
    if ((((stmt)->left) != ((AstNode*)(0)))) {
    if ((((((stmt)->left))->tag) == NODE_IDENT)) {
    is_direct_ident = true;
    }
    }
    if ((!is_direct_ident)) {
    if ((!check_expr_use(v, ((stmt)->left)))) {
    return false;
    }
    }
    else {
    int32_t lhs_idx = validator_find_var(v, ((((((stmt)->left))->tok)).slice));
    if ((lhs_idx >= 0)) {
    if ((!((__OC_AT((((v)->vars)), (({ __auto_type _v = (lhs_idx); if (_v < 0) { fprintf(stderr, "panic: invalid unsigned cast\n"); exit(1); } (size_t)_v; })))).is_mut))) {
    fwrite("security error: cannot assign to immutable variable", 1, sizeof("security error: cannot assign to immutable variable") - 1, stdout); fputc('\n', stdout);
    ((v)->has_error) = true;
    return false;
    }
    if ((((stmt)->right) != ((AstNode*)(0)))) {
    if ((((((stmt)->right))->tag) == NODE_IDENT)) {
    bool is_arena_assign = false;
    if (str_eq(((__OC_AT((((v)->vars)), (({ __auto_type _v = (lhs_idx); if (_v < 0) { fprintf(stderr, "panic: invalid unsigned cast\n"); exit(1); } (size_t)_v; })))).type_name), (oc_string){ .ptr = (const uint8_t*)("Arena"), .len = sizeof("Arena") - 1 })) {
    is_arena_assign = true;
    }
    else {
    int32_t rhs_idx = validator_find_var(v, ((((((stmt)->right))->tok)).slice));
    if ((rhs_idx >= 0)) {
    if (str_eq(((__OC_AT((((v)->vars)), (({ __auto_type _v = (rhs_idx); if (_v < 0) { fprintf(stderr, "panic: invalid unsigned cast\n"); exit(1); } (size_t)_v; })))).type_name), (oc_string){ .ptr = (const uint8_t*)("Arena"), .len = sizeof("Arena") - 1 })) {
    is_arena_assign = true;
    }
    }
    }
    if (is_arena_assign) {
    fwrite("security error: Arena cannot be copied by value (linear resource)", 1, sizeof("security error: Arena cannot be copied by value (linear resource)") - 1, stdout); fputc('\n', stdout);
    ((v)->has_error) = true;
    return false;
    }
    }
    }
    }
    }
    oc_string src_opt = get_underlying_address_of(((stmt)->right));
    if ((((src_opt).len) == 0)) {
    src_opt = get_local_array_slice_source(v, ((stmt)->right));
    }
    oc_string tgt_opt = get_ident_name(((stmt)->left));
    oc_string val_ident = get_ident_name(((stmt)->right));
    if (is_scalar_load(v, ((stmt)->right))) {
    val_ident = (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 };
    }
    oc_string rhs_arena = get_expr_arena_borrow(v, ((stmt)->right));
    if ((((rhs_arena).len) > 0)) {
    if ((((tgt_opt).len) > 0)) {
    int32_t t_idx = validator_find_var(v, tgt_opt);
    if ((t_idx >= 0)) {
    int32_t a_idx = validator_find_var(v, rhs_arena);
    uint32_t arena_depth = ((v)->scope_depth);
    bool arena_is_local = true;
    if ((a_idx >= 0)) {
    arena_depth = ((__OC_AT((((v)->vars)), (({ __auto_type _v = (a_idx); if (_v < 0) { fprintf(stderr, "panic: invalid unsigned cast\n"); exit(1); } (size_t)_v; })))).scope_depth);
    arena_is_local = ((__OC_AT((((v)->vars)), (({ __auto_type _v = (a_idx); if (_v < 0) { fprintf(stderr, "panic: invalid unsigned cast\n"); exit(1); } (size_t)_v; })))).is_stack_local);
    }
    bool cross_escape = false;
    if ((((__OC_AT((((v)->vars)), (({ __auto_type _v = (t_idx); if (_v < 0) { fprintf(stderr, "panic: invalid unsigned cast\n"); exit(1); } (size_t)_v; })))).scope_depth) < arena_depth)) {
    cross_escape = true;
    }
    else {
    if ((!((__OC_AT((((v)->vars)), (({ __auto_type _v = (t_idx); if (_v < 0) { fprintf(stderr, "panic: invalid unsigned cast\n"); exit(1); } (size_t)_v; })))).is_stack_local))) {
    cross_escape = arena_is_local;
    }
    }
    if (cross_escape) {
    print_err_cross_arena(rhs_arena, tgt_opt);
    ((v)->has_error) = true;
    return false;
    }
    }
    }
    }
    if ((!is_direct_ident)) {
    if ((((rhs_arena).len) > 0)) {
    if ((((validator_stack_place_type(v, ((stmt)->left))).len) > 0)) {
    int32_t agg_idx = validator_find_var(v, tgt_opt);
    if ((agg_idx >= 0)) {
    if ((((((__OC_AT((((v)->vars)), (({ __auto_type _v = (agg_idx); if (_v < 0) { fprintf(stderr, "panic: invalid unsigned cast\n"); exit(1); } (size_t)_v; })))).borrowed_from_arena)).len) == 0)) {
    ((__OC_AT((((v)->vars)), (({ __auto_type _v = (agg_idx); if (_v < 0) { fprintf(stderr, "panic: invalid unsigned cast\n"); exit(1); } (size_t)_v; })))).borrowed_from_arena) = rhs_arena;
    }
    }
    }
    }
    }
    if (is_direct_ident) {
    int32_t lhs_idx = validator_find_var(v, ((((((stmt)->left))->tok)).slice));
    if ((lhs_idx >= 0)) {
    ((__OC_AT((((v)->vars)), (({ __auto_type _v = (lhs_idx); if (_v < 0) { fprintf(stderr, "panic: invalid unsigned cast\n"); exit(1); } (size_t)_v; })))).is_invalidated) = false;
    ((__OC_AT((((v)->vars)), (({ __auto_type _v = (lhs_idx); if (_v < 0) { fprintf(stderr, "panic: invalid unsigned cast\n"); exit(1); } (size_t)_v; })))).borrowed_from_arena) = rhs_arena;
    oc_string new_var_borrow = (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 };
    if ((((stmt)->right) != ((AstNode*)(0)))) {
    if ((((((stmt)->right))->tag) == NODE_ADDRESS_OF)) {
    new_var_borrow = get_ident_name(((((stmt)->right))->left));
    }
    else {
    if ((((val_ident).len) > 0)) {
    int32_t vi = validator_find_var(v, val_ident);
    if ((vi >= 0)) {
    new_var_borrow = ((__OC_AT((((v)->vars)), (({ __auto_type _v = (vi); if (_v < 0) { fprintf(stderr, "panic: invalid unsigned cast\n"); exit(1); } (size_t)_v; })))).borrowed_from_var);
    }
    }
    }
    }
    ((__OC_AT((((v)->vars)), (({ __auto_type _v = (lhs_idx); if (_v < 0) { fprintf(stderr, "panic: invalid unsigned cast\n"); exit(1); } (size_t)_v; })))).borrowed_from_var) = new_var_borrow;
    }
    }
    if ((((src_opt).len) > 0)) {
    int32_t s_idx = validator_find_var(v, src_opt);
    if ((s_idx >= 0)) {
    if ((((tgt_opt).len) > 0)) {
    int32_t t_idx = validator_find_var(v, tgt_opt);
    if ((t_idx >= 0)) {
    bool ptr_escape = false;
    if ((((__OC_AT((((v)->vars)), (({ __auto_type _v = (t_idx); if (_v < 0) { fprintf(stderr, "panic: invalid unsigned cast\n"); exit(1); } (size_t)_v; })))).scope_depth) < ((__OC_AT((((v)->vars)), (({ __auto_type _v = (s_idx); if (_v < 0) { fprintf(stderr, "panic: invalid unsigned cast\n"); exit(1); } (size_t)_v; })))).scope_depth))) {
    ptr_escape = true;
    }
    else {
    if ((!((__OC_AT((((v)->vars)), (({ __auto_type _v = (t_idx); if (_v < 0) { fprintf(stderr, "panic: invalid unsigned cast\n"); exit(1); } (size_t)_v; })))).is_stack_local))) {
    if (((__OC_AT((((v)->vars)), (({ __auto_type _v = (s_idx); if (_v < 0) { fprintf(stderr, "panic: invalid unsigned cast\n"); exit(1); } (size_t)_v; })))).is_stack_local)) {
    ptr_escape = true;
    }
    }
    }
    if (ptr_escape) {
    print_src_loc(((v)->srcmap), ((stmt)->tok));
    fwrite("error: borrowed pointer escapes enclosing scope", 1, sizeof("error: borrowed pointer escapes enclosing scope") - 1, stdout); fputc('\n', stdout);
    ((v)->has_error) = true;
    return false;
    }
    }
    }
    if (((__OC_AT((((v)->vars)), (({ __auto_type _v = (s_idx); if (_v < 0) { fprintf(stderr, "panic: invalid unsigned cast\n"); exit(1); } (size_t)_v; })))).is_stack_local)) {
    if ((((tgt_opt).len) > 0)) {
    validator_set_points_to_stack_local(v, tgt_opt);
    }
    }
    }
    }
    else {
    if ((((val_ident).len) > 0)) {
    int32_t val_idx = validator_find_var(v, val_ident);
    if ((val_idx >= 0)) {
    if (((__OC_AT((((v)->vars)), (({ __auto_type _v = (val_idx); if (_v < 0) { fprintf(stderr, "panic: invalid unsigned cast\n"); exit(1); } (size_t)_v; })))).points_to_stack_local)) {
    if ((((tgt_opt).len) > 0)) {
    int32_t t_idx = validator_find_var(v, tgt_opt);
    if ((t_idx >= 0)) {
    bool val_escape = false;
    if ((((__OC_AT((((v)->vars)), (({ __auto_type _v = (t_idx); if (_v < 0) { fprintf(stderr, "panic: invalid unsigned cast\n"); exit(1); } (size_t)_v; })))).scope_depth) < ((__OC_AT((((v)->vars)), (({ __auto_type _v = (val_idx); if (_v < 0) { fprintf(stderr, "panic: invalid unsigned cast\n"); exit(1); } (size_t)_v; })))).scope_depth))) {
    val_escape = true;
    }
    else {
    if ((!((__OC_AT((((v)->vars)), (({ __auto_type _v = (t_idx); if (_v < 0) { fprintf(stderr, "panic: invalid unsigned cast\n"); exit(1); } (size_t)_v; })))).is_stack_local))) {
    val_escape = true;
    }
    }
    if (val_escape) {
    print_src_loc(((v)->srcmap), ((stmt)->tok));
    fwrite("error: borrowed pointer escapes enclosing scope", 1, sizeof("error: borrowed pointer escapes enclosing scope") - 1, stdout); fputc('\n', stdout);
    ((v)->has_error) = true;
    return false;
    }
    }
    validator_set_points_to_stack_local(v, tgt_opt);
    }
    }
    }
    }
    }
    return true;
    }
    if ((((stmt)->tag) == NODE_RETURN)) {
    if ((((stmt)->left) != ((AstNode*)(0)))) {
    if ((!check_expr_use(v, ((stmt)->left)))) {
    return false;
    }
    if ((!validator_check_conversion(v, ((v)->ret_type), ((stmt)->left), (oc_string){ .ptr = (const uint8_t*)("a return value"), .len = sizeof("a return value") - 1 }))) {
    return false;
    }
    oc_string src_name = contains_stack_ref(v, ((stmt)->left));
    if ((((src_name).len) > 0)) {
    print_err_ret_local(src_name);
    ((v)->has_error) = true;
    return false;
    }
    oc_string arena_name = get_expr_arena_borrow(v, ((stmt)->left));
    if ((((arena_name).len) > 0)) {
    int32_t a_idx = validator_find_var(v, arena_name);
    if ((a_idx >= 0)) {
    if (((__OC_AT((((v)->vars)), (({ __auto_type _v = (a_idx); if (_v < 0) { fprintf(stderr, "panic: invalid unsigned cast\n"); exit(1); } (size_t)_v; })))).is_stack_local)) {
    print_err_ret_arena(arena_name);
    ((v)->has_error) = true;
    return false;
    }
    }
    }
    }
    return true;
    }
    if ((((stmt)->tag) == NODE_BLOCK)) {
    ((v)->scope_depth) = __OC_ADD(((v)->scope_depth), 1);
    size_t prev_len = ((v)->vars_count);
    AstNode* s = ((stmt)->left);
    while ((s != ((AstNode*)(0)))) {
    if ((!validate_stmt(v, s))) {
    return false;
    }
    s = ((s)->next);
    }
    ((v)->scope_depth) = __OC_SUB(((v)->scope_depth), 1);
    ((v)->vars_count) = prev_len;
    return true;
    }
    if ((((stmt)->tag) == NODE_IF)) {
    if ((!check_expr_use(v, ((stmt)->left)))) {
    return false;
    }
    bool saved[512] = {0};
    bool* saved_ptr = ((bool*)((&(saved))));
    size_t saved_deinit = ((v)->deinit_count);
    validator_save_invalidated(v, saved_ptr);
    if ((!validate_stmt(v, ((stmt)->right)))) {
    return false;
    }
    if (block_always_returns(((stmt)->right))) {
    validator_restore_invalidated(v, saved_ptr);
    ((v)->deinit_count) = saved_deinit;
    }
    if ((((stmt)->right) != ((AstNode*)(0)))) {
    if ((((((stmt)->right))->next) != ((AstNode*)(0)))) {
    size_t else_deinit = ((v)->deinit_count);
    validator_save_invalidated(v, saved_ptr);
    if ((!validate_stmt(v, ((((stmt)->right))->next)))) {
    return false;
    }
    if (block_always_returns(((((stmt)->right))->next))) {
    validator_restore_invalidated(v, saved_ptr);
    ((v)->deinit_count) = else_deinit;
    }
    }
    }
    return true;
    }
    if ((((stmt)->tag) == NODE_WHILE)) {
    size_t pre_vars_len = ((v)->vars_count);
    if ((!check_expr_use(v, ((stmt)->left)))) {
    return false;
    }
    if ((!validate_stmt(v, ((stmt)->right)))) {
    return false;
    }
    ((v)->vars_count) = pre_vars_len;
    if ((!check_expr_use(v, ((stmt)->left)))) {
    return false;
    }
    if ((!validate_stmt(v, ((stmt)->right)))) {
    return false;
    }
    ((v)->vars_count) = pre_vars_len;
    return true;
    }
    if ((((stmt)->tag) == NODE_PRINT)) {
    return check_expr_use(v, ((stmt)->left));
    }
    return check_expr_use(v, stmt);
}

bool validate_function(AstNode* f, FnEffects* fx, AstNode* program, SourceMap* m, Generics* g) {
    if ((((f)->right) == ((AstNode*)(0)))) {
    return true;
    }
    Validator v = (Validator){ .fx = fx, .program = program, .ret_type = ((f)->type_name), .vars = { (VarInfo){ .name = (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 }, .type_name = (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 }, .scope_depth = 0, .is_stack_local = false, .points_to_stack_local = false, .borrowed_from_arena = (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 }, .borrowed_from_var = (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 }, .is_invalidated = false, .is_mut = false } }, .vars_count = 0, .deinit_arenas = { (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 } }, .deinit_count = 0, .scope_depth = 1, .has_error = false, .initializing_var = (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 }, .srcmap = m, .fn_name = ((((f)->tok)).slice), .gen = g };
    AstNode* p = ((f)->left);
    while ((p != ((AstNode*)(0)))) {
    if ((((v).vars_count) >= 512)) {
    fwrite("security error: validator symbol table capacity exceeded", 1, sizeof("security error: validator symbol table capacity exceeded") - 1, stdout); fputc('\n', stdout);
    ((v).has_error) = true;
    return false;
    }
    bool param_is_mut = false;
    if (starts_with(((p)->type_name), (oc_string){ .ptr = (const uint8_t*)("&mut "), .len = sizeof("&mut ") - 1 })) {
    param_is_mut = true;
    }
    if (starts_with(((p)->type_name), (oc_string){ .ptr = (const uint8_t*)("*mut "), .len = sizeof("*mut ") - 1 })) {
    param_is_mut = true;
    }
    bool param_is_local = true;
    if (starts_with(((p)->type_name), (oc_string){ .ptr = (const uint8_t*)("*"), .len = sizeof("*") - 1 })) {
    param_is_local = false;
    }
    if (starts_with(((p)->type_name), (oc_string){ .ptr = (const uint8_t*)("&"), .len = sizeof("&") - 1 })) {
    param_is_local = false;
    }
    if (str_eq(((p)->type_name), (oc_string){ .ptr = (const uint8_t*)("string"), .len = sizeof("string") - 1 })) {
    param_is_local = false;
    }
    if (starts_with(((p)->type_name), (oc_string){ .ptr = (const uint8_t*)("[]"), .len = sizeof("[]") - 1 })) {
    param_is_local = false;
    }
    __OC_AT((((v).vars)), (((v).vars_count))) = (VarInfo){ .name = ((((p)->tok)).slice), .type_name = ((p)->type_name), .scope_depth = 0, .is_stack_local = param_is_local, .points_to_stack_local = false, .borrowed_from_arena = (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 }, .borrowed_from_var = (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 }, .is_invalidated = false, .is_mut = param_is_mut };
    ((v).vars_count) = __OC_ADD(((v).vars_count), 1);
    p = ((p)->next);
    }
    AstNode* body_block = ((f)->right);
    AstNode* stmt = ((body_block)->left);
    while ((stmt != ((AstNode*)(0)))) {
    if ((!validate_stmt((&(v)), stmt))) {
    return false;
    }
    stmt = ((stmt)->next);
    }
    if (((v).has_error)) {
    return false;
    }
    return true;
}

bool validate_program(AstNode* root, SourceMap* m, Generics* g) {
    if ((!check_unique_names(root, m))) {
    return false;
    }
    if ((!check_calls_resolved(root, m, ((root)->left)))) {
    return false;
    }
    if ((!check_string_escapes(m, ((root)->left)))) {
    return false;
    }
    AstNode* top = ((root)->left);
    while ((top != ((AstNode*)(0)))) {
    if ((((top)->tag) == NODE_LET)) {
    bind_to_target(((top)->type_name), ((top)->left));
    }
    top = ((top)->next);
    }
    FnEffects fx = (FnEffects){ .names = { (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 } }, .effects = { 0 }, .borrows = { false }, .count = 0 };
    AstNode* decl = ((root)->left);
    while ((decl != ((AstNode*)(0)))) {
    if ((((decl)->tag) == NODE_FN)) {
    if ((((decl)->right) != ((AstNode*)(0)))) {
    if ((((fx).count) < 256)) {
    __OC_AT((((fx).names)), (((fx).count))) = ((((decl)->tok)).slice);
    ((fx).count) = __OC_ADD(((fx).count), 1);
    }
    }
    }
    decl = ((decl)->next);
    }
    bool changed = true;
    while (changed) {
    changed = false;
    size_t slot = 0;
    decl = ((root)->left);
    while ((decl != ((AstNode*)(0)))) {
    if ((((decl)->tag) == NODE_FN)) {
    if ((((decl)->right) != ((AstNode*)(0)))) {
    if ((slot < 256)) {
    bool ret_ptr = (((((decl)->type_name)).len) > 0);
    if (is_scalar_value_type(((decl)->type_name))) {
    ret_ptr = false;
    }
    if (fx_scan((&(fx)), slot, ((decl)->left), ret_ptr, ((decl)->right))) {
    changed = true;
    }
    }
    slot = __OC_ADD(slot, 1);
    }
    }
    decl = ((decl)->next);
    }
    }
    decl = ((root)->left);
    while ((decl != ((AstNode*)(0)))) {
    if ((((decl)->tag) == NODE_FN)) {
    if ((!validate_function(decl, (&(fx)), root, m, g))) {
    return false;
    }
    }
    decl = ((decl)->next);
    }
    return true;
}

AstNode* parse_program(Parser* p) {
    AstNode* prog_node = ast_alloc_node(((p)->arena), NODE_PROGRAM, ((p)->current));
    AstNode* first_decl = ((AstNode*)(0));
    AstNode* last_decl = ((AstNode*)(0));
    while ((((((p)->current)).tag) != TOK_EOF)) {
    uint32_t prev_tag = ((((p)->current)).tag);
    size_t prev_idx = ((((p)->lex)).index);
    AstNode* decl = ((AstNode*)(0));
    if ((((((p)->current)).tag) == TOK_FN)) {
    decl = parse_fn_decl(p);
    }
    else {
    if ((((((p)->current)).tag) == TOK_EXTERN)) {
    parser_advance(p);
    decl = parse_fn_decl(p);
    }
    else {
    if ((((((p)->current)).tag) == TOK_ENUM)) {
    decl = parse_enum_decl(p);
    }
    else {
    if ((((((p)->current)).tag) == TOK_STRUCT)) {
    parser_advance(p);
    decl = parse_struct_decl(p, false);
    }
    else {
    if ((((((p)->current)).tag) == TOK_PACKED)) {
    parser_advance(p);
    parser_expect(p, TOK_STRUCT);
    decl = parse_struct_decl(p, true);
    }
    else {
    if ((((((p)->current)).tag) == TOK_LET)) {
    decl = parse_let_stmt(p);
    }
    else {
    if ((((((p)->current)).tag) == TOK_IMPORT)) {
    parser_advance(p);
    if ((((((p)->current)).tag) == TOK_STRING)) {
    parser_advance(p);
    }
    if ((((((p)->current)).tag) == TOK_SEMICOLON)) {
    parser_advance(p);
    }
    }
    else {
    if ((((((p)->current)).tag) == TOK_SEMICOLON)) {
    parser_advance(p);
    }
    else {
    parser_error(p, ((p)->current), (oc_string){ .ptr = (const uint8_t*)("parse error: unexpected at top level:"), .len = sizeof("parse error: unexpected at top level:") - 1 });
    ((p)->has_error) = true;
    parser_advance(p);
    }
    }
    }
    }
    }
    }
    }
    }
    if ((decl != ((AstNode*)(0)))) {
    if ((first_decl == ((AstNode*)(0)))) {
    first_decl = decl;
    last_decl = decl;
    }
    else {
    ((last_decl)->next) = decl;
    last_decl = decl;
    }
    }
    if ((((((p)->current)).tag) == prev_tag)) {
    if ((((((p)->lex)).index) == prev_idx)) {
    parser_advance(p);
    }
    }
    }
    ((prog_node)->left) = first_decl;
    return prog_node;
}

oc_string gx_done(StrBuf sb) {
    return (oc_string){ .ptr = ((uint8_t*)(((sb).buf))), .len = ((sb).len) };
}

void gx_err(Generics* g, Token tok, oc_string msg, oc_string name) {
    print_src_loc(((g)->m), tok);
    print_str_raw((oc_string){ .ptr = (const uint8_t*)("error: "), .len = sizeof("error: ") - 1 });
    print_str_raw(msg);
    print_str_raw(name);
    print_str_raw((oc_string){ .ptr = (const uint8_t*)("\012"), .len = sizeof("\012") - 1 });
    ((g)->ok) = false;
}

bool gx_is_ident_char(uint8_t c) {
    if (is_alpha(c)) {
    return true;
    }
    if (is_digit(c)) {
    return true;
    }
    return (c == 95);
}

oc_string gx_canon(Generics* g, oc_string t) {
    StrBuf sb = strbuf_init(((g)->arena), __OC_ADD(((t).len), 1));
    size_t i = 0;
    while ((i < ((t).len))) {
    uint8_t c = __oc_index_str(t, i);
    if (is_space(c)) {
    bool keep = (((sb).len) > 0);
    if (keep) {
    uint8_t prev = __OC_AT((((sb).buf)), (__OC_SUB(((sb).len), 1)));
    if ((!gx_is_ident_char(prev))) {
    keep = false;
    }
    }
    size_t j = i;
    while ((j < ((t).len))) {
    if ((!is_space(__oc_index_str(t, j)))) {
    break;
    }
    j = __OC_ADD(j, 1);
    }
    if ((j >= ((t).len))) {
    keep = false;
    }
    else {
    if ((!gx_is_ident_char(__oc_index_str(t, j)))) {
    keep = false;
    }
    }
    if (keep) {
    strbuf_push((&(sb)), 32);
    }
    i = j;
    }
    else {
    strbuf_push((&(sb)), c);
    i = __OC_ADD(i, 1);
    }
    }
    return gx_done(sb);
}

GArgs gx_split(Generics* g, oc_string text) {
    GArgs r = (GArgs){ .n = 0, .items = { (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 } } };
    int32_t depth = 0;
    size_t start = 0;
    size_t i = 0;
    while ((i <= ((text).len))) {
    bool cut = (i == ((text).len));
    if ((!cut)) {
    uint8_t c = __oc_index_str(text, i);
    if ((c == 60)) {
    depth = __OC_ADD(depth, 1);
    }
    if ((c == 91)) {
    depth = __OC_ADD(depth, 1);
    }
    if ((c == 62)) {
    depth = __OC_SUB(depth, 1);
    }
    if ((c == 93)) {
    depth = __OC_SUB(depth, 1);
    }
    if ((c == 44)) {
    if ((depth == 0)) {
    cut = true;
    }
    }
    }
    if (cut) {
    if ((((r).n) < 8)) {
    __OC_AT((((r).items)), (((r).n))) = gx_canon(g, trim(__oc_slice_str(text, start, i)));
    ((r).n) = __OC_ADD(((r).n), 1);
    }
    start = __OC_ADD(i, 1);
    }
    i = __OC_ADD(i, 1);
    }
    return r;
}

void gx_mangle_into(StrBuf* sb, oc_string t) {
    size_t i = 0;
    while ((i < ((t).len))) {
    uint8_t c = __oc_index_str(t, i);
    if (gx_is_ident_char(c)) {
    strbuf_push(sb, c);
    }
    else {
    if ((c == 42)) {
    strbuf_push_str(sb, (oc_string){ .ptr = (const uint8_t*)("P"), .len = sizeof("P") - 1 });
    }
    else {
    if ((c == 38)) {
    strbuf_push_str(sb, (oc_string){ .ptr = (const uint8_t*)("R"), .len = sizeof("R") - 1 });
    }
    else {
    if ((c == 91)) {
    strbuf_push_str(sb, (oc_string){ .ptr = (const uint8_t*)("A"), .len = sizeof("A") - 1 });
    }
    else {
    if ((c == 93)) {
    strbuf_push_str(sb, (oc_string){ .ptr = (const uint8_t*)("_"), .len = sizeof("_") - 1 });
    }
    else {
    if ((c == 60)) {
    strbuf_push_str(sb, (oc_string){ .ptr = (const uint8_t*)("_L_"), .len = sizeof("_L_") - 1 });
    }
    else {
    if ((c == 62)) {
    strbuf_push_str(sb, (oc_string){ .ptr = (const uint8_t*)("_G_"), .len = sizeof("_G_") - 1 });
    }
    else {
    if ((c == 44)) {
    strbuf_push_str(sb, (oc_string){ .ptr = (const uint8_t*)("_C_"), .len = sizeof("_C_") - 1 });
    }
    }
    }
    }
    }
    }
    }
    }
    i = __OC_ADD(i, 1);
    }
}

oc_string gx_mangled(Generics* g, oc_string name, GArgs args) {
    StrBuf sb = strbuf_init(((g)->arena), 32);
    strbuf_push_str((&(sb)), name);
    size_t i = 0;
    while ((i < ((args).n))) {
    strbuf_push_str((&(sb)), (oc_string){ .ptr = (const uint8_t*)("__"), .len = sizeof("__") - 1 });
    gx_mangle_into((&(sb)), __OC_AT((((args).items)), (i)));
    i = __OC_ADD(i, 1);
    }
    return gx_done(sb);
}

oc_string gx_join(Generics* g, GArgs args) {
    StrBuf sb = strbuf_init(((g)->arena), 32);
    size_t i = 0;
    while ((i < ((args).n))) {
    if ((i > 0)) {
    strbuf_push_str((&(sb)), (oc_string){ .ptr = (const uint8_t*)(", "), .len = sizeof(", ") - 1 });
    }
    strbuf_push_str((&(sb)), __OC_AT((((args).items)), (i)));
    i = __OC_ADD(i, 1);
    }
    return gx_done(sb);
}

AstNode* gx_template(Generics* g, oc_string name) {
    size_t i = 0;
    while ((i < ((g)->tmpl_count))) {
    AstNode* t = __OC_AT((((g)->tmpl)), (i));
    if (str_eq(((((t)->tok)).slice), name)) {
    return t;
    }
    i = __OC_ADD(i, 1);
    }
    return ((AstNode*)(0));
}

size_t gx_count(AstNode* list) {
    size_t n = 0;
    AstNode* p = list;
    while ((p != ((AstNode*)(0)))) {
    n = __OC_ADD(n, 1);
    p = ((p)->next);
    }
    return n;
}

oc_string gx_subst(Generics* g, oc_string t, AstNode* params, GArgs args) {
    if ((((t).len) == 0)) {
    return t;
    }
    StrBuf sb = strbuf_init(((g)->arena), __OC_ADD(((t).len), 16));
    size_t i = 0;
    while ((i < ((t).len))) {
    if (gx_is_ident_char(__oc_index_str(t, i))) {
    size_t j = i;
    while ((j < ((t).len))) {
    if ((!gx_is_ident_char(__oc_index_str(t, j)))) {
    break;
    }
    j = __OC_ADD(j, 1);
    }
    oc_string word = __oc_slice_str(t, i, j);
    bool replaced = false;
    AstNode* p = params;
    size_t k = 0;
    while ((p != ((AstNode*)(0)))) {
    if ((k < ((args).n))) {
    if (str_eq(((((p)->tok)).slice), word)) {
    strbuf_push_str((&(sb)), __OC_AT((((args).items)), (k)));
    replaced = true;
    break;
    }
    }
    p = ((p)->next);
    k = __OC_ADD(k, 1);
    }
    if ((!replaced)) {
    strbuf_push_str((&(sb)), __oc_slice_str(t, i, j));
    }
    i = j;
    }
    else {
    strbuf_push((&(sb)), __oc_index_str(t, i));
    i = __OC_ADD(i, 1);
    }
    }
    return gx_done(sb);
}

AstNode* gx_clone(Generics* g, AstNode* n) {
    if ((n == ((AstNode*)(0)))) {
    return n;
    }
    AstNode* c = ast_alloc_node(((g)->arena), ((n)->tag), ((n)->tok));
    ((c)->type_name) = ((n)->type_name);
    ((c)->left) = gx_clone(g, ((n)->left));
    ((c)->right) = gx_clone(g, ((n)->right));
    ((c)->next) = gx_clone(g, ((n)->next));
    return c;
}

void gx_fill(Generics* g, AstNode* n, AstNode* params, GArgs args) {
    AstNode* cur = n;
    while ((cur != ((AstNode*)(0)))) {
    if ((((((cur)->type_name)).len) > 0)) {
    if ((((cur)->tag) != NODE_ADDRESS_OF)) {
    ((cur)->type_name) = gx_subst(g, ((cur)->type_name), params, args);
    }
    }
    if ((((cur)->tag) == NODE_IDENT)) {
    AstNode* p = params;
    size_t k = 0;
    while ((p != ((AstNode*)(0)))) {
    if ((k < ((args).n))) {
    if ((((((p)->type_name)).len) > 0)) {
    if (str_eq(((((p)->tok)).slice), ((((cur)->tok)).slice))) {
    ((cur)->tag) = NODE_INT;
    ((((cur)->tok)).slice) = __OC_AT((((args).items)), (k));
    ((cur)->type_name) = (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 };
    }
    }
    }
    p = ((p)->next);
    k = __OC_ADD(k, 1);
    }
    }
    gx_fill(g, ((cur)->left), params, args);
    gx_fill(g, ((cur)->right), params, args);
    cur = ((cur)->next);
    }
}

void gx_append(Generics* g, AstNode* d) {
    ((d)->next) = ((AstNode*)(0));
    if ((((g)->last) == ((AstNode*)(0)))) {
    ((((g)->root))->left) = d;
    }
    else {
    ((((g)->last))->next) = d;
    }
    ((g)->last) = d;
}

oc_string gx_instantiate(Generics* g, AstNode* t, GArgs args, Token site) {
    size_t want = gx_count(((t)->left));
    if ((((args).n) != want)) {
    print_src_loc(((g)->m), site);
    print_str_raw((oc_string){ .ptr = (const uint8_t*)("error: '"), .len = sizeof("error: '") - 1 });
    print_str_raw(((((t)->tok)).slice));
    print_str_raw((oc_string){ .ptr = (const uint8_t*)("' takes "), .len = sizeof("' takes ") - 1 });
    print_u32_raw(({ __auto_type _v = (want); if (_v < 0 || (uint64_t)_v > 4294967295ULL) { fprintf(stderr, "panic: invalid unsigned cast\n"); exit(1); } (uint32_t)_v; }));
    print_str_raw((oc_string){ .ptr = (const uint8_t*)(" type argument(s), not "), .len = sizeof(" type argument(s), not ") - 1 });
    print_u32_raw(({ __auto_type _v = (((args).n)); if (_v < 0 || (uint64_t)_v > 4294967295ULL) { fprintf(stderr, "panic: invalid unsigned cast\n"); exit(1); } (uint32_t)_v; }));
    print_str_raw((oc_string){ .ptr = (const uint8_t*)("\012"), .len = sizeof("\012") - 1 });
    ((g)->ok) = false;
    return (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 };
    }
    oc_string name = gx_mangled(g, ((((t)->tok)).slice), args);
    size_t i = 0;
    while ((i < ((g)->inst_count))) {
    if (str_eq(__OC_AT((((g)->inst_name)), (i)), name)) {
    return name;
    }
    i = __OC_ADD(i, 1);
    }
    if ((((g)->inst_count) >= 1024)) {
    gx_err(g, site, (oc_string){ .ptr = (const uint8_t*)("too many generic instantiations (1024); is a generic instantiating itself with ever larger types\077 "), .len = sizeof("too many generic instantiations (1024); is a generic instantiating itself with ever larger types\077 ") - 1 }, ((((t)->tok)).slice));
    return (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 };
    }
    AstNode* p = ((t)->left);
    size_t k = 0;
    while ((p != ((AstNode*)(0)))) {
    oc_string a = __OC_AT((((args).items)), (k));
    bool numeric = (((a).len) > 0);
    size_t q = 0;
    while ((q < ((a).len))) {
    if ((!is_digit(__oc_index_str(a, q)))) {
    numeric = false;
    }
    q = __OC_ADD(q, 1);
    }
    if ((((((p)->type_name)).len) > 0)) {
    if ((!numeric)) {
    gx_err(g, site, (oc_string){ .ptr = (const uint8_t*)("const parameter needs an integer argument: "), .len = sizeof("const parameter needs an integer argument: ") - 1 }, ((((p)->tok)).slice));
    return (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 };
    }
    }
    else {
    if (numeric) {
    gx_err(g, site, (oc_string){ .ptr = (const uint8_t*)("type parameter needs a type, not a number: "), .len = sizeof("type parameter needs a type, not a number: ") - 1 }, ((((p)->tok)).slice));
    return (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 };
    }
    }
    p = ((p)->next);
    k = __OC_ADD(k, 1);
    }
    __OC_AT((((g)->inst_name)), (((g)->inst_count))) = name;
    __OC_AT((((g)->inst_tmpl)), (((g)->inst_count))) = ((((t)->tok)).slice);
    __OC_AT((((g)->inst_args)), (((g)->inst_count))) = gx_join(g, args);
    __OC_AT((((g)->inst_site)), (((g)->inst_count))) = site;
    ((g)->inst_count) = __OC_ADD(((g)->inst_count), 1);
    AstNode* copy = gx_clone(g, ((t)->right));
    ((copy)->next) = ((AstNode*)(0));
    ((((copy)->tok)).slice) = name;
    oc_string struct_kind = ((copy)->type_name);
    gx_fill(g, copy, ((t)->left), args);
    if ((((copy)->tag) == NODE_STRUCT_DECL)) {
    ((copy)->type_name) = struct_kind;
    }
    gx_append(g, copy);
    return name;
}

oc_string gx_resolve(Generics* g, oc_string t, Token site) {
    bool has_lt = false;
    size_t i = 0;
    while ((i < ((t).len))) {
    if ((__oc_index_str(t, i) == 60)) {
    has_lt = true;
    }
    i = __OC_ADD(i, 1);
    }
    if ((!has_lt)) {
    return t;
    }
    oc_string ct = gx_canon(g, t);
    StrBuf sb = strbuf_init(((g)->arena), __OC_ADD(((ct).len), 16));
    i = 0;
    while ((i < ((ct).len))) {
    if (gx_is_ident_char(__oc_index_str(ct, i))) {
    size_t j = i;
    while ((j < ((ct).len))) {
    if ((!gx_is_ident_char(__oc_index_str(ct, j)))) {
    break;
    }
    j = __OC_ADD(j, 1);
    }
    oc_string word = __oc_slice_str(ct, i, j);
    bool done = false;
    if ((j < ((ct).len))) {
    if ((__oc_index_str(ct, j) == 60)) {
    int32_t depth = 0;
    size_t k = j;
    while ((k < ((ct).len))) {
    if ((__oc_index_str(ct, k) == 60)) {
    depth = __OC_ADD(depth, 1);
    }
    if ((__oc_index_str(ct, k) == 62)) {
    depth = __OC_SUB(depth, 1);
    if ((depth == 0)) {
    break;
    }
    }
    k = __OC_ADD(k, 1);
    }
    oc_string inner = gx_resolve(g, __oc_slice_str(ct, __OC_ADD(j, 1), k), site);
    AstNode* tmpl = gx_template(g, word);
    if ((tmpl == ((AstNode*)(0)))) {
    gx_err(g, site, (oc_string){ .ptr = (const uint8_t*)("not a generic struct: "), .len = sizeof("not a generic struct: ") - 1 }, word);
    }
    else {
    if ((((((tmpl)->right))->tag) != NODE_STRUCT_DECL)) {
    gx_err(g, site, (oc_string){ .ptr = (const uint8_t*)("a generic function is not a type: "), .len = sizeof("a generic function is not a type: ") - 1 }, word);
    }
    else {
    strbuf_push_str((&(sb)), gx_instantiate(g, tmpl, gx_split(g, inner), site));
    }
    }
    i = __OC_ADD(k, 1);
    done = true;
    }
    }
    if ((!done)) {
    strbuf_push_str((&(sb)), __oc_slice_str(ct, i, j));
    i = j;
    }
    }
    else {
    strbuf_push((&(sb)), __oc_index_str(ct, i));
    i = __OC_ADD(i, 1);
    }
    }
    return gx_done(sb);
}

int32_t gx_param_index(AstNode* params, oc_string word) {
    AstNode* p = params;
    int32_t k = 0;
    while ((p != ((AstNode*)(0)))) {
    if (str_eq(((((p)->tok)).slice), word)) {
    return k;
    }
    p = ((p)->next);
    k = __OC_ADD(k, 1);
    }
    return (-1);
}

bool gx_bind(GBind* b, oc_string name, oc_string val) {
    size_t i = 0;
    while ((i < ((b)->n))) {
    if (str_eq(__OC_AT((((b)->b_name)), (i)), name)) {
    if (str_eq(__OC_AT((((b)->b_val)), (i)), val)) {
    return true;
    }
    ((b)->conflict) = name;
    return false;
    }
    i = __OC_ADD(i, 1);
    }
    if ((((b)->n) < 8)) {
    __OC_AT((((b)->b_name)), (((b)->n))) = name;
    __OC_AT((((b)->b_val)), (((b)->n))) = val;
    ((b)->n) = __OC_ADD(((b)->n), 1);
    }
    return true;
}

bool gx_unify(Generics* g, AstNode* params, oc_string pat, oc_string con, GBind* b) {
    if ((gx_param_index(params, pat) >= 0)) {
    return gx_bind(b, pat, con);
    }
    oc_string prefixes[4] = { (oc_string){ .ptr = (const uint8_t*)("*mut "), .len = sizeof("*mut ") - 1 }, (oc_string){ .ptr = (const uint8_t*)("&mut "), .len = sizeof("&mut ") - 1 }, (oc_string){ .ptr = (const uint8_t*)("*"), .len = sizeof("*") - 1 }, (oc_string){ .ptr = (const uint8_t*)("&"), .len = sizeof("&") - 1 } };
    size_t k = 0;
    while ((k < 4)) {
    if (starts_with(pat, __OC_AT((prefixes), (k)))) {
    if ((!starts_with(con, __OC_AT((prefixes), (k))))) {
    return false;
    }
    return gx_unify(g, params, __oc_slice_str(pat, ((__OC_AT((prefixes), (k))).len), ((pat).len)), __oc_slice_str(con, ((__OC_AT((prefixes), (k))).len), ((con).len)), b);
    }
    k = __OC_ADD(k, 1);
    }
    if (starts_with(pat, (oc_string){ .ptr = (const uint8_t*)("["), .len = sizeof("[") - 1 })) {
    if ((!starts_with(con, (oc_string){ .ptr = (const uint8_t*)("["), .len = sizeof("[") - 1 }))) {
    return false;
    }
    int64_t pe = find(pat, (oc_string){ .ptr = (const uint8_t*)("]"), .len = sizeof("]") - 1 });
    int64_t ce = find(con, (oc_string){ .ptr = (const uint8_t*)("]"), .len = sizeof("]") - 1 });
    if ((pe < 0)) {
    return false;
    }
    if ((ce < 0)) {
    return false;
    }
    oc_string plen = __oc_slice_str(pat, 1, ({ __auto_type _v = (pe); if (_v < 0) { fprintf(stderr, "panic: invalid unsigned cast\n"); exit(1); } (size_t)_v; }));
    oc_string clen = __oc_slice_str(con, 1, ({ __auto_type _v = (ce); if (_v < 0) { fprintf(stderr, "panic: invalid unsigned cast\n"); exit(1); } (size_t)_v; }));
    if ((gx_param_index(params, plen) >= 0)) {
    if ((!gx_bind(b, plen, clen))) {
    return false;
    }
    }
    else {
    if ((!str_eq(plen, clen))) {
    return false;
    }
    }
    return gx_unify(g, params, __oc_slice_str(pat, __OC_ADD(({ __auto_type _v = (pe); if (_v < 0) { fprintf(stderr, "panic: invalid unsigned cast\n"); exit(1); } (size_t)_v; }), 1), ((pat).len)), __oc_slice_str(con, __OC_ADD(({ __auto_type _v = (ce); if (_v < 0) { fprintf(stderr, "panic: invalid unsigned cast\n"); exit(1); } (size_t)_v; }), 1), ((con).len)), b);
    }
    int64_t lt = find(pat, (oc_string){ .ptr = (const uint8_t*)("<"), .len = sizeof("<") - 1 });
    if ((lt > 0)) {
    size_t i = 0;
    while ((i < ((g)->inst_count))) {
    if (str_eq(__OC_AT((((g)->inst_name)), (i)), con)) {
    if ((!str_eq(__OC_AT((((g)->inst_tmpl)), (i)), __oc_slice_str(pat, 0, ({ __auto_type _v = (lt); if (_v < 0) { fprintf(stderr, "panic: invalid unsigned cast\n"); exit(1); } (size_t)_v; }))))) {
    return false;
    }
    GArgs pa = gx_split(g, __oc_slice_str(pat, __OC_ADD(({ __auto_type _v = (lt); if (_v < 0) { fprintf(stderr, "panic: invalid unsigned cast\n"); exit(1); } (size_t)_v; }), 1), __OC_SUB(((pat).len), 1)));
    GArgs ca = gx_split(g, __OC_AT((((g)->inst_args)), (i)));
    if ((((pa).n) != ((ca).n))) {
    return false;
    }
    size_t j = 0;
    while ((j < ((pa).n))) {
    if ((!gx_unify(g, params, __OC_AT((((pa).items)), (j)), __OC_AT((((ca).items)), (j)), b))) {
    return false;
    }
    j = __OC_ADD(j, 1);
    }
    return true;
    }
    i = __OC_ADD(i, 1);
    }
    return false;
    }
    return str_eq(pat, con);
}

oc_string gx_arg_type(Generics* g, Validator* v, AstNode* arg) {
    if ((((arg)->tag) == NODE_ADDRESS_OF)) {
    oc_string inner = validator_expr_type(v, ((arg)->left));
    if ((((inner).len) == 0)) {
    return (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 };
    }
    if (str_eq(((arg)->type_name), (oc_string){ .ptr = (const uint8_t*)("mut"), .len = sizeof("mut") - 1 })) {
    return str_concat(((g)->arena), (oc_string){ .ptr = (const uint8_t*)("*mut "), .len = sizeof("*mut ") - 1 }, inner);
    }
    return str_concat(((g)->arena), (oc_string){ .ptr = (const uint8_t*)("*"), .len = sizeof("*") - 1 }, inner);
    }
    return validator_expr_type(v, arg);
}

GArgs gx_infer(Generics* g, Validator* v, AstNode* t, AstNode* call) {
    GArgs fail = (GArgs){ .n = 0, .items = { (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 } } };
    GBind b = (GBind){ .n = 0, .b_name = { (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 } }, .b_val = { (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 } }, .conflict = (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 } };
    GBind defaults = (GBind){ .n = 0, .b_name = { (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 } }, .b_val = { (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 } }, .conflict = (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 } };
    AstNode* prm = ((((t)->right))->left);
    AstNode* arg = ((call)->left);
    oc_string pat = (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 };
    oc_string con = (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 };
    while ((prm != ((AstNode*)(0)))) {
    if ((arg == ((AstNode*)(0)))) {
    break;
    }
    pat = gx_canon(g, ((prm)->type_name));
    con = gx_canon(g, gx_arg_type(g, v, arg));
    if ((((con).len) == 0)) {
    if (is_int_literal(arg)) {
    if ((gx_param_index(((t)->left), pat) >= 0)) {
    gx_bind((&(defaults)), pat, (oc_string){ .ptr = (const uint8_t*)("i64"), .len = sizeof("i64") - 1 });
    }
    }
    }
    else {
    if ((!gx_unify(g, ((t)->left), pat, con, (&(b))))) {
    print_src_loc(((g)->m), ((arg)->tok));
    print_str_raw((oc_string){ .ptr = (const uint8_t*)("error: argument of type "), .len = sizeof("error: argument of type ") - 1 });
    print_str_raw(con);
    print_str_raw((oc_string){ .ptr = (const uint8_t*)(" does not fit parameter type "), .len = sizeof(" does not fit parameter type ") - 1 });
    print_str_raw(pat);
    print_str_raw((oc_string){ .ptr = (const uint8_t*)(" of '"), .len = sizeof(" of '") - 1 });
    print_str_raw(((((t)->tok)).slice));
    if ((((((b).conflict)).len) > 0)) {
    print_str_raw((oc_string){ .ptr = (const uint8_t*)("' ("), .len = sizeof("' (") - 1 });
    print_str_raw(((b).conflict));
    print_str_raw((oc_string){ .ptr = (const uint8_t*)(" would have two types)"), .len = sizeof(" would have two types)") - 1 });
    }
    else {
    print_str_raw((oc_string){ .ptr = (const uint8_t*)("'"), .len = sizeof("'") - 1 });
    }
    print_str_raw((oc_string){ .ptr = (const uint8_t*)("; write the type arguments: "), .len = sizeof("; write the type arguments: ") - 1 });
    print_str_raw(((((t)->tok)).slice));
    print_str_raw((oc_string){ .ptr = (const uint8_t*)("<...>(...)\012"), .len = sizeof("<...>(...)\012") - 1 });
    ((g)->ok) = false;
    return fail;
    }
    }
    prm = ((prm)->next);
    arg = ((arg)->next);
    }
    GArgs r = (GArgs){ .n = 0, .items = { (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 } } };
    AstNode* p = ((t)->left);
    while ((p != ((AstNode*)(0)))) {
    oc_string found = (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 };
    size_t i = 0;
    while ((i < ((b).n))) {
    if (str_eq(__OC_AT((((b).b_name)), (i)), ((((p)->tok)).slice))) {
    found = __OC_AT((((b).b_val)), (i));
    }
    i = __OC_ADD(i, 1);
    }
    if ((((found).len) == 0)) {
    i = 0;
    while ((i < ((defaults).n))) {
    if (str_eq(__OC_AT((((defaults).b_name)), (i)), ((((p)->tok)).slice))) {
    found = __OC_AT((((defaults).b_val)), (i));
    }
    i = __OC_ADD(i, 1);
    }
    }
    if ((((found).len) == 0)) {
    print_src_loc(((g)->m), ((call)->tok));
    print_str_raw((oc_string){ .ptr = (const uint8_t*)("error: cannot infer "), .len = sizeof("error: cannot infer ") - 1 });
    print_str_raw(((((p)->tok)).slice));
    print_str_raw((oc_string){ .ptr = (const uint8_t*)(" for '"), .len = sizeof(" for '") - 1 });
    print_str_raw(((((t)->tok)).slice));
    print_str_raw((oc_string){ .ptr = (const uint8_t*)("'; write the type arguments: "), .len = sizeof("'; write the type arguments: ") - 1 });
    print_str_raw(((((t)->tok)).slice));
    print_str_raw((oc_string){ .ptr = (const uint8_t*)("<...>(...)\012"), .len = sizeof("<...>(...)\012") - 1 });
    ((g)->ok) = false;
    return fail;
    }
    if ((((r).n) < 8)) {
    __OC_AT((((r).items)), (((r).n))) = found;
    ((r).n) = __OC_ADD(((r).n), 1);
    }
    p = ((p)->next);
    }
    return r;
}

void gx_add_var(Validator* v, oc_string name, oc_string t) {
    if ((((v)->vars_count) >= 512)) {
    return;
    }
    __OC_AT((((v)->vars)), (((v)->vars_count))) = (VarInfo){ .name = name, .type_name = t, .scope_depth = 1, .is_stack_local = true, .points_to_stack_local = false, .borrowed_from_arena = (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 }, .borrowed_from_var = (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 }, .is_invalidated = false, .is_mut = true };
    ((v)->vars_count) = __OC_ADD(((v)->vars_count), 1);
}

void gx_walk(Generics* g, Validator* v, AstNode* n) {
    AstNode* cur = n;
    while ((cur != ((AstNode*)(0)))) {
    gx_walk(g, v, ((cur)->left));
    gx_walk(g, v, ((cur)->right));
    if ((((((cur)->type_name)).len) > 0)) {
    bool is_type = (((cur)->tag) != NODE_ADDRESS_OF);
    if ((((cur)->tag) == NODE_FLOAT)) {
    is_type = false;
    }
    if (is_type) {
    ((cur)->type_name) = gx_resolve(g, ((cur)->type_name), ((cur)->tok));
    }
    }
    if ((((cur)->tag) == NODE_CALL)) {
    AstNode* t = gx_template(g, ((((cur)->tok)).slice));
    if ((t != ((AstNode*)(0)))) {
    if ((((((t)->right))->tag) != NODE_FN)) {
    gx_err(g, ((cur)->tok), (oc_string){ .ptr = (const uint8_t*)("a generic struct is not a function: "), .len = sizeof("a generic struct is not a function: ") - 1 }, ((((cur)->tok)).slice));
    }
    else {
    GArgs args = (GArgs){ .n = 0, .items = { (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 } } };
    if ((((((cur)->type_name)).len) > 0)) {
    args = gx_split(g, ((cur)->type_name));
    }
    else {
    args = gx_infer(g, v, t, cur);
    }
    if (((g)->ok)) {
    oc_string name = gx_instantiate(g, t, args, ((cur)->tok));
    if ((((name).len) > 0)) {
    ((((cur)->tok)).slice) = name;
    }
    }
    }
    ((cur)->type_name) = (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 };
    }
    else {
    if ((((((cur)->type_name)).len) > 0)) {
    gx_err(g, ((cur)->tok), (oc_string){ .ptr = (const uint8_t*)("type arguments given to a function that is not generic: "), .len = sizeof("type arguments given to a function that is not generic: ") - 1 }, ((((cur)->tok)).slice));
    ((cur)->type_name) = (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 };
    }
    }
    }
    if ((((cur)->tag) == NODE_STRUCT_INIT)) {
    AstNode* t = gx_template(g, ((((cur)->tok)).slice));
    if ((t != ((AstNode*)(0)))) {
    if ((((((cur)->type_name)).len) == 0)) {
    gx_err(g, ((cur)->tok), (oc_string){ .ptr = (const uint8_t*)("a generic struct needs its type arguments: "), .len = sizeof("a generic struct needs its type arguments: ") - 1 }, ((((cur)->tok)).slice));
    }
    else {
    if ((((((t)->right))->tag) != NODE_STRUCT_DECL)) {
    gx_err(g, ((cur)->tok), (oc_string){ .ptr = (const uint8_t*)("a generic function is not a struct: "), .len = sizeof("a generic function is not a struct: ") - 1 }, ((((cur)->tok)).slice));
    }
    else {
    oc_string name = gx_instantiate(g, t, gx_split(g, ((cur)->type_name)), ((cur)->tok));
    if ((((name).len) > 0)) {
    ((((cur)->tok)).slice) = name;
    }
    }
    }
    ((cur)->type_name) = (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 };
    }
    }
    if ((((cur)->tag) == NODE_LET)) {
    oc_string var_t = ((cur)->type_name);
    if ((((var_t).len) == 0)) {
    var_t = validator_expr_type(v, ((cur)->left));
    }
    gx_add_var(v, ((((cur)->tok)).slice), var_t);
    }
    cur = ((cur)->next);
    }
}

bool expand_generics(AstNode* root, Arena* arena, SourceMap* m, Generics* g) {
    ((g)->arena) = arena;
    ((g)->m) = m;
    ((g)->root) = root;
    ((g)->ok) = true;
    AstNode* prev = ((AstNode*)(0));
    AstNode* d = ((root)->left);
    while ((d != ((AstNode*)(0)))) {
    AstNode* nxt = ((d)->next);
    if ((((d)->tag) == NODE_GENERIC)) {
    if ((gx_template(g, ((((d)->tok)).slice)) != ((AstNode*)(0)))) {
    gx_err(g, ((d)->tok), (oc_string){ .ptr = (const uint8_t*)("generic defined twice: "), .len = sizeof("generic defined twice: ") - 1 }, ((((d)->tok)).slice));
    }
    else {
    if ((((g)->tmpl_count) < 256)) {
    __OC_AT((((g)->tmpl)), (((g)->tmpl_count))) = d;
    ((g)->tmpl_count) = __OC_ADD(((g)->tmpl_count), 1);
    }
    }
    if ((prev == ((AstNode*)(0)))) {
    ((root)->left) = nxt;
    }
    else {
    ((prev)->next) = nxt;
    }
    }
    else {
    prev = d;
    }
    d = nxt;
    }
    ((g)->last) = prev;
    size_t i = 0;
    while ((i < ((g)->tmpl_count))) {
    AstNode* t = __OC_AT((((g)->tmpl)), (i));
    if ((program_find_fn(root, ((((t)->tok)).slice)) != ((AstNode*)(0)))) {
    gx_err(g, ((t)->tok), (oc_string){ .ptr = (const uint8_t*)("a generic and an ordinary function share the name "), .len = sizeof("a generic and an ordinary function share the name ") - 1 }, ((((t)->tok)).slice));
    }
    i = __OC_ADD(i, 1);
    }
    Validator v = (Validator){ .fx = ((FnEffects*)(0)), .program = root, .ret_type = (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 }, .vars = { (VarInfo){ .name = (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 }, .type_name = (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 }, .scope_depth = 0, .is_stack_local = false, .points_to_stack_local = false, .borrowed_from_arena = (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 }, .borrowed_from_var = (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 }, .is_invalidated = false, .is_mut = false } }, .vars_count = 0, .deinit_arenas = { (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 } }, .deinit_count = 0, .scope_depth = 1, .has_error = false, .initializing_var = (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 }, .srcmap = m, .fn_name = (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 }, .gen = g };
    d = ((root)->left);
    while ((d != ((AstNode*)(0)))) {
    if ((((d)->tag) == NODE_STRUCT_DECL)) {
    AstNode* f = ((d)->left);
    while ((f != ((AstNode*)(0)))) {
    ((f)->type_name) = gx_resolve(g, ((f)->type_name), ((f)->tok));
    f = ((f)->next);
    }
    }
    else {
    if ((((d)->tag) == NODE_LET)) {
    ((d)->type_name) = gx_resolve(g, ((d)->type_name), ((d)->tok));
    gx_walk(g, (&(v)), ((d)->left));
    }
    else {
    if ((((d)->tag) == NODE_FN)) {
    ((d)->type_name) = gx_resolve(g, ((d)->type_name), ((d)->tok));
    ((v).vars_count) = 0;
    AstNode* prm = ((d)->left);
    while ((prm != ((AstNode*)(0)))) {
    ((prm)->type_name) = gx_resolve(g, ((prm)->type_name), ((prm)->tok));
    gx_add_var((&(v)), ((((prm)->tok)).slice), ((prm)->type_name));
    prm = ((prm)->next);
    }
    if ((((d)->right) != ((AstNode*)(0)))) {
    gx_walk(g, (&(v)), ((((d)->right))->left));
    }
    }
    }
    }
    d = ((d)->next);
    }
    return ((g)->ok);
}

void dz_err(Errs* e, Token tok, oc_string msg, oc_string name, oc_string tail) {
    print_src_loc(((e)->m), tok);
    print_str_raw((oc_string){ .ptr = (const uint8_t*)("error: "), .len = sizeof("error: ") - 1 });
    print_str_raw(msg);
    print_str_raw(name);
    print_str_raw(tail);
    print_str_raw((oc_string){ .ptr = (const uint8_t*)("\012"), .len = sizeof("\012") - 1 });
    ((e)->ok) = false;
}

AstNode* dz_enum(Errs* e, oc_string name) {
    size_t i = 0;
    while ((i < ((e)->enum_count))) {
    AstNode* en = __OC_AT((((e)->enums)), (i));
    if (str_eq(((((en)->tok)).slice), name)) {
    return en;
    }
    i = __OC_ADD(i, 1);
    }
    return ((AstNode*)(0));
}

int32_t dz_variant(AstNode* en, oc_string name) {
    AstNode* v = ((en)->left);
    int32_t k = 0;
    while ((v != ((AstNode*)(0)))) {
    if (str_eq(((((v)->tok)).slice), name)) {
    return k;
    }
    v = ((v)->next);
    k = __OC_ADD(k, 1);
    }
    return (-1);
}

int64_t dz_bang(oc_string t) {
    int32_t depth = 0;
    size_t i = 0;
    while ((i < ((t).len))) {
    if ((__oc_index_str(t, i) == 60)) {
    depth = __OC_ADD(depth, 1);
    }
    if ((__oc_index_str(t, i) == 62)) {
    depth = __OC_SUB(depth, 1);
    }
    if ((__oc_index_str(t, i) == 33)) {
    if ((depth == 0)) {
    return ({ __auto_type _v = (i); if (!(_v < 0) && (uint64_t)_v > 9223372036854775807ULL) { fprintf(stderr, "panic: invalid signed cast\n"); exit(1); } (int64_t)_v; });
    }
    }
    i = __OC_ADD(i, 1);
    }
    return (-1);
}

oc_string dz_ok_type(oc_string t) {
    return trim(__oc_slice_str(t, 0, ({ __auto_type _v = (dz_bang(t)); if (_v < 0) { fprintf(stderr, "panic: invalid unsigned cast\n"); exit(1); } (size_t)_v; })));
}

oc_string dz_err_type(oc_string t) {
    return trim(__oc_slice_str(t, __OC_ADD(({ __auto_type _v = (dz_bang(t)); if (_v < 0) { fprintf(stderr, "panic: invalid unsigned cast\n"); exit(1); } (size_t)_v; }), 1), ((t).len)));
}

void dz_sym_add(Errs* e, oc_string name, oc_string t) {
    if ((((e)->sym_count) >= 512)) {
    return;
    }
    __OC_AT((((e)->syms)), (((e)->sym_count))) = name;
    __OC_AT((((e)->sym_types)), (((e)->sym_count))) = t;
    ((e)->sym_count) = __OC_ADD(((e)->sym_count), 1);
}

oc_string dz_sym(Errs* e, oc_string name) {
    size_t i = ((e)->sym_count);
    while ((i > 0)) {
    i = __OC_SUB(i, 1);
    if (str_eq(__OC_AT((((e)->syms)), (i)), name)) {
    return __OC_AT((((e)->sym_types)), (i));
    }
    }
    return (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 };
}

oc_string dz_type_of(Errs* e, AstNode* x) {
    if ((x == ((AstNode*)(0)))) {
    return (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 };
    }
    if ((((x)->tag) == NODE_IDENT)) {
    return dz_sym(e, ((((x)->tok)).slice));
    }
    if ((((x)->tag) == NODE_CALL)) {
    AstNode* f = program_find_fn(((e)->root), ((((x)->tok)).slice));
    if ((f != ((AstNode*)(0)))) {
    return ((f)->type_name);
    }
    return (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 };
    }
    if ((((x)->tag) == NODE_FIELD_ACCESS)) {
    if ((((x)->left) != ((AstNode*)(0)))) {
    if ((((((x)->left))->tag) == NODE_IDENT)) {
    if ((dz_enum(e, ((((((x)->left))->tok)).slice)) != ((AstNode*)(0)))) {
    return ((((((x)->left))->tok)).slice);
    }
    }
    oc_string st = dz_type_of(e, ((x)->left));
    if (starts_with(st, (oc_string){ .ptr = (const uint8_t*)("*mut "), .len = sizeof("*mut ") - 1 })) {
    st = __oc_slice_str(st, 5, ((st).len));
    }
    if (starts_with(st, (oc_string){ .ptr = (const uint8_t*)("*"), .len = sizeof("*") - 1 })) {
    st = __oc_slice_str(st, 1, ((st).len));
    }
    AstNode* d = ((((e)->root))->left);
    while ((d != ((AstNode*)(0)))) {
    if ((((d)->tag) == NODE_STRUCT_DECL)) {
    if (str_eq(((((d)->tok)).slice), st)) {
    AstNode* f = ((d)->left);
    while ((f != ((AstNode*)(0)))) {
    if (str_eq(((((f)->tok)).slice), ((((x)->tok)).slice))) {
    return ((f)->type_name);
    }
    f = ((f)->next);
    }
    }
    }
    d = ((d)->next);
    }
    }
    }
    if ((((x)->tag) == NODE_CAST)) {
    return ((x)->type_name);
    }
    return (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 };
}

Token dz_tok(Token at, uint32_t tag, oc_string text) {
    Token t = at;
    ((t).tag) = tag;
    ((t).slice) = text;
    return t;
}

AstNode* dz_node(Errs* e, uint32_t tag, Token at, oc_string text) {
    return ast_alloc_node(((e)->arena), tag, dz_tok(at, ((at).tag), text));
}

AstNode* dz_ident(Errs* e, oc_string name, Token at) {
    return ast_alloc_node(((e)->arena), NODE_IDENT, dz_tok(at, TOK_IDENT, name));
}

AstNode* dz_field(Errs* e, AstNode* obj, oc_string field, Token at) {
    AstNode* f = ast_alloc_node(((e)->arena), NODE_FIELD_ACCESS, dz_tok(at, TOK_IDENT, field));
    ((f)->left) = obj;
    return f;
}

AstNode* dz_bin(Errs* e, uint32_t tag, oc_string op, AstNode* l, AstNode* r, Token at) {
    AstNode* b = ast_alloc_node(((e)->arena), NODE_BINARY, dz_tok(at, tag, op));
    ((b)->left) = l;
    ((b)->right) = r;
    return b;
}

AstNode* dz_int(Errs* e, oc_string text, Token at) {
    return ast_alloc_node(((e)->arena), NODE_INT, dz_tok(at, TOK_INT, text));
}

AstNode* dz_let(Errs* e, oc_string name, oc_string t, AstNode* init, bool mutable, Token at) {
    uint32_t tag = TOK_IDENT;
    if (mutable) {
    tag = TOK_MUT;
    }
    AstNode* l = ast_alloc_node(((e)->arena), NODE_LET, dz_tok(at, tag, name));
    ((l)->type_name) = t;
    ((l)->left) = init;
    dz_sym_add(e, name, t);
    return l;
}

AstNode* dz_assign(Errs* e, AstNode* lhs, AstNode* rhs, Token at) {
    AstNode* a = ast_alloc_node(((e)->arena), NODE_ASSIGN, dz_tok(at, TOK_EQUAL, (oc_string){ .ptr = (const uint8_t*)("="), .len = sizeof("=") - 1 }));
    ((a)->left) = lhs;
    ((a)->right) = rhs;
    return a;
}

AstNode* dz_block(Errs* e, AstNode* stmts, Token at) {
    AstNode* b = ast_alloc_node(((e)->arena), NODE_BLOCK, dz_tok(at, TOK_LBRACE, (oc_string){ .ptr = (const uint8_t*)("{"), .len = sizeof("{") - 1 }));
    ((b)->left) = stmts;
    return b;
}

AstNode* dz_if(Errs* e, AstNode* cond, AstNode* then_stmts, AstNode* else_node, Token at) {
    AstNode* i = ast_alloc_node(((e)->arena), NODE_IF, dz_tok(at, TOK_IF, (oc_string){ .ptr = (const uint8_t*)("if"), .len = sizeof("if") - 1 }));
    ((i)->left) = cond;
    ((i)->right) = dz_block(e, then_stmts, at);
    ((((i)->right))->next) = else_node;
    return i;
}

oc_string dz_tmp(Errs* e) {
    ((e)->tmp_n) = __OC_ADD(((e)->tmp_n), 1);
    StrBuf sb = strbuf_init(((e)->arena), 16);
    strbuf_push_str((&(sb)), (oc_string){ .ptr = (const uint8_t*)("__err"), .len = sizeof("__err") - 1 });
    uint8_t digits[10] = {0};
    size_t k = 10;
    uint32_t v = ((e)->tmp_n);
    while ((k > 0)) {
    k = __OC_SUB(k, 1);
    __OC_AT((digits), (k)) = ({ __auto_type _v = (__OC_ADD(48, __OC_MOD(v, 10))); if (_v < 0 || (uint64_t)_v > 255ULL) { fprintf(stderr, "panic: invalid unsigned cast\n"); exit(1); } (uint8_t)_v; });
    v = __OC_DIV(v, 10);
    if ((v == 0)) {
    break;
    }
    }
    while ((k < 10)) {
    strbuf_push((&(sb)), __OC_AT((digits), (k)));
    k = __OC_ADD(k, 1);
    }
    return gx_done(sb);
}

AstNode* dz_union_lit(Errs* e, oc_string ut, AstNode* code, AstNode* value, Token at) {
    AstNode* s = ast_alloc_node(((e)->arena), NODE_STRUCT_INIT, dz_tok(at, TOK_IDENT, (oc_string){ .ptr = (const uint8_t*)("__ErrorUnion"), .len = sizeof("__ErrorUnion") - 1 }));
    ((s)->type_name) = str_concat(((e)->arena), str_concat(((e)->arena), dz_ok_type(ut), (oc_string){ .ptr = (const uint8_t*)(", "), .len = sizeof(", ") - 1 }), dz_err_type(ut));
    AstNode* fc = ast_alloc_node(((e)->arena), NODE_PARAM, dz_tok(at, TOK_IDENT, (oc_string){ .ptr = (const uint8_t*)("code"), .len = sizeof("code") - 1 }));
    ((fc)->left) = code;
    ((s)->left) = fc;
    if ((value != ((AstNode*)(0)))) {
    AstNode* fv = ast_alloc_node(((e)->arena), NODE_PARAM, dz_tok(at, TOK_IDENT, (oc_string){ .ptr = (const uint8_t*)("value"), .len = sizeof("value") - 1 }));
    ((fv)->left) = value;
    ((fc)->next) = fv;
    }
    return s;
}

void dz_expect(Errs* e, AstNode* x, oc_string expected) {
    if ((x == ((AstNode*)(0)))) {
    return;
    }
    AstNode* want = dz_enum(e, expected);
    if ((want != ((AstNode*)(0)))) {
    if ((((x)->tag) == NODE_INT)) {
    dz_err(e, ((x)->tok), (oc_string){ .ptr = (const uint8_t*)("values of "), .len = sizeof("values of ") - 1 }, expected, str_concat(((e)->arena), (oc_string){ .ptr = (const uint8_t*)(" are written as "), .len = sizeof(" are written as ") - 1 }, str_concat(((e)->arena), expected, (oc_string){ .ptr = (const uint8_t*)(".Variant, not as numbers"), .len = sizeof(".Variant, not as numbers") - 1 })));
    return;
    }
    oc_string got = dz_type_of(e, x);
    if ((dz_enum(e, got) != ((AstNode*)(0)))) {
    if ((!str_eq(got, expected))) {
    dz_err(e, ((x)->tok), (oc_string){ .ptr = (const uint8_t*)("expected "), .len = sizeof("expected ") - 1 }, expected, str_concat(((e)->arena), (oc_string){ .ptr = (const uint8_t*)(", found "), .len = sizeof(", found ") - 1 }, got));
    return;
    }
    }
    }
    if ((((x)->tag) == NODE_CALL)) {
    bool is_ok = str_eq(((((x)->tok)).slice), (oc_string){ .ptr = (const uint8_t*)("Ok"), .len = sizeof("Ok") - 1 });
    bool is_err = str_eq(((((x)->tok)).slice), (oc_string){ .ptr = (const uint8_t*)("Err"), .len = sizeof("Err") - 1 });
    bool ctor = is_ok;
    if (is_err) {
    ctor = true;
    }
    if (ctor) {
    if ((dz_bang(expected) < 0)) {
    dz_err(e, ((x)->tok), ((((x)->tok)).slice), (oc_string){ .ptr = (const uint8_t*)("(...) needs an error-union type (T!E) here: return it, or store it in a variable declared with one"), .len = sizeof("(...) needs an error-union type (T!E) here: return it, or store it in a variable declared with one") - 1 }, (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 });
    return;
    }
    AstNode* arg = ((x)->left);
    if ((arg == ((AstNode*)(0)))) {
    dz_err(e, ((x)->tok), ((((x)->tok)).slice), (oc_string){ .ptr = (const uint8_t*)("(...) takes one value"), .len = sizeof("(...) takes one value") - 1 }, (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 });
    return;
    }
    AstNode* lit = ((AstNode*)(0));
    if (is_ok) {
    dz_expect(e, arg, dz_ok_type(expected));
    lit = dz_union_lit(e, expected, dz_int(e, (oc_string){ .ptr = (const uint8_t*)("0"), .len = sizeof("0") - 1 }, ((x)->tok)), arg, ((x)->tok));
    }
    else {
    dz_expect(e, arg, dz_err_type(expected));
    AstNode* as_code = dz_node(e, NODE_CAST, ((arg)->tok), ((((arg)->tok)).slice));
    ((as_code)->type_name) = (oc_string){ .ptr = (const uint8_t*)("u64"), .len = sizeof("u64") - 1 };
    ((as_code)->left) = arg;
    lit = dz_union_lit(e, expected, dz_bin(e, TOK_PLUS, (oc_string){ .ptr = (const uint8_t*)("+"), .len = sizeof("+") - 1 }, as_code, dz_int(e, (oc_string){ .ptr = (const uint8_t*)("1"), .len = sizeof("1") - 1 }, ((x)->tok)), ((x)->tok)), ((AstNode*)(0)), ((x)->tok));
    }
    ((x)->tag) = ((lit)->tag);
    ((x)->tok) = ((lit)->tok);
    ((x)->type_name) = ((lit)->type_name);
    ((x)->left) = ((lit)->left);
    ((x)->right) = ((AstNode*)(0));
    return;
    }
    }
    dz_expr(e, x);
}

void dz_expr(Errs* e, AstNode* x) {
    if ((x == ((AstNode*)(0)))) {
    return;
    }
    if ((((x)->tag) == NODE_TRY)) {
    dz_try(e, x);
    return;
    }
    if ((((x)->tag) == NODE_CATCH)) {
    dz_catch(e, x);
    return;
    }
    if ((((x)->tag) == NODE_FIELD_ACCESS)) {
    if ((((x)->left) != ((AstNode*)(0)))) {
    if ((((((x)->left))->tag) == NODE_IDENT)) {
    AstNode* en = dz_enum(e, ((((((x)->left))->tok)).slice));
    if ((en != ((AstNode*)(0)))) {
    int32_t k = dz_variant(en, ((((x)->tok)).slice));
    if ((k < 0)) {
    dz_err(e, ((x)->tok), (oc_string){ .ptr = (const uint8_t*)("no such variant in "), .len = sizeof("no such variant in ") - 1 }, ((((en)->tok)).slice), str_concat(((e)->arena), (oc_string){ .ptr = (const uint8_t*)(": "), .len = sizeof(": ") - 1 }, ((((x)->tok)).slice)));
    return;
    }
    AstNode* num = dz_int(e, (oc_string){ .ptr = (const uint8_t*)("0"), .len = sizeof("0") - 1 }, ((x)->tok));
    uint8_t digits[10] = {0};
    size_t pos = 10;
    uint32_t val = ({ __auto_type _v = (k); if (_v < 0 || (uint64_t)_v > 4294967295ULL) { fprintf(stderr, "panic: invalid unsigned cast\n"); exit(1); } (uint32_t)_v; });
    while ((pos > 0)) {
    pos = __OC_SUB(pos, 1);
    __OC_AT((digits), (pos)) = ({ __auto_type _v = (__OC_ADD(48, __OC_MOD(val, 10))); if (_v < 0 || (uint64_t)_v > 255ULL) { fprintf(stderr, "panic: invalid unsigned cast\n"); exit(1); } (uint8_t)_v; });
    val = __OC_DIV(val, 10);
    if ((val == 0)) {
    break;
    }
    }
    StrBuf sb = strbuf_init(((e)->arena), 12);
    while ((pos < 10)) {
    strbuf_push((&(sb)), __OC_AT((digits), (pos)));
    pos = __OC_ADD(pos, 1);
    }
    ((x)->tag) = NODE_INT;
    ((x)->tok) = ((num)->tok);
    ((((x)->tok)).slice) = gx_done(sb);
    ((x)->left) = ((AstNode*)(0));
    return;
    }
    if ((dz_bang(dz_sym(e, ((((((x)->left))->tok)).slice))) >= 0)) {
    bool internal = str_eq(((((x)->tok)).slice), (oc_string){ .ptr = (const uint8_t*)("code"), .len = sizeof("code") - 1 });
    if (str_eq(((((x)->tok)).slice), (oc_string){ .ptr = (const uint8_t*)("value"), .len = sizeof("value") - 1 })) {
    internal = true;
    }
    if (internal) {
    if ((!starts_with(((((((x)->left))->tok)).slice), (oc_string){ .ptr = (const uint8_t*)("__err"), .len = sizeof("__err") - 1 }))) {
    dz_err(e, ((x)->tok), (oc_string){ .ptr = (const uint8_t*)("the payload of an error union is read with match, try or catch, not ."), .len = sizeof("the payload of an error union is read with match, try or catch, not .") - 1 }, ((((x)->tok)).slice), (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 });
    return;
    }
    }
    }
    }
    }
    }
    if ((((x)->tag) == NODE_CALL)) {
    bool is_ctor = str_eq(((((x)->tok)).slice), (oc_string){ .ptr = (const uint8_t*)("Ok"), .len = sizeof("Ok") - 1 });
    if (str_eq(((((x)->tok)).slice), (oc_string){ .ptr = (const uint8_t*)("Err"), .len = sizeof("Err") - 1 })) {
    is_ctor = true;
    }
    if (is_ctor) {
    dz_expect(e, x, (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 });
    return;
    }
    AstNode* f = program_find_fn(((e)->root), ((((x)->tok)).slice));
    AstNode* prm = ((AstNode*)(0));
    if ((f != ((AstNode*)(0)))) {
    prm = ((f)->left);
    }
    AstNode* a = ((x)->left);
    while ((a != ((AstNode*)(0)))) {
    if ((prm != ((AstNode*)(0)))) {
    dz_expect(e, a, ((prm)->type_name));
    prm = ((prm)->next);
    }
    else {
    dz_expr(e, a);
    }
    a = ((a)->next);
    }
    return;
    }
    bool is_list = (((x)->tag) == NODE_STRUCT_INIT);
    if ((((x)->tag) == NODE_ARRAY_INIT)) {
    is_list = true;
    }
    if (is_list) {
    AstNode* el = ((x)->left);
    while ((el != ((AstNode*)(0)))) {
    if ((((x)->tag) == NODE_STRUCT_INIT)) {
    dz_expr(e, ((el)->left));
    }
    else {
    dz_expr(e, el);
    }
    el = ((el)->next);
    }
    return;
    }
    dz_expr(e, ((x)->left));
    dz_expr(e, ((x)->right));
}

AstNode* dz_arm_body(Errs* e, AstNode* body) {
    if ((body == ((AstNode*)(0)))) {
    return body;
    }
    if ((((body)->tag) == NODE_BLOCK)) {
    return dz_stmts(e, ((body)->left));
    }
    ((body)->next) = ((AstNode*)(0));
    return dz_stmts(e, body);
}

AstNode* dz_append(AstNode* first, AstNode* more) {
    if ((first == ((AstNode*)(0)))) {
    return more;
    }
    AstNode* t = first;
    while ((((t)->next) != ((AstNode*)(0)))) {
    t = ((t)->next);
    }
    ((t)->next) = more;
    return first;
}

AstNode* dz_match(Errs* e, AstNode* s) {
    dz_expr(e, ((s)->left));
    oc_string st = dz_type_of(e, ((s)->left));
    oc_string tmp = dz_tmp(e);
    Token at = ((s)->tok);
    if ((dz_bang(st) >= 0)) {
    AstNode* ok_arm = ((AstNode*)(0));
    AstNode* err_arm = ((AstNode*)(0));
    AstNode* any_arm = ((AstNode*)(0));
    AstNode* a = ((s)->right);
    while ((a != ((AstNode*)(0)))) {
    if (str_eq(((((a)->tok)).slice), (oc_string){ .ptr = (const uint8_t*)("Ok"), .len = sizeof("Ok") - 1 })) {
    if ((ok_arm != ((AstNode*)(0)))) {
    dz_err(e, ((a)->tok), (oc_string){ .ptr = (const uint8_t*)("Ok is matched twice"), .len = sizeof("Ok is matched twice") - 1 }, (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 }, (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 });
    }
    ok_arm = a;
    }
    else {
    if (str_eq(((((a)->tok)).slice), (oc_string){ .ptr = (const uint8_t*)("Err"), .len = sizeof("Err") - 1 })) {
    if ((err_arm != ((AstNode*)(0)))) {
    dz_err(e, ((a)->tok), (oc_string){ .ptr = (const uint8_t*)("Err is matched twice"), .len = sizeof("Err is matched twice") - 1 }, (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 }, (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 });
    }
    err_arm = a;
    }
    else {
    if (str_eq(((((a)->tok)).slice), (oc_string){ .ptr = (const uint8_t*)("_"), .len = sizeof("_") - 1 })) {
    any_arm = a;
    }
    else {
    dz_err(e, ((a)->tok), (oc_string){ .ptr = (const uint8_t*)("a match on an error union has Ok(..) and Err(..) arms, not "), .len = sizeof("a match on an error union has Ok(..) and Err(..) arms, not ") - 1 }, ((((a)->tok)).slice), (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 });
    }
    }
    }
    a = ((a)->next);
    }
    if ((ok_arm == ((AstNode*)(0)))) {
    ok_arm = any_arm;
    }
    if ((err_arm == ((AstNode*)(0)))) {
    err_arm = any_arm;
    }
    if ((ok_arm == ((AstNode*)(0)))) {
    dz_err(e, at, (oc_string){ .ptr = (const uint8_t*)("this match does not handle Ok(..)"), .len = sizeof("this match does not handle Ok(..)") - 1 }, (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 }, (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 });
    return s;
    }
    if ((err_arm == ((AstNode*)(0)))) {
    dz_err(e, at, (oc_string){ .ptr = (const uint8_t*)("this match does not handle Err(..): every error must be handled (or add `_ =>`)"), .len = sizeof("this match does not handle Err(..): every error must be handled (or add `_ =>`)") - 1 }, (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 }, (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 });
    return s;
    }
    AstNode* head = dz_let(e, tmp, st, ((s)->left), false, at);
    AstNode* ok_stmts = ((AstNode*)(0));
    if (str_eq(((((ok_arm)->tok)).slice), (oc_string){ .ptr = (const uint8_t*)("Ok"), .len = sizeof("Ok") - 1 })) {
    if ((((((ok_arm)->type_name)).len) > 0)) {
    if ((!str_eq(((ok_arm)->type_name), (oc_string){ .ptr = (const uint8_t*)("_"), .len = sizeof("_") - 1 }))) {
    ok_stmts = dz_let(e, ((ok_arm)->type_name), dz_ok_type(st), dz_field(e, dz_ident(e, tmp, at), (oc_string){ .ptr = (const uint8_t*)("value"), .len = sizeof("value") - 1 }, at), false, ((ok_arm)->tok));
    }
    }
    }
    ok_stmts = dz_append(ok_stmts, dz_arm_body(e, ((ok_arm)->left)));
    AstNode* err_stmts = ((AstNode*)(0));
    if (str_eq(((((err_arm)->tok)).slice), (oc_string){ .ptr = (const uint8_t*)("Err"), .len = sizeof("Err") - 1 })) {
    if ((((((err_arm)->type_name)).len) > 0)) {
    if ((!str_eq(((err_arm)->type_name), (oc_string){ .ptr = (const uint8_t*)("_"), .len = sizeof("_") - 1 }))) {
    AstNode* minus = dz_bin(e, TOK_MINUS, (oc_string){ .ptr = (const uint8_t*)("-"), .len = sizeof("-") - 1 }, dz_field(e, dz_ident(e, tmp, at), (oc_string){ .ptr = (const uint8_t*)("code"), .len = sizeof("code") - 1 }, at), dz_int(e, (oc_string){ .ptr = (const uint8_t*)("1"), .len = sizeof("1") - 1 }, at), at);
    AstNode* as_e = dz_node(e, NODE_CAST, at, (oc_string){ .ptr = (const uint8_t*)("as"), .len = sizeof("as") - 1 });
    ((as_e)->type_name) = dz_err_type(st);
    ((as_e)->left) = minus;
    err_stmts = dz_let(e, ((err_arm)->type_name), dz_err_type(st), as_e, false, ((err_arm)->tok));
    }
    }
    }
    err_stmts = dz_append(err_stmts, dz_arm_body(e, ((err_arm)->left)));
    AstNode* cond = dz_bin(e, TOK_EQEQ, (oc_string){ .ptr = (const uint8_t*)("=="), .len = sizeof("==") - 1 }, dz_field(e, dz_ident(e, tmp, at), (oc_string){ .ptr = (const uint8_t*)("code"), .len = sizeof("code") - 1 }, at), dz_int(e, (oc_string){ .ptr = (const uint8_t*)("0"), .len = sizeof("0") - 1 }, at), at);
    ((head)->next) = dz_if(e, cond, ok_stmts, dz_block(e, err_stmts, at), at);
    return head;
    }
    AstNode* en = dz_enum(e, st);
    if ((en == ((AstNode*)(0)))) {
    dz_err(e, at, (oc_string){ .ptr = (const uint8_t*)("match needs an enum value or an error union (T!E); this value's type is not known to be one"), .len = sizeof("match needs an enum value or an error union (T!E); this value's type is not known to be one") - 1 }, (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 }, (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 });
    return s;
    }
    bool seen[64] = {0};
    AstNode* any_arm = ((AstNode*)(0));
    AstNode* chain_head = ((AstNode*)(0));
    AstNode* chain_tail = ((AstNode*)(0));
    AstNode* a = ((s)->right);
    while ((a != ((AstNode*)(0)))) {
    if (str_eq(((((a)->tok)).slice), (oc_string){ .ptr = (const uint8_t*)("_"), .len = sizeof("_") - 1 })) {
    any_arm = a;
    }
    else {
    if ((((a)->right) != ((AstNode*)(0)))) {
    if ((!str_eq(((((((a)->right))->tok)).slice), ((((en)->tok)).slice)))) {
    dz_err(e, ((a)->tok), (oc_string){ .ptr = (const uint8_t*)("this match is on "), .len = sizeof("this match is on ") - 1 }, ((((en)->tok)).slice), str_concat(((e)->arena), (oc_string){ .ptr = (const uint8_t*)(", not "), .len = sizeof(", not ") - 1 }, ((((((a)->right))->tok)).slice)));
    a = ((a)->next);
    continue;
    }
    }
    int32_t k = dz_variant(en, ((((a)->tok)).slice));
    if ((k < 0)) {
    dz_err(e, ((a)->tok), (oc_string){ .ptr = (const uint8_t*)("no such variant in "), .len = sizeof("no such variant in ") - 1 }, ((((en)->tok)).slice), str_concat(((e)->arena), (oc_string){ .ptr = (const uint8_t*)(": "), .len = sizeof(": ") - 1 }, ((((a)->tok)).slice)));
    }
    else {
    if ((k < 64)) {
    if (__OC_AT((seen), (({ __auto_type _v = (k); if (_v < 0) { fprintf(stderr, "panic: invalid unsigned cast\n"); exit(1); } (size_t)_v; })))) {
    dz_err(e, ((a)->tok), (oc_string){ .ptr = (const uint8_t*)("variant matched twice: "), .len = sizeof("variant matched twice: ") - 1 }, ((((a)->tok)).slice), (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 });
    }
    __OC_AT((seen), (({ __auto_type _v = (k); if (_v < 0) { fprintf(stderr, "panic: invalid unsigned cast\n"); exit(1); } (size_t)_v; }))) = true;
    size_t pos = 10;
    uint8_t digits[10] = {0};
    uint32_t val = ({ __auto_type _v = (k); if (_v < 0 || (uint64_t)_v > 4294967295ULL) { fprintf(stderr, "panic: invalid unsigned cast\n"); exit(1); } (uint32_t)_v; });
    while ((pos > 0)) {
    pos = __OC_SUB(pos, 1);
    __OC_AT((digits), (pos)) = ({ __auto_type _v = (__OC_ADD(48, __OC_MOD(val, 10))); if (_v < 0 || (uint64_t)_v > 255ULL) { fprintf(stderr, "panic: invalid unsigned cast\n"); exit(1); } (uint8_t)_v; });
    val = __OC_DIV(val, 10);
    if ((val == 0)) {
    break;
    }
    }
    StrBuf sb = strbuf_init(((e)->arena), 12);
    while ((pos < 10)) {
    strbuf_push((&(sb)), __OC_AT((digits), (pos)));
    pos = __OC_ADD(pos, 1);
    }
    AstNode* cond = dz_bin(e, TOK_EQEQ, (oc_string){ .ptr = (const uint8_t*)("=="), .len = sizeof("==") - 1 }, dz_ident(e, tmp, at), dz_int(e, gx_done(sb), ((a)->tok)), ((a)->tok));
    AstNode* arm_if = dz_if(e, cond, dz_arm_body(e, ((a)->left)), ((AstNode*)(0)), ((a)->tok));
    if ((chain_head == ((AstNode*)(0)))) {
    chain_head = arm_if;
    }
    else {
    ((((chain_tail)->right))->next) = arm_if;
    }
    chain_tail = arm_if;
    }
    }
    }
    a = ((a)->next);
    }
    if ((any_arm == ((AstNode*)(0)))) {
    AstNode* v = ((en)->left);
    size_t k = 0;
    while ((v != ((AstNode*)(0)))) {
    if ((k < 64)) {
    if ((!__OC_AT((seen), (k)))) {
    dz_err(e, at, str_concat(((e)->arena), str_concat(((e)->arena), (oc_string){ .ptr = (const uint8_t*)("this match on "), .len = sizeof("this match on ") - 1 }, ((((en)->tok)).slice)), (oc_string){ .ptr = (const uint8_t*)(" does not handle "), .len = sizeof(" does not handle ") - 1 }), ((((v)->tok)).slice), (oc_string){ .ptr = (const uint8_t*)(" (add its arm, or `_ =>`)"), .len = sizeof(" (add its arm, or `_ =>`)") - 1 });
    }
    }
    v = ((v)->next);
    k = __OC_ADD(k, 1);
    }
    }
    else {
    AstNode* rest = dz_block(e, dz_arm_body(e, ((any_arm)->left)), at);
    if ((chain_head == ((AstNode*)(0)))) {
    chain_head = rest;
    }
    else {
    ((((chain_tail)->right))->next) = rest;
    }
    }
    AstNode* head = dz_let(e, tmp, st, ((s)->left), false, at);
    ((head)->next) = chain_head;
    return head;
}

void dz_try(Errs* e, AstNode* t) {
    Token at = ((t)->tok);
    AstNode* call = ((t)->left);
    dz_expr(e, call);
    oc_string ut = dz_type_of(e, call);
    if ((dz_bang(ut) < 0)) {
    dz_err(e, at, (oc_string){ .ptr = (const uint8_t*)("try needs an error union (T!E)"), .len = sizeof("try needs an error union (T!E)") - 1 }, (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 }, (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 });
    return;
    }
    if ((dz_bang(((e)->fn_ret)) < 0)) {
    dz_err(e, at, (oc_string){ .ptr = (const uint8_t*)("try passes the error on to the caller, so this function must return an error union (T!E)"), .len = sizeof("try passes the error on to the caller, so this function must return an error union (T!E)") - 1 }, (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 }, (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 });
    return;
    }
    if ((!str_eq(dz_err_type(ut), dz_err_type(((e)->fn_ret))))) {
    dz_err(e, at, (oc_string){ .ptr = (const uint8_t*)("try passes on errors of type "), .len = sizeof("try passes on errors of type ") - 1 }, dz_err_type(ut), str_concat(((e)->arena), (oc_string){ .ptr = (const uint8_t*)(", but this function's errors are "), .len = sizeof(", but this function's errors are ") - 1 }, dz_err_type(((e)->fn_ret))));
    return;
    }
    oc_string tmp = dz_tmp(e);
    AstNode* head = dz_let(e, tmp, ut, call, false, at);
    AstNode* fail = dz_bin(e, TOK_NEQ, (oc_string){ .ptr = (const uint8_t*)("!="), .len = sizeof("!=") - 1 }, dz_field(e, dz_ident(e, tmp, at), (oc_string){ .ptr = (const uint8_t*)("code"), .len = sizeof("code") - 1 }, at), dz_int(e, (oc_string){ .ptr = (const uint8_t*)("0"), .len = sizeof("0") - 1 }, at), at);
    AstNode* ret = ast_alloc_node(((e)->arena), NODE_RETURN, dz_tok(at, TOK_RETURN, (oc_string){ .ptr = (const uint8_t*)("return"), .len = sizeof("return") - 1 }));
    ((ret)->left) = dz_union_lit(e, ((e)->fn_ret), dz_field(e, dz_ident(e, tmp, at), (oc_string){ .ptr = (const uint8_t*)("code"), .len = sizeof("code") - 1 }, at), ((AstNode*)(0)), at);
    ((head)->next) = dz_if(e, fail, ret, ((AstNode*)(0)), at);
    ((e)->pre) = dz_append(((e)->pre), head);
    AstNode* value = dz_field(e, dz_ident(e, tmp, at), (oc_string){ .ptr = (const uint8_t*)("value"), .len = sizeof("value") - 1 }, at);
    ((t)->tag) = NODE_FIELD_ACCESS;
    ((t)->tok) = ((value)->tok);
    ((t)->left) = ((value)->left);
    ((t)->right) = ((AstNode*)(0));
}

void dz_catch(Errs* e, AstNode* c) {
    Token at = ((c)->tok);
    dz_expr(e, ((c)->left));
    oc_string ut = dz_type_of(e, ((c)->left));
    if ((dz_bang(ut) < 0)) {
    dz_err(e, at, (oc_string){ .ptr = (const uint8_t*)("catch needs an error union (T!E) on its left"), .len = sizeof("catch needs an error union (T!E) on its left") - 1 }, (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 }, (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 });
    return;
    }
    oc_string ok_t = dz_ok_type(ut);
    oc_string tmp = dz_tmp(e);
    oc_string val = dz_tmp(e);
    AstNode* head = dz_let(e, tmp, ut, ((c)->left), false, at);
    AstNode* v = dz_let(e, val, ok_t, dz_field(e, dz_ident(e, tmp, at), (oc_string){ .ptr = (const uint8_t*)("value"), .len = sizeof("value") - 1 }, at), true, at);
    ((head)->next) = v;
    AstNode* outer = ((e)->pre);
    ((e)->pre) = ((AstNode*)(0));
    dz_expect(e, ((c)->right), ok_t);
    AstNode* on_err = dz_append(((e)->pre), dz_assign(e, dz_ident(e, val, at), ((c)->right), at));
    ((e)->pre) = outer;
    AstNode* fail = dz_bin(e, TOK_NEQ, (oc_string){ .ptr = (const uint8_t*)("!="), .len = sizeof("!=") - 1 }, dz_field(e, dz_ident(e, tmp, at), (oc_string){ .ptr = (const uint8_t*)("code"), .len = sizeof("code") - 1 }, at), dz_int(e, (oc_string){ .ptr = (const uint8_t*)("0"), .len = sizeof("0") - 1 }, at), at);
    ((v)->next) = dz_if(e, fail, on_err, ((AstNode*)(0)), at);
    ((e)->pre) = dz_append(((e)->pre), head);
    ((c)->tag) = NODE_IDENT;
    ((c)->tok) = dz_tok(at, TOK_IDENT, val);
    ((c)->left) = ((AstNode*)(0));
    ((c)->right) = ((AstNode*)(0));
}

AstNode* dz_stmt_in(Errs* e, AstNode* s) {
    if ((((s)->tag) == NODE_BLOCK)) {
    ((s)->left) = dz_stmts(e, ((s)->left));
    return s;
    }
    if ((((s)->tag) == NODE_IF)) {
    dz_expr(e, ((s)->left));
    ((((s)->right))->left) = dz_stmts(e, ((((s)->right))->left));
    AstNode* els = ((((s)->right))->next);
    if ((els != ((AstNode*)(0)))) {
    if ((((els)->tag) == NODE_BLOCK)) {
    ((els)->left) = dz_stmts(e, ((els)->left));
    }
    if ((((els)->tag) == NODE_IF)) {
    AstNode* lowered = dz_stmt(e, els);
    if ((lowered != els)) {
    ((((s)->right))->next) = dz_block(e, lowered, ((els)->tok));
    }
    }
    }
    return s;
    }
    if ((((s)->tag) == NODE_WHILE)) {
    dz_expr(e, ((s)->left));
    if ((((e)->pre) != ((AstNode*)(0)))) {
    dz_err(e, ((((s)->left))->tok), (oc_string){ .ptr = (const uint8_t*)("try and catch cannot be used in a while condition (it is evaluated again on every iteration): compute the value in the loop body"), .len = sizeof("try and catch cannot be used in a while condition (it is evaluated again on every iteration): compute the value in the loop body") - 1 }, (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 }, (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 });
    }
    if ((((s)->right) != ((AstNode*)(0)))) {
    ((((s)->right))->left) = dz_stmts(e, ((((s)->right))->left));
    }
    return s;
    }
    if ((((s)->tag) == NODE_MATCH)) {
    return dz_match(e, s);
    }
    if ((((s)->tag) == NODE_TRY)) {
    dz_try(e, s);
    return ((AstNode*)(0));
    }
    if ((((s)->tag) == NODE_LET)) {
    if ((((((s)->type_name)).len) == 0)) {
    if ((((s)->left) != ((AstNode*)(0)))) {
    AstNode* inner = ((AstNode*)(0));
    if ((((((s)->left))->tag) == NODE_TRY)) {
    inner = ((((s)->left))->left);
    }
    if ((((((s)->left))->tag) == NODE_CATCH)) {
    inner = ((((s)->left))->left);
    }
    if ((inner != ((AstNode*)(0)))) {
    oc_string ut = dz_type_of(e, inner);
    if ((dz_bang(ut) >= 0)) {
    ((s)->type_name) = dz_ok_type(ut);
    }
    }
    }
    }
    dz_expect(e, ((s)->left), ((s)->type_name));
    oc_string t = ((s)->type_name);
    if ((((t).len) == 0)) {
    t = dz_type_of(e, ((s)->left));
    }
    dz_sym_add(e, ((((s)->tok)).slice), t);
    return s;
    }
    if ((((s)->tag) == NODE_ASSIGN)) {
    dz_expr(e, ((s)->left));
    dz_expect(e, ((s)->right), dz_type_of(e, ((s)->left)));
    return s;
    }
    if ((((s)->tag) == NODE_RETURN)) {
    if ((((s)->left) != ((AstNode*)(0)))) {
    if ((((((s)->left))->tag) == NODE_TRY)) {
    if ((dz_bang(((e)->fn_ret)) >= 0)) {
    AstNode* value = ((s)->left);
    dz_expr(e, value);
    ((s)->left) = dz_union_lit(e, ((e)->fn_ret), dz_int(e, (oc_string){ .ptr = (const uint8_t*)("0"), .len = sizeof("0") - 1 }, ((s)->tok)), value, ((s)->tok));
    return s;
    }
    }
    }
    dz_expect(e, ((s)->left), ((e)->fn_ret));
    return s;
    }
    if ((((s)->tag) == NODE_CALL)) {
    oc_string rt = dz_type_of(e, s);
    if ((dz_bang(rt) >= 0)) {
    dz_err(e, ((s)->tok), (oc_string){ .ptr = (const uint8_t*)("the error union returned by "), .len = sizeof("the error union returned by ") - 1 }, ((((s)->tok)).slice), (oc_string){ .ptr = (const uint8_t*)(" is ignored: handle it with match, try or catch"), .len = sizeof(" is ignored: handle it with match, try or catch") - 1 });
    }
    }
    dz_expr(e, s);
    return s;
}

AstNode* dz_stmt(Errs* e, AstNode* s) {
    AstNode* outer = ((e)->pre);
    ((e)->pre) = ((AstNode*)(0));
    AstNode* lowered = dz_stmt_in(e, s);
    AstNode* out = dz_append(((e)->pre), lowered);
    ((e)->pre) = outer;
    return out;
}

AstNode* dz_stmts(Errs* e, AstNode* first) {
    size_t scope = ((e)->sym_count);
    AstNode* head = ((AstNode*)(0));
    AstNode* s = first;
    while ((s != ((AstNode*)(0)))) {
    AstNode* nxt = ((s)->next);
    ((s)->next) = ((AstNode*)(0));
    head = dz_append(head, dz_stmt(e, s));
    s = nxt;
    }
    ((e)->sym_count) = scope;
    return head;
}

oc_string dz_type(Errs* e, oc_string t) {
    if ((((t).len) == 0)) {
    return t;
    }
    StrBuf sb = strbuf_init(((e)->arena), __OC_ADD(((t).len), 24));
    size_t i = 0;
    while ((i < ((t).len))) {
    if (gx_is_ident_char(__oc_index_str(t, i))) {
    size_t j = i;
    while ((j < ((t).len))) {
    if ((!gx_is_ident_char(__oc_index_str(t, j)))) {
    break;
    }
    j = __OC_ADD(j, 1);
    }
    if ((dz_enum(e, __oc_slice_str(t, i, j)) != ((AstNode*)(0)))) {
    strbuf_push_str((&(sb)), (oc_string){ .ptr = (const uint8_t*)("u32"), .len = sizeof("u32") - 1 });
    }
    else {
    strbuf_push_str((&(sb)), __oc_slice_str(t, i, j));
    }
    i = j;
    }
    else {
    strbuf_push((&(sb)), __oc_index_str(t, i));
    i = __OC_ADD(i, 1);
    }
    }
    oc_string s1 = gx_done(sb);
    int64_t b = dz_bang(s1);
    if ((b < 0)) {
    return s1;
    }
    StrBuf u = strbuf_init(((e)->arena), __OC_ADD(((s1).len), 24));
    strbuf_push_str((&(u)), (oc_string){ .ptr = (const uint8_t*)("__ErrorUnion<"), .len = sizeof("__ErrorUnion<") - 1 });
    strbuf_push_str((&(u)), dz_type(e, trim(__oc_slice_str(s1, 0, ({ __auto_type _v = (b); if (_v < 0) { fprintf(stderr, "panic: invalid unsigned cast\n"); exit(1); } (size_t)_v; })))));
    strbuf_push_str((&(u)), (oc_string){ .ptr = (const uint8_t*)(","), .len = sizeof(",") - 1 });
    strbuf_push_str((&(u)), dz_type(e, trim(__oc_slice_str(s1, __OC_ADD(({ __auto_type _v = (b); if (_v < 0) { fprintf(stderr, "panic: invalid unsigned cast\n"); exit(1); } (size_t)_v; }), 1), ((s1).len)))));
    strbuf_push_str((&(u)), (oc_string){ .ptr = (const uint8_t*)(">"), .len = sizeof(">") - 1 });
    return gx_done(u);
}

void dz_types(Errs* e, AstNode* n) {
    AstNode* cur = n;
    while ((cur != ((AstNode*)(0)))) {
    if ((((((cur)->type_name)).len) > 0)) {
    bool is_type = (((cur)->tag) != NODE_ADDRESS_OF);
    if ((((cur)->tag) == NODE_FLOAT)) {
    is_type = false;
    }
    if ((((cur)->tag) == NODE_STRUCT_DECL)) {
    is_type = false;
    }
    if (is_type) {
    ((cur)->type_name) = dz_type(e, ((cur)->type_name));
    }
    }
    dz_types(e, ((cur)->left));
    dz_types(e, ((cur)->right));
    cur = ((cur)->next);
    }
}

void dz_fn(Errs* e, AstNode* f) {
    if ((((f)->right) == ((AstNode*)(0)))) {
    return;
    }
    ((e)->fn_ret) = ((f)->type_name);
    ((e)->sym_count) = 0;
    AstNode* p = ((f)->left);
    while ((p != ((AstNode*)(0)))) {
    dz_sym_add(e, ((((p)->tok)).slice), ((p)->type_name));
    p = ((p)->next);
    }
    ((((f)->right))->left) = dz_stmts(e, ((((f)->right))->left));
}

bool desugar_errors(AstNode* root, Arena* arena, SourceMap* m) {
    Errs e = (Errs){ .enums = { ((AstNode*)(0)) }, .enum_count = 0, .arena = arena, .m = m, .root = root, .ok = true, .tmp_n = 0, .fn_ret = (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 }, .syms = { (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 } }, .sym_types = { (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 } }, .sym_count = 0, .pre = ((AstNode*)(0)) };
    AstNode* prev = ((AstNode*)(0));
    AstNode* last = ((AstNode*)(0));
    AstNode* d = ((root)->left);
    while ((d != ((AstNode*)(0)))) {
    AstNode* nxt = ((d)->next);
    if ((((d)->tag) == NODE_ENUM)) {
    if ((dz_enum((&(e)), ((((d)->tok)).slice)) != ((AstNode*)(0)))) {
    dz_err((&(e)), ((d)->tok), (oc_string){ .ptr = (const uint8_t*)("enum defined twice: "), .len = sizeof("enum defined twice: ") - 1 }, ((((d)->tok)).slice), (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 });
    }
    else {
    if ((((e).enum_count) < 64)) {
    __OC_AT((((e).enums)), (((e).enum_count))) = d;
    ((e).enum_count) = __OC_ADD(((e).enum_count), 1);
    }
    }
    if ((prev == ((AstNode*)(0)))) {
    ((root)->left) = nxt;
    }
    else {
    ((prev)->next) = nxt;
    }
    }
    else {
    prev = d;
    }
    d = nxt;
    }
    last = prev;
    Token at = ((root)->tok);
    AstNode* tmpl = ast_alloc_node(arena, NODE_GENERIC, dz_tok(at, ((at).tag), (oc_string){ .ptr = (const uint8_t*)("__ErrorUnion"), .len = sizeof("__ErrorUnion") - 1 }));
    AstNode* pt = ast_alloc_node(arena, NODE_PARAM, dz_tok(at, ((at).tag), (oc_string){ .ptr = (const uint8_t*)("T"), .len = sizeof("T") - 1 }));
    ((pt)->next) = ast_alloc_node(arena, NODE_PARAM, dz_tok(at, ((at).tag), (oc_string){ .ptr = (const uint8_t*)("E"), .len = sizeof("E") - 1 }));
    ((tmpl)->left) = pt;
    AstNode* sd = ast_alloc_node(arena, NODE_STRUCT_DECL, dz_tok(at, ((at).tag), (oc_string){ .ptr = (const uint8_t*)("__ErrorUnion"), .len = sizeof("__ErrorUnion") - 1 }));
    AstNode* fcode = ast_alloc_node(arena, NODE_PARAM, dz_tok(at, ((at).tag), (oc_string){ .ptr = (const uint8_t*)("code"), .len = sizeof("code") - 1 }));
    ((fcode)->type_name) = (oc_string){ .ptr = (const uint8_t*)("u64"), .len = sizeof("u64") - 1 };
    AstNode* fval = ast_alloc_node(arena, NODE_PARAM, dz_tok(at, ((at).tag), (oc_string){ .ptr = (const uint8_t*)("value"), .len = sizeof("value") - 1 }));
    ((fval)->type_name) = (oc_string){ .ptr = (const uint8_t*)("T"), .len = sizeof("T") - 1 };
    ((fcode)->next) = fval;
    ((sd)->left) = fcode;
    ((tmpl)->right) = sd;
    if ((last == ((AstNode*)(0)))) {
    ((root)->left) = tmpl;
    }
    else {
    ((last)->next) = tmpl;
    }
    d = ((root)->left);
    while ((d != ((AstNode*)(0)))) {
    if ((((d)->tag) == NODE_FN)) {
    dz_fn((&(e)), d);
    }
    if ((((d)->tag) == NODE_GENERIC)) {
    if ((((((d)->right))->tag) == NODE_FN)) {
    dz_fn((&(e)), ((d)->right));
    }
    }
    if ((((d)->tag) == NODE_LET)) {
    dz_expect((&(e)), ((d)->left), ((d)->type_name));
    }
    d = ((d)->next);
    }
    d = ((root)->left);
    while ((d != ((AstNode*)(0)))) {
    AstNode* nxt = ((d)->next);
    ((d)->next) = ((AstNode*)(0));
    dz_types((&(e)), d);
    ((d)->next) = nxt;
    d = nxt;
    }
    return ((e).ok);
}

CodeBuffer buf_init(size_t capacity) {
    void* raw = sys_mmap(0, capacity, 3, 34, (-1), 0);
    return (CodeBuffer){ .buffer = ((uint8_t*)(raw)), .len = 0, .capacity = capacity, .ptr_vars = { (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 } }, .ptr_vars_count = 0, .str_vars = { (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 } }, .str_vars_count = 0, .ptr_fields = { (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 } }, .ptr_fields_count = 0, .str_fields = { (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 } }, .str_fields_count = 0, .ptr_fns = { (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 } }, .ptr_fns_count = 0, .str_fns = { (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 } }, .str_fns_count = 0, .var_names = { (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 } }, .var_types = { (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 } }, .var_types_len = 0, .flt_names = { (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 } }, .flt_types = { (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 } }, .flt_count = 0, .fp_trap = false, .arena_poison = false, .srcmap = ((SourceMap*)(0)), .program = ((AstNode*)(0)) };
}

void buf_append_u8(CodeBuffer* buf, uint8_t byte) {
    if ((((buf)->len) < ((buf)->capacity))) {
    __OC_AT((((buf)->buffer)), (((buf)->len))) = byte;
    ((buf)->len) = __OC_ADD(((buf)->len), 1);
    }
}

void buf_append(CodeBuffer* buf, oc_string text) {
    size_t i = 0;
    while ((i < ((text).len))) {
    buf_append_u8(buf, __oc_index_str(text, i));
    i = __OC_ADD(i, 1);
    }
}

void buf_append_u32(CodeBuffer* buf, uint32_t v) {
    uint8_t digits[10] = {0};
    size_t n = 0;
    uint32_t x = v;
    while ((n < 10)) {
    __OC_AT((digits), (n)) = ({ __auto_type _v = (__OC_ADD(48, __OC_MOD(x, 10))); if (_v < 0 || (uint64_t)_v > 255ULL) { fprintf(stderr, "panic: invalid unsigned cast\n"); exit(1); } (uint8_t)_v; });
    n = __OC_ADD(n, 1);
    x = __OC_DIV(x, 10);
    if ((x == 0)) {
    break;
    }
    }
    while ((n > 0)) {
    n = __OC_SUB(n, 1);
    buf_append_u8(buf, __OC_AT((digits), (n)));
    }
}

void buf_append_src_loc(CodeBuffer* buf, SourceMap* m, Token tok, bool c_escape) {
    int32_t f = srcmap_file(m, ((tok).line));
    if ((f >= 0)) {
    oc_string path = __OC_AT((((m)->path)), (({ __auto_type _v = (f); if (_v < 0) { fprintf(stderr, "panic: invalid unsigned cast\n"); exit(1); } (size_t)_v; })));
    size_t i = 0;
    while ((i < ((path).len))) {
    if (c_escape) {
    if ((__oc_index_str(path, i) == 92)) {
    buf_append_u8(buf, 92);
    }
    if ((__oc_index_str(path, i) == 34)) {
    buf_append_u8(buf, 92);
    }
    }
    buf_append_u8(buf, __oc_index_str(path, i));
    i = __OC_ADD(i, 1);
    }
    buf_append_u8(buf, 58);
    buf_append_u32(buf, __OC_ADD(__OC_SUB(((tok).line), __OC_AT((((m)->first_line)), (({ __auto_type _v = (f); if (_v < 0) { fprintf(stderr, "panic: invalid unsigned cast\n"); exit(1); } (size_t)_v; })))), 1));
    }
    else {
    buf_append_u32(buf, ((tok).line));
    }
    buf_append_u8(buf, 58);
    buf_append_u32(buf, ((tok).col));
}

void cg_fchk_close(CodeBuffer* out, Token tok) {
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)(", \042"), .len = sizeof(", \042") - 1 });
    buf_append_src_loc(out, ((out)->srcmap), tok, true);
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("\042)"), .len = sizeof("\042)") - 1 });
}

bool is_c_reserved(oc_string name) {
    oc_string words[56] = {0};
    __OC_AT((words), (0)) = (oc_string){ .ptr = (const uint8_t*)("auto"), .len = sizeof("auto") - 1 };
    __OC_AT((words), (1)) = (oc_string){ .ptr = (const uint8_t*)("break"), .len = sizeof("break") - 1 };
    __OC_AT((words), (2)) = (oc_string){ .ptr = (const uint8_t*)("case"), .len = sizeof("case") - 1 };
    __OC_AT((words), (3)) = (oc_string){ .ptr = (const uint8_t*)("char"), .len = sizeof("char") - 1 };
    __OC_AT((words), (4)) = (oc_string){ .ptr = (const uint8_t*)("const"), .len = sizeof("const") - 1 };
    __OC_AT((words), (5)) = (oc_string){ .ptr = (const uint8_t*)("continue"), .len = sizeof("continue") - 1 };
    __OC_AT((words), (6)) = (oc_string){ .ptr = (const uint8_t*)("default"), .len = sizeof("default") - 1 };
    __OC_AT((words), (7)) = (oc_string){ .ptr = (const uint8_t*)("do"), .len = sizeof("do") - 1 };
    __OC_AT((words), (8)) = (oc_string){ .ptr = (const uint8_t*)("double"), .len = sizeof("double") - 1 };
    __OC_AT((words), (9)) = (oc_string){ .ptr = (const uint8_t*)("else"), .len = sizeof("else") - 1 };
    __OC_AT((words), (10)) = (oc_string){ .ptr = (const uint8_t*)("enum"), .len = sizeof("enum") - 1 };
    __OC_AT((words), (11)) = (oc_string){ .ptr = (const uint8_t*)("extern"), .len = sizeof("extern") - 1 };
    __OC_AT((words), (12)) = (oc_string){ .ptr = (const uint8_t*)("float"), .len = sizeof("float") - 1 };
    __OC_AT((words), (13)) = (oc_string){ .ptr = (const uint8_t*)("for"), .len = sizeof("for") - 1 };
    __OC_AT((words), (14)) = (oc_string){ .ptr = (const uint8_t*)("goto"), .len = sizeof("goto") - 1 };
    __OC_AT((words), (15)) = (oc_string){ .ptr = (const uint8_t*)("if"), .len = sizeof("if") - 1 };
    __OC_AT((words), (16)) = (oc_string){ .ptr = (const uint8_t*)("inline"), .len = sizeof("inline") - 1 };
    __OC_AT((words), (17)) = (oc_string){ .ptr = (const uint8_t*)("int"), .len = sizeof("int") - 1 };
    __OC_AT((words), (18)) = (oc_string){ .ptr = (const uint8_t*)("long"), .len = sizeof("long") - 1 };
    __OC_AT((words), (19)) = (oc_string){ .ptr = (const uint8_t*)("register"), .len = sizeof("register") - 1 };
    __OC_AT((words), (20)) = (oc_string){ .ptr = (const uint8_t*)("restrict"), .len = sizeof("restrict") - 1 };
    __OC_AT((words), (21)) = (oc_string){ .ptr = (const uint8_t*)("return"), .len = sizeof("return") - 1 };
    __OC_AT((words), (22)) = (oc_string){ .ptr = (const uint8_t*)("short"), .len = sizeof("short") - 1 };
    __OC_AT((words), (23)) = (oc_string){ .ptr = (const uint8_t*)("signed"), .len = sizeof("signed") - 1 };
    __OC_AT((words), (24)) = (oc_string){ .ptr = (const uint8_t*)("sizeof"), .len = sizeof("sizeof") - 1 };
    __OC_AT((words), (25)) = (oc_string){ .ptr = (const uint8_t*)("static"), .len = sizeof("static") - 1 };
    __OC_AT((words), (26)) = (oc_string){ .ptr = (const uint8_t*)("struct"), .len = sizeof("struct") - 1 };
    __OC_AT((words), (27)) = (oc_string){ .ptr = (const uint8_t*)("switch"), .len = sizeof("switch") - 1 };
    __OC_AT((words), (28)) = (oc_string){ .ptr = (const uint8_t*)("typedef"), .len = sizeof("typedef") - 1 };
    __OC_AT((words), (29)) = (oc_string){ .ptr = (const uint8_t*)("union"), .len = sizeof("union") - 1 };
    __OC_AT((words), (30)) = (oc_string){ .ptr = (const uint8_t*)("unsigned"), .len = sizeof("unsigned") - 1 };
    __OC_AT((words), (31)) = (oc_string){ .ptr = (const uint8_t*)("void"), .len = sizeof("void") - 1 };
    __OC_AT((words), (32)) = (oc_string){ .ptr = (const uint8_t*)("volatile"), .len = sizeof("volatile") - 1 };
    __OC_AT((words), (33)) = (oc_string){ .ptr = (const uint8_t*)("while"), .len = sizeof("while") - 1 };
    __OC_AT((words), (34)) = (oc_string){ .ptr = (const uint8_t*)("bool"), .len = sizeof("bool") - 1 };
    __OC_AT((words), (35)) = (oc_string){ .ptr = (const uint8_t*)("alignas"), .len = sizeof("alignas") - 1 };
    __OC_AT((words), (36)) = (oc_string){ .ptr = (const uint8_t*)("alignof"), .len = sizeof("alignof") - 1 };
    __OC_AT((words), (37)) = (oc_string){ .ptr = (const uint8_t*)("constexpr"), .len = sizeof("constexpr") - 1 };
    __OC_AT((words), (38)) = (oc_string){ .ptr = (const uint8_t*)("nullptr"), .len = sizeof("nullptr") - 1 };
    __OC_AT((words), (39)) = (oc_string){ .ptr = (const uint8_t*)("static_assert"), .len = sizeof("static_assert") - 1 };
    __OC_AT((words), (40)) = (oc_string){ .ptr = (const uint8_t*)("thread_local"), .len = sizeof("thread_local") - 1 };
    __OC_AT((words), (41)) = (oc_string){ .ptr = (const uint8_t*)("typeof"), .len = sizeof("typeof") - 1 };
    __OC_AT((words), (42)) = (oc_string){ .ptr = (const uint8_t*)("typeof_unqual"), .len = sizeof("typeof_unqual") - 1 };
    __OC_AT((words), (43)) = (oc_string){ .ptr = (const uint8_t*)("asm"), .len = sizeof("asm") - 1 };
    __OC_AT((words), (44)) = (oc_string){ .ptr = (const uint8_t*)("int8_t"), .len = sizeof("int8_t") - 1 };
    __OC_AT((words), (45)) = (oc_string){ .ptr = (const uint8_t*)("int16_t"), .len = sizeof("int16_t") - 1 };
    __OC_AT((words), (46)) = (oc_string){ .ptr = (const uint8_t*)("int32_t"), .len = sizeof("int32_t") - 1 };
    __OC_AT((words), (47)) = (oc_string){ .ptr = (const uint8_t*)("int64_t"), .len = sizeof("int64_t") - 1 };
    __OC_AT((words), (48)) = (oc_string){ .ptr = (const uint8_t*)("uint8_t"), .len = sizeof("uint8_t") - 1 };
    __OC_AT((words), (49)) = (oc_string){ .ptr = (const uint8_t*)("uint16_t"), .len = sizeof("uint16_t") - 1 };
    __OC_AT((words), (50)) = (oc_string){ .ptr = (const uint8_t*)("uint32_t"), .len = sizeof("uint32_t") - 1 };
    __OC_AT((words), (51)) = (oc_string){ .ptr = (const uint8_t*)("uint64_t"), .len = sizeof("uint64_t") - 1 };
    __OC_AT((words), (52)) = (oc_string){ .ptr = (const uint8_t*)("size_t"), .len = sizeof("size_t") - 1 };
    __OC_AT((words), (53)) = (oc_string){ .ptr = (const uint8_t*)("oc_string"), .len = sizeof("oc_string") - 1 };
    __OC_AT((words), (54)) = (oc_string){ .ptr = (const uint8_t*)("NULL"), .len = sizeof("NULL") - 1 };
    __OC_AT((words), (55)) = (oc_string){ .ptr = (const uint8_t*)("errno"), .len = sizeof("errno") - 1 };
    size_t i = 0;
    while ((i < 56)) {
    if (str_eq(__OC_AT((words), (i)), name)) {
    return true;
    }
    i = __OC_ADD(i, 1);
    }
    return false;
}

void buf_append_ident(CodeBuffer* buf, oc_string name) {
    buf_append(buf, name);
    if (is_c_reserved(name)) {
    buf_append(buf, (oc_string){ .ptr = (const uint8_t*)("__oc"), .len = sizeof("__oc") - 1 });
    }
}

uint8_t hex_digit_value(uint8_t c) {
    if ((c >= 48)) {
    if ((c <= 57)) {
    return __OC_SUB(c, 48);
    }
    }
    if ((c >= 97)) {
    if ((c <= 102)) {
    return __OC_SUB(c, 87);
    }
    }
    if ((c >= 65)) {
    if ((c <= 70)) {
    return __OC_SUB(c, 55);
    }
    }
    return 255;
}

LitByte lit_next_byte(oc_string raw, size_t k) {
    uint8_t c = __oc_index_str(raw, k);
    if ((c != 92)) {
    return (LitByte){ .b = c, .next = __OC_ADD(k, 1) };
    }
    if ((__OC_ADD(k, 1) >= ((raw).len))) {
    return (LitByte){ .b = c, .next = __OC_ADD(k, 1) };
    }
    uint8_t e = __oc_index_str(raw, __OC_ADD(k, 1));
    if ((e == 110)) {
    return (LitByte){ .b = 10, .next = __OC_ADD(k, 2) };
    }
    if ((e == 116)) {
    return (LitByte){ .b = 9, .next = __OC_ADD(k, 2) };
    }
    if ((e == 114)) {
    return (LitByte){ .b = 13, .next = __OC_ADD(k, 2) };
    }
    if ((e == 48)) {
    return (LitByte){ .b = 0, .next = __OC_ADD(k, 2) };
    }
    if ((e == 92)) {
    return (LitByte){ .b = 92, .next = __OC_ADD(k, 2) };
    }
    if ((e == 34)) {
    return (LitByte){ .b = 34, .next = __OC_ADD(k, 2) };
    }
    if ((e == 120)) {
    if ((__OC_ADD(k, 3) < ((raw).len))) {
    uint8_t hi = hex_digit_value(__oc_index_str(raw, __OC_ADD(k, 2)));
    uint8_t lo = hex_digit_value(__oc_index_str(raw, __OC_ADD(k, 3)));
    if ((hi < 16)) {
    if ((lo < 16)) {
    return (LitByte){ .b = __OC_ADD(__OC_MUL(hi, 16), lo), .next = __OC_ADD(k, 4) };
    }
    }
    }
    }
    return (LitByte){ .b = c, .next = __OC_ADD(k, 1) };
}

void buf_append_c_literal(CodeBuffer* out, oc_string tok_text) {
    oc_string raw = __oc_slice_str(tok_text, 1, __OC_SUB(((tok_text).len), 1));
    buf_append_u8(out, 34);
    size_t k = 0;
    while ((k < ((raw).len))) {
    LitByte lb = lit_next_byte(raw, k);
    uint8_t b = ((lb).b);
    bool plain = (b >= 32);
    if ((b > 126)) {
    plain = false;
    }
    if ((b == 34)) {
    plain = false;
    }
    if ((b == 92)) {
    plain = false;
    }
    if ((b == 63)) {
    plain = false;
    }
    if (plain) {
    buf_append_u8(out, b);
    }
    else {
    buf_append_u8(out, 92);
    buf_append_u8(out, __OC_ADD(48, __OC_DIV(b, 64)));
    buf_append_u8(out, __OC_ADD(48, __OC_MOD(__OC_DIV(b, 8), 8)));
    buf_append_u8(out, __OC_ADD(48, __OC_MOD(b, 8)));
    }
    k = ((lb).next);
    }
    buf_append_u8(out, 34);
}

oc_string buf_to_string(CodeBuffer* buf) {
    return (oc_string){ .ptr = ((uint8_t*)(((buf)->buffer))), .len = ((buf)->len) };
}

void buf_deinit(CodeBuffer* buf) {
    if ((((buf)->capacity) > 0)) {
    sys_munmap(((void*)(((buf)->buffer))), ((buf)->capacity));
    }
    ((buf)->buffer) = ((uint8_t*)(0));
    ((buf)->len) = 0;
    ((buf)->capacity) = 0;
}

oc_string to_c_type(oc_string type_name) {
    if (str_eq(type_name, (oc_string){ .ptr = (const uint8_t*)("i32"), .len = sizeof("i32") - 1 })) {
    return (oc_string){ .ptr = (const uint8_t*)("int32_t"), .len = sizeof("int32_t") - 1 };
    }
    if (str_eq(type_name, (oc_string){ .ptr = (const uint8_t*)("u32"), .len = sizeof("u32") - 1 })) {
    return (oc_string){ .ptr = (const uint8_t*)("uint32_t"), .len = sizeof("uint32_t") - 1 };
    }
    if (str_eq(type_name, (oc_string){ .ptr = (const uint8_t*)("u8"), .len = sizeof("u8") - 1 })) {
    return (oc_string){ .ptr = (const uint8_t*)("uint8_t"), .len = sizeof("uint8_t") - 1 };
    }
    if (str_eq(type_name, (oc_string){ .ptr = (const uint8_t*)("i8"), .len = sizeof("i8") - 1 })) {
    return (oc_string){ .ptr = (const uint8_t*)("int8_t"), .len = sizeof("int8_t") - 1 };
    }
    if (str_eq(type_name, (oc_string){ .ptr = (const uint8_t*)("u16"), .len = sizeof("u16") - 1 })) {
    return (oc_string){ .ptr = (const uint8_t*)("uint16_t"), .len = sizeof("uint16_t") - 1 };
    }
    if (str_eq(type_name, (oc_string){ .ptr = (const uint8_t*)("i16"), .len = sizeof("i16") - 1 })) {
    return (oc_string){ .ptr = (const uint8_t*)("int16_t"), .len = sizeof("int16_t") - 1 };
    }
    if (str_eq(type_name, (oc_string){ .ptr = (const uint8_t*)("u64"), .len = sizeof("u64") - 1 })) {
    return (oc_string){ .ptr = (const uint8_t*)("uint64_t"), .len = sizeof("uint64_t") - 1 };
    }
    if (str_eq(type_name, (oc_string){ .ptr = (const uint8_t*)("i64"), .len = sizeof("i64") - 1 })) {
    return (oc_string){ .ptr = (const uint8_t*)("int64_t"), .len = sizeof("int64_t") - 1 };
    }
    if (str_eq(type_name, (oc_string){ .ptr = (const uint8_t*)("usize"), .len = sizeof("usize") - 1 })) {
    return (oc_string){ .ptr = (const uint8_t*)("size_t"), .len = sizeof("size_t") - 1 };
    }
    if (str_eq(type_name, (oc_string){ .ptr = (const uint8_t*)("bool"), .len = sizeof("bool") - 1 })) {
    return (oc_string){ .ptr = (const uint8_t*)("bool"), .len = sizeof("bool") - 1 };
    }
    if (str_eq(type_name, (oc_string){ .ptr = (const uint8_t*)("f64"), .len = sizeof("f64") - 1 })) {
    return (oc_string){ .ptr = (const uint8_t*)("double"), .len = sizeof("double") - 1 };
    }
    if (str_eq(type_name, (oc_string){ .ptr = (const uint8_t*)("f32"), .len = sizeof("f32") - 1 })) {
    return (oc_string){ .ptr = (const uint8_t*)("float"), .len = sizeof("float") - 1 };
    }
    if (str_eq(type_name, (oc_string){ .ptr = (const uint8_t*)("string"), .len = sizeof("string") - 1 })) {
    return (oc_string){ .ptr = (const uint8_t*)("oc_string"), .len = sizeof("oc_string") - 1 };
    }
    if (str_eq(type_name, (oc_string){ .ptr = (const uint8_t*)("void"), .len = sizeof("void") - 1 })) {
    return (oc_string){ .ptr = (const uint8_t*)("void"), .len = sizeof("void") - 1 };
    }
    if ((((type_name).len) == 0)) {
    return (oc_string){ .ptr = (const uint8_t*)("void"), .len = sizeof("void") - 1 };
    }
    return type_name;
}

void emit_c_type(oc_string type_name, CodeBuffer* out) {
    if ((((type_name).len) == 0)) {
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("void"), .len = sizeof("void") - 1 });
    return;
    }
    bool is_ptr_type = false;
    if ((__oc_index_str(type_name, 0) == 42)) {
    is_ptr_type = true;
    }
    if ((__oc_index_str(type_name, 0) == 38)) {
    is_ptr_type = true;
    }
    if (is_ptr_type) {
    if (str_eq(type_name, (oc_string){ .ptr = (const uint8_t*)("**u8"), .len = sizeof("**u8") - 1 })) {
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("uint8_t**"), .len = sizeof("uint8_t**") - 1 });
    return;
    }
    if (str_eq(type_name, (oc_string){ .ptr = (const uint8_t*)("*u8"), .len = sizeof("*u8") - 1 })) {
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("uint8_t*"), .len = sizeof("uint8_t*") - 1 });
    return;
    }
    if (str_eq(type_name, (oc_string){ .ptr = (const uint8_t*)("*mut u8"), .len = sizeof("*mut u8") - 1 })) {
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("uint8_t*"), .len = sizeof("uint8_t*") - 1 });
    return;
    }
    if (str_eq(type_name, (oc_string){ .ptr = (const uint8_t*)("&u8"), .len = sizeof("&u8") - 1 })) {
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("uint8_t*"), .len = sizeof("uint8_t*") - 1 });
    return;
    }
    if (str_eq(type_name, (oc_string){ .ptr = (const uint8_t*)("&mut u8"), .len = sizeof("&mut u8") - 1 })) {
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("uint8_t*"), .len = sizeof("uint8_t*") - 1 });
    return;
    }
    if (str_eq(type_name, (oc_string){ .ptr = (const uint8_t*)("*mut void"), .len = sizeof("*mut void") - 1 })) {
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("void*"), .len = sizeof("void*") - 1 });
    return;
    }
    if (str_eq(type_name, (oc_string){ .ptr = (const uint8_t*)("*void"), .len = sizeof("*void") - 1 })) {
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("void*"), .len = sizeof("void*") - 1 });
    return;
    }
    size_t base_start = 1;
    if ((((type_name).len) > 5)) {
    if ((__oc_index_str(type_name, 1) == 109)) {
    if ((__oc_index_str(type_name, 2) == 117)) {
    if ((__oc_index_str(type_name, 3) == 116)) {
    if ((__oc_index_str(type_name, 4) == 32)) {
    base_start = 5;
    }
    }
    }
    }
    }
    oc_string base = __oc_slice_str(type_name, base_start, ((type_name).len));
    buf_append_c_type(out, base);
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("*"), .len = sizeof("*") - 1 });
    return;
    }
    buf_append_c_type(out, type_name);
}

void buf_append_c_type(CodeBuffer* out, oc_string t) {
    oc_string c = to_c_type(t);
    bool user_type = str_eq(c, t);
    if (str_eq(t, (oc_string){ .ptr = (const uint8_t*)("bool"), .len = sizeof("bool") - 1 })) {
    user_type = false;
    }
    if (str_eq(t, (oc_string){ .ptr = (const uint8_t*)("void"), .len = sizeof("void") - 1 })) {
    user_type = false;
    }
    if (user_type) {
    buf_append_ident(out, t);
    }
    else {
    buf_append(out, c);
    }
}

bool is_type_ptr(oc_string t) {
    if ((((t).len) == 0)) {
    return false;
    }
    if ((__oc_index_str(t, 0) == 42)) {
    return true;
    }
    if ((__oc_index_str(t, 0) == 38)) {
    return true;
    }
    return false;
}

bool is_type_str(oc_string t) {
    return str_eq(t, (oc_string){ .ptr = (const uint8_t*)("string"), .len = sizeof("string") - 1 });
}

oc_string cg_expr_type(AstNode* expr, CodeBuffer* out) {
    if ((expr == ((AstNode*)(0)))) {
    return (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 };
    }
    if ((((expr)->tag) == NODE_IDENT)) {
    return cg_var_type(out, ((((expr)->tok)).slice));
    }
    if ((((expr)->tag) == NODE_CAST)) {
    return ((expr)->type_name);
    }
    AstNode* d = ((AstNode*)(0));
    if ((((out)->program) != ((AstNode*)(0)))) {
    d = ((((out)->program))->left);
    }
    if ((((expr)->tag) == NODE_CALL)) {
    while ((d != ((AstNode*)(0)))) {
    if ((((d)->tag) == NODE_FN)) {
    if (str_eq(((((d)->tok)).slice), ((((expr)->tok)).slice))) {
    return ((d)->type_name);
    }
    }
    d = ((d)->next);
    }
    return (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 };
    }
    if ((((expr)->tag) == NODE_FIELD_ACCESS)) {
    oc_string st = cg_expr_type(((expr)->left), out);
    if ((((st).len) > 5)) {
    if (str_eq(__oc_slice_str(st, 0, 5), (oc_string){ .ptr = (const uint8_t*)("*mut "), .len = sizeof("*mut ") - 1 })) {
    st = __oc_slice_str(st, 5, ((st).len));
    }
    }
    if ((((st).len) > 1)) {
    if ((__oc_index_str(st, 0) == 42)) {
    st = __oc_slice_str(st, 1, ((st).len));
    }
    }
    if (str_eq(st, (oc_string){ .ptr = (const uint8_t*)("string"), .len = sizeof("string") - 1 })) {
    if (str_eq(((((expr)->tok)).slice), (oc_string){ .ptr = (const uint8_t*)("len"), .len = sizeof("len") - 1 })) {
    return (oc_string){ .ptr = (const uint8_t*)("usize"), .len = sizeof("usize") - 1 };
    }
    if (str_eq(((((expr)->tok)).slice), (oc_string){ .ptr = (const uint8_t*)("ptr"), .len = sizeof("ptr") - 1 })) {
    return (oc_string){ .ptr = (const uint8_t*)("*u8"), .len = sizeof("*u8") - 1 };
    }
    return (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 };
    }
    while ((d != ((AstNode*)(0)))) {
    if ((((d)->tag) == NODE_STRUCT_DECL)) {
    if (str_eq(((((d)->tok)).slice), st)) {
    AstNode* f = ((d)->left);
    while ((f != ((AstNode*)(0)))) {
    if (str_eq(((((f)->tok)).slice), ((((expr)->tok)).slice))) {
    return ((f)->type_name);
    }
    f = ((f)->next);
    }
    return (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 };
    }
    }
    d = ((d)->next);
    }
    }
    return (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 };
}

bool is_expr_string(AstNode* expr, CodeBuffer* out) {
    if ((expr == ((AstNode*)(0)))) {
    return false;
    }
    if ((((expr)->tag) == NODE_STRING)) {
    return true;
    }
    if ((((expr)->tag) == NODE_SLICE)) {
    return true;
    }
    if ((((expr)->tag) == NODE_IDENT)) {
    oc_string id = ((((expr)->tok)).slice);
    size_t i = 0;
    while ((i < ((out)->str_vars_count))) {
    if (str_eq(__OC_AT((((out)->str_vars)), (i)), id)) {
    return true;
    }
    i = __OC_ADD(i, 1);
    }
    return false;
    }
    if ((((expr)->tag) == NODE_FIELD_ACCESS)) {
    oc_string ft = cg_expr_type(expr, out);
    if ((((ft).len) > 0)) {
    return is_type_str(ft);
    }
    oc_string f = ((((expr)->tok)).slice);
    size_t i = 0;
    while ((i < ((out)->str_fields_count))) {
    if (str_eq(__OC_AT((((out)->str_fields)), (i)), f)) {
    return true;
    }
    i = __OC_ADD(i, 1);
    }
    return false;
    }
    if ((((expr)->tag) == NODE_CALL)) {
    oc_string fn_name = ((((expr)->tok)).slice);
    size_t i = 0;
    while ((i < ((out)->str_fns_count))) {
    if (str_eq(__OC_AT((((out)->str_fns)), (i)), fn_name)) {
    return true;
    }
    i = __OC_ADD(i, 1);
    }
    return false;
    }
    return false;
}

bool is_expr_pointer(AstNode* expr, CodeBuffer* out) {
    if ((expr == ((AstNode*)(0)))) {
    return false;
    }
    if ((((expr)->tag) == NODE_ADDRESS_OF)) {
    return true;
    }
    if ((((expr)->tag) == NODE_DEREF)) {
    return false;
    }
    if ((((expr)->tag) == NODE_CAST)) {
    return is_type_ptr(((expr)->type_name));
    }
    if ((((expr)->tag) == NODE_FIELD_ACCESS)) {
    oc_string ft = cg_expr_type(expr, out);
    if ((((ft).len) > 0)) {
    return is_type_ptr(ft);
    }
    oc_string f = ((((expr)->tok)).slice);
    size_t i = 0;
    while ((i < ((out)->ptr_fields_count))) {
    if (str_eq(__OC_AT((((out)->ptr_fields)), (i)), f)) {
    return true;
    }
    i = __OC_ADD(i, 1);
    }
    return false;
    }
    if ((((expr)->tag) == NODE_CALL)) {
    oc_string fn_name = ((((expr)->tok)).slice);
    size_t i = 0;
    while ((i < ((out)->ptr_fns_count))) {
    if (str_eq(__OC_AT((((out)->ptr_fns)), (i)), fn_name)) {
    return true;
    }
    i = __OC_ADD(i, 1);
    }
    return false;
    }
    if ((((expr)->tag) == NODE_IDENT)) {
    oc_string id = ((((expr)->tok)).slice);
    size_t i = 0;
    while ((i < ((out)->ptr_vars_count))) {
    if (str_eq(__OC_AT((((out)->ptr_vars)), (i)), id)) {
    return true;
    }
    i = __OC_ADD(i, 1);
    }
    return false;
    }
    if ((((expr)->tag) == NODE_BINARY)) {
    if (is_expr_pointer(((expr)->left), out)) {
    return true;
    }
    return is_expr_pointer(((expr)->right), out);
    }
    return false;
}

bool is_float_type(oc_string t) {
    if (str_eq(t, (oc_string){ .ptr = (const uint8_t*)("f64"), .len = sizeof("f64") - 1 })) {
    return true;
    }
    return str_eq(t, (oc_string){ .ptr = (const uint8_t*)("f32"), .len = sizeof("f32") - 1 });
}

void cg_flt_add(CodeBuffer* out, oc_string name, oc_string t) {
    if ((!is_float_type(t))) {
    return;
    }
    if ((((out)->flt_count) < 256)) {
    __OC_AT((((out)->flt_names)), (((out)->flt_count))) = name;
    __OC_AT((((out)->flt_types)), (((out)->flt_count))) = t;
    ((out)->flt_count) = __OC_ADD(((out)->flt_count), 1);
    }
}

oc_string cg_flt_lookup(CodeBuffer* out, oc_string name) {
    size_t i = 0;
    while ((i < ((out)->flt_count))) {
    if (str_eq(__OC_AT((((out)->flt_names)), (i)), name)) {
    return __OC_AT((((out)->flt_types)), (i));
    }
    i = __OC_ADD(i, 1);
    }
    return (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 };
}

oc_string cg_elem_type(oc_string t) {
    if ((((t).len) < 2)) {
    return (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 };
    }
    if ((__oc_index_str(t, 0) == 91)) {
    size_t close = 1;
    while ((close < ((t).len))) {
    if ((__oc_index_str(t, close) == 93)) {
    break;
    }
    close = __OC_ADD(close, 1);
    }
    if ((__OC_ADD(close, 1) >= ((t).len))) {
    return (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 };
    }
    return __oc_slice_str(t, __OC_ADD(close, 1), ((t).len));
    }
    if ((__oc_index_str(t, 0) == 42)) {
    if ((((t).len) > 5)) {
    if (str_eq(__oc_slice_str(t, 0, 5), (oc_string){ .ptr = (const uint8_t*)("*mut "), .len = sizeof("*mut ") - 1 })) {
    return __oc_slice_str(t, 5, ((t).len));
    }
    }
    return __oc_slice_str(t, 1, ((t).len));
    }
    return (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 };
}

oc_string cg_float_type(AstNode* expr, CodeBuffer* out) {
    if ((expr == ((AstNode*)(0)))) {
    return (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 };
    }
    if ((((expr)->tag) == NODE_FLOAT)) {
    if ((((((expr)->type_name)).len) > 0)) {
    return ((expr)->type_name);
    }
    return (oc_string){ .ptr = (const uint8_t*)("f64"), .len = sizeof("f64") - 1 };
    }
    if ((((expr)->tag) == NODE_CAST)) {
    if (is_float_type(((expr)->type_name))) {
    return ((expr)->type_name);
    }
    return (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 };
    }
    if ((((expr)->tag) == NODE_IDENT)) {
    oc_string vt = cg_var_type(out, ((((expr)->tok)).slice));
    if ((((vt).len) > 0)) {
    if (is_float_type(vt)) {
    return vt;
    }
    return (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 };
    }
    return cg_flt_lookup(out, ((((expr)->tok)).slice));
    }
    if ((((expr)->tag) == NODE_UNARY)) {
    if ((((((expr)->tok)).tag) == TOK_MINUS)) {
    return cg_float_type(((expr)->left), out);
    }
    return (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 };
    }
    if ((((expr)->tag) == NODE_BINARY)) {
    oc_string lf = cg_float_type(((expr)->left), out);
    oc_string rf = cg_float_type(((expr)->right), out);
    if ((((((expr)->tok)).tag) == TOK_EQEQ)) {
    return (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 };
    }
    if ((((((expr)->tok)).tag) == TOK_NEQ)) {
    return (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 };
    }
    if ((((((expr)->tok)).tag) == TOK_LT)) {
    return (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 };
    }
    if ((((((expr)->tok)).tag) == TOK_LTE)) {
    return (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 };
    }
    if ((((((expr)->tok)).tag) == TOK_GT)) {
    return (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 };
    }
    if ((((((expr)->tok)).tag) == TOK_GTE)) {
    return (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 };
    }
    if (str_eq(lf, (oc_string){ .ptr = (const uint8_t*)("f64"), .len = sizeof("f64") - 1 })) {
    return lf;
    }
    if (str_eq(rf, (oc_string){ .ptr = (const uint8_t*)("f64"), .len = sizeof("f64") - 1 })) {
    return rf;
    }
    if ((((lf).len) > 0)) {
    return lf;
    }
    return rf;
    }
    if ((((expr)->tag) == NODE_CALL)) {
    if (str_eq(((((expr)->tok)).slice), (oc_string){ .ptr = (const uint8_t*)("sqrt_f64"), .len = sizeof("sqrt_f64") - 1 })) {
    return (oc_string){ .ptr = (const uint8_t*)("f64"), .len = sizeof("f64") - 1 };
    }
    return cg_flt_lookup(out, ((((expr)->tok)).slice));
    }
    if ((((expr)->tag) == NODE_FIELD_ACCESS)) {
    oc_string ft = cg_expr_type(expr, out);
    if ((((ft).len) > 0)) {
    if (is_float_type(ft)) {
    return ft;
    }
    return (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 };
    }
    return cg_flt_lookup(out, ((((expr)->tok)).slice));
    }
    bool via_elem = (((expr)->tag) == NODE_INDEX);
    if ((((expr)->tag) == NODE_DEREF)) {
    via_elem = true;
    }
    if (via_elem) {
    if ((((expr)->left) != ((AstNode*)(0)))) {
    if ((((((expr)->left))->tag) == NODE_IDENT)) {
    oc_string et = cg_elem_type(cg_var_type(out, ((((((expr)->left))->tok)).slice)));
    if (is_float_type(et)) {
    return et;
    }
    }
    }
    return (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 };
    }
    return (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 };
}

oc_string float_cast_lo(oc_string t) {
    if (str_eq(t, (oc_string){ .ptr = (const uint8_t*)("i8"), .len = sizeof("i8") - 1 })) {
    return (oc_string){ .ptr = (const uint8_t*)("-129.0"), .len = sizeof("-129.0") - 1 };
    }
    if (str_eq(t, (oc_string){ .ptr = (const uint8_t*)("i16"), .len = sizeof("i16") - 1 })) {
    return (oc_string){ .ptr = (const uint8_t*)("-32769.0"), .len = sizeof("-32769.0") - 1 };
    }
    if (str_eq(t, (oc_string){ .ptr = (const uint8_t*)("i32"), .len = sizeof("i32") - 1 })) {
    return (oc_string){ .ptr = (const uint8_t*)("-2147483649.0"), .len = sizeof("-2147483649.0") - 1 };
    }
    if (str_eq(t, (oc_string){ .ptr = (const uint8_t*)("i64"), .len = sizeof("i64") - 1 })) {
    return (oc_string){ .ptr = (const uint8_t*)("-9223372036854777856.0"), .len = sizeof("-9223372036854777856.0") - 1 };
    }
    return (oc_string){ .ptr = (const uint8_t*)("-1.0"), .len = sizeof("-1.0") - 1 };
}

oc_string float_cast_hi(oc_string t) {
    if (str_eq(t, (oc_string){ .ptr = (const uint8_t*)("i8"), .len = sizeof("i8") - 1 })) {
    return (oc_string){ .ptr = (const uint8_t*)("128.0"), .len = sizeof("128.0") - 1 };
    }
    if (str_eq(t, (oc_string){ .ptr = (const uint8_t*)("i16"), .len = sizeof("i16") - 1 })) {
    return (oc_string){ .ptr = (const uint8_t*)("32768.0"), .len = sizeof("32768.0") - 1 };
    }
    if (str_eq(t, (oc_string){ .ptr = (const uint8_t*)("i32"), .len = sizeof("i32") - 1 })) {
    return (oc_string){ .ptr = (const uint8_t*)("2147483648.0"), .len = sizeof("2147483648.0") - 1 };
    }
    if (str_eq(t, (oc_string){ .ptr = (const uint8_t*)("i64"), .len = sizeof("i64") - 1 })) {
    return (oc_string){ .ptr = (const uint8_t*)("9223372036854775808.0"), .len = sizeof("9223372036854775808.0") - 1 };
    }
    if (str_eq(t, (oc_string){ .ptr = (const uint8_t*)("u8"), .len = sizeof("u8") - 1 })) {
    return (oc_string){ .ptr = (const uint8_t*)("256.0"), .len = sizeof("256.0") - 1 };
    }
    if (str_eq(t, (oc_string){ .ptr = (const uint8_t*)("u16"), .len = sizeof("u16") - 1 })) {
    return (oc_string){ .ptr = (const uint8_t*)("65536.0"), .len = sizeof("65536.0") - 1 };
    }
    if (str_eq(t, (oc_string){ .ptr = (const uint8_t*)("u32"), .len = sizeof("u32") - 1 })) {
    return (oc_string){ .ptr = (const uint8_t*)("4294967296.0"), .len = sizeof("4294967296.0") - 1 };
    }
    return (oc_string){ .ptr = (const uint8_t*)("18446744073709551616.0"), .len = sizeof("18446744073709551616.0") - 1 };
}

oc_string wrapping_op(oc_string fn_name) {
    if (str_eq(fn_name, (oc_string){ .ptr = (const uint8_t*)("wrapping_add"), .len = sizeof("wrapping_add") - 1 })) {
    return (oc_string){ .ptr = (const uint8_t*)("add"), .len = sizeof("add") - 1 };
    }
    if (str_eq(fn_name, (oc_string){ .ptr = (const uint8_t*)("wrapping_sub"), .len = sizeof("wrapping_sub") - 1 })) {
    return (oc_string){ .ptr = (const uint8_t*)("sub"), .len = sizeof("sub") - 1 };
    }
    if (str_eq(fn_name, (oc_string){ .ptr = (const uint8_t*)("wrapping_mul"), .len = sizeof("wrapping_mul") - 1 })) {
    return (oc_string){ .ptr = (const uint8_t*)("mul"), .len = sizeof("mul") - 1 };
    }
    return (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 };
}

void codegen_expr(AstNode* expr, CodeBuffer* out) {
    if ((expr == ((AstNode*)(0)))) {
    return;
    }
    if ((((expr)->tag) == NODE_INT)) {
    size_t lit_start = ((out)->len);
    size_t di = 0;
    while ((di < ((((((expr)->tok)).slice)).len))) {
    if ((__oc_index_str(((((expr)->tok)).slice), di) != 95)) {
    buf_append_u8(out, __oc_index_str(((((expr)->tok)).slice), di));
    }
    di = __OC_ADD(di, 1);
    }
    oc_string digits = (oc_string){ .ptr = ((uint8_t*)((((out)->buffer) + lit_start))), .len = __OC_SUB(((out)->len), lit_start) };
    bool is_decimal = true;
    if ((((digits).len) > 1)) {
    if ((__oc_index_str(digits, 0) == 48)) {
    if ((__oc_index_str(digits, 1) > 57)) {
    is_decimal = false;
    }
    }
    }
    if (is_decimal) {
    bool too_big = (((digits).len) > 19);
    if ((((digits).len) == 19)) {
    oc_string limit = (oc_string){ .ptr = (const uint8_t*)("9223372036854775807"), .len = sizeof("9223372036854775807") - 1 };
    size_t k = 0;
    while ((k < 19)) {
    if ((__oc_index_str(digits, k) != __oc_index_str(limit, k))) {
    too_big = (__oc_index_str(digits, k) > __oc_index_str(limit, k));
    break;
    }
    k = __OC_ADD(k, 1);
    }
    }
    if (too_big) {
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("ULL"), .len = sizeof("ULL") - 1 });
    }
    }
    return;
    }
    if ((((expr)->tag) == NODE_FLOAT)) {
    buf_append(out, ((((expr)->tok)).slice));
    if (str_eq(((expr)->type_name), (oc_string){ .ptr = (const uint8_t*)("f32"), .len = sizeof("f32") - 1 })) {
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("f"), .len = sizeof("f") - 1 });
    }
    return;
    }
    if ((((expr)->tag) == NODE_STRING)) {
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("(oc_string){ .ptr = (const uint8_t*)("), .len = sizeof("(oc_string){ .ptr = (const uint8_t*)(") - 1 });
    buf_append_c_literal(out, ((((expr)->tok)).slice));
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("), .len = sizeof("), .len = sizeof("), .len = sizeof(") - 1 });
    buf_append_c_literal(out, ((((expr)->tok)).slice));
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)(") - 1 }"), .len = sizeof(") - 1 }") - 1 });
    return;
    }
    if ((((expr)->tag) == NODE_BOOL)) {
    buf_append(out, ((((expr)->tok)).slice));
    return;
    }
    if ((((expr)->tag) == NODE_IDENT)) {
    buf_append_ident(out, ((((expr)->tok)).slice));
    return;
    }
    if ((((expr)->tag) == NODE_UNARY)) {
    bool is_literal = false;
    if ((((expr)->left) != ((AstNode*)(0)))) {
    is_literal = (((((expr)->left))->tag) == NODE_INT);
    }
    if ((((expr)->left) != ((AstNode*)(0)))) {
    if ((((((expr)->left))->tag) == NODE_FLOAT)) {
    is_literal = true;
    }
    }
    if ((((cg_float_type(((expr)->left), out)).len) > 0)) {
    is_literal = true;
    }
    if ((((((expr)->tok)).tag) == TOK_MINUS)) {
    if (is_literal) {
    if (str_eq(((((((expr)->left))->tok)).slice), (oc_string){ .ptr = (const uint8_t*)("9223372036854775808"), .len = sizeof("9223372036854775808") - 1 })) {
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("(-9223372036854775807LL - 1)"), .len = sizeof("(-9223372036854775807LL - 1)") - 1 });
    return;
    }
    }
    if ((!is_literal)) {
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("__OC_SUB(0, "), .len = sizeof("__OC_SUB(0, ") - 1 });
    codegen_expr(((expr)->left), out);
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)(")"), .len = sizeof(")") - 1 });
    return;
    }
    }
    if ((((((expr)->tok)).tag) == TOK_TILDE)) {
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("((__typeof__("), .len = sizeof("((__typeof__(") - 1 });
    codegen_expr(((expr)->left), out);
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("))~("), .len = sizeof("))~(") - 1 });
    codegen_expr(((expr)->left), out);
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("))"), .len = sizeof("))") - 1 });
    return;
    }
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("("), .len = sizeof("(") - 1 });
    buf_append(out, ((((expr)->tok)).slice));
    codegen_expr(((expr)->left), out);
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)(")"), .len = sizeof(")") - 1 });
    return;
    }
    if ((((expr)->tag) == NODE_ADDRESS_OF)) {
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("(&("), .len = sizeof("(&(") - 1 });
    codegen_expr(((expr)->left), out);
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("))"), .len = sizeof("))") - 1 });
    return;
    }
    if ((((expr)->tag) == NODE_DEREF)) {
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("(*(__oc_notnull("), .len = sizeof("(*(__oc_notnull(") - 1 });
    codegen_expr(((expr)->left), out);
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)(")))"), .len = sizeof(")))") - 1 });
    return;
    }
    if ((((expr)->tag) == NODE_CAST)) {
    bool is_unsigned_num = false;
    bool is_signed_num = false;
    bool from_float = (((cg_float_type(((expr)->left), out)).len) > 0);
    if (is_float_type(((expr)->type_name))) {
    from_float = false;
    }
    if (is_type_ptr(((expr)->type_name))) {
    from_float = false;
    }
    if (str_eq(((expr)->type_name), (oc_string){ .ptr = (const uint8_t*)("bool"), .len = sizeof("bool") - 1 })) {
    from_float = false;
    }
    if (from_float) {
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("({ double _f = (double)("), .len = sizeof("({ double _f = (double)(") - 1 });
    codegen_expr(((expr)->left), out);
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("); if (!(_f > "), .len = sizeof("); if (!(_f > ") - 1 });
    buf_append(out, float_cast_lo(((expr)->type_name)));
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)(" && _f < "), .len = sizeof(" && _f < ") - 1 });
    buf_append(out, float_cast_hi(((expr)->type_name)));
    if (str_eq(float_cast_lo(((expr)->type_name)), (oc_string){ .ptr = (const uint8_t*)("-1.0"), .len = sizeof("-1.0") - 1 })) {
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)(")) { fprintf(stderr, \042panic: invalid unsigned cast\134n\042); exit(1); } ("), .len = sizeof(")) { fprintf(stderr, \042panic: invalid unsigned cast\134n\042); exit(1); } (") - 1 });
    }
    else {
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)(")) { fprintf(stderr, \042panic: invalid signed cast\134n\042); exit(1); } ("), .len = sizeof(")) { fprintf(stderr, \042panic: invalid signed cast\134n\042); exit(1); } (") - 1 });
    }
    emit_c_type(((expr)->type_name), out);
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)(")_f; })"), .len = sizeof(")_f; })") - 1 });
    return;
    }
    if ((!is_type_ptr(((expr)->type_name)))) {
    if ((!is_expr_pointer(((expr)->left), out))) {
    if (str_eq(((expr)->type_name), (oc_string){ .ptr = (const uint8_t*)("u8"), .len = sizeof("u8") - 1 })) {
    is_unsigned_num = true;
    }
    else {
    if (str_eq(((expr)->type_name), (oc_string){ .ptr = (const uint8_t*)("u16"), .len = sizeof("u16") - 1 })) {
    is_unsigned_num = true;
    }
    else {
    if (str_eq(((expr)->type_name), (oc_string){ .ptr = (const uint8_t*)("u32"), .len = sizeof("u32") - 1 })) {
    is_unsigned_num = true;
    }
    else {
    if (str_eq(((expr)->type_name), (oc_string){ .ptr = (const uint8_t*)("u64"), .len = sizeof("u64") - 1 })) {
    is_unsigned_num = true;
    }
    else {
    if (str_eq(((expr)->type_name), (oc_string){ .ptr = (const uint8_t*)("usize"), .len = sizeof("usize") - 1 })) {
    is_unsigned_num = true;
    }
    else {
    if (str_eq(((expr)->type_name), (oc_string){ .ptr = (const uint8_t*)("i8"), .len = sizeof("i8") - 1 })) {
    is_signed_num = true;
    }
    else {
    if (str_eq(((expr)->type_name), (oc_string){ .ptr = (const uint8_t*)("i16"), .len = sizeof("i16") - 1 })) {
    is_signed_num = true;
    }
    else {
    if (str_eq(((expr)->type_name), (oc_string){ .ptr = (const uint8_t*)("i32"), .len = sizeof("i32") - 1 })) {
    is_signed_num = true;
    }
    else {
    if (str_eq(((expr)->type_name), (oc_string){ .ptr = (const uint8_t*)("i64"), .len = sizeof("i64") - 1 })) {
    is_signed_num = true;
    }
    }
    }
    }
    }
    }
    }
    }
    }
    }
    }
    if (is_unsigned_num) {
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("({ __auto_type _v = ("), .len = sizeof("({ __auto_type _v = (") - 1 });
    codegen_expr(((expr)->left), out);
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("); if (_v < 0"), .len = sizeof("); if (_v < 0") - 1 });
    if (str_eq(((expr)->type_name), (oc_string){ .ptr = (const uint8_t*)("u8"), .len = sizeof("u8") - 1 })) {
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)(" || (uint64_t)_v > 255ULL"), .len = sizeof(" || (uint64_t)_v > 255ULL") - 1 });
    }
    else {
    if (str_eq(((expr)->type_name), (oc_string){ .ptr = (const uint8_t*)("u16"), .len = sizeof("u16") - 1 })) {
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)(" || (uint64_t)_v > 65535ULL"), .len = sizeof(" || (uint64_t)_v > 65535ULL") - 1 });
    }
    else {
    if (str_eq(((expr)->type_name), (oc_string){ .ptr = (const uint8_t*)("u32"), .len = sizeof("u32") - 1 })) {
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)(" || (uint64_t)_v > 4294967295ULL"), .len = sizeof(" || (uint64_t)_v > 4294967295ULL") - 1 });
    }
    }
    }
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)(") { fprintf(stderr, \042panic: invalid unsigned cast\134n\042); exit(1); } ("), .len = sizeof(") { fprintf(stderr, \042panic: invalid unsigned cast\134n\042); exit(1); } (") - 1 });
    emit_c_type(((expr)->type_name), out);
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)(")_v; })"), .len = sizeof(")_v; })") - 1 });
    return;
    }
    if (is_signed_num) {
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("({ __auto_type _v = ("), .len = sizeof("({ __auto_type _v = (") - 1 });
    codegen_expr(((expr)->left), out);
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("); if ("), .len = sizeof("); if (") - 1 });
    if (str_eq(((expr)->type_name), (oc_string){ .ptr = (const uint8_t*)("i8"), .len = sizeof("i8") - 1 })) {
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("(_v < 0 && (int64_t)_v < -128LL) || (!(_v < 0) && (uint64_t)_v > 127ULL)"), .len = sizeof("(_v < 0 && (int64_t)_v < -128LL) || (!(_v < 0) && (uint64_t)_v > 127ULL)") - 1 });
    }
    else {
    if (str_eq(((expr)->type_name), (oc_string){ .ptr = (const uint8_t*)("i16"), .len = sizeof("i16") - 1 })) {
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("(_v < 0 && (int64_t)_v < -32768LL) || (!(_v < 0) && (uint64_t)_v > 32767ULL)"), .len = sizeof("(_v < 0 && (int64_t)_v < -32768LL) || (!(_v < 0) && (uint64_t)_v > 32767ULL)") - 1 });
    }
    else {
    if (str_eq(((expr)->type_name), (oc_string){ .ptr = (const uint8_t*)("i32"), .len = sizeof("i32") - 1 })) {
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("(_v < 0 && (int64_t)_v < -2147483648LL) || (!(_v < 0) && (uint64_t)_v > 2147483647ULL)"), .len = sizeof("(_v < 0 && (int64_t)_v < -2147483648LL) || (!(_v < 0) && (uint64_t)_v > 2147483647ULL)") - 1 });
    }
    else {
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("!(_v < 0) && (uint64_t)_v > 9223372036854775807ULL"), .len = sizeof("!(_v < 0) && (uint64_t)_v > 9223372036854775807ULL") - 1 });
    }
    }
    }
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)(") { fprintf(stderr, \042panic: invalid signed cast\134n\042); exit(1); } ("), .len = sizeof(") { fprintf(stderr, \042panic: invalid signed cast\134n\042); exit(1); } (") - 1 });
    emit_c_type(((expr)->type_name), out);
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)(")_v; })"), .len = sizeof(")_v; })") - 1 });
    return;
    }
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("(("), .len = sizeof("((") - 1 });
    emit_c_type(((expr)->type_name), out);
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)(")("), .len = sizeof(")(") - 1 });
    codegen_expr(((expr)->left), out);
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("))"), .len = sizeof("))") - 1 });
    return;
    }
    if ((((expr)->tag) == NODE_INDEX)) {
    if (is_expr_string(((expr)->left), out)) {
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("__oc_index_str("), .len = sizeof("__oc_index_str(") - 1 });
    codegen_expr(((expr)->left), out);
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)(", "), .len = sizeof(", ") - 1 });
    codegen_expr(((expr)->right), out);
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)(")"), .len = sizeof(")") - 1 });
    return;
    }
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("__OC_AT(("), .len = sizeof("__OC_AT((") - 1 });
    codegen_expr(((expr)->left), out);
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("), ("), .len = sizeof("), (") - 1 });
    codegen_expr(((expr)->right), out);
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("))"), .len = sizeof("))") - 1 });
    return;
    }
    if ((((expr)->tag) == NODE_SLICE)) {
    bool is_str = is_expr_string(((expr)->left), out);
    bool has_start = false;
    bool has_end = false;
    if ((((expr)->right) != ((AstNode*)(0)))) {
    if ((((((expr)->right))->left) != ((AstNode*)(0)))) {
    has_start = true;
    }
    if ((((((expr)->right))->right) != ((AstNode*)(0)))) {
    has_end = true;
    }
    }
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("__oc_slice_str("), .len = sizeof("__oc_slice_str(") - 1 });
    if ((!is_str)) {
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("__OC_ARR("), .len = sizeof("__OC_ARR(") - 1 });
    }
    codegen_expr(((expr)->left), out);
    if ((!is_str)) {
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)(")"), .len = sizeof(")") - 1 });
    }
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)(", "), .len = sizeof(", ") - 1 });
    if (has_start) {
    codegen_expr(((((expr)->right))->left), out);
    }
    else {
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("0"), .len = sizeof("0") - 1 });
    }
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)(", "), .len = sizeof(", ") - 1 });
    if (has_end) {
    codegen_expr(((((expr)->right))->right), out);
    }
    else {
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("(("), .len = sizeof("((") - 1 });
    if ((!is_str)) {
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("__OC_ARR("), .len = sizeof("__OC_ARR(") - 1 });
    }
    codegen_expr(((expr)->left), out);
    if ((!is_str)) {
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)(")"), .len = sizeof(")") - 1 });
    }
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)(").len)"), .len = sizeof(").len)") - 1 });
    }
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)(")"), .len = sizeof(")") - 1 });
    return;
    }
    if ((((expr)->tag) == NODE_FIELD_ACCESS)) {
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("(("), .len = sizeof("((") - 1 });
    codegen_expr(((expr)->left), out);
    if (is_expr_pointer(((expr)->left), out)) {
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)(")->"), .len = sizeof(")->") - 1 });
    }
    else {
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)(")."), .len = sizeof(").") - 1 });
    }
    buf_append_ident(out, ((((expr)->tok)).slice));
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)(")"), .len = sizeof(")") - 1 });
    return;
    }
    if ((((expr)->tag) == NODE_STRUCT_INIT)) {
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("("), .len = sizeof("(") - 1 });
    buf_append_c_type(out, ((((expr)->tok)).slice));
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("){ "), .len = sizeof("){ ") - 1 });
    AstNode* f = ((expr)->left);
    while ((f != ((AstNode*)(0)))) {
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("."), .len = sizeof(".") - 1 });
    buf_append_ident(out, ((((f)->tok)).slice));
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)(" = "), .len = sizeof(" = ") - 1 });
    codegen_expr(((f)->left), out);
    if ((((f)->next) != ((AstNode*)(0)))) {
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)(", "), .len = sizeof(", ") - 1 });
    }
    f = ((f)->next);
    }
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)(" }"), .len = sizeof(" }") - 1 });
    return;
    }
    if ((((expr)->tag) == NODE_ARRAY_INIT)) {
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("{ "), .len = sizeof("{ ") - 1 });
    AstNode* elem = ((expr)->left);
    while ((elem != ((AstNode*)(0)))) {
    codegen_expr(elem, out);
    if ((((elem)->next) != ((AstNode*)(0)))) {
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)(", "), .len = sizeof(", ") - 1 });
    }
    elem = ((elem)->next);
    }
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)(" }"), .len = sizeof(" }") - 1 });
    return;
    }
    if ((((expr)->tag) == NODE_CALL)) {
    if (str_eq(((((expr)->tok)).slice), (oc_string){ .ptr = (const uint8_t*)("sqrt_f64"), .len = sizeof("sqrt_f64") - 1 })) {
    if (((out)->fp_trap)) {
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("__OC_FCHK("), .len = sizeof("__OC_FCHK(") - 1 });
    }
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("__builtin_sqrt((double)("), .len = sizeof("__builtin_sqrt((double)(") - 1 });
    codegen_expr(((expr)->left), out);
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("))"), .len = sizeof("))") - 1 });
    if (((out)->fp_trap)) {
    cg_fchk_close(out, ((expr)->tok));
    }
    return;
    }
    oc_string w_op = wrapping_op(((((expr)->tok)).slice));
    if ((((w_op).len) > 0)) {
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("__OC_WRAP("), .len = sizeof("__OC_WRAP(") - 1 });
    buf_append(out, w_op);
    AstNode* w_arg = ((expr)->left);
    while ((w_arg != ((AstNode*)(0)))) {
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)(", ("), .len = sizeof(", (") - 1 });
    codegen_expr(w_arg, out);
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)(")"), .len = sizeof(")") - 1 });
    w_arg = ((w_arg)->next);
    }
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)(")"), .len = sizeof(")") - 1 });
    return;
    }
    buf_append_ident(out, ((((expr)->tok)).slice));
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("("), .len = sizeof("(") - 1 });
    AstNode* arg = ((expr)->left);
    while ((arg != ((AstNode*)(0)))) {
    codegen_expr(arg, out);
    if ((((arg)->next) != ((AstNode*)(0)))) {
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)(", "), .len = sizeof(", ") - 1 });
    }
    arg = ((arg)->next);
    }
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)(")"), .len = sizeof(")") - 1 });
    return;
    }
    if ((((expr)->tag) == NODE_BINARY)) {
    bool float_op = (((cg_float_type(((expr)->left), out)).len) > 0);
    if ((((cg_float_type(((expr)->right), out)).len) > 0)) {
    float_op = true;
    }
    if (float_op) {
    bool checked = ((out)->fp_trap);
    if (is_comparison_tok(((((expr)->tok)).tag))) {
    checked = false;
    }
    if (checked) {
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("__OC_FCHK("), .len = sizeof("__OC_FCHK(") - 1 });
    }
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("("), .len = sizeof("(") - 1 });
    codegen_expr(((expr)->left), out);
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)(" "), .len = sizeof(" ") - 1 });
    buf_append(out, ((((expr)->tok)).slice));
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)(" "), .len = sizeof(" ") - 1 });
    codegen_expr(((expr)->right), out);
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)(")"), .len = sizeof(")") - 1 });
    if (checked) {
    cg_fchk_close(out, ((expr)->tok));
    }
    return;
    }
    if ((((((expr)->tok)).tag) == TOK_SLASH)) {
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("__OC_DIV("), .len = sizeof("__OC_DIV(") - 1 });
    codegen_expr(((expr)->left), out);
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)(", "), .len = sizeof(", ") - 1 });
    codegen_expr(((expr)->right), out);
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)(")"), .len = sizeof(")") - 1 });
    return;
    }
    if ((((((expr)->tok)).tag) == TOK_PERCENT)) {
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("__OC_MOD("), .len = sizeof("__OC_MOD(") - 1 });
    codegen_expr(((expr)->left), out);
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)(", "), .len = sizeof(", ") - 1 });
    codegen_expr(((expr)->right), out);
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)(")"), .len = sizeof(")") - 1 });
    return;
    }
    if ((!is_expr_pointer(((expr)->left), out))) {
    if ((!is_expr_pointer(((expr)->right), out))) {
    if ((((((expr)->tok)).tag) == TOK_PLUS)) {
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("__OC_ADD("), .len = sizeof("__OC_ADD(") - 1 });
    codegen_expr(((expr)->left), out);
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)(", "), .len = sizeof(", ") - 1 });
    codegen_expr(((expr)->right), out);
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)(")"), .len = sizeof(")") - 1 });
    return;
    }
    if ((((((expr)->tok)).tag) == TOK_MINUS)) {
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("__OC_SUB("), .len = sizeof("__OC_SUB(") - 1 });
    codegen_expr(((expr)->left), out);
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)(", "), .len = sizeof(", ") - 1 });
    codegen_expr(((expr)->right), out);
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)(")"), .len = sizeof(")") - 1 });
    return;
    }
    if ((((((expr)->tok)).tag) == TOK_STAR)) {
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("__OC_MUL("), .len = sizeof("__OC_MUL(") - 1 });
    codegen_expr(((expr)->left), out);
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)(", "), .len = sizeof(", ") - 1 });
    codegen_expr(((expr)->right), out);
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)(")"), .len = sizeof(")") - 1 });
    return;
    }
    }
    }
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("("), .len = sizeof("(") - 1 });
    codegen_expr(((expr)->left), out);
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)(" "), .len = sizeof(" ") - 1 });
    buf_append(out, ((((expr)->tok)).slice));
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)(" "), .len = sizeof(" ") - 1 });
    codegen_expr(((expr)->right), out);
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)(")"), .len = sizeof(")") - 1 });
    return;
    }
}

bool is_comparison_tok(uint32_t tag) {
    if ((tag == TOK_EQEQ)) {
    return true;
    }
    if ((tag == TOK_NEQ)) {
    return true;
    }
    if ((tag == TOK_LT)) {
    return true;
    }
    if ((tag == TOK_LTE)) {
    return true;
    }
    if ((tag == TOK_GT)) {
    return true;
    }
    if ((tag == TOK_GTE)) {
    return true;
    }
    return false;
}

bool is_bool_expr(AstNode* expr) {
    if ((expr == ((AstNode*)(0)))) {
    return false;
    }
    if ((((expr)->tag) == NODE_BOOL)) {
    return true;
    }
    if ((((expr)->tag) == NODE_UNARY)) {
    return (((((expr)->tok)).tag) == TOK_BANG);
    }
    if ((((expr)->tag) == NODE_BINARY)) {
    return is_comparison_tok(((((expr)->tok)).tag));
    }
    return false;
}

oc_string cg_var_type(CodeBuffer* out, oc_string name) {
    size_t i = ((out)->var_types_len);
    while ((i > 0)) {
    i = __OC_SUB(i, 1);
    if (str_eq(__OC_AT((((out)->var_names)), (i)), name)) {
    return __OC_AT((((out)->var_types)), (i));
    }
    }
    return (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 };
}

bool is_u8_array_type(oc_string t) {
    if ((((t).len) < 4)) {
    return false;
    }
    if ((__oc_index_str(t, 0) != 91)) {
    return false;
    }
    if ((__oc_index_str(t, __OC_SUB(((t).len), 3)) != 93)) {
    return false;
    }
    if ((__oc_index_str(t, __OC_SUB(((t).len), 2)) != 117)) {
    return false;
    }
    return (__oc_index_str(t, __OC_SUB(((t).len), 1)) == 56);
}

void codegen_print(AstNode* node, CodeBuffer* out) {
    AstNode* arg = ((node)->left);
    if ((arg != ((AstNode*)(0)))) {
    if ((((arg)->tag) == NODE_STRING)) {
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("    fwrite("), .len = sizeof("    fwrite(") - 1 });
    buf_append_c_literal(out, ((((arg)->tok)).slice));
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)(", 1, sizeof("), .len = sizeof(", 1, sizeof(") - 1 });
    buf_append_c_literal(out, ((((arg)->tok)).slice));
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)(") - 1, stdout); fputc('\134n', stdout);\012"), .len = sizeof(") - 1, stdout); fputc('\134n', stdout);\012") - 1 });
    return;
    }
    if ((((arg)->tag) == NODE_IDENT)) {
    if (is_u8_array_type(cg_var_type(out, ((((arg)->tok)).slice)))) {
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("    printf(\042%.*s\134n\042, (int)sizeof("), .len = sizeof("    printf(\042%.*s\134n\042, (int)sizeof(") - 1 });
    buf_append_ident(out, ((((arg)->tok)).slice));
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("), (const char*)"), .len = sizeof("), (const char*)") - 1 });
    buf_append_ident(out, ((((arg)->tok)).slice));
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)(");\012"), .len = sizeof(");\012") - 1 });
    return;
    }
    }
    }
    oc_string print_ft = cg_float_type(arg, out);
    if ((((print_ft).len) > 0)) {
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("    fmt_print_f64((double)("), .len = sizeof("    fmt_print_f64((double)(") - 1 });
    codegen_expr(arg, out);
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("));\012"), .len = sizeof("));\012") - 1 });
    return;
    }
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("    __oc_print("), .len = sizeof("    __oc_print(") - 1 });
    if (is_bool_expr(arg)) {
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("(bool)("), .len = sizeof("(bool)(") - 1 });
    codegen_expr(arg, out);
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)(")"), .len = sizeof(")") - 1 });
    }
    else {
    codegen_expr(arg, out);
    }
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)(");\012"), .len = sizeof(");\012") - 1 });
}

void codegen_stmt(AstNode* stmt, CodeBuffer* out) {
    if ((stmt == ((AstNode*)(0)))) {
    return;
    }
    if ((((stmt)->left) == ((AstNode*)(0)))) {
    if (str_eq(((((stmt)->tok)).slice), (oc_string){ .ptr = (const uint8_t*)("break"), .len = sizeof("break") - 1 })) {
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("    break;\012"), .len = sizeof("    break;\012") - 1 });
    return;
    }
    if (str_eq(((((stmt)->tok)).slice), (oc_string){ .ptr = (const uint8_t*)("continue"), .len = sizeof("continue") - 1 })) {
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("    continue;\012"), .len = sizeof("    continue;\012") - 1 });
    return;
    }
    }
    if ((((stmt)->tag) == NODE_PRINT)) {
    codegen_print(stmt, out);
    return;
    }
    if ((((stmt)->tag) == NODE_LET)) {
    if ((((out)->var_types_len) < 512)) {
    oc_string let_type = ((stmt)->type_name);
    if ((((let_type).len) == 0)) {
    let_type = cg_float_type(((stmt)->left), out);
    }
    __OC_AT((((out)->var_names)), (((out)->var_types_len))) = ((((stmt)->tok)).slice);
    __OC_AT((((out)->var_types)), (((out)->var_types_len))) = let_type;
    ((out)->var_types_len) = __OC_ADD(((out)->var_types_len), 1);
    }
    if (is_type_ptr(((stmt)->type_name))) {
    if ((((out)->ptr_vars_count) < 512)) {
    __OC_AT((((out)->ptr_vars)), (((out)->ptr_vars_count))) = ((((stmt)->tok)).slice);
    ((out)->ptr_vars_count) = __OC_ADD(((out)->ptr_vars_count), 1);
    }
    }
    else {
    if (is_type_str(((stmt)->type_name))) {
    if ((((out)->str_vars_count) < 512)) {
    __OC_AT((((out)->str_vars)), (((out)->str_vars_count))) = ((((stmt)->tok)).slice);
    ((out)->str_vars_count) = __OC_ADD(((out)->str_vars_count), 1);
    }
    }
    else {
    if ((((((stmt)->type_name)).len) == 0)) {
    if (is_expr_pointer(((stmt)->left), out)) {
    if ((((out)->ptr_vars_count) < 512)) {
    __OC_AT((((out)->ptr_vars)), (((out)->ptr_vars_count))) = ((((stmt)->tok)).slice);
    ((out)->ptr_vars_count) = __OC_ADD(((out)->ptr_vars_count), 1);
    }
    }
    else {
    if (is_expr_string(((stmt)->left), out)) {
    if ((((out)->str_vars_count) < 512)) {
    __OC_AT((((out)->str_vars)), (((out)->str_vars_count))) = ((((stmt)->tok)).slice);
    ((out)->str_vars_count) = __OC_ADD(((out)->str_vars_count), 1);
    }
    }
    }
    }
    }
    }
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("    "), .len = sizeof("    ") - 1 });
    if ((((((stmt)->type_name)).len) > 0)) {
    if ((__oc_index_str(((stmt)->type_name), 0) == 91)) {
    size_t close_idx = 1;
    while ((close_idx < ((((stmt)->type_name)).len))) {
    if ((__oc_index_str(((stmt)->type_name), close_idx) == 93)) {
    break;
    }
    close_idx = __OC_ADD(close_idx, 1);
    }
    oc_string sz_str = __oc_slice_str(((stmt)->type_name), 1, close_idx);
    oc_string elem_str = __oc_slice_str(((stmt)->type_name), __OC_ADD(close_idx, 1), ((((stmt)->type_name)).len));
    emit_c_type(elem_str, out);
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)(" "), .len = sizeof(" ") - 1 });
    buf_append_ident(out, ((((stmt)->tok)).slice));
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("["), .len = sizeof("[") - 1 });
    buf_append(out, sz_str);
    bool use_zero_init = true;
    if ((((stmt)->left) != ((AstNode*)(0)))) {
    if ((((((stmt)->left))->tag) == NODE_ARRAY_INIT)) {
    AstNode* first_el = ((((stmt)->left))->left);
    if ((first_el != ((AstNode*)(0)))) {
    if ((((first_el)->next) != ((AstNode*)(0)))) {
    use_zero_init = false;
    }
    else {
    bool is_single_zero = false;
    if ((((first_el)->tag) == NODE_INT)) {
    if (str_eq(((((first_el)->tok)).slice), (oc_string){ .ptr = (const uint8_t*)("0"), .len = sizeof("0") - 1 })) {
    is_single_zero = true;
    }
    }
    else {
    if ((((first_el)->tag) == NODE_STRING)) {
    if (str_eq(((((first_el)->tok)).slice), (oc_string){ .ptr = (const uint8_t*)("\042\042"), .len = sizeof("\042\042") - 1 })) {
    is_single_zero = true;
    }
    }
    else {
    if ((((first_el)->tag) == NODE_BOOL)) {
    if (str_eq(((((first_el)->tok)).slice), (oc_string){ .ptr = (const uint8_t*)("false"), .len = sizeof("false") - 1 })) {
    is_single_zero = true;
    }
    }
    }
    }
    if ((!is_single_zero)) {
    use_zero_init = false;
    }
    }
    }
    }
    }
    if (use_zero_init) {
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("] = {0};\012"), .len = sizeof("] = {0};\012") - 1 });
    }
    else {
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("] = "), .len = sizeof("] = ") - 1 });
    codegen_expr(((stmt)->left), out);
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)(";\012"), .len = sizeof(";\012") - 1 });
    }
    return;
    }
    emit_c_type(((stmt)->type_name), out);
    }
    else {
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("__auto_type"), .len = sizeof("__auto_type") - 1 });
    }
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)(" "), .len = sizeof(" ") - 1 });
    buf_append_ident(out, ((((stmt)->tok)).slice));
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)(" = "), .len = sizeof(" = ") - 1 });
    codegen_expr(((stmt)->left), out);
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)(";\012"), .len = sizeof(";\012") - 1 });
    return;
    }
    if ((((stmt)->tag) == NODE_RETURN)) {
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("    return"), .len = sizeof("    return") - 1 });
    if ((((stmt)->left) != ((AstNode*)(0)))) {
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)(" "), .len = sizeof(" ") - 1 });
    codegen_expr(((stmt)->left), out);
    }
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)(";\012"), .len = sizeof(";\012") - 1 });
    return;
    }
    if ((((stmt)->tag) == NODE_ASSIGN)) {
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("    "), .len = sizeof("    ") - 1 });
    codegen_expr(((stmt)->left), out);
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)(" = "), .len = sizeof(" = ") - 1 });
    codegen_expr(((stmt)->right), out);
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)(";\012"), .len = sizeof(";\012") - 1 });
    return;
    }
    size_t scope_mark = ((out)->var_types_len);
    if ((((stmt)->tag) == NODE_IF)) {
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("    if ("), .len = sizeof("    if (") - 1 });
    codegen_expr(((stmt)->left), out);
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)(") {\012"), .len = sizeof(") {\012") - 1 });
    if ((((stmt)->right) != ((AstNode*)(0)))) {
    AstNode* s = ((((stmt)->right))->left);
    while ((s != ((AstNode*)(0)))) {
    codegen_stmt(s, out);
    s = ((s)->next);
    }
    }
    ((out)->var_types_len) = scope_mark;
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("    }\012"), .len = sizeof("    }\012") - 1 });
    if ((((stmt)->right) != ((AstNode*)(0)))) {
    if ((((((stmt)->right))->next) != ((AstNode*)(0)))) {
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("    else {\012"), .len = sizeof("    else {\012") - 1 });
    if ((((((((stmt)->right))->next))->tag) == NODE_IF)) {
    codegen_stmt(((((stmt)->right))->next), out);
    }
    else {
    AstNode* es = ((((((stmt)->right))->next))->left);
    while ((es != ((AstNode*)(0)))) {
    codegen_stmt(es, out);
    es = ((es)->next);
    }
    }
    ((out)->var_types_len) = scope_mark;
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("    }\012"), .len = sizeof("    }\012") - 1 });
    }
    }
    return;
    }
    if ((((stmt)->tag) == NODE_WHILE)) {
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("    while ("), .len = sizeof("    while (") - 1 });
    codegen_expr(((stmt)->left), out);
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)(") {\012"), .len = sizeof(") {\012") - 1 });
    if ((((stmt)->right) != ((AstNode*)(0)))) {
    AstNode* s = ((((stmt)->right))->left);
    while ((s != ((AstNode*)(0)))) {
    codegen_stmt(s, out);
    s = ((s)->next);
    }
    }
    ((out)->var_types_len) = scope_mark;
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("    }\012"), .len = sizeof("    }\012") - 1 });
    return;
    }
    if ((((stmt)->tag) == NODE_BLOCK)) {
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("    {\012"), .len = sizeof("    {\012") - 1 });
    AstNode* s = ((stmt)->left);
    while ((s != ((AstNode*)(0)))) {
    codegen_stmt(s, out);
    s = ((s)->next);
    }
    ((out)->var_types_len) = scope_mark;
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("    }\012"), .len = sizeof("    }\012") - 1 });
    return;
    }
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("    "), .len = sizeof("    ") - 1 });
    codegen_expr(stmt, out);
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)(";\012"), .len = sizeof(";\012") - 1 });
}

bool is_builtin_c_fn(oc_string fn_name) {
    if (str_eq(fn_name, (oc_string){ .ptr = (const uint8_t*)("system"), .len = sizeof("system") - 1 })) {
    return true;
    }
    if (str_eq(fn_name, (oc_string){ .ptr = (const uint8_t*)("getenv"), .len = sizeof("getenv") - 1 })) {
    return true;
    }
    if (str_eq(fn_name, (oc_string){ .ptr = (const uint8_t*)("sys_mkdtemp"), .len = sizeof("sys_mkdtemp") - 1 })) {
    return true;
    }
    if (str_eq(fn_name, (oc_string){ .ptr = (const uint8_t*)("sys_rmdir"), .len = sizeof("sys_rmdir") - 1 })) {
    return true;
    }
    if (str_eq(fn_name, (oc_string){ .ptr = (const uint8_t*)("sys_exe_path"), .len = sizeof("sys_exe_path") - 1 })) {
    return true;
    }
    if (str_eq(fn_name, (oc_string){ .ptr = (const uint8_t*)("sys_f64_bits"), .len = sizeof("sys_f64_bits") - 1 })) {
    return true;
    }
    if (str_eq(fn_name, (oc_string){ .ptr = (const uint8_t*)("sys_f32_bits"), .len = sizeof("sys_f32_bits") - 1 })) {
    return true;
    }
    if (str_eq(fn_name, (oc_string){ .ptr = (const uint8_t*)("exit"), .len = sizeof("exit") - 1 })) {
    return true;
    }
    if (str_eq(fn_name, (oc_string){ .ptr = (const uint8_t*)("sys_read"), .len = sizeof("sys_read") - 1 })) {
    return true;
    }
    if (str_eq(fn_name, (oc_string){ .ptr = (const uint8_t*)("sys_write"), .len = sizeof("sys_write") - 1 })) {
    return true;
    }
    if (str_eq(fn_name, (oc_string){ .ptr = (const uint8_t*)("sys_open"), .len = sizeof("sys_open") - 1 })) {
    return true;
    }
    if (str_eq(fn_name, (oc_string){ .ptr = (const uint8_t*)("sys_close"), .len = sizeof("sys_close") - 1 })) {
    return true;
    }
    if (str_eq(fn_name, (oc_string){ .ptr = (const uint8_t*)("sys_mmap"), .len = sizeof("sys_mmap") - 1 })) {
    return true;
    }
    if (str_eq(fn_name, (oc_string){ .ptr = (const uint8_t*)("sys_munmap"), .len = sizeof("sys_munmap") - 1 })) {
    return true;
    }
    if (str_eq(fn_name, (oc_string){ .ptr = (const uint8_t*)("sys_chmod"), .len = sizeof("sys_chmod") - 1 })) {
    return true;
    }
    if (str_eq(fn_name, (oc_string){ .ptr = (const uint8_t*)("sys_unlink"), .len = sizeof("sys_unlink") - 1 })) {
    return true;
    }
    if (str_eq(fn_name, (oc_string){ .ptr = (const uint8_t*)("sys_socket"), .len = sizeof("sys_socket") - 1 })) {
    return true;
    }
    if (str_eq(fn_name, (oc_string){ .ptr = (const uint8_t*)("sys_connect"), .len = sizeof("sys_connect") - 1 })) {
    return true;
    }
    if (str_eq(fn_name, (oc_string){ .ptr = (const uint8_t*)("sys_exit"), .len = sizeof("sys_exit") - 1 })) {
    return true;
    }
    if (str_eq(fn_name, (oc_string){ .ptr = (const uint8_t*)("sys_host_target"), .len = sizeof("sys_host_target") - 1 })) {
    return true;
    }
    if (str_eq(fn_name, (oc_string){ .ptr = (const uint8_t*)("sys_mprotect"), .len = sizeof("sys_mprotect") - 1 })) {
    return true;
    }
    if (str_eq(fn_name, (oc_string){ .ptr = (const uint8_t*)("sys_arena_poison"), .len = sizeof("sys_arena_poison") - 1 })) {
    return true;
    }
    return false;
}

void codegen_program(AstNode* root, CodeBuffer* out) {
    ((out)->program) = root;
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("#pragma GCC diagnostic ignored \042-Wmain\042\012"), .len = sizeof("#pragma GCC diagnostic ignored \042-Wmain\042\012") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("#pragma GCC diagnostic ignored \042-Wdiscarded-qualifiers\042\012"), .len = sizeof("#pragma GCC diagnostic ignored \042-Wdiscarded-qualifiers\042\012") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("#pragma GCC diagnostic ignored \042-Wpointer-sign\042\012"), .len = sizeof("#pragma GCC diagnostic ignored \042-Wpointer-sign\042\012") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("#pragma GCC diagnostic ignored \042-Wincompatible-pointer-types\042\012"), .len = sizeof("#pragma GCC diagnostic ignored \042-Wincompatible-pointer-types\042\012") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("#include <stdio.h>\012"), .len = sizeof("#include <stdio.h>\012") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("#include <stdint.h>\012"), .len = sizeof("#include <stdint.h>\012") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("#include <stdbool.h>\012"), .len = sizeof("#include <stdbool.h>\012") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("#include <stdlib.h>\012"), .len = sizeof("#include <stdlib.h>\012") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("#include <string.h>\012"), .len = sizeof("#include <string.h>\012") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("#include <fcntl.h>\012"), .len = sizeof("#include <fcntl.h>\012") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("#include <sys/stat.h>\012"), .len = sizeof("#include <sys/stat.h>\012") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("#ifdef _WIN32\012"), .len = sizeof("#ifdef _WIN32\012") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("#include <io.h>\012"), .len = sizeof("#include <io.h>\012") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("__declspec(dllimport) void* __stdcall VirtualAlloc(void*, size_t, unsigned long, unsigned long);\012"), .len = sizeof("__declspec(dllimport) void* __stdcall VirtualAlloc(void*, size_t, unsigned long, unsigned long);\012") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("__declspec(dllimport) int __stdcall VirtualFree(void*, size_t, unsigned long);\012"), .len = sizeof("__declspec(dllimport) int __stdcall VirtualFree(void*, size_t, unsigned long);\012") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("__declspec(dllimport) int __stdcall VirtualProtect(void*, size_t, unsigned long, unsigned long*);\012"), .len = sizeof("__declspec(dllimport) int __stdcall VirtualProtect(void*, size_t, unsigned long, unsigned long*);\012") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("#define O_CLOEXEC_HOST _O_NOINHERIT\012"), .len = sizeof("#define O_CLOEXEC_HOST _O_NOINHERIT\012") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("#else\012"), .len = sizeof("#else\012") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("#include <unistd.h>\012"), .len = sizeof("#include <unistd.h>\012") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("#include <sys/mman.h>\012"), .len = sizeof("#include <sys/mman.h>\012") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("#include <sys/socket.h>\012"), .len = sizeof("#include <sys/socket.h>\012") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("#ifndef MAP_ANONYMOUS\012#define MAP_ANONYMOUS MAP_ANON\012#endif\012"), .len = sizeof("#ifndef MAP_ANONYMOUS\012#define MAP_ANONYMOUS MAP_ANON\012#endif\012") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("#define O_CLOEXEC_HOST O_CLOEXEC\012"), .len = sizeof("#define O_CLOEXEC_HOST O_CLOEXEC\012") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("#endif\012"), .len = sizeof("#endif\012") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("#ifndef O_NOFOLLOW\012#define O_NOFOLLOW 0\012#endif\012"), .len = sizeof("#ifndef O_NOFOLLOW\012#define O_NOFOLLOW 0\012#endif\012") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("#ifndef O_BINARY\012#define O_BINARY 0\012#endif\012"), .len = sizeof("#ifndef O_BINARY\012#define O_BINARY 0\012#endif\012") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("#ifdef htons\012#undef htons\012#endif\012"), .len = sizeof("#ifdef htons\012#undef htons\012#endif\012") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("#ifdef ntohs\012#undef ntohs\012#endif\012"), .len = sizeof("#ifdef ntohs\012#undef ntohs\012#endif\012") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("#ifdef htonl\012#undef htonl\012#endif\012"), .len = sizeof("#ifdef htonl\012#undef htonl\012#endif\012") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("#ifdef ntohl\012#undef ntohl\012#endif\012\012"), .len = sizeof("#ifdef ntohl\012#undef ntohl\012#endif\012\012") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("static inline int __oc_open_flags(int32_t f) { int r = O_BINARY; int acc = f & 3; if (acc == 1) r |= O_WRONLY; else if (acc == 2) r |= O_RDWR; else r |= O_RDONLY; if (f & 64) r |= O_CREAT; if (f & 512) r |= O_TRUNC; if (f & 1024) r |= O_APPEND; if (f & 131072) r |= O_NOFOLLOW; if (f & 524288) r |= O_CLOEXEC_HOST; return r; }\012"), .len = sizeof("static inline int __oc_open_flags(int32_t f) { int r = O_BINARY; int acc = f & 3; if (acc == 1) r |= O_WRONLY; else if (acc == 2) r |= O_RDWR; else r |= O_RDONLY; if (f & 64) r |= O_CREAT; if (f & 512) r |= O_TRUNC; if (f & 1024) r |= O_APPEND; if (f & 131072) r |= O_NOFOLLOW; if (f & 524288) r |= O_CLOEXEC_HOST; return r; }\012") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("#ifdef _WIN32\012"), .len = sizeof("#ifdef _WIN32\012") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("static inline int64_t sys_read(int32_t fd, char* buf, size_t count) { return (int64_t)_read(fd, buf, (unsigned)count); }\012"), .len = sizeof("static inline int64_t sys_read(int32_t fd, char* buf, size_t count) { return (int64_t)_read(fd, buf, (unsigned)count); }\012") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("static inline int64_t sys_write(int32_t fd, const char* buf, size_t count) { return (int64_t)_write(fd, buf, (unsigned)count); }\012"), .len = sizeof("static inline int64_t sys_write(int32_t fd, const char* buf, size_t count) { return (int64_t)_write(fd, buf, (unsigned)count); }\012") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("static inline int32_t sys_open(const char* path, int32_t flags, uint32_t mode) { (void)mode; return (int32_t)_open(path, __oc_open_flags(flags), _S_IREAD | _S_IWRITE); }\012"), .len = sizeof("static inline int32_t sys_open(const char* path, int32_t flags, uint32_t mode) { (void)mode; return (int32_t)_open(path, __oc_open_flags(flags), _S_IREAD | _S_IWRITE); }\012") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("static inline int32_t sys_close(int32_t fd) { return (int32_t)_close(fd); }\012"), .len = sizeof("static inline int32_t sys_close(int32_t fd) { return (int32_t)_close(fd); }\012") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("static inline void* sys_mmap(size_t addr, size_t len, int32_t prot, int32_t flags, int32_t fd, size_t off) { (void)addr; (void)prot; (void)flags; (void)fd; (void)off; void* p = VirtualAlloc(0, len, 0x3000, 0x04); return p \077 p : (void*)-1; }\012"), .len = sizeof("static inline void* sys_mmap(size_t addr, size_t len, int32_t prot, int32_t flags, int32_t fd, size_t off) { (void)addr; (void)prot; (void)flags; (void)fd; (void)off; void* p = VirtualAlloc(0, len, 0x3000, 0x04); return p \077 p : (void*)-1; }\012") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("static inline int32_t sys_munmap(void* addr, size_t len) { (void)len; return VirtualFree(addr, 0, 0x8000) \077 0 : -1; }\012"), .len = sizeof("static inline int32_t sys_munmap(void* addr, size_t len) { (void)len; return VirtualFree(addr, 0, 0x8000) \077 0 : -1; }\012") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("static inline int32_t sys_mprotect(void* addr, size_t len, int32_t prot) { unsigned long old; return VirtualProtect(addr, len, prot == 0 \077 0x01 : 0x04, &old) \077 0 : -1; }\012"), .len = sizeof("static inline int32_t sys_mprotect(void* addr, size_t len, int32_t prot) { unsigned long old; return VirtualProtect(addr, len, prot == 0 \077 0x01 : 0x04, &old) \077 0 : -1; }\012") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("static inline int32_t sys_chmod(const char* path, uint32_t mode) { return (int32_t)_chmod(path, _S_IREAD | ((mode & 128) \077 _S_IWRITE : 0)); }\012"), .len = sizeof("static inline int32_t sys_chmod(const char* path, uint32_t mode) { return (int32_t)_chmod(path, _S_IREAD | ((mode & 128) \077 _S_IWRITE : 0)); }\012") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("static inline int32_t sys_unlink(const char* path) { return (int32_t)_unlink(path); }\012"), .len = sizeof("static inline int32_t sys_unlink(const char* path) { return (int32_t)_unlink(path); }\012") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("static inline int32_t sys_socket(int32_t domain, int32_t type, int32_t protocol) { (void)domain; (void)type; (void)protocol; return -1; }\012"), .len = sizeof("static inline int32_t sys_socket(int32_t domain, int32_t type, int32_t protocol) { (void)domain; (void)type; (void)protocol; return -1; }\012") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("static inline int32_t sys_connect(int32_t fd, const void* addr, uint32_t len) { (void)fd; (void)addr; (void)len; return -1; }\012"), .len = sizeof("static inline int32_t sys_connect(int32_t fd, const void* addr, uint32_t len) { (void)fd; (void)addr; (void)len; return -1; }\012") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("#else\012"), .len = sizeof("#else\012") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("static inline int64_t sys_read(int32_t fd, char* buf, size_t count) { return (int64_t)read(fd, buf, count); }\012"), .len = sizeof("static inline int64_t sys_read(int32_t fd, char* buf, size_t count) { return (int64_t)read(fd, buf, count); }\012") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("static inline int64_t sys_write(int32_t fd, const char* buf, size_t count) { return (int64_t)write(fd, buf, count); }\012"), .len = sizeof("static inline int64_t sys_write(int32_t fd, const char* buf, size_t count) { return (int64_t)write(fd, buf, count); }\012") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("static inline int32_t sys_open(const char* path, int32_t flags, uint32_t mode) { return (int32_t)open(path, __oc_open_flags(flags), (mode_t)mode); }\012"), .len = sizeof("static inline int32_t sys_open(const char* path, int32_t flags, uint32_t mode) { return (int32_t)open(path, __oc_open_flags(flags), (mode_t)mode); }\012") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("static inline int32_t sys_close(int32_t fd) { return (int32_t)close(fd); }\012"), .len = sizeof("static inline int32_t sys_close(int32_t fd) { return (int32_t)close(fd); }\012") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("static inline void* sys_mmap(size_t addr, size_t len, int32_t prot, int32_t flags, int32_t fd, size_t off) { int p = 0; if (prot & 1) p |= PROT_READ; if (prot & 2) p |= PROT_WRITE; if (prot & 4) p |= PROT_EXEC; int m = 0; if (flags & 1) m |= MAP_SHARED; if (flags & 2) m |= MAP_PRIVATE; if (flags & 32) m |= MAP_ANONYMOUS; return mmap((void*)addr, len, p, m, fd, (off_t)off); }\012"), .len = sizeof("static inline void* sys_mmap(size_t addr, size_t len, int32_t prot, int32_t flags, int32_t fd, size_t off) { int p = 0; if (prot & 1) p |= PROT_READ; if (prot & 2) p |= PROT_WRITE; if (prot & 4) p |= PROT_EXEC; int m = 0; if (flags & 1) m |= MAP_SHARED; if (flags & 2) m |= MAP_PRIVATE; if (flags & 32) m |= MAP_ANONYMOUS; return mmap((void*)addr, len, p, m, fd, (off_t)off); }\012") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("static inline int32_t sys_munmap(void* addr, size_t len) { return (int32_t)munmap(addr, len); }\012"), .len = sizeof("static inline int32_t sys_munmap(void* addr, size_t len) { return (int32_t)munmap(addr, len); }\012") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("static inline int32_t sys_mprotect(void* addr, size_t len, int32_t prot) { int p = 0; if (prot & 1) p |= PROT_READ; if (prot & 2) p |= PROT_WRITE; return (int32_t)mprotect(addr, len, p); }\012"), .len = sizeof("static inline int32_t sys_mprotect(void* addr, size_t len, int32_t prot) { int p = 0; if (prot & 1) p |= PROT_READ; if (prot & 2) p |= PROT_WRITE; return (int32_t)mprotect(addr, len, p); }\012") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("static inline int32_t sys_chmod(const char* path, uint32_t mode) { return (int32_t)chmod(path, (mode_t)mode); }\012"), .len = sizeof("static inline int32_t sys_chmod(const char* path, uint32_t mode) { return (int32_t)chmod(path, (mode_t)mode); }\012") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("static inline int32_t sys_unlink(const char* path) { return (int32_t)unlink(path); }\012"), .len = sizeof("static inline int32_t sys_unlink(const char* path) { return (int32_t)unlink(path); }\012") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("static inline int32_t sys_socket(int32_t domain, int32_t type, int32_t protocol) { return (int32_t)socket(domain, type, protocol); }\012"), .len = sizeof("static inline int32_t sys_socket(int32_t domain, int32_t type, int32_t protocol) { return (int32_t)socket(domain, type, protocol); }\012") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("#if defined(__APPLE__) || defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__) || defined(__DragonFly__)\012"), .len = sizeof("#if defined(__APPLE__) || defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__) || defined(__DragonFly__)\012") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("static inline int32_t sys_connect(int32_t fd, const void* addr, uint32_t len) { uint8_t sa[128]; if (len > sizeof sa) return -1; memcpy(sa, addr, len); sa[1] = sa[0]; sa[0] = (uint8_t)len; return (int32_t)connect(fd, (const struct sockaddr*)sa, (socklen_t)len); }\012"), .len = sizeof("static inline int32_t sys_connect(int32_t fd, const void* addr, uint32_t len) { uint8_t sa[128]; if (len > sizeof sa) return -1; memcpy(sa, addr, len); sa[1] = sa[0]; sa[0] = (uint8_t)len; return (int32_t)connect(fd, (const struct sockaddr*)sa, (socklen_t)len); }\012") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("#else\012"), .len = sizeof("#else\012") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("static inline int32_t sys_connect(int32_t fd, const void* addr, uint32_t len) { return (int32_t)connect(fd, (const struct sockaddr*)addr, (socklen_t)len); }\012"), .len = sizeof("static inline int32_t sys_connect(int32_t fd, const void* addr, uint32_t len) { return (int32_t)connect(fd, (const struct sockaddr*)addr, (socklen_t)len); }\012") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("#endif\012"), .len = sizeof("#endif\012") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("#endif\012"), .len = sizeof("#endif\012") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("#ifdef _WIN32\012"), .len = sizeof("#ifdef _WIN32\012") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("#include <direct.h>\012"), .len = sizeof("#include <direct.h>\012") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("static inline int32_t sys_mkdtemp(char* tmpl) { return (_mktemp(tmpl) != 0 && _mkdir(tmpl) == 0) \077 0 : -1; }\012"), .len = sizeof("static inline int32_t sys_mkdtemp(char* tmpl) { return (_mktemp(tmpl) != 0 && _mkdir(tmpl) == 0) \077 0 : -1; }\012") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("static inline int32_t sys_rmdir(const char* path) { return (int32_t)_rmdir(path); }\012"), .len = sizeof("static inline int32_t sys_rmdir(const char* path) { return (int32_t)_rmdir(path); }\012") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("#else\012"), .len = sizeof("#else\012") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("static inline int32_t sys_mkdtemp(char* tmpl) { return mkdtemp(tmpl) \077 0 : -1; }\012"), .len = sizeof("static inline int32_t sys_mkdtemp(char* tmpl) { return mkdtemp(tmpl) \077 0 : -1; }\012") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("static inline int32_t sys_rmdir(const char* path) { return (int32_t)rmdir(path); }\012"), .len = sizeof("static inline int32_t sys_rmdir(const char* path) { return (int32_t)rmdir(path); }\012") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("#endif\012"), .len = sizeof("#endif\012") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("#if defined(_WIN32)\012"), .len = sizeof("#if defined(_WIN32)\012") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("__declspec(dllimport) unsigned long __stdcall GetModuleFileNameA(void*, char*, unsigned long);\012"), .len = sizeof("__declspec(dllimport) unsigned long __stdcall GetModuleFileNameA(void*, char*, unsigned long);\012") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("static inline int64_t sys_exe_path(char* buf, size_t cap) { unsigned long n = GetModuleFileNameA(0, buf, (unsigned long)cap); return (n == 0 || n >= cap) \077 -1 : (int64_t)n; }\012"), .len = sizeof("static inline int64_t sys_exe_path(char* buf, size_t cap) { unsigned long n = GetModuleFileNameA(0, buf, (unsigned long)cap); return (n == 0 || n >= cap) \077 -1 : (int64_t)n; }\012") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("#elif defined(__APPLE__)\012"), .len = sizeof("#elif defined(__APPLE__)\012") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("#include <limits.h>\012"), .len = sizeof("#include <limits.h>\012") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("#include <mach-o/dyld.h>\012"), .len = sizeof("#include <mach-o/dyld.h>\012") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("static inline int64_t sys_exe_path(char* buf, size_t cap) { char raw[PATH_MAX]; char real[PATH_MAX]; uint32_t sz = sizeof raw; if (_NSGetExecutablePath(raw, &sz) != 0 || !realpath(raw, real)) return -1; size_t n = strlen(real); if (n >= cap) return -1; memcpy(buf, real, n + 1); return (int64_t)n; }\012"), .len = sizeof("static inline int64_t sys_exe_path(char* buf, size_t cap) { char raw[PATH_MAX]; char real[PATH_MAX]; uint32_t sz = sizeof raw; if (_NSGetExecutablePath(raw, &sz) != 0 || !realpath(raw, real)) return -1; size_t n = strlen(real); if (n >= cap) return -1; memcpy(buf, real, n + 1); return (int64_t)n; }\012") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("#else\012"), .len = sizeof("#else\012") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("static inline int64_t sys_exe_path(char* buf, size_t cap) { ssize_t n = cap \077 readlink(\042/proc/self/exe\042, buf, cap - 1) : -1; if (n <= 0) return -1; buf[n] = 0; return (int64_t)n; }\012"), .len = sizeof("static inline int64_t sys_exe_path(char* buf, size_t cap) { ssize_t n = cap \077 readlink(\042/proc/self/exe\042, buf, cap - 1) : -1; if (n <= 0) return -1; buf[n] = 0; return (int64_t)n; }\012") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("#endif\012"), .len = sizeof("#endif\012") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("static inline uint64_t sys_f64_bits(const char* text) { double d = strtod(text, 0); uint64_t u; memcpy(&u, &d, sizeof u); return u; }\012"), .len = sizeof("static inline uint64_t sys_f64_bits(const char* text) { double d = strtod(text, 0); uint64_t u; memcpy(&u, &d, sizeof u); return u; }\012") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("static inline uint64_t sys_f32_bits(const char* text) { float f = strtof(text, 0); uint32_t u; memcpy(&u, &f, sizeof u); return u; }\012"), .len = sizeof("static inline uint64_t sys_f32_bits(const char* text) { float f = strtof(text, 0); uint32_t u; memcpy(&u, &f, sizeof u); return u; }\012") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("static inline void sys_exit(int32_t code) { _exit(code); }\012"), .len = sizeof("static inline void sys_exit(int32_t code) { _exit(code); }\012") - 1 });
    if (((out)->arena_poison)) {
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("static inline bool sys_arena_poison(void) { return 1; }\012"), .len = sizeof("static inline bool sys_arena_poison(void) { return 1; }\012") - 1 });
    }
    else {
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("static inline bool sys_arena_poison(void) { return 0; }\012"), .len = sizeof("static inline bool sys_arena_poison(void) { return 0; }\012") - 1 });
    }
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("#if defined(_WIN32)\012static inline int32_t sys_host_target(void) { return 2; }\012"), .len = sizeof("#if defined(_WIN32)\012static inline int32_t sys_host_target(void) { return 2; }\012") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("#elif defined(__APPLE__) && (defined(__aarch64__) || defined(__arm64__))\012static inline int32_t sys_host_target(void) { return 4; }\012"), .len = sizeof("#elif defined(__APPLE__) && (defined(__aarch64__) || defined(__arm64__))\012static inline int32_t sys_host_target(void) { return 4; }\012") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("#elif defined(__APPLE__) && defined(__x86_64__)\012static inline int32_t sys_host_target(void) { return 3; }\012"), .len = sizeof("#elif defined(__APPLE__) && defined(__x86_64__)\012static inline int32_t sys_host_target(void) { return 3; }\012") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("#elif defined(__linux__) && defined(__x86_64__)\012static inline int32_t sys_host_target(void) { return 1; }\012"), .len = sizeof("#elif defined(__linux__) && defined(__x86_64__)\012static inline int32_t sys_host_target(void) { return 1; }\012") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("#else\012static inline int32_t sys_host_target(void) { return 0; }\012#endif\012\012"), .len = sizeof("#else\012static inline int32_t sys_host_target(void) { return 0; }\012#endif\012\012") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("#ifdef _WIN32\012#define __OC_STDOUT_BUF _IONBF\012#else\012#define __OC_STDOUT_BUF _IOLBF\012#endif\012\012"), .len = sizeof("#ifdef _WIN32\012#define __OC_STDOUT_BUF _IONBF\012#else\012#define __OC_STDOUT_BUF _IOLBF\012#endif\012\012") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("typedef struct {\012"), .len = sizeof("typedef struct {\012") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("    const uint8_t* ptr;\012"), .len = sizeof("    const uint8_t* ptr;\012") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("    size_t len;\012"), .len = sizeof("    size_t len;\012") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("} oc_string;\012\012"), .len = sizeof("} oc_string;\012\012") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("static inline void __oc_print_str(const char* s) { printf(\042%s\134n\042, s); }\012"), .len = sizeof("static inline void __oc_print_str(const char* s) { printf(\042%s\134n\042, s); }\012") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("static inline void __oc_print_u8_ptr(uint8_t* s) { printf(\042%s\134n\042, (const char*)s); }\012"), .len = sizeof("static inline void __oc_print_u8_ptr(uint8_t* s) { printf(\042%s\134n\042, (const char*)s); }\012") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("static inline void __oc_print_const_u8_ptr(const uint8_t* s) { printf(\042%s\134n\042, (const char*)s); }\012"), .len = sizeof("static inline void __oc_print_const_u8_ptr(const uint8_t* s) { printf(\042%s\134n\042, (const char*)s); }\012") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("static inline void __oc_print_oc_string(oc_string s) { printf(\042%.*s\134n\042, (int)s.len, (const char*)s.ptr); }\012"), .len = sizeof("static inline void __oc_print_oc_string(oc_string s) { printf(\042%.*s\134n\042, (int)s.len, (const char*)s.ptr); }\012") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("static inline void __oc_print_bool(bool b) { printf(\042%s\134n\042, b \077 \042true\042 : \042false\042); }\012"), .len = sizeof("static inline void __oc_print_bool(bool b) { printf(\042%s\134n\042, b \077 \042true\042 : \042false\042); }\012") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("static inline void __oc_print_u64(uint64_t u) { printf(\042%llu\134n\042, (unsigned long long)u); }\012"), .len = sizeof("static inline void __oc_print_u64(uint64_t u) { printf(\042%llu\134n\042, (unsigned long long)u); }\012") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("static inline void __oc_print_i64(int64_t i) { printf(\042%lld\134n\042, (long long)i); }\012\012"), .len = sizeof("static inline void __oc_print_i64(int64_t i) { printf(\042%lld\134n\042, (long long)i); }\012\012") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("#define __oc_print(x) _Generic((x), \134\012"), .len = sizeof("#define __oc_print(x) _Generic((x), \134\012") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("    char*: __oc_print_str, \134\012"), .len = sizeof("    char*: __oc_print_str, \134\012") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("    const char*: __oc_print_str, \134\012"), .len = sizeof("    const char*: __oc_print_str, \134\012") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("    uint8_t*: __oc_print_u8_ptr, \134\012"), .len = sizeof("    uint8_t*: __oc_print_u8_ptr, \134\012") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("    const uint8_t*: __oc_print_const_u8_ptr, \134\012"), .len = sizeof("    const uint8_t*: __oc_print_const_u8_ptr, \134\012") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("    oc_string: __oc_print_oc_string, \134\012"), .len = sizeof("    oc_string: __oc_print_oc_string, \134\012") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("    bool: __oc_print_bool, \134\012"), .len = sizeof("    bool: __oc_print_bool, \134\012") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("    signed char: __oc_print_i64, \134\012"), .len = sizeof("    signed char: __oc_print_i64, \134\012") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("    short: __oc_print_i64, \134\012"), .len = sizeof("    short: __oc_print_i64, \134\012") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("    int: __oc_print_i64, \134\012"), .len = sizeof("    int: __oc_print_i64, \134\012") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("    long: __oc_print_i64, \134\012"), .len = sizeof("    long: __oc_print_i64, \134\012") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("    long long: __oc_print_i64, \134\012"), .len = sizeof("    long long: __oc_print_i64, \134\012") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("    default: __oc_print_u64 \134\012"), .len = sizeof("    default: __oc_print_u64 \134\012") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)(")(x)\012\012"), .len = sizeof(")(x)\012\012") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("static inline oc_string __oc_slice_str(oc_string s, size_t start, size_t end) {\012"), .len = sizeof("static inline oc_string __oc_slice_str(oc_string s, size_t start, size_t end) {\012") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("    if (start > end || end > s.len) { fprintf(stderr, \042panic: slice index out of bounds\134n\042); exit(1); }\012"), .len = sizeof("    if (start > end || end > s.len) { fprintf(stderr, \042panic: slice index out of bounds\134n\042); exit(1); }\012") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("    return (oc_string){ .ptr = s.ptr + start, .len = end - start };\012"), .len = sizeof("    return (oc_string){ .ptr = s.ptr + start, .len = end - start };\012") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("}\012\012"), .len = sizeof("}\012\012") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("static inline uint64_t f64_to_bits(double x) { uint64_t u; memcpy(&u, &x, 8); return u; }\012"), .len = sizeof("static inline uint64_t f64_to_bits(double x) { uint64_t u; memcpy(&u, &x, 8); return u; }\012") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("#define __OC_FCHK(x, loc) ({ __auto_type _r = (x); if (!(_r - _r == 0)) { fprintf(stderr, \042panic: floating-point result is NaN or infinite at %s\134n\042, loc); exit(1); } _r; })\012\012"), .len = sizeof("#define __OC_FCHK(x, loc) ({ __auto_type _r = (x); if (!(_r - _r == 0)) { fprintf(stderr, \042panic: floating-point result is NaN or infinite at %s\134n\042, loc); exit(1); } _r; })\012\012") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("#define __OC_ARR(a) ((oc_string){ .ptr = (const uint8_t*)(a), .len = __builtin_choose_expr(__builtin_types_compatible_p(__typeof__(a), __typeof__(&(a)[0])), (void)0, sizeof(a)) })\012\012"), .len = sizeof("#define __OC_ARR(a) ((oc_string){ .ptr = (const uint8_t*)(a), .len = __builtin_choose_expr(__builtin_types_compatible_p(__typeof__(a), __typeof__(&(a)[0])), (void)0, sizeof(a)) })\012\012") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("static inline uint8_t __oc_index_str(oc_string s, size_t idx) {\012"), .len = sizeof("static inline uint8_t __oc_index_str(oc_string s, size_t idx) {\012") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("    if (idx >= s.len) { fprintf(stderr, \042panic: slice index out of bounds\134n\042); exit(1); }\012"), .len = sizeof("    if (idx >= s.len) { fprintf(stderr, \042panic: slice index out of bounds\134n\042); exit(1); }\012") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("    return s.ptr[idx];\012"), .len = sizeof("    return s.ptr[idx];\012") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("}\012\012"), .len = sizeof("}\012\012") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("#define __OC_AT(a, i) (*__builtin_choose_expr(__builtin_types_compatible_p(__typeof__(a), __typeof__(&(a)[0])), &(a)[i], ({ size_t _i = (size_t)(i); if (_i >= (sizeof(a) + 0) / sizeof((a)[0])) { fprintf(stderr, \042panic: slice index out of bounds\134n\042); exit(1); } &(a)[_i]; })))\012\012"), .len = sizeof("#define __OC_AT(a, i) (*__builtin_choose_expr(__builtin_types_compatible_p(__typeof__(a), __typeof__(&(a)[0])), &(a)[i], ({ size_t _i = (size_t)(i); if (_i >= (sizeof(a) + 0) / sizeof((a)[0])) { fprintf(stderr, \042panic: slice index out of bounds\134n\042); exit(1); } &(a)[_i]; })))\012\012") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("static inline int64_t __oc_div_check(int64_t a, int64_t b) {\012"), .len = sizeof("static inline int64_t __oc_div_check(int64_t a, int64_t b) {\012") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("    if (b == 0) { fprintf(stderr, \042panic: division by zero\134n\042); exit(1); }\012"), .len = sizeof("    if (b == 0) { fprintf(stderr, \042panic: division by zero\134n\042); exit(1); }\012") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("    return a / b;\012"), .len = sizeof("    return a / b;\012") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("}\012\012"), .len = sizeof("}\012\012") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("static inline int64_t __oc_mod_check(int64_t a, int64_t b) {\012"), .len = sizeof("static inline int64_t __oc_mod_check(int64_t a, int64_t b) {\012") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("    if (b == 0) { fprintf(stderr, \042panic: division by zero\134n\042); exit(1); }\012"), .len = sizeof("    if (b == 0) { fprintf(stderr, \042panic: division by zero\134n\042); exit(1); }\012") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("    return a % b;\012"), .len = sizeof("    return a % b;\012") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("}\012\012"), .len = sizeof("}\012\012") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("#define __oc_notnull(p) ({ __auto_type _p = (p); if (!_p) { fprintf(stderr, \042panic: null pointer dereference\134n\042); exit(1); } _p; })\012"), .len = sizeof("#define __oc_notnull(p) ({ __auto_type _p = (p); if (!_p) { fprintf(stderr, \042panic: null pointer dereference\134n\042); exit(1); } _p; })\012") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("#define __OC_DIV(a, b) ({ __auto_type _a = (a); __auto_type _b = (b); if (_b == 0) { fprintf(stderr, \042panic: division by zero\134n\042); exit(1); } _a / _b; })\012"), .len = sizeof("#define __OC_DIV(a, b) ({ __auto_type _a = (a); __auto_type _b = (b); if (_b == 0) { fprintf(stderr, \042panic: division by zero\134n\042); exit(1); } _a / _b; })\012") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("#define __OC_MOD(a, b) ({ __auto_type _a = (a); __auto_type _b = (b); if (_b == 0) { fprintf(stderr, \042panic: division by zero\134n\042); exit(1); } _a % _b; })\012"), .len = sizeof("#define __OC_MOD(a, b) ({ __auto_type _a = (a); __auto_type _b = (b); if (_b == 0) { fprintf(stderr, \042panic: division by zero\134n\042); exit(1); } _a % _b; })\012") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("#define __OC_ADD(a, b) ({ __auto_type _a = (a); __auto_type _b = (b); __typeof__(__builtin_choose_expr(sizeof(_a) < 4, _a, __builtin_choose_expr(sizeof(_b) < 4, _b, _a + _b))) _r; if (__builtin_add_overflow(_a, _b, &_r)) { fprintf(stderr, \042panic: integer overflow\134n\042); exit(1); } _r; })\012"), .len = sizeof("#define __OC_ADD(a, b) ({ __auto_type _a = (a); __auto_type _b = (b); __typeof__(__builtin_choose_expr(sizeof(_a) < 4, _a, __builtin_choose_expr(sizeof(_b) < 4, _b, _a + _b))) _r; if (__builtin_add_overflow(_a, _b, &_r)) { fprintf(stderr, \042panic: integer overflow\134n\042); exit(1); } _r; })\012") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("#define __OC_SUB(a, b) ({ __auto_type _a = (a); __auto_type _b = (b); __typeof__(__builtin_choose_expr(sizeof(_a) < 4, _a, __builtin_choose_expr(sizeof(_b) < 4, _b, _a - _b))) _r; if (__builtin_sub_overflow(_a, _b, &_r)) { fprintf(stderr, \042panic: integer overflow\134n\042); exit(1); } _r; })\012"), .len = sizeof("#define __OC_SUB(a, b) ({ __auto_type _a = (a); __auto_type _b = (b); __typeof__(__builtin_choose_expr(sizeof(_a) < 4, _a, __builtin_choose_expr(sizeof(_b) < 4, _b, _a - _b))) _r; if (__builtin_sub_overflow(_a, _b, &_r)) { fprintf(stderr, \042panic: integer overflow\134n\042); exit(1); } _r; })\012") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("#define __OC_MUL(a, b) ({ __auto_type _a = (a); __auto_type _b = (b); __typeof__(__builtin_choose_expr(sizeof(_a) < 4, _a, __builtin_choose_expr(sizeof(_b) < 4, _b, _a * _b))) _r; if (__builtin_mul_overflow(_a, _b, &_r)) { fprintf(stderr, \042panic: integer overflow\134n\042); exit(1); } _r; })\012"), .len = sizeof("#define __OC_MUL(a, b) ({ __auto_type _a = (a); __auto_type _b = (b); __typeof__(__builtin_choose_expr(sizeof(_a) < 4, _a, __builtin_choose_expr(sizeof(_b) < 4, _b, _a * _b))) _r; if (__builtin_mul_overflow(_a, _b, &_r)) { fprintf(stderr, \042panic: integer overflow\134n\042); exit(1); } _r; })\012") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("#define __OC_WRAP(op, a, b) ({ __auto_type _a = (a); __auto_type _b = (b); __typeof__(__builtin_choose_expr(sizeof(_a) < 4, _a, __builtin_choose_expr(sizeof(_b) < 4, _b, _a + _b))) _r; (void)__builtin_##op##_overflow(_a, _b, &_r); _r; })\012\012"), .len = sizeof("#define __OC_WRAP(op, a, b) ({ __auto_type _a = (a); __auto_type _b = (b); __typeof__(__builtin_choose_expr(sizeof(_a) < 4, _a, __builtin_choose_expr(sizeof(_b) < 4, _b, _a + _b))) _r; (void)__builtin_##op##_overflow(_a, _b, &_r); _r; })\012\012") - 1 });
    __OC_AT((((out)->ptr_fields)), (0)) = (oc_string){ .ptr = (const uint8_t*)("ptr"), .len = sizeof("ptr") - 1 };
    ((out)->ptr_fields_count) = 1;
    AstNode* scan_decl = ((root)->left);
    while ((scan_decl != ((AstNode*)(0)))) {
    if ((((scan_decl)->tag) == NODE_STRUCT_DECL)) {
    AstNode* sf = ((scan_decl)->left);
    while ((sf != ((AstNode*)(0)))) {
    cg_flt_add(out, ((((sf)->tok)).slice), ((sf)->type_name));
    if (is_type_ptr(((sf)->type_name))) {
    if ((((out)->ptr_fields_count) < 256)) {
    __OC_AT((((out)->ptr_fields)), (((out)->ptr_fields_count))) = ((((sf)->tok)).slice);
    ((out)->ptr_fields_count) = __OC_ADD(((out)->ptr_fields_count), 1);
    }
    }
    else {
    if (is_type_str(((sf)->type_name))) {
    if ((((out)->str_fields_count) < 256)) {
    __OC_AT((((out)->str_fields)), (((out)->str_fields_count))) = ((((sf)->tok)).slice);
    ((out)->str_fields_count) = __OC_ADD(((out)->str_fields_count), 1);
    }
    }
    }
    sf = ((sf)->next);
    }
    }
    else {
    if ((((scan_decl)->tag) == NODE_LET)) {
    cg_flt_add(out, ((((scan_decl)->tok)).slice), ((scan_decl)->type_name));
    }
    else {
    if ((((scan_decl)->tag) == NODE_FN)) {
    cg_flt_add(out, ((((scan_decl)->tok)).slice), ((scan_decl)->type_name));
    if (is_type_ptr(((scan_decl)->type_name))) {
    if ((((out)->ptr_fns_count) < 256)) {
    __OC_AT((((out)->ptr_fns)), (((out)->ptr_fns_count))) = ((((scan_decl)->tok)).slice);
    ((out)->ptr_fns_count) = __OC_ADD(((out)->ptr_fns_count), 1);
    }
    }
    else {
    if (is_type_str(((scan_decl)->type_name))) {
    if ((((out)->str_fns_count) < 256)) {
    __OC_AT((((out)->str_fns)), (((out)->str_fns_count))) = ((((scan_decl)->tok)).slice);
    ((out)->str_fns_count) = __OC_ADD(((out)->str_fns_count), 1);
    }
    }
    }
    }
    }
    }
    scan_decl = ((scan_decl)->next);
    }
    AstNode* g_decl = ((root)->left);
    while ((g_decl != ((AstNode*)(0)))) {
    if ((((g_decl)->tag) == NODE_LET)) {
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("static const "), .len = sizeof("static const ") - 1 });
    if ((((((g_decl)->type_name)).len) > 0)) {
    emit_c_type(((g_decl)->type_name), out);
    }
    else {
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("uint32_t"), .len = sizeof("uint32_t") - 1 });
    }
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)(" "), .len = sizeof(" ") - 1 });
    buf_append_ident(out, ((((g_decl)->tok)).slice));
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)(" = "), .len = sizeof(" = ") - 1 });
    codegen_expr(((g_decl)->left), out);
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)(";\012"), .len = sizeof(";\012") - 1 });
    }
    g_decl = ((g_decl)->next);
    }
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("\012"), .len = sizeof("\012") - 1 });
    AstNode* s_decl = ((root)->left);
    while ((s_decl != ((AstNode*)(0)))) {
    if ((((s_decl)->tag) == NODE_STRUCT_DECL)) {
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("typedef struct "), .len = sizeof("typedef struct ") - 1 });
    buf_append_ident(out, ((((s_decl)->tok)).slice));
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)(" "), .len = sizeof(" ") - 1 });
    buf_append_ident(out, ((((s_decl)->tok)).slice));
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)(";\012"), .len = sizeof(";\012") - 1 });
    }
    s_decl = ((s_decl)->next);
    }
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("\012"), .len = sizeof("\012") - 1 });
    s_decl = ((root)->left);
    while ((s_decl != ((AstNode*)(0)))) {
    if ((((s_decl)->tag) == NODE_STRUCT_DECL)) {
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("struct "), .len = sizeof("struct ") - 1 });
    if (str_eq(((s_decl)->type_name), (oc_string){ .ptr = (const uint8_t*)("packed"), .len = sizeof("packed") - 1 })) {
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("__attribute__((packed)) "), .len = sizeof("__attribute__((packed)) ") - 1 });
    }
    buf_append_ident(out, ((((s_decl)->tok)).slice));
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)(" {\012"), .len = sizeof(" {\012") - 1 });
    AstNode* f = ((s_decl)->left);
    while ((f != ((AstNode*)(0)))) {
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("    "), .len = sizeof("    ") - 1 });
    bool is_arr = false;
    if ((((((f)->type_name)).len) > 0)) {
    if ((__oc_index_str(((f)->type_name), 0) == 91)) {
    is_arr = true;
    }
    }
    if (is_arr) {
    size_t close_idx = 1;
    while ((close_idx < ((((f)->type_name)).len))) {
    if ((__oc_index_str(((f)->type_name), close_idx) == 93)) {
    break;
    }
    close_idx = __OC_ADD(close_idx, 1);
    }
    oc_string sz_str = __oc_slice_str(((f)->type_name), 1, close_idx);
    oc_string elem_str = __oc_slice_str(((f)->type_name), __OC_ADD(close_idx, 1), ((((f)->type_name)).len));
    emit_c_type(elem_str, out);
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)(" "), .len = sizeof(" ") - 1 });
    buf_append_ident(out, ((((f)->tok)).slice));
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("["), .len = sizeof("[") - 1 });
    buf_append(out, sz_str);
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("];\012"), .len = sizeof("];\012") - 1 });
    }
    else {
    emit_c_type(((f)->type_name), out);
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)(" "), .len = sizeof(" ") - 1 });
    buf_append_ident(out, ((((f)->tok)).slice));
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)(";\012"), .len = sizeof(";\012") - 1 });
    }
    f = ((f)->next);
    }
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("};\012\012"), .len = sizeof("};\012\012") - 1 });
    }
    s_decl = ((s_decl)->next);
    }
    AstNode* decl = ((root)->left);
    while ((decl != ((AstNode*)(0)))) {
    if ((((decl)->tag) == NODE_FN)) {
    oc_string fn_name = ((((decl)->tok)).slice);
    if (is_builtin_c_fn(fn_name)) {
    decl = ((decl)->next);
    continue;
    }
    bool is_main = str_eq(fn_name, (oc_string){ .ptr = (const uint8_t*)("main"), .len = sizeof("main") - 1 });
    if (is_main) {
    if ((((decl)->left) == ((AstNode*)(0)))) {
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("int main(void);\012"), .len = sizeof("int main(void);\012") - 1 });
    }
    else {
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("int main(int argc, char** argv);\012"), .len = sizeof("int main(int argc, char** argv);\012") - 1 });
    }
    }
    else {
    emit_c_type(((decl)->type_name), out);
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)(" "), .len = sizeof(" ") - 1 });
    buf_append_ident(out, fn_name);
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("("), .len = sizeof("(") - 1 });
    AstNode* param = ((decl)->left);
    if ((param == ((AstNode*)(0)))) {
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("void"), .len = sizeof("void") - 1 });
    }
    else {
    while ((param != ((AstNode*)(0)))) {
    emit_c_type(((param)->type_name), out);
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)(" "), .len = sizeof(" ") - 1 });
    buf_append_ident(out, ((((param)->tok)).slice));
    if ((((param)->next) != ((AstNode*)(0)))) {
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)(", "), .len = sizeof(", ") - 1 });
    }
    param = ((param)->next);
    }
    }
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)(");\012"), .len = sizeof(");\012") - 1 });
    }
    }
    decl = ((decl)->next);
    }
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("\012"), .len = sizeof("\012") - 1 });
    decl = ((root)->left);
    while ((decl != ((AstNode*)(0)))) {
    if ((((decl)->tag) == NODE_FN)) {
    if ((((decl)->right) != ((AstNode*)(0)))) {
    oc_string fn_name = ((((decl)->tok)).slice);
    if (is_builtin_c_fn(fn_name)) {
    decl = ((decl)->next);
    continue;
    }
    ((out)->ptr_vars_count) = 0;
    ((out)->str_vars_count) = 0;
    ((out)->var_types_len) = 0;
    AstNode* p_scan = ((decl)->left);
    while ((p_scan != ((AstNode*)(0)))) {
    if ((((out)->var_types_len) < 512)) {
    __OC_AT((((out)->var_names)), (((out)->var_types_len))) = ((((p_scan)->tok)).slice);
    __OC_AT((((out)->var_types)), (((out)->var_types_len))) = ((p_scan)->type_name);
    ((out)->var_types_len) = __OC_ADD(((out)->var_types_len), 1);
    }
    if (is_type_ptr(((p_scan)->type_name))) {
    if ((((out)->ptr_vars_count) < 512)) {
    __OC_AT((((out)->ptr_vars)), (((out)->ptr_vars_count))) = ((((p_scan)->tok)).slice);
    ((out)->ptr_vars_count) = __OC_ADD(((out)->ptr_vars_count), 1);
    }
    }
    else {
    if (is_type_str(((p_scan)->type_name))) {
    if ((((out)->str_vars_count) < 512)) {
    __OC_AT((((out)->str_vars)), (((out)->str_vars_count))) = ((((p_scan)->tok)).slice);
    ((out)->str_vars_count) = __OC_ADD(((out)->str_vars_count), 1);
    }
    }
    }
    p_scan = ((p_scan)->next);
    }
    bool is_main = str_eq(fn_name, (oc_string){ .ptr = (const uint8_t*)("main"), .len = sizeof("main") - 1 });
    if (is_main) {
    if ((((decl)->left) == ((AstNode*)(0)))) {
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("int main(void) {\012"), .len = sizeof("int main(void) {\012") - 1 });
    }
    else {
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("int main(int argc, char** argv) {\012"), .len = sizeof("int main(int argc, char** argv) {\012") - 1 });
    }
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("    setvbuf(stdout, NULL, __OC_STDOUT_BUF, 0);\012"), .len = sizeof("    setvbuf(stdout, NULL, __OC_STDOUT_BUF, 0);\012") - 1 });
    AstNode* body_block = ((decl)->right);
    if ((body_block != ((AstNode*)(0)))) {
    AstNode* stmt = ((body_block)->left);
    while ((stmt != ((AstNode*)(0)))) {
    codegen_stmt(stmt, out);
    stmt = ((stmt)->next);
    }
    }
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("    return 0;\012"), .len = sizeof("    return 0;\012") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("}\012\012"), .len = sizeof("}\012\012") - 1 });
    }
    else {
    emit_c_type(((decl)->type_name), out);
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)(" "), .len = sizeof(" ") - 1 });
    buf_append_ident(out, fn_name);
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("("), .len = sizeof("(") - 1 });
    AstNode* param = ((decl)->left);
    if ((param == ((AstNode*)(0)))) {
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("void"), .len = sizeof("void") - 1 });
    }
    else {
    while ((param != ((AstNode*)(0)))) {
    emit_c_type(((param)->type_name), out);
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)(" "), .len = sizeof(" ") - 1 });
    buf_append_ident(out, ((((param)->tok)).slice));
    if ((((param)->next) != ((AstNode*)(0)))) {
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)(", "), .len = sizeof(", ") - 1 });
    }
    param = ((param)->next);
    }
    }
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)(") {\012"), .len = sizeof(") {\012") - 1 });
    AstNode* body_block = ((decl)->right);
    AstNode* stmt = ((body_block)->left);
    while ((stmt != ((AstNode*)(0)))) {
    codegen_stmt(stmt, out);
    stmt = ((stmt)->next);
    }
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("}\012\012"), .len = sizeof("}\012\012") - 1 });
    }
    }
    }
    decl = ((decl)->next);
    }
}

void elf_append_u16(CodeBuffer* buf, uint32_t v) {
    buf_append_u8(buf, ({ __auto_type _v = (__OC_MOD(v, 256)); if (_v < 0 || (uint64_t)_v > 255ULL) { fprintf(stderr, "panic: invalid unsigned cast\n"); exit(1); } (uint8_t)_v; }));
    buf_append_u8(buf, ({ __auto_type _v = (__OC_MOD(__OC_DIV(v, 256), 256)); if (_v < 0 || (uint64_t)_v > 255ULL) { fprintf(stderr, "panic: invalid unsigned cast\n"); exit(1); } (uint8_t)_v; }));
}

void elf_append_u32(CodeBuffer* buf, uint32_t v) {
    buf_append_u8(buf, ({ __auto_type _v = (__OC_MOD(v, 256)); if (_v < 0 || (uint64_t)_v > 255ULL) { fprintf(stderr, "panic: invalid unsigned cast\n"); exit(1); } (uint8_t)_v; }));
    buf_append_u8(buf, ({ __auto_type _v = (__OC_MOD(__OC_DIV(v, 256), 256)); if (_v < 0 || (uint64_t)_v > 255ULL) { fprintf(stderr, "panic: invalid unsigned cast\n"); exit(1); } (uint8_t)_v; }));
    buf_append_u8(buf, ({ __auto_type _v = (__OC_MOD(__OC_DIV(v, 65536), 256)); if (_v < 0 || (uint64_t)_v > 255ULL) { fprintf(stderr, "panic: invalid unsigned cast\n"); exit(1); } (uint8_t)_v; }));
    buf_append_u8(buf, ({ __auto_type _v = (__OC_MOD(__OC_DIV(v, 16777216), 256)); if (_v < 0 || (uint64_t)_v > 255ULL) { fprintf(stderr, "panic: invalid unsigned cast\n"); exit(1); } (uint8_t)_v; }));
}

void elf_append_u64(CodeBuffer* buf, uint64_t v) {
    uint32_t low = ({ __auto_type _v = (__OC_MOD(v, 4294967296)); if (_v < 0 || (uint64_t)_v > 4294967295ULL) { fprintf(stderr, "panic: invalid unsigned cast\n"); exit(1); } (uint32_t)_v; });
    uint32_t high = ({ __auto_type _v = (__OC_DIV(v, 4294967296)); if (_v < 0 || (uint64_t)_v > 4294967295ULL) { fprintf(stderr, "panic: invalid unsigned cast\n"); exit(1); } (uint32_t)_v; });
    elf_append_u32(buf, low);
    elf_append_u32(buf, high);
}

void elf_patch_u32(CodeBuffer* buf, size_t offset, uint32_t v) {
    __OC_AT((((buf)->buffer)), (offset)) = ({ __auto_type _v = (__OC_MOD(v, 256)); if (_v < 0 || (uint64_t)_v > 255ULL) { fprintf(stderr, "panic: invalid unsigned cast\n"); exit(1); } (uint8_t)_v; });
    __OC_AT((((buf)->buffer)), (__OC_ADD(offset, 1))) = ({ __auto_type _v = (__OC_MOD(__OC_DIV(v, 256), 256)); if (_v < 0 || (uint64_t)_v > 255ULL) { fprintf(stderr, "panic: invalid unsigned cast\n"); exit(1); } (uint8_t)_v; });
    __OC_AT((((buf)->buffer)), (__OC_ADD(offset, 2))) = ({ __auto_type _v = (__OC_MOD(__OC_DIV(v, 65536), 256)); if (_v < 0 || (uint64_t)_v > 255ULL) { fprintf(stderr, "panic: invalid unsigned cast\n"); exit(1); } (uint8_t)_v; });
    __OC_AT((((buf)->buffer)), (__OC_ADD(offset, 3))) = ({ __auto_type _v = (__OC_MOD(__OC_DIV(v, 16777216), 256)); if (_v < 0 || (uint64_t)_v > 255ULL) { fprintf(stderr, "panic: invalid unsigned cast\n"); exit(1); } (uint8_t)_v; });
}

void elf_patch_u64(CodeBuffer* buf, size_t offset, uint64_t v) {
    uint32_t low = ({ __auto_type _v = (__OC_MOD(v, 4294967296)); if (_v < 0 || (uint64_t)_v > 4294967295ULL) { fprintf(stderr, "panic: invalid unsigned cast\n"); exit(1); } (uint32_t)_v; });
    uint32_t high = ({ __auto_type _v = (__OC_DIV(v, 4294967296)); if (_v < 0 || (uint64_t)_v > 4294967295ULL) { fprintf(stderr, "panic: invalid unsigned cast\n"); exit(1); } (uint32_t)_v; });
    elf_patch_u32(buf, offset, low);
    elf_patch_u32(buf, __OC_ADD(offset, 4), high);
}

void elf_append_i32(CodeBuffer* buf, int32_t v) {
    uint64_t uv = 0;
    if ((v < 0)) {
    int64_t neg = __OC_SUB(0, ({ __auto_type _v = (v); if (!(_v < 0) && (uint64_t)_v > 9223372036854775807ULL) { fprintf(stderr, "panic: invalid signed cast\n"); exit(1); } (int64_t)_v; }));
    uv = __OC_SUB(4294967296, ({ __auto_type _v = (neg); if (_v < 0) { fprintf(stderr, "panic: invalid unsigned cast\n"); exit(1); } (uint64_t)_v; }));
    }
    else {
    uv = ({ __auto_type _v = (v); if (_v < 0) { fprintf(stderr, "panic: invalid unsigned cast\n"); exit(1); } (uint64_t)_v; });
    }
    elf_append_u32(buf, ({ __auto_type _v = (uv); if (_v < 0 || (uint64_t)_v > 4294967295ULL) { fprintf(stderr, "panic: invalid unsigned cast\n"); exit(1); } (uint32_t)_v; }));
}

void elf_patch_i32(CodeBuffer* buf, size_t offset, int32_t v) {
    uint64_t uv = 0;
    if ((v < 0)) {
    int64_t neg = __OC_SUB(0, ({ __auto_type _v = (v); if (!(_v < 0) && (uint64_t)_v > 9223372036854775807ULL) { fprintf(stderr, "panic: invalid signed cast\n"); exit(1); } (int64_t)_v; }));
    uv = __OC_SUB(4294967296, ({ __auto_type _v = (neg); if (_v < 0) { fprintf(stderr, "panic: invalid unsigned cast\n"); exit(1); } (uint64_t)_v; }));
    }
    else {
    uv = ({ __auto_type _v = (v); if (_v < 0) { fprintf(stderr, "panic: invalid unsigned cast\n"); exit(1); } (uint64_t)_v; });
    }
    elf_patch_u32(buf, offset, ({ __auto_type _v = (uv); if (_v < 0 || (uint64_t)_v > 4294967295ULL) { fprintf(stderr, "panic: invalid unsigned cast\n"); exit(1); } (uint32_t)_v; }));
}

oc_string ng_imp_name(size_t slot) {
    if ((slot == 0)) {
    return (oc_string){ .ptr = (const uint8_t*)("ExitProcess"), .len = sizeof("ExitProcess") - 1 };
    }
    if ((slot == 1)) {
    return (oc_string){ .ptr = (const uint8_t*)("GetStdHandle"), .len = sizeof("GetStdHandle") - 1 };
    }
    if ((slot == 2)) {
    return (oc_string){ .ptr = (const uint8_t*)("WriteFile"), .len = sizeof("WriteFile") - 1 };
    }
    if ((slot == 3)) {
    return (oc_string){ .ptr = (const uint8_t*)("ReadFile"), .len = sizeof("ReadFile") - 1 };
    }
    if ((slot == 4)) {
    return (oc_string){ .ptr = (const uint8_t*)("CreateFileA"), .len = sizeof("CreateFileA") - 1 };
    }
    if ((slot == 5)) {
    return (oc_string){ .ptr = (const uint8_t*)("CloseHandle"), .len = sizeof("CloseHandle") - 1 };
    }
    if ((slot == 6)) {
    return (oc_string){ .ptr = (const uint8_t*)("DeleteFileA"), .len = sizeof("DeleteFileA") - 1 };
    }
    if ((slot == 7)) {
    return (oc_string){ .ptr = (const uint8_t*)("VirtualAlloc"), .len = sizeof("VirtualAlloc") - 1 };
    }
    if ((slot == 8)) {
    return (oc_string){ .ptr = (const uint8_t*)("VirtualFree"), .len = sizeof("VirtualFree") - 1 };
    }
    return (oc_string){ .ptr = (const uint8_t*)("VirtualProtect"), .len = sizeof("VirtualProtect") - 1 };
}

int32_t ng_imp_slot(oc_string name) {
    size_t k = 0;
    while ((k < NG_IMP_COUNT)) {
    if (str_eq(ng_imp_name(k), name)) {
    return ({ __auto_type _v = (k); if ((_v < 0 && (int64_t)_v < -2147483648LL) || (!(_v < 0) && (uint64_t)_v > 2147483647ULL)) { fprintf(stderr, "panic: invalid signed cast\n"); exit(1); } (int32_t)_v; });
    }
    k = __OC_ADD(k, 1);
    }
    return (-1);
}

void ng_fail(NativeGen* ng, oc_string msg) {
    if ((!((ng)->has_error))) {
    __oc_print(msg);
    }
    ((ng)->has_error) = true;
}

void ng_add_fixup(NativeGen* ng, size_t code_off, oc_string fn_name, bool is_6byte) {
    if ((((ng)->fixup_count) < 1024)) {
    __OC_AT((((ng)->fixup_offsets)), (((ng)->fixup_count))) = code_off;
    __OC_AT((((ng)->fixup_names)), (((ng)->fixup_count))) = fn_name;
    __OC_AT((((ng)->fixup_is_6byte)), (((ng)->fixup_count))) = is_6byte;
    ((ng)->fixup_count) = __OC_ADD(((ng)->fixup_count), 1);
    }
    else {
    ng_fail(ng, (oc_string){ .ptr = (const uint8_t*)("native codegen error: call fixup table full (1024)"), .len = sizeof("native codegen error: call fixup table full (1024)") - 1 });
    }
}

void ng_add_data_reloc(NativeGen* ng, size_t code_off, size_t data_off) {
    if ((((ng)->data_reloc_count) < 4096)) {
    __OC_AT((((ng)->data_reloc_code)), (((ng)->data_reloc_count))) = code_off;
    __OC_AT((((ng)->data_reloc_data)), (((ng)->data_reloc_count))) = data_off;
    ((ng)->data_reloc_count) = __OC_ADD(((ng)->data_reloc_count), 1);
    }
    else {
    ng_fail(ng, (oc_string){ .ptr = (const uint8_t*)("native codegen error: data relocation table full (4096)"), .len = sizeof("native codegen error: data relocation table full (4096)") - 1 });
    }
}

void ng_add_iat_fixup(NativeGen* ng, size_t code_off, size_t slot) {
    if ((((ng)->iat_fix_count) < 64)) {
    __OC_AT((((ng)->iat_fix_code)), (((ng)->iat_fix_count))) = code_off;
    __OC_AT((((ng)->iat_fix_slot)), (((ng)->iat_fix_count))) = slot;
    ((ng)->iat_fix_count) = __OC_ADD(((ng)->iat_fix_count), 1);
    }
    else {
    ng_fail(ng, (oc_string){ .ptr = (const uint8_t*)("native codegen error: import fixup table full (64)"), .len = sizeof("native codegen error: import fixup table full (64)") - 1 });
    }
}

void ng_emit_call(NativeGen* ng, CodeBuffer* code, oc_string fn_name) {
    ng_add_fixup(ng, ((code)->len), fn_name, false);
    buf_append_u8(code, 232);
    elf_append_u32(code, 0);
}

void ng_emit_iat_call(NativeGen* ng, CodeBuffer* code, size_t slot) {
    buf_append_u8(code, 255);
    buf_append_u8(code, 21);
    ng_add_iat_fixup(ng, ((code)->len), slot);
    elf_append_u32(code, 0);
}

uint64_t parse_literal_u64(oc_string s) {
    uint64_t res = 0;
    if ((((s).len) > 2)) {
    if ((__oc_index_str(s, 0) == 48)) {
    bool is_hex = false;
    if ((__oc_index_str(s, 1) == 120)) {
    is_hex = true;
    }
    if ((__oc_index_str(s, 1) == 88)) {
    is_hex = true;
    }
    if (is_hex) {
    size_t i = 2;
    while ((i < ((s).len))) {
    uint8_t c = __oc_index_str(s, i);
    if ((c >= 48)) {
    if ((c <= 57)) {
    res = __OC_ADD(__OC_MUL(res, 16), ({ __auto_type _v = (__OC_SUB(c, 48)); if (_v < 0) { fprintf(stderr, "panic: invalid unsigned cast\n"); exit(1); } (uint64_t)_v; }));
    }
    else {
    if ((c >= 97)) {
    if ((c <= 102)) {
    res = __OC_ADD(__OC_MUL(res, 16), ({ __auto_type _v = (__OC_SUB(c, 87)); if (_v < 0) { fprintf(stderr, "panic: invalid unsigned cast\n"); exit(1); } (uint64_t)_v; }));
    }
    }
    else {
    if ((c >= 65)) {
    if ((c <= 70)) {
    res = __OC_ADD(__OC_MUL(res, 16), ({ __auto_type _v = (__OC_SUB(c, 55)); if (_v < 0) { fprintf(stderr, "panic: invalid unsigned cast\n"); exit(1); } (uint64_t)_v; }));
    }
    }
    }
    }
    }
    i = __OC_ADD(i, 1);
    }
    return res;
    }
    bool is_bin = false;
    if ((__oc_index_str(s, 1) == 98)) {
    is_bin = true;
    }
    if ((__oc_index_str(s, 1) == 66)) {
    is_bin = true;
    }
    if (is_bin) {
    size_t i = 2;
    while ((i < ((s).len))) {
    uint8_t c = __oc_index_str(s, i);
    if ((c == 48)) {
    res = __OC_MUL(res, 2);
    }
    else {
    if ((c == 49)) {
    res = __OC_ADD(__OC_MUL(res, 2), 1);
    }
    }
    i = __OC_ADD(i, 1);
    }
    return res;
    }
    }
    }
    size_t i = 0;
    while ((i < ((s).len))) {
    uint8_t c = __oc_index_str(s, i);
    if ((c >= 48)) {
    if ((c <= 57)) {
    res = __OC_ADD(__OC_MUL(res, 10), ({ __auto_type _v = (__OC_SUB(c, 48)); if (_v < 0) { fprintf(stderr, "panic: invalid unsigned cast\n"); exit(1); } (uint64_t)_v; }));
    }
    }
    i = __OC_ADD(i, 1);
    }
    return res;
}

int32_t ng_find_var_idx(NativeGen* ng, oc_string name) {
    size_t i = ((ng)->var_count);
    while ((i > 0)) {
    i = __OC_SUB(i, 1);
    if (__OC_AT((((ng)->var_active)), (i))) {
    if (str_eq(__OC_AT((((ng)->var_names)), (i)), name)) {
    return ({ __auto_type _v = (i); if ((_v < 0 && (int64_t)_v < -2147483648LL) || (!(_v < 0) && (uint64_t)_v > 2147483647ULL)) { fprintf(stderr, "panic: invalid signed cast\n"); exit(1); } (int32_t)_v; });
    }
    }
    }
    return (-1);
}

int32_t ng_find_var_offset(NativeGen* ng, oc_string name) {
    int32_t idx = ng_find_var_idx(ng, name);
    if ((idx >= 0)) {
    return __OC_AT((((ng)->var_offsets)), (({ __auto_type _v = (idx); if (_v < 0) { fprintf(stderr, "panic: invalid unsigned cast\n"); exit(1); } (size_t)_v; })));
    }
    return 0;
}

oc_string ng_find_var_type(NativeGen* ng, oc_string name) {
    int32_t idx = ng_find_var_idx(ng, name);
    if ((idx >= 0)) {
    return __OC_AT((((ng)->var_types)), (({ __auto_type _v = (idx); if (_v < 0) { fprintf(stderr, "panic: invalid unsigned cast\n"); exit(1); } (size_t)_v; })));
    }
    return (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 };
}

bool ng_is_var_slice(NativeGen* ng, oc_string name) {
    int32_t idx = ng_find_var_idx(ng, name);
    if ((idx >= 0)) {
    return __OC_AT((((ng)->var_is_slice)), (({ __auto_type _v = (idx); if (_v < 0) { fprintf(stderr, "panic: invalid unsigned cast\n"); exit(1); } (size_t)_v; })));
    }
    return false;
}

int32_t ng_struct_raw_size(NativeGen* ng, oc_string name) {
    size_t i = 0;
    while ((i < ((ng)->struct_count))) {
    if (str_eq(__OC_AT((((ng)->struct_names)), (i)), name)) {
    return __OC_AT((((ng)->struct_sizes)), (i));
    }
    i = __OC_ADD(i, 1);
    }
    return 0;
}

int32_t ng_find_struct_size(NativeGen* ng, oc_string name) {
    return __OC_MUL(__OC_DIV(__OC_ADD(ng_struct_raw_size(ng, name), 7), 8), 8);
}

int32_t ng_find_field_idx(NativeGen* ng, oc_string s_name, oc_string f_name) {
    size_t i = 0;
    while ((i < ((ng)->field_count))) {
    if (str_eq(__OC_AT((((ng)->field_struct)), (i)), s_name)) {
    if (str_eq(__OC_AT((((ng)->field_names)), (i)), f_name)) {
    return ({ __auto_type _v = (i); if ((_v < 0 && (int64_t)_v < -2147483648LL) || (!(_v < 0) && (uint64_t)_v > 2147483647ULL)) { fprintf(stderr, "panic: invalid signed cast\n"); exit(1); } (int32_t)_v; });
    }
    }
    i = __OC_ADD(i, 1);
    }
    i = 0;
    while ((i < ((ng)->field_count))) {
    if (str_eq(__OC_AT((((ng)->field_names)), (i)), f_name)) {
    return ({ __auto_type _v = (i); if ((_v < 0 && (int64_t)_v < -2147483648LL) || (!(_v < 0) && (uint64_t)_v > 2147483647ULL)) { fprintf(stderr, "panic: invalid signed cast\n"); exit(1); } (int32_t)_v; });
    }
    i = __OC_ADD(i, 1);
    }
    return (-1);
}

int32_t ng_find_field_offset(NativeGen* ng, oc_string s_name, oc_string f_name) {
    int32_t idx = ng_find_field_idx(ng, s_name, f_name);
    if ((idx < 0)) {
    return 0;
    }
    return __OC_AT((((ng)->field_offsets)), (({ __auto_type _v = (idx); if (_v < 0) { fprintf(stderr, "panic: invalid unsigned cast\n"); exit(1); } (size_t)_v; })));
}

oc_string ng_find_field_type(NativeGen* ng, oc_string s_name, oc_string f_name) {
    if (str_eq(f_name, (oc_string){ .ptr = (const uint8_t*)("len"), .len = sizeof("len") - 1 })) {
    return (oc_string){ .ptr = (const uint8_t*)("usize"), .len = sizeof("usize") - 1 };
    }
    if (str_eq(f_name, (oc_string){ .ptr = (const uint8_t*)("ptr"), .len = sizeof("ptr") - 1 })) {
    return (oc_string){ .ptr = (const uint8_t*)("*u8"), .len = sizeof("*u8") - 1 };
    }
    int32_t idx = ng_find_field_idx(ng, s_name, f_name);
    if ((idx < 0)) {
    return (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 };
    }
    return __OC_AT((((ng)->field_types)), (({ __auto_type _v = (idx); if (_v < 0) { fprintf(stderr, "panic: invalid unsigned cast\n"); exit(1); } (size_t)_v; })));
}

int32_t ng_scalar_width(oc_string t) {
    if (str_eq(t, (oc_string){ .ptr = (const uint8_t*)("u8"), .len = sizeof("u8") - 1 })) {
    return 1;
    }
    if (str_eq(t, (oc_string){ .ptr = (const uint8_t*)("i8"), .len = sizeof("i8") - 1 })) {
    return 1;
    }
    if (str_eq(t, (oc_string){ .ptr = (const uint8_t*)("bool"), .len = sizeof("bool") - 1 })) {
    return 1;
    }
    if (str_eq(t, (oc_string){ .ptr = (const uint8_t*)("u16"), .len = sizeof("u16") - 1 })) {
    return 2;
    }
    if (str_eq(t, (oc_string){ .ptr = (const uint8_t*)("i16"), .len = sizeof("i16") - 1 })) {
    return 2;
    }
    if (str_eq(t, (oc_string){ .ptr = (const uint8_t*)("u32"), .len = sizeof("u32") - 1 })) {
    return 4;
    }
    if (str_eq(t, (oc_string){ .ptr = (const uint8_t*)("i32"), .len = sizeof("i32") - 1 })) {
    return 4;
    }
    if (str_eq(t, (oc_string){ .ptr = (const uint8_t*)("f32"), .len = sizeof("f32") - 1 })) {
    return 4;
    }
    return 8;
}

int32_t ng_field_width(NativeGen* ng, oc_string s_name, oc_string f_name) {
    int32_t idx = ng_find_field_idx(ng, s_name, f_name);
    if ((idx < 0)) {
    return 8;
    }
    int32_t w = ng_scalar_width(__OC_AT((((ng)->field_types)), (({ __auto_type _v = (idx); if (_v < 0) { fprintf(stderr, "panic: invalid unsigned cast\n"); exit(1); } (size_t)_v; }))));
    if (__OC_AT((((ng)->field_packed)), (({ __auto_type _v = (idx); if (_v < 0) { fprintf(stderr, "panic: invalid unsigned cast\n"); exit(1); } (size_t)_v; })))) {
    return w;
    }
    if ((w == 1)) {
    return 1;
    }
    return 8;
}

oc_string ng_find_fn_ret_type(NativeGen* ng, oc_string fn_name) {
    size_t i = 0;
    while ((i < ((ng)->fn_count))) {
    if (str_eq(__OC_AT((((ng)->fn_names)), (i)), fn_name)) {
    return __OC_AT((((ng)->fn_ret_types)), (i));
    }
    i = __OC_ADD(i, 1);
    }
    return (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 };
}

oc_string ng_strip_ptr_type(oc_string t) {
    if ((((t).len) == 0)) {
    return (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 };
    }
    bool is_p = false;
    if ((__oc_index_str(t, 0) == 42)) {
    is_p = true;
    }
    if ((__oc_index_str(t, 0) == 38)) {
    is_p = true;
    }
    if (is_p) {
    size_t start = 1;
    if ((((t).len) > 4)) {
    if ((__oc_index_str(t, 1) == 109)) {
    if ((__oc_index_str(t, 2) == 117)) {
    if ((__oc_index_str(t, 3) == 116)) {
    start = 4;
    while ((start < ((t).len))) {
    if ((__oc_index_str(t, start) == 32)) {
    start = __OC_ADD(start, 1);
    }
    else {
    break;
    }
    }
    }
    }
    }
    }
    return __oc_slice_str(t, start, ((t).len));
    }
    return t;
}

size_t ng_parse_array_len(oc_string t) {
    if ((((t).len) < 3)) {
    return 0;
    }
    if ((__oc_index_str(t, 0) != 91)) {
    return 0;
    }
    size_t close_idx = 1;
    while ((close_idx < ((t).len))) {
    if ((__oc_index_str(t, close_idx) == 93)) {
    break;
    }
    close_idx = __OC_ADD(close_idx, 1);
    }
    if ((close_idx >= ((t).len))) {
    return 0;
    }
    return ({ __auto_type _v = (parse_literal_u64(__oc_slice_str(t, 1, close_idx))); if (_v < 0) { fprintf(stderr, "panic: invalid unsigned cast\n"); exit(1); } (size_t)_v; });
}

oc_string ng_parse_array_elem(oc_string t) {
    if ((((t).len) < 3)) {
    return (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 };
    }
    if ((__oc_index_str(t, 0) != 91)) {
    return (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 };
    }
    size_t close_idx = 1;
    while ((close_idx < ((t).len))) {
    if ((__oc_index_str(t, close_idx) == 93)) {
    break;
    }
    close_idx = __OC_ADD(close_idx, 1);
    }
    if ((__OC_ADD(close_idx, 1) >= ((t).len))) {
    return (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 };
    }
    return __oc_slice_str(t, __OC_ADD(close_idx, 1), ((t).len));
}

int32_t ng_type_stride(oc_string t) {
    if (str_eq(t, (oc_string){ .ptr = (const uint8_t*)("u8"), .len = sizeof("u8") - 1 })) {
    return 1;
    }
    if (str_eq(t, (oc_string){ .ptr = (const uint8_t*)("i8"), .len = sizeof("i8") - 1 })) {
    return 1;
    }
    if (str_eq(t, (oc_string){ .ptr = (const uint8_t*)("bool"), .len = sizeof("bool") - 1 })) {
    return 1;
    }
    if (str_eq(t, (oc_string){ .ptr = (const uint8_t*)("string"), .len = sizeof("string") - 1 })) {
    return 16;
    }
    return 8;
}

bool ng_is_byte_type(oc_string t) {
    if (str_eq(t, (oc_string){ .ptr = (const uint8_t*)("u8"), .len = sizeof("u8") - 1 })) {
    return true;
    }
    if (str_eq(t, (oc_string){ .ptr = (const uint8_t*)("bool"), .len = sizeof("bool") - 1 })) {
    return true;
    }
    return false;
}

bool is_unsigned_type_name(oc_string t) {
    if ((((t).len) == 0)) {
    return false;
    }
    if (str_eq(t, (oc_string){ .ptr = (const uint8_t*)("u8"), .len = sizeof("u8") - 1 })) {
    return true;
    }
    if (str_eq(t, (oc_string){ .ptr = (const uint8_t*)("u16"), .len = sizeof("u16") - 1 })) {
    return true;
    }
    if (str_eq(t, (oc_string){ .ptr = (const uint8_t*)("u32"), .len = sizeof("u32") - 1 })) {
    return true;
    }
    if (str_eq(t, (oc_string){ .ptr = (const uint8_t*)("u64"), .len = sizeof("u64") - 1 })) {
    return true;
    }
    if (str_eq(t, (oc_string){ .ptr = (const uint8_t*)("usize"), .len = sizeof("usize") - 1 })) {
    return true;
    }
    if ((__oc_index_str(t, 0) == 42)) {
    return true;
    }
    if ((__oc_index_str(t, 0) == 38)) {
    return true;
    }
    return false;
}

oc_string ng_float_result_type(oc_string lt, oc_string rt) {
    if (str_eq(lt, (oc_string){ .ptr = (const uint8_t*)("f64"), .len = sizeof("f64") - 1 })) {
    return lt;
    }
    if (str_eq(rt, (oc_string){ .ptr = (const uint8_t*)("f64"), .len = sizeof("f64") - 1 })) {
    return rt;
    }
    if (str_eq(lt, (oc_string){ .ptr = (const uint8_t*)("f32"), .len = sizeof("f32") - 1 })) {
    return lt;
    }
    if (str_eq(rt, (oc_string){ .ptr = (const uint8_t*)("f32"), .len = sizeof("f32") - 1 })) {
    return rt;
    }
    return (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 };
}

int32_t ng_int_class(oc_string t) {
    if ((((t).len) == 0)) {
    return 0;
    }
    if (str_eq(t, (oc_string){ .ptr = (const uint8_t*)("u64"), .len = sizeof("u64") - 1 })) {
    return 3;
    }
    if (str_eq(t, (oc_string){ .ptr = (const uint8_t*)("usize"), .len = sizeof("usize") - 1 })) {
    return 3;
    }
    if ((__oc_index_str(t, 0) == 42)) {
    return 3;
    }
    if ((__oc_index_str(t, 0) == 38)) {
    return 3;
    }
    if (str_eq(t, (oc_string){ .ptr = (const uint8_t*)("i64"), .len = sizeof("i64") - 1 })) {
    return 2;
    }
    if (str_eq(t, (oc_string){ .ptr = (const uint8_t*)("u32"), .len = sizeof("u32") - 1 })) {
    return 1;
    }
    return 0;
}

oc_string ng_expr_type(NativeGen* ng, AstNode* expr) {
    if ((expr == ((AstNode*)(0)))) {
    return (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 };
    }
    if ((((expr)->tag) == NODE_STRING)) {
    return (oc_string){ .ptr = (const uint8_t*)("string"), .len = sizeof("string") - 1 };
    }
    if ((((expr)->tag) == NODE_SLICE)) {
    return (oc_string){ .ptr = (const uint8_t*)("string"), .len = sizeof("string") - 1 };
    }
    if ((((expr)->tag) == NODE_BOOL)) {
    return (oc_string){ .ptr = (const uint8_t*)("bool"), .len = sizeof("bool") - 1 };
    }
    if ((((expr)->tag) == NODE_CAST)) {
    return ((expr)->type_name);
    }
    if ((((expr)->tag) == NODE_STRUCT_INIT)) {
    return ((((expr)->tok)).slice);
    }
    if ((((expr)->tag) == NODE_FLOAT)) {
    if ((((((expr)->type_name)).len) > 0)) {
    return ((expr)->type_name);
    }
    return (oc_string){ .ptr = (const uint8_t*)("f64"), .len = sizeof("f64") - 1 };
    }
    if ((((expr)->tag) == NODE_IDENT)) {
    if ((ng_find_var_idx(ng, ((((expr)->tok)).slice)) >= 0)) {
    return ng_find_var_type(ng, ((((expr)->tok)).slice));
    }
    size_t ci = 0;
    while ((ci < ((ng)->const_count))) {
    if (str_eq(__OC_AT((((ng)->const_names)), (ci)), ((((expr)->tok)).slice))) {
    return __OC_AT((((ng)->const_types)), (ci));
    }
    ci = __OC_ADD(ci, 1);
    }
    return (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 };
    }
    if ((((expr)->tag) == NODE_CALL)) {
    if (str_eq(((((expr)->tok)).slice), (oc_string){ .ptr = (const uint8_t*)("sqrt_f64"), .len = sizeof("sqrt_f64") - 1 })) {
    return (oc_string){ .ptr = (const uint8_t*)("f64"), .len = sizeof("f64") - 1 };
    }
    if (str_eq(((((expr)->tok)).slice), (oc_string){ .ptr = (const uint8_t*)("f64_to_bits"), .len = sizeof("f64_to_bits") - 1 })) {
    return (oc_string){ .ptr = (const uint8_t*)("u64"), .len = sizeof("u64") - 1 };
    }
    if ((((wrapping_op(((((expr)->tok)).slice))).len) > 0)) {
    oc_string lt = (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 };
    oc_string rt = (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 };
    if ((((expr)->left) != ((AstNode*)(0)))) {
    lt = ng_expr_type(ng, ((expr)->left));
    rt = ng_expr_type(ng, ((((expr)->left))->next));
    }
    if ((ng_scalar_width(lt) < 4)) {
    return lt;
    }
    if ((ng_scalar_width(rt) < 4)) {
    return rt;
    }
    if ((((lt).len) == 0)) {
    return rt;
    }
    if ((((rt).len) == 0)) {
    return lt;
    }
    if ((ng_scalar_width(lt) == 8)) {
    if (is_unsigned_type_name(lt)) {
    return lt;
    }
    if ((ng_scalar_width(rt) == 8)) {
    return rt;
    }
    return lt;
    }
    if ((ng_scalar_width(rt) == 8)) {
    return rt;
    }
    if (is_unsigned_type_name(lt)) {
    return lt;
    }
    return rt;
    }
    return ng_find_fn_ret_type(ng, ((((expr)->tok)).slice));
    }
    if ((((expr)->tag) == NODE_ADDRESS_OF)) {
    oc_string inner = ng_expr_type(ng, ((expr)->left));
    if ((((inner).len) > 0)) {
    if ((__oc_index_str(inner, 0) == 91)) {
    oc_string el = ng_parse_array_elem(inner);
    if (str_eq(el, (oc_string){ .ptr = (const uint8_t*)("u8"), .len = sizeof("u8") - 1 })) {
    return (oc_string){ .ptr = (const uint8_t*)("*mut u8"), .len = sizeof("*mut u8") - 1 };
    }
    }
    }
    return (oc_string){ .ptr = (const uint8_t*)("*u8"), .len = sizeof("*u8") - 1 };
    }
    if ((((expr)->tag) == NODE_DEREF)) {
    oc_string pt = ng_expr_type(ng, ((expr)->left));
    return ng_strip_ptr_type(pt);
    }
    if ((((expr)->tag) == NODE_FIELD_ACCESS)) {
    oc_string bt = ng_strip_ptr_type(ng_expr_type(ng, ((expr)->left)));
    return ng_find_field_type(ng, bt, ((((expr)->tok)).slice));
    }
    if ((((expr)->tag) == NODE_INDEX)) {
    oc_string lt = ng_expr_type(ng, ((expr)->left));
    if (str_eq(lt, (oc_string){ .ptr = (const uint8_t*)("string"), .len = sizeof("string") - 1 })) {
    return (oc_string){ .ptr = (const uint8_t*)("u8"), .len = sizeof("u8") - 1 };
    }
    if ((((lt).len) > 0)) {
    if ((__oc_index_str(lt, 0) == 91)) {
    return ng_parse_array_elem(lt);
    }
    if ((__oc_index_str(lt, 0) == 42)) {
    return ng_strip_ptr_type(lt);
    }
    if ((__oc_index_str(lt, 0) == 38)) {
    return ng_strip_ptr_type(lt);
    }
    }
    return (oc_string){ .ptr = (const uint8_t*)("i64"), .len = sizeof("i64") - 1 };
    }
    if ((((expr)->tag) == NODE_UNARY)) {
    if ((((((expr)->tok)).tag) == TOK_MINUS)) {
    oc_string neg_t = ng_expr_type(ng, ((expr)->left));
    if ((((neg_t).len) > 0)) {
    return neg_t;
    }
    return (oc_string){ .ptr = (const uint8_t*)("i64"), .len = sizeof("i64") - 1 };
    }
    if ((((((expr)->tok)).tag) == TOK_BANG)) {
    return (oc_string){ .ptr = (const uint8_t*)("bool"), .len = sizeof("bool") - 1 };
    }
    return ng_expr_type(ng, ((expr)->left));
    }
    if ((((expr)->tag) == NODE_BINARY)) {
    if (is_comparison_tok(((((expr)->tok)).tag))) {
    return (oc_string){ .ptr = (const uint8_t*)("bool"), .len = sizeof("bool") - 1 };
    }
    oc_string lt = ng_expr_type(ng, ((expr)->left));
    oc_string rt = ng_expr_type(ng, ((expr)->right));
    oc_string float_t = ng_float_result_type(lt, rt);
    if ((((float_t).len) > 0)) {
    return float_t;
    }
    bool is_shift = (((((expr)->tok)).tag) == TOK_SHL);
    if ((((((expr)->tok)).tag) == TOK_SHR)) {
    is_shift = true;
    }
    if ((!is_shift)) {
    int32_t cl = ng_int_class(lt);
    int32_t cr = ng_int_class(rt);
    if ((cr > cl)) {
    return rt;
    }
    }
    if ((((lt).len) > 0)) {
    return lt;
    }
    return rt;
    }
    return (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 };
}

bool ng_is_unsigned_expr(NativeGen* ng, AstNode* expr) {
    if ((expr == ((AstNode*)(0)))) {
    return false;
    }
    if ((((expr)->tag) == NODE_UNARY)) {
    if ((((((expr)->tok)).tag) == TOK_MINUS)) {
    return false;
    }
    }
    if ((((expr)->tag) == NODE_BINARY)) {
    if (is_comparison_tok(((((expr)->tok)).tag))) {
    return false;
    }
    bool is_shift = (((((expr)->tok)).tag) == TOK_SHL);
    if ((((((expr)->tok)).tag) == TOK_SHR)) {
    is_shift = true;
    }
    if (is_shift) {
    return ng_is_unsigned_expr(ng, ((expr)->left));
    }
    return ng_binary_unsigned(ng, ((expr)->left), ((expr)->right));
    }
    return is_unsigned_type_name(ng_expr_type(ng, expr));
}

bool ng_binary_unsigned(NativeGen* ng, AstNode* left, AstNode* right) {
    int32_t cl = ng_int_class(ng_expr_type(ng, left));
    int32_t cr = ng_int_class(ng_expr_type(ng, right));
    if ((cl == 3)) {
    return true;
    }
    if ((cr == 3)) {
    return true;
    }
    if ((cl == 2)) {
    return false;
    }
    if ((cr == 2)) {
    return false;
    }
    if ((cl == 1)) {
    return true;
    }
    return (cr == 1);
}

bool ng_is_expr_slice(NativeGen* ng, AstNode* expr) {
    if ((expr == ((AstNode*)(0)))) {
    return false;
    }
    if ((((expr)->tag) == NODE_STRING)) {
    return true;
    }
    if ((((expr)->tag) == NODE_SLICE)) {
    return true;
    }
    if ((((expr)->tag) == NODE_IDENT)) {
    return ng_is_var_slice(ng, ((((expr)->tok)).slice));
    }
    oc_string t = ng_expr_type(ng, expr);
    return str_eq(t, (oc_string){ .ptr = (const uint8_t*)("string"), .len = sizeof("string") - 1 });
}

void ng_emit_disp_store_rax(CodeBuffer* code, int32_t off) {
    buf_append_u8(code, 72);
    buf_append_u8(code, 137);
    buf_append_u8(code, 133);
    elf_append_i32(code, off);
}

void ng_emit_disp_store_rdx(CodeBuffer* code, int32_t off) {
    buf_append_u8(code, 72);
    buf_append_u8(code, 137);
    buf_append_u8(code, 149);
    elf_append_i32(code, off);
}

void ng_emit_disp_store_rcx(CodeBuffer* code, int32_t off) {
    buf_append_u8(code, 72);
    buf_append_u8(code, 137);
    buf_append_u8(code, 141);
    elf_append_i32(code, off);
}

void ng_emit_disp_store_r8(CodeBuffer* code, int32_t off) {
    buf_append_u8(code, 76);
    buf_append_u8(code, 137);
    buf_append_u8(code, 133);
    elf_append_i32(code, off);
}

void ng_emit_disp_load_rax(CodeBuffer* code, int32_t off) {
    buf_append_u8(code, 72);
    buf_append_u8(code, 139);
    buf_append_u8(code, 133);
    elf_append_i32(code, off);
}

void ng_emit_disp_load_rdx(CodeBuffer* code, int32_t off) {
    buf_append_u8(code, 72);
    buf_append_u8(code, 139);
    buf_append_u8(code, 149);
    elf_append_i32(code, off);
}

void ng_emit_disp_load_rcx(CodeBuffer* code, int32_t off) {
    buf_append_u8(code, 72);
    buf_append_u8(code, 139);
    buf_append_u8(code, 141);
    elf_append_i32(code, off);
}

void ng_emit_disp_load_r8(CodeBuffer* code, int32_t off) {
    buf_append_u8(code, 76);
    buf_append_u8(code, 139);
    buf_append_u8(code, 133);
    elf_append_i32(code, off);
}

void ng_emit_disp_lea_rax(CodeBuffer* code, int32_t off) {
    buf_append_u8(code, 72);
    buf_append_u8(code, 141);
    buf_append_u8(code, 133);
    elf_append_i32(code, off);
}

size_t ng_append_unescaped_str(CodeBuffer* data, oc_string raw) {
    size_t k = 0;
    size_t emitted = 0;
    while ((k < ((raw).len))) {
    LitByte lb = lit_next_byte(raw, k);
    buf_append_u8(data, ((lb).b));
    emitted = __OC_ADD(emitted, 1);
    k = ((lb).next);
    }
    return emitted;
}

void ng_emit_range_check(NativeGen* ng, CodeBuffer* code, oc_string t, oc_string trap) {
    if ((((t).len) == 0)) {
    return;
    }
    if (str_eq(t, (oc_string){ .ptr = (const uint8_t*)("u8"), .len = sizeof("u8") - 1 })) {
    buf_append_u8(code, 72);
    buf_append_u8(code, 61);
    elf_append_u32(code, 255);
    ng_add_fixup(ng, ((code)->len), trap, true);
    buf_append_u8(code, 15);
    buf_append_u8(code, 135);
    elf_append_u32(code, 0);
    return;
    }
    if (str_eq(t, (oc_string){ .ptr = (const uint8_t*)("u16"), .len = sizeof("u16") - 1 })) {
    buf_append_u8(code, 72);
    buf_append_u8(code, 61);
    elf_append_u32(code, 65535);
    ng_add_fixup(ng, ((code)->len), trap, true);
    buf_append_u8(code, 15);
    buf_append_u8(code, 135);
    elf_append_u32(code, 0);
    return;
    }
    if (str_eq(t, (oc_string){ .ptr = (const uint8_t*)("u32"), .len = sizeof("u32") - 1 })) {
    buf_append_u8(code, 72);
    buf_append_u8(code, 185);
    elf_append_u64(code, 4294967295);
    buf_append_u8(code, 72);
    buf_append_u8(code, 57);
    buf_append_u8(code, 200);
    ng_add_fixup(ng, ((code)->len), trap, true);
    buf_append_u8(code, 15);
    buf_append_u8(code, 135);
    elf_append_u32(code, 0);
    return;
    }
    if (str_eq(t, (oc_string){ .ptr = (const uint8_t*)("i8"), .len = sizeof("i8") - 1 })) {
    buf_append_u8(code, 72);
    buf_append_u8(code, 61);
    elf_append_i32(code, (-128));
    ng_add_fixup(ng, ((code)->len), trap, true);
    buf_append_u8(code, 15);
    buf_append_u8(code, 140);
    elf_append_u32(code, 0);
    buf_append_u8(code, 72);
    buf_append_u8(code, 61);
    elf_append_u32(code, 127);
    ng_add_fixup(ng, ((code)->len), trap, true);
    buf_append_u8(code, 15);
    buf_append_u8(code, 143);
    elf_append_u32(code, 0);
    return;
    }
    if (str_eq(t, (oc_string){ .ptr = (const uint8_t*)("i16"), .len = sizeof("i16") - 1 })) {
    buf_append_u8(code, 72);
    buf_append_u8(code, 61);
    elf_append_i32(code, (-32768));
    ng_add_fixup(ng, ((code)->len), trap, true);
    buf_append_u8(code, 15);
    buf_append_u8(code, 140);
    elf_append_u32(code, 0);
    buf_append_u8(code, 72);
    buf_append_u8(code, 61);
    elf_append_u32(code, 32767);
    ng_add_fixup(ng, ((code)->len), trap, true);
    buf_append_u8(code, 15);
    buf_append_u8(code, 143);
    elf_append_u32(code, 0);
    return;
    }
    if (str_eq(t, (oc_string){ .ptr = (const uint8_t*)("i32"), .len = sizeof("i32") - 1 })) {
    buf_append_u8(code, 72);
    buf_append_u8(code, 185);
    elf_append_u32(code, 2147483648);
    elf_append_u32(code, 4294967295);
    buf_append_u8(code, 72);
    buf_append_u8(code, 57);
    buf_append_u8(code, 200);
    ng_add_fixup(ng, ((code)->len), trap, true);
    buf_append_u8(code, 15);
    buf_append_u8(code, 140);
    elf_append_u32(code, 0);
    buf_append_u8(code, 72);
    buf_append_u8(code, 185);
    elf_append_u64(code, 2147483647);
    buf_append_u8(code, 72);
    buf_append_u8(code, 57);
    buf_append_u8(code, 200);
    ng_add_fixup(ng, ((code)->len), trap, true);
    buf_append_u8(code, 15);
    buf_append_u8(code, 143);
    elf_append_u32(code, 0);
    return;
    }
}

void ng_emit_load_at_rax(CodeBuffer* code, oc_string t, int32_t width) {
    bool is_signed = false;
    if (str_eq(t, (oc_string){ .ptr = (const uint8_t*)("i8"), .len = sizeof("i8") - 1 })) {
    is_signed = true;
    }
    if (str_eq(t, (oc_string){ .ptr = (const uint8_t*)("i16"), .len = sizeof("i16") - 1 })) {
    is_signed = true;
    }
    if (str_eq(t, (oc_string){ .ptr = (const uint8_t*)("i32"), .len = sizeof("i32") - 1 })) {
    is_signed = true;
    }
    if ((width == 1)) {
    if (is_signed) {
    buf_append_u8(code, 72);
    buf_append_u8(code, 15);
    buf_append_u8(code, 190);
    buf_append_u8(code, 0);
    }
    else {
    buf_append_u8(code, 15);
    buf_append_u8(code, 182);
    buf_append_u8(code, 0);
    }
    }
    else {
    if ((width == 2)) {
    if (is_signed) {
    buf_append_u8(code, 72);
    buf_append_u8(code, 15);
    buf_append_u8(code, 191);
    buf_append_u8(code, 0);
    }
    else {
    buf_append_u8(code, 15);
    buf_append_u8(code, 183);
    buf_append_u8(code, 0);
    }
    }
    else {
    if ((width == 4)) {
    if (is_signed) {
    buf_append_u8(code, 72);
    buf_append_u8(code, 99);
    buf_append_u8(code, 0);
    }
    else {
    buf_append_u8(code, 139);
    buf_append_u8(code, 0);
    }
    }
    else {
    buf_append_u8(code, 72);
    buf_append_u8(code, 139);
    buf_append_u8(code, 0);
    }
    }
    }
}

int32_t ng_value_words(NativeGen* ng, oc_string t) {
    if (str_eq(t, (oc_string){ .ptr = (const uint8_t*)("string"), .len = sizeof("string") - 1 })) {
    return 2;
    }
    int32_t s_sz = ng_find_struct_size(ng, t);
    if ((s_sz > 8)) {
    return __OC_DIV(s_sz, 8);
    }
    return 1;
}

bool ng_emit_load_struct_at_rax(NativeGen* ng, CodeBuffer* code, oc_string s_name) {
    int32_t size = ng_find_struct_size(ng, s_name);
    if ((size <= 8)) {
    return false;
    }
    if ((size > 32)) {
    ng_fail(ng, (oc_string){ .ptr = (const uint8_t*)("native codegen error: a struct larger than 32 bytes cannot be read by value; use a pointer to it"), .len = sizeof("native codegen error: a struct larger than 32 bytes cannot be read by value; use a pointer to it") - 1 });
    }
    else {
    if ((size != ng_struct_raw_size(ng, s_name))) {
    ng_fail(ng, (oc_string){ .ptr = (const uint8_t*)("native codegen error: a packed struct whose size is not a multiple of 8 cannot be read by value; use a pointer to it"), .len = sizeof("native codegen error: a packed struct whose size is not a multiple of 8 cannot be read by value; use a pointer to it") - 1 });
    }
    }
    buf_append_u8(code, 72);
    buf_append_u8(code, 139);
    buf_append_u8(code, 80);
    buf_append_u8(code, 8);
    if ((size >= 24)) {
    buf_append_u8(code, 72);
    buf_append_u8(code, 139);
    buf_append_u8(code, 72);
    buf_append_u8(code, 16);
    }
    if ((size >= 32)) {
    buf_append_u8(code, 76);
    buf_append_u8(code, 139);
    buf_append_u8(code, 64);
    buf_append_u8(code, 24);
    }
    buf_append_u8(code, 72);
    buf_append_u8(code, 139);
    buf_append_u8(code, 0);
    return true;
}

void ng_emit_truncate(CodeBuffer* code, oc_string t) {
    if (str_eq(t, (oc_string){ .ptr = (const uint8_t*)("u8"), .len = sizeof("u8") - 1 })) {
    buf_append_u8(code, 15);
    buf_append_u8(code, 182);
    buf_append_u8(code, 192);
    }
    else {
    if (str_eq(t, (oc_string){ .ptr = (const uint8_t*)("i8"), .len = sizeof("i8") - 1 })) {
    buf_append_u8(code, 72);
    buf_append_u8(code, 15);
    buf_append_u8(code, 190);
    buf_append_u8(code, 192);
    }
    else {
    if (str_eq(t, (oc_string){ .ptr = (const uint8_t*)("u16"), .len = sizeof("u16") - 1 })) {
    buf_append_u8(code, 15);
    buf_append_u8(code, 183);
    buf_append_u8(code, 192);
    }
    else {
    if (str_eq(t, (oc_string){ .ptr = (const uint8_t*)("i16"), .len = sizeof("i16") - 1 })) {
    buf_append_u8(code, 72);
    buf_append_u8(code, 15);
    buf_append_u8(code, 191);
    buf_append_u8(code, 192);
    }
    else {
    if (str_eq(t, (oc_string){ .ptr = (const uint8_t*)("u32"), .len = sizeof("u32") - 1 })) {
    buf_append_u8(code, 137);
    buf_append_u8(code, 192);
    }
    else {
    if (str_eq(t, (oc_string){ .ptr = (const uint8_t*)("i32"), .len = sizeof("i32") - 1 })) {
    buf_append_u8(code, 72);
    buf_append_u8(code, 99);
    buf_append_u8(code, 192);
    }
    }
    }
    }
    }
    }
}

void ng_emit_store_at_rcx(CodeBuffer* code, int32_t width) {
    if ((width == 1)) {
    buf_append_u8(code, 136);
    buf_append_u8(code, 1);
    }
    else {
    if ((width == 2)) {
    buf_append_u8(code, 102);
    buf_append_u8(code, 137);
    buf_append_u8(code, 1);
    }
    else {
    if ((width == 4)) {
    buf_append_u8(code, 137);
    buf_append_u8(code, 1);
    }
    else {
    buf_append_u8(code, 72);
    buf_append_u8(code, 137);
    buf_append_u8(code, 1);
    }
    }
    }
}

uint64_t ng_literal_bits(NativeGen* ng, oc_string text, bool single) {
    uint8_t buf[64] = {0};
    if ((((text).len) > 63)) {
    ng_fail(ng, (oc_string){ .ptr = (const uint8_t*)("native codegen error: float literal longer than 63 characters"), .len = sizeof("native codegen error: float literal longer than 63 characters") - 1 });
    return 0;
    }
    size_t i = 0;
    while ((i < ((text).len))) {
    __OC_AT((buf), (i)) = __oc_index_str(text, i);
    i = __OC_ADD(i, 1);
    }
    uint64_t bits = 0;
    if (single) {
    bits = sys_f32_bits(((uint8_t*)((&(buf)))));
    }
    else {
    bits = sys_f64_bits(((uint8_t*)((&(buf)))));
    }
    return bits;
}

void ng_emit_rax_to_xmm0(CodeBuffer* code, bool wide) {
    buf_append_u8(code, 102);
    if (wide) {
    buf_append_u8(code, 72);
    }
    buf_append_u8(code, 15);
    buf_append_u8(code, 110);
    buf_append_u8(code, 192);
}

void ng_emit_rbx_to_xmm1(CodeBuffer* code, bool wide) {
    buf_append_u8(code, 102);
    if (wide) {
    buf_append_u8(code, 72);
    }
    buf_append_u8(code, 15);
    buf_append_u8(code, 110);
    buf_append_u8(code, 203);
}

void ng_emit_xmm0_to_rax(CodeBuffer* code, bool wide) {
    buf_append_u8(code, 102);
    if (wide) {
    buf_append_u8(code, 72);
    }
    buf_append_u8(code, 15);
    buf_append_u8(code, 126);
    buf_append_u8(code, 192);
}

uint8_t ng_sse_prefix(bool wide) {
    if (wide) {
    return 242;
    }
    return 243;
}

void ng_emit_convert(NativeGen* ng, CodeBuffer* code, oc_string from_t, oc_string to_t) {
    if ((!is_float_type(to_t))) {
    return;
    }
    if (str_eq(from_t, to_t)) {
    return;
    }
    bool wide = str_eq(to_t, (oc_string){ .ptr = (const uint8_t*)("f64"), .len = sizeof("f64") - 1 });
    if (is_float_type(from_t)) {
    ng_emit_rax_to_xmm0(code, (!wide));
    buf_append_u8(code, ng_sse_prefix((!wide)));
    buf_append_u8(code, 15);
    buf_append_u8(code, 90);
    buf_append_u8(code, 192);
    ng_emit_xmm0_to_rax(code, wide);
    return;
    }
    if ((ng_int_class(from_t) == 3)) {
    buf_append_u8(code, 72);
    buf_append_u8(code, 133);
    buf_append_u8(code, 192);
    buf_append_u8(code, 120);
    buf_append_u8(code, 7);
    buf_append_u8(code, ng_sse_prefix(wide));
    buf_append_u8(code, 72);
    buf_append_u8(code, 15);
    buf_append_u8(code, 42);
    buf_append_u8(code, 192);
    buf_append_u8(code, 235);
    buf_append_u8(code, 21);
    buf_append_u8(code, 72);
    buf_append_u8(code, 137);
    buf_append_u8(code, 193);
    buf_append_u8(code, 72);
    buf_append_u8(code, 209);
    buf_append_u8(code, 232);
    buf_append_u8(code, 131);
    buf_append_u8(code, 225);
    buf_append_u8(code, 1);
    buf_append_u8(code, 72);
    buf_append_u8(code, 9);
    buf_append_u8(code, 200);
    buf_append_u8(code, ng_sse_prefix(wide));
    buf_append_u8(code, 72);
    buf_append_u8(code, 15);
    buf_append_u8(code, 42);
    buf_append_u8(code, 192);
    buf_append_u8(code, ng_sse_prefix(wide));
    buf_append_u8(code, 15);
    buf_append_u8(code, 88);
    buf_append_u8(code, 192);
    }
    else {
    buf_append_u8(code, ng_sse_prefix(wide));
    buf_append_u8(code, 72);
    buf_append_u8(code, 15);
    buf_append_u8(code, 42);
    buf_append_u8(code, 192);
    }
    ng_emit_xmm0_to_rax(code, wide);
}

uint64_t ng_float_cast_lo(oc_string t) {
    if (str_eq(t, (oc_string){ .ptr = (const uint8_t*)("i8"), .len = sizeof("i8") - 1 })) {
    return 13862114837418475520ULL;
    }
    if (str_eq(t, (oc_string){ .ptr = (const uint8_t*)("i16"), .len = sizeof("i16") - 1 })) {
    return 13898108587504304128ULL;
    }
    if (str_eq(t, (oc_string){ .ptr = (const uint8_t*)("i32"), .len = sizeof("i32") - 1 })) {
    return 13970166044105375744ULL;
    }
    if (str_eq(t, (oc_string){ .ptr = (const uint8_t*)("i64"), .len = sizeof("i64") - 1 })) {
    return 14114281232179134465ULL;
    }
    return 13830554455654793216ULL;
}

uint64_t ng_float_cast_hi(oc_string t) {
    if (str_eq(t, (oc_string){ .ptr = (const uint8_t*)("i8"), .len = sizeof("i8") - 1 })) {
    return 4638707616191610880;
    }
    if (str_eq(t, (oc_string){ .ptr = (const uint8_t*)("i16"), .len = sizeof("i16") - 1 })) {
    return 4674736413210574848;
    }
    if (str_eq(t, (oc_string){ .ptr = (const uint8_t*)("i32"), .len = sizeof("i32") - 1 })) {
    return 4746794007248502784;
    }
    if (str_eq(t, (oc_string){ .ptr = (const uint8_t*)("i64"), .len = sizeof("i64") - 1 })) {
    return 4890909195324358656;
    }
    if (str_eq(t, (oc_string){ .ptr = (const uint8_t*)("u8"), .len = sizeof("u8") - 1 })) {
    return 4643211215818981376;
    }
    if (str_eq(t, (oc_string){ .ptr = (const uint8_t*)("u16"), .len = sizeof("u16") - 1 })) {
    return 4679240012837945344;
    }
    if (str_eq(t, (oc_string){ .ptr = (const uint8_t*)("u32"), .len = sizeof("u32") - 1 })) {
    return 4751297606875873280;
    }
    return 4895412794951729152;
}

void ng_emit_xmm1_const(CodeBuffer* code, uint64_t bits) {
    buf_append_u8(code, 72);
    buf_append_u8(code, 185);
    elf_append_u64(code, bits);
    buf_append_u8(code, 102);
    buf_append_u8(code, 72);
    buf_append_u8(code, 15);
    buf_append_u8(code, 110);
    buf_append_u8(code, 201);
}

void ng_emit_float_to_int(NativeGen* ng, CodeBuffer* code, oc_string src, oc_string dst) {
    oc_string trap = (oc_string){ .ptr = (const uint8_t*)("__trap_cast_signed"), .len = sizeof("__trap_cast_signed") - 1 };
    if (is_unsigned_type_name(dst)) {
    trap = (oc_string){ .ptr = (const uint8_t*)("__trap_cast_unsigned"), .len = sizeof("__trap_cast_unsigned") - 1 };
    }
    if (str_eq(src, (oc_string){ .ptr = (const uint8_t*)("f32"), .len = sizeof("f32") - 1 })) {
    ng_emit_rax_to_xmm0(code, false);
    buf_append_u8(code, 243);
    buf_append_u8(code, 15);
    buf_append_u8(code, 90);
    buf_append_u8(code, 192);
    }
    else {
    ng_emit_rax_to_xmm0(code, true);
    }
    ng_emit_xmm1_const(code, ng_float_cast_lo(dst));
    buf_append_u8(code, 102);
    buf_append_u8(code, 15);
    buf_append_u8(code, 46);
    buf_append_u8(code, 193);
    ng_add_fixup(ng, ((code)->len), trap, true);
    buf_append_u8(code, 15);
    buf_append_u8(code, 134);
    elf_append_u32(code, 0);
    ng_emit_xmm1_const(code, ng_float_cast_hi(dst));
    buf_append_u8(code, 102);
    buf_append_u8(code, 15);
    buf_append_u8(code, 46);
    buf_append_u8(code, 200);
    ng_add_fixup(ng, ((code)->len), trap, true);
    buf_append_u8(code, 15);
    buf_append_u8(code, 134);
    elf_append_u32(code, 0);
    if ((ng_int_class(dst) == 3)) {
    ng_emit_xmm1_const(code, 4890909195324358656);
    buf_append_u8(code, 102);
    buf_append_u8(code, 15);
    buf_append_u8(code, 46);
    buf_append_u8(code, 193);
    buf_append_u8(code, 114);
    buf_append_u8(code, 16);
    buf_append_u8(code, 242);
    buf_append_u8(code, 15);
    buf_append_u8(code, 92);
    buf_append_u8(code, 193);
    buf_append_u8(code, 242);
    buf_append_u8(code, 72);
    buf_append_u8(code, 15);
    buf_append_u8(code, 44);
    buf_append_u8(code, 192);
    buf_append_u8(code, 72);
    buf_append_u8(code, 15);
    buf_append_u8(code, 186);
    buf_append_u8(code, 232);
    buf_append_u8(code, 63);
    buf_append_u8(code, 235);
    buf_append_u8(code, 5);
    }
    buf_append_u8(code, 242);
    buf_append_u8(code, 72);
    buf_append_u8(code, 15);
    buf_append_u8(code, 44);
    buf_append_u8(code, 192);
}

void ng_emit_fp_check(NativeGen* ng, CodeBuffer* code, CodeBuffer* data, bool wide, Token tok) {
    if ((!((ng)->fp_trap))) {
    return;
    }
    if (wide) {
    buf_append_u8(code, 72);
    buf_append_u8(code, 137);
    buf_append_u8(code, 193);
    buf_append_u8(code, 72);
    buf_append_u8(code, 209);
    buf_append_u8(code, 225);
    buf_append_u8(code, 72);
    buf_append_u8(code, 186);
    elf_append_u64(code, 0xffe0000000000000);
    buf_append_u8(code, 72);
    buf_append_u8(code, 57);
    buf_append_u8(code, 209);
    }
    else {
    buf_append_u8(code, 137);
    buf_append_u8(code, 193);
    buf_append_u8(code, 209);
    buf_append_u8(code, 225);
    buf_append_u8(code, 129);
    buf_append_u8(code, 249);
    elf_append_u32(code, 0xff000000);
    }
    buf_append_u8(code, 114);
    buf_append_u8(code, 20);
    size_t loc_off = ((data)->len);
    buf_append_src_loc(data, ((ng)->srcmap), tok, false);
    buf_append_u8(code, 72);
    buf_append_u8(code, 190);
    size_t loc_patch = ((code)->len);
    elf_append_u64(code, 0);
    ng_add_data_reloc(ng, loc_patch, loc_off);
    buf_append_u8(code, 186);
    elf_append_u32(code, ({ __auto_type _v = (__OC_SUB(((data)->len), loc_off)); if (_v < 0 || (uint64_t)_v > 4294967295ULL) { fprintf(stderr, "panic: invalid unsigned cast\n"); exit(1); } (uint32_t)_v; }));
    ng_add_fixup(ng, ((code)->len), (oc_string){ .ptr = (const uint8_t*)("__trap_fp"), .len = sizeof("__trap_fp") - 1 }, false);
    buf_append_u8(code, 233);
    elf_append_u32(code, 0);
}

void ng_emit_stderr_write(NativeGen* ng, CodeBuffer* code, bool is_win) {
    buf_append_u8(code, 72);
    buf_append_u8(code, 199);
    buf_append_u8(code, 199);
    elf_append_u32(code, 2);
    if (is_win) {
    ng_emit_call(ng, code, (oc_string){ .ptr = (const uint8_t*)("__win_write"), .len = sizeof("__win_write") - 1 });
    }
    else {
    buf_append_u8(code, 72);
    buf_append_u8(code, 199);
    buf_append_u8(code, 192);
    elf_append_u32(code, 1);
    buf_append_u8(code, 15);
    buf_append_u8(code, 5);
    }
}

void ng_emit_store_disp(CodeBuffer* code, uint8_t reg, int32_t width, int32_t disp, bool at_rsp) {
    if ((width == 1)) {
    buf_append_u8(code, 136);
    }
    else {
    buf_append_u8(code, 72);
    buf_append_u8(code, 137);
    }
    if (at_rsp) {
    buf_append_u8(code, __OC_ADD(132, __OC_MUL(reg, 8)));
    buf_append_u8(code, 36);
    }
    else {
    buf_append_u8(code, __OC_ADD(133, __OC_MUL(reg, 8)));
    }
    elf_append_i32(code, disp);
}

void ng_emit_array_init_at(NativeGen* ng, CodeBuffer* code, CodeBuffer* data, int32_t off, oc_string arr_t, AstNode* init, bool at_rsp) {
    int32_t alen = ({ __auto_type _v = (ng_parse_array_len(arr_t)); if ((_v < 0 && (int64_t)_v < -2147483648LL) || (!(_v < 0) && (uint64_t)_v > 2147483647ULL)) { fprintf(stderr, "panic: invalid signed cast\n"); exit(1); } (int32_t)_v; });
    oc_string elem_t = ng_parse_array_elem(arr_t);
    int32_t stride = ng_type_stride(elem_t);
    int32_t bytes = __OC_MUL(alen, stride);
    buf_append_u8(code, 72);
    buf_append_u8(code, 49);
    buf_append_u8(code, 192);
    int32_t z = 0;
    while ((__OC_ADD(z, 8) <= bytes)) {
    ng_emit_store_disp(code, 0, 8, __OC_ADD(off, z), at_rsp);
    z = __OC_ADD(z, 8);
    }
    while ((z < bytes)) {
    ng_emit_store_disp(code, 0, 1, __OC_ADD(off, z), at_rsp);
    z = __OC_ADD(z, 1);
    }
    AstNode* el = ((init)->left);
    int32_t k = 0;
    while ((el != ((AstNode*)(0)))) {
    if ((k >= alen)) {
    break;
    }
    ng_emit_expr_as(ng, code, data, el, elem_t);
    int32_t el_off = __OC_ADD(off, __OC_MUL(k, stride));
    if ((stride == 1)) {
    ng_emit_store_disp(code, 0, 1, el_off, at_rsp);
    }
    else {
    ng_emit_store_disp(code, 0, 8, el_off, at_rsp);
    if ((stride == 16)) {
    ng_emit_store_disp(code, 2, 8, __OC_ADD(el_off, 8), at_rsp);
    }
    }
    k = __OC_ADD(k, 1);
    el = ((el)->next);
    }
}

oc_string ng_fn_param_type(NativeGen* ng, oc_string fn_name, size_t idx) {
    AstNode* decl = ((((ng)->root))->left);
    while ((decl != ((AstNode*)(0)))) {
    if ((((decl)->tag) == NODE_FN)) {
    if (str_eq(((((decl)->tok)).slice), fn_name)) {
    AstNode* prm = ((decl)->left);
    size_t i = 0;
    while ((prm != ((AstNode*)(0)))) {
    if ((i == idx)) {
    return ((prm)->type_name);
    }
    prm = ((prm)->next);
    i = __OC_ADD(i, 1);
    }
    return (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 };
    }
    }
    decl = ((decl)->next);
    }
    return (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 };
}

bool ng_is_place(NativeGen* ng, AstNode* e) {
    if ((e == ((AstNode*)(0)))) {
    return false;
    }
    if ((((e)->tag) == NODE_IDENT)) {
    return true;
    }
    if ((((e)->tag) == NODE_DEREF)) {
    return true;
    }
    if ((((e)->tag) == NODE_INDEX)) {
    return true;
    }
    if ((((e)->tag) == NODE_FIELD_ACCESS)) {
    oc_string t = ng_expr_type(ng, ((e)->left));
    if ((((t).len) > 0)) {
    if ((__oc_index_str(t, 0) == 42)) {
    return true;
    }
    if ((__oc_index_str(t, 0) == 38)) {
    return true;
    }
    }
    return ng_is_place(ng, ((e)->left));
    }
    return false;
}

void ng_emit_expr_as(NativeGen* ng, CodeBuffer* code, CodeBuffer* data, AstNode* expr, oc_string to_t) {
    ng_emit_expr(ng, code, data, expr);
    ng_emit_convert(ng, code, ng_expr_type(ng, expr), to_t);
}

void ng_emit_lvalue_addr(NativeGen* ng, CodeBuffer* code, CodeBuffer* data, AstNode* expr) {
    if ((expr == ((AstNode*)(0)))) {
    return;
    }
    if ((((expr)->tag) == NODE_IDENT)) {
    int32_t off = ng_find_var_offset(ng, ((((expr)->tok)).slice));
    ng_emit_disp_lea_rax(code, off);
    return;
    }
    if ((((expr)->tag) == NODE_DEREF)) {
    ng_emit_expr(ng, code, data, ((expr)->left));
    buf_append_u8(code, 72);
    buf_append_u8(code, 133);
    buf_append_u8(code, 192);
    ng_add_fixup(ng, ((code)->len), (oc_string){ .ptr = (const uint8_t*)("__trap_null_deref"), .len = sizeof("__trap_null_deref") - 1 }, true);
    buf_append_u8(code, 15);
    buf_append_u8(code, 132);
    elf_append_u32(code, 0);
    return;
    }
    if ((((expr)->tag) == NODE_FIELD_ACCESS)) {
    oc_string bt = ng_expr_type(ng, ((expr)->left));
    oc_string st = ng_strip_ptr_type(bt);
    int32_t f_off = ng_find_field_offset(ng, st, ((((expr)->tok)).slice));
    bool is_ptr_base = false;
    if ((((bt).len) > 0)) {
    if ((__oc_index_str(bt, 0) == 42)) {
    is_ptr_base = true;
    }
    if ((__oc_index_str(bt, 0) == 38)) {
    is_ptr_base = true;
    }
    }
    if (is_ptr_base) {
    ng_emit_expr(ng, code, data, ((expr)->left));
    buf_append_u8(code, 72);
    buf_append_u8(code, 133);
    buf_append_u8(code, 192);
    ng_add_fixup(ng, ((code)->len), (oc_string){ .ptr = (const uint8_t*)("__trap_null_deref"), .len = sizeof("__trap_null_deref") - 1 }, true);
    buf_append_u8(code, 15);
    buf_append_u8(code, 132);
    elf_append_u32(code, 0);
    }
    else {
    ng_emit_lvalue_addr(ng, code, data, ((expr)->left));
    }
    if ((f_off != 0)) {
    buf_append_u8(code, 72);
    buf_append_u8(code, 129);
    buf_append_u8(code, 192);
    elf_append_i32(code, f_off);
    }
    return;
    }
    if ((((expr)->tag) == NODE_INDEX)) {
    oc_string lt = ng_expr_type(ng, ((expr)->left));
    size_t arr_len = 0;
    oc_string elem_t = (oc_string){ .ptr = (const uint8_t*)("i64"), .len = sizeof("i64") - 1 };
    if ((((lt).len) > 0)) {
    if ((__oc_index_str(lt, 0) == 91)) {
    arr_len = ng_parse_array_len(lt);
    elem_t = ng_parse_array_elem(lt);
    ng_emit_lvalue_addr(ng, code, data, ((expr)->left));
    }
    else {
    elem_t = ng_strip_ptr_type(lt);
    ng_emit_expr(ng, code, data, ((expr)->left));
    buf_append_u8(code, 72);
    buf_append_u8(code, 133);
    buf_append_u8(code, 192);
    ng_add_fixup(ng, ((code)->len), (oc_string){ .ptr = (const uint8_t*)("__trap_null_deref"), .len = sizeof("__trap_null_deref") - 1 }, true);
    buf_append_u8(code, 15);
    buf_append_u8(code, 132);
    elf_append_u32(code, 0);
    }
    }
    else {
    ng_emit_expr(ng, code, data, ((expr)->left));
    }
    buf_append_u8(code, 80);
    ng_emit_expr(ng, code, data, ((expr)->right));
    buf_append_u8(code, 89);
    if ((arr_len > 0)) {
    buf_append_u8(code, 72);
    buf_append_u8(code, 129);
    buf_append_u8(code, 248);
    elf_append_u32(code, ({ __auto_type _v = (arr_len); if (_v < 0 || (uint64_t)_v > 4294967295ULL) { fprintf(stderr, "panic: invalid unsigned cast\n"); exit(1); } (uint32_t)_v; }));
    ng_add_fixup(ng, ((code)->len), (oc_string){ .ptr = (const uint8_t*)("__trap_slice_bounds"), .len = sizeof("__trap_slice_bounds") - 1 }, true);
    buf_append_u8(code, 15);
    buf_append_u8(code, 131);
    elf_append_u32(code, 0);
    }
    int32_t stride = ng_type_stride(elem_t);
    if ((stride == 8)) {
    buf_append_u8(code, 72);
    buf_append_u8(code, 193);
    buf_append_u8(code, 224);
    buf_append_u8(code, 3);
    }
    else {
    if ((stride == 16)) {
    buf_append_u8(code, 72);
    buf_append_u8(code, 193);
    buf_append_u8(code, 224);
    buf_append_u8(code, 4);
    }
    }
    buf_append_u8(code, 72);
    buf_append_u8(code, 1);
    buf_append_u8(code, 200);
    return;
    }
}

void ng_emit_expr(NativeGen* ng, CodeBuffer* code, CodeBuffer* data, AstNode* expr) {
    if ((expr == ((AstNode*)(0)))) {
    return;
    }
    if ((((expr)->tag) == NODE_INT)) {
    uint64_t val = parse_literal_u64(((((expr)->tok)).slice));
    buf_append_u8(code, 72);
    buf_append_u8(code, 184);
    elf_append_u64(code, val);
    return;
    }
    if ((((expr)->tag) == NODE_FLOAT)) {
    buf_append_u8(code, 72);
    buf_append_u8(code, 184);
    elf_append_u64(code, ng_literal_bits(ng, ((((expr)->tok)).slice), str_eq(((expr)->type_name), (oc_string){ .ptr = (const uint8_t*)("f32"), .len = sizeof("f32") - 1 })));
    return;
    }
    if ((((expr)->tag) == NODE_BOOL)) {
    uint64_t val = 0;
    if (str_eq(((((expr)->tok)).slice), (oc_string){ .ptr = (const uint8_t*)("true"), .len = sizeof("true") - 1 })) {
    val = 1;
    }
    buf_append_u8(code, 72);
    buf_append_u8(code, 184);
    elf_append_u64(code, val);
    return;
    }
    if ((((expr)->tag) == NODE_STRING)) {
    oc_string raw = ((((expr)->tok)).slice);
    if ((((raw).len) >= 2)) {
    if ((__oc_index_str(raw, 0) == 34)) {
    raw = __oc_slice_str(raw, 1, __OC_SUB(((raw).len), 1));
    }
    }
    size_t data_off = ((data)->len);
    size_t emitted_len = ng_append_unescaped_str(data, raw);
    buf_append_u8(data, 0);
    buf_append_u8(code, 72);
    buf_append_u8(code, 184);
    size_t patch_off = ((code)->len);
    elf_append_u64(code, 0);
    ng_add_data_reloc(ng, patch_off, data_off);
    buf_append_u8(code, 72);
    buf_append_u8(code, 199);
    buf_append_u8(code, 194);
    elf_append_u32(code, ({ __auto_type _v = (emitted_len); if (_v < 0 || (uint64_t)_v > 4294967295ULL) { fprintf(stderr, "panic: invalid unsigned cast\n"); exit(1); } (uint32_t)_v; }));
    return;
    }
    if ((((expr)->tag) == NODE_IDENT)) {
    int32_t idx = ng_find_var_idx(ng, ((((expr)->tok)).slice));
    if ((idx >= 0)) {
    size_t u_idx = ({ __auto_type _v = (idx); if (_v < 0) { fprintf(stderr, "panic: invalid unsigned cast\n"); exit(1); } (size_t)_v; });
    int32_t off = __OC_AT((((ng)->var_offsets)), (u_idx));
    oc_string vt = __OC_AT((((ng)->var_types)), (u_idx));
    if ((__OC_AT((((ng)->var_arr_len)), (u_idx)) > 0)) {
    ng_emit_disp_lea_rax(code, off);
    return;
    }
    int32_t s_sz = ng_find_struct_size(ng, vt);
    if ((s_sz > 32)) {
    ng_fail(ng, (oc_string){ .ptr = (const uint8_t*)("native codegen error: a struct larger than 32 bytes cannot be copied by value; use a pointer to it"), .len = sizeof("native codegen error: a struct larger than 32 bytes cannot be copied by value; use a pointer to it") - 1 });
    }
    if ((s_sz > 0)) {
    ng_emit_disp_load_rax(code, off);
    if ((s_sz >= 16)) {
    ng_emit_disp_load_rdx(code, __OC_ADD(off, 8));
    }
    if ((s_sz >= 24)) {
    ng_emit_disp_load_rcx(code, __OC_ADD(off, 16));
    }
    if ((s_sz >= 32)) {
    ng_emit_disp_load_r8(code, __OC_ADD(off, 24));
    }
    return;
    }
    ng_emit_disp_load_rax(code, off);
    if (__OC_AT((((ng)->var_is_slice)), (u_idx))) {
    ng_emit_disp_load_rdx(code, __OC_SUB(off, 8));
    }
    return;
    }
    size_t ci = 0;
    while ((ci < ((ng)->const_count))) {
    if (str_eq(__OC_AT((((ng)->const_names)), (ci)), ((((expr)->tok)).slice))) {
    buf_append_u8(code, 72);
    buf_append_u8(code, 184);
    elf_append_u64(code, __OC_AT((((ng)->const_vals)), (ci)));
    return;
    }
    ci = __OC_ADD(ci, 1);
    }
    return;
    }
    if ((((expr)->tag) == NODE_ADDRESS_OF)) {
    if ((((expr)->left) != ((AstNode*)(0)))) {
    ng_emit_lvalue_addr(ng, code, data, ((expr)->left));
    }
    return;
    }
    if ((((expr)->tag) == NODE_STRUCT_INIT)) {
    if (str_eq(((((expr)->tok)).slice), (oc_string){ .ptr = (const uint8_t*)("string"), .len = sizeof("string") - 1 })) {
    AstNode* ptr_expr = ((AstNode*)(0));
    AstNode* len_expr = ((AstNode*)(0));
    AstNode* f = ((expr)->left);
    while ((f != ((AstNode*)(0)))) {
    if (str_eq(((((f)->tok)).slice), (oc_string){ .ptr = (const uint8_t*)("ptr"), .len = sizeof("ptr") - 1 })) {
    ptr_expr = ((f)->left);
    }
    else {
    if (str_eq(((((f)->tok)).slice), (oc_string){ .ptr = (const uint8_t*)("len"), .len = sizeof("len") - 1 })) {
    len_expr = ((f)->left);
    }
    }
    f = ((f)->next);
    }
    if ((ptr_expr != ((AstNode*)(0)))) {
    ng_emit_expr(ng, code, data, ptr_expr);
    }
    else {
    buf_append_u8(code, 72);
    buf_append_u8(code, 49);
    buf_append_u8(code, 192);
    }
    buf_append_u8(code, 80);
    if ((len_expr != ((AstNode*)(0)))) {
    ng_emit_expr(ng, code, data, len_expr);
    }
    else {
    buf_append_u8(code, 72);
    buf_append_u8(code, 49);
    buf_append_u8(code, 192);
    }
    buf_append_u8(code, 72);
    buf_append_u8(code, 137);
    buf_append_u8(code, 194);
    buf_append_u8(code, 88);
    return;
    }
    if ((ng_find_struct_size(ng, ((((expr)->tok)).slice)) > 32)) {
    ng_fail(ng, (oc_string){ .ptr = (const uint8_t*)("native codegen error: a struct literal larger than 32 bytes can only initialize a variable directly"), .len = sizeof("native codegen error: a struct literal larger than 32 bytes can only initialize a variable directly") - 1 });
    }
    buf_append_u8(code, 72);
    buf_append_u8(code, 131);
    buf_append_u8(code, 236);
    buf_append_u8(code, 32);
    buf_append_u8(code, 72);
    buf_append_u8(code, 49);
    buf_append_u8(code, 192);
    int32_t zq = 0;
    while ((zq < 32)) {
    ng_emit_store_disp(code, 0, 8, zq, true);
    zq = __OC_ADD(zq, 8);
    }
    AstNode* f = ((expr)->left);
    while ((f != ((AstNode*)(0)))) {
    int32_t f_off = ng_find_field_offset(ng, ((((expr)->tok)).slice), ((((f)->tok)).slice));
    int32_t f_w = ng_field_width(ng, ((((expr)->tok)).slice), ((((f)->tok)).slice));
    oc_string f_type = ng_find_field_type(ng, ((((expr)->tok)).slice), ((((f)->tok)).slice));
    if ((((f_type).len) > 0)) {
    if ((__oc_index_str(f_type, 0) == 91)) {
    if ((((((f)->left))->tag) == NODE_ARRAY_INIT)) {
    ng_emit_array_init_at(ng, code, data, f_off, f_type, ((f)->left), true);
    f = ((f)->next);
    continue;
    }
    }
    }
    ng_emit_expr_as(ng, code, data, ((f)->left), f_type);
    int32_t f_words = ng_value_words(ng, ng_find_field_type(ng, ((((expr)->tok)).slice), ((((f)->tok)).slice)));
    if ((f_words > 1)) {
    if ((__OC_ADD(f_off, __OC_MUL(f_words, 8)) <= 32)) {
    int32_t w = 0;
    while ((w < f_words)) {
    if ((w == 3)) {
    buf_append_u8(code, 76);
    }
    else {
    buf_append_u8(code, 72);
    }
    buf_append_u8(code, 137);
    if ((w == 1)) {
    buf_append_u8(code, 84);
    }
    else {
    if ((w == 2)) {
    buf_append_u8(code, 76);
    }
    else {
    buf_append_u8(code, 68);
    }
    }
    buf_append_u8(code, 36);
    buf_append_u8(code, ({ __auto_type _v = (__OC_ADD(f_off, __OC_MUL(w, 8))); if (_v < 0 || (uint64_t)_v > 255ULL) { fprintf(stderr, "panic: invalid unsigned cast\n"); exit(1); } (uint8_t)_v; }));
    w = __OC_ADD(w, 1);
    }
    }
    }
    else {
    if ((f_off >= 0)) {
    if ((__OC_ADD(f_off, f_w) <= 32)) {
    buf_append_u8(code, 72);
    buf_append_u8(code, 141);
    buf_append_u8(code, 76);
    buf_append_u8(code, 36);
    buf_append_u8(code, ({ __auto_type _v = (f_off); if (_v < 0 || (uint64_t)_v > 255ULL) { fprintf(stderr, "panic: invalid unsigned cast\n"); exit(1); } (uint8_t)_v; }));
    ng_emit_store_at_rcx(code, f_w);
    }
    }
    }
    f = ((f)->next);
    }
    buf_append_u8(code, 88);
    buf_append_u8(code, 90);
    buf_append_u8(code, 89);
    buf_append_u8(code, 65);
    buf_append_u8(code, 88);
    return;
    }
    if ((((expr)->tag) == NODE_FIELD_ACCESS)) {
    if (ng_is_expr_slice(ng, ((expr)->left))) {
    ng_emit_expr(ng, code, data, ((expr)->left));
    if (str_eq(((((expr)->tok)).slice), (oc_string){ .ptr = (const uint8_t*)("len"), .len = sizeof("len") - 1 })) {
    buf_append_u8(code, 72);
    buf_append_u8(code, 137);
    buf_append_u8(code, 208);
    }
    return;
    }
    oc_string raw_bt = ng_expr_type(ng, ((expr)->left));
    oc_string bt = ng_strip_ptr_type(raw_bt);
    oc_string ft = ng_find_field_type(ng, bt, ((((expr)->tok)).slice));
    bool by_ptr = false;
    if ((((raw_bt).len) > 0)) {
    if ((__oc_index_str(raw_bt, 0) == 42)) {
    by_ptr = true;
    }
    if ((__oc_index_str(raw_bt, 0) == 38)) {
    by_ptr = true;
    }
    }
    bool spilled = false;
    if ((!by_ptr)) {
    if ((!ng_is_place(ng, ((expr)->left)))) {
    ng_emit_expr(ng, code, data, ((expr)->left));
    buf_append_u8(code, 65);
    buf_append_u8(code, 80);
    buf_append_u8(code, 81);
    buf_append_u8(code, 82);
    buf_append_u8(code, 80);
    buf_append_u8(code, 72);
    buf_append_u8(code, 137);
    buf_append_u8(code, 224);
    int32_t f_off = ng_find_field_offset(ng, bt, ((((expr)->tok)).slice));
    if ((f_off != 0)) {
    buf_append_u8(code, 72);
    buf_append_u8(code, 129);
    buf_append_u8(code, 192);
    elf_append_i32(code, f_off);
    }
    spilled = true;
    }
    }
    if ((!spilled)) {
    ng_emit_lvalue_addr(ng, code, data, expr);
    }
    if (spilled) {
    if (str_eq(ft, (oc_string){ .ptr = (const uint8_t*)("string"), .len = sizeof("string") - 1 })) {
    buf_append_u8(code, 72);
    buf_append_u8(code, 139);
    buf_append_u8(code, 80);
    buf_append_u8(code, 8);
    buf_append_u8(code, 72);
    buf_append_u8(code, 139);
    buf_append_u8(code, 0);
    }
    else {
    if ((!ng_emit_load_struct_at_rax(ng, code, ft))) {
    ng_emit_load_at_rax(code, ft, ng_field_width(ng, bt, ((((expr)->tok)).slice)));
    }
    }
    buf_append_u8(code, 72);
    buf_append_u8(code, 131);
    buf_append_u8(code, 196);
    buf_append_u8(code, 32);
    return;
    }
    if (str_eq(ft, (oc_string){ .ptr = (const uint8_t*)("string"), .len = sizeof("string") - 1 })) {
    buf_append_u8(code, 72);
    buf_append_u8(code, 139);
    buf_append_u8(code, 80);
    buf_append_u8(code, 8);
    buf_append_u8(code, 72);
    buf_append_u8(code, 139);
    buf_append_u8(code, 0);
    return;
    }
    if (ng_emit_load_struct_at_rax(ng, code, ft)) {
    return;
    }
    ng_emit_load_at_rax(code, ft, ng_field_width(ng, bt, ((((expr)->tok)).slice)));
    return;
    }
    if ((((expr)->tag) == NODE_CAST)) {
    ng_emit_expr(ng, code, data, ((expr)->left));
    oc_string dst = ((expr)->type_name);
    oc_string src = ng_expr_type(ng, ((expr)->left));
    if (is_float_type(dst)) {
    ng_emit_convert(ng, code, src, dst);
    return;
    }
    if (is_float_type(src)) {
    bool int_dst = (ng_scalar_width(dst) < 8);
    if ((ng_int_class(dst) >= 2)) {
    int_dst = true;
    }
    if (str_eq(dst, (oc_string){ .ptr = (const uint8_t*)("bool"), .len = sizeof("bool") - 1 })) {
    int_dst = false;
    }
    if ((((dst).len) > 0)) {
    if ((__oc_index_str(dst, 0) == 42)) {
    int_dst = false;
    }
    if ((__oc_index_str(dst, 0) == 38)) {
    int_dst = false;
    }
    }
    if ((!int_dst)) {
    ng_fail(ng, (oc_string){ .ptr = (const uint8_t*)("native codegen error: a float can only be cast to an integer or float type"), .len = sizeof("native codegen error: a float can only be cast to an integer or float type") - 1 });
    return;
    }
    ng_emit_float_to_int(ng, code, src, dst);
    return;
    }
    if ((((dst).len) > 0)) {
    if ((__oc_index_str(dst, 0) == 42)) {
    return;
    }
    if ((__oc_index_str(dst, 0) == 38)) {
    return;
    }
    }
    if ((((src).len) > 0)) {
    if ((__oc_index_str(src, 0) == 42)) {
    return;
    }
    if ((__oc_index_str(src, 0) == 38)) {
    return;
    }
    }
    bool src_uns = ng_is_unsigned_expr(ng, ((expr)->left));
    oc_string cast_trap = (oc_string){ .ptr = (const uint8_t*)("__trap_cast_signed"), .len = sizeof("__trap_cast_signed") - 1 };
    if (is_unsigned_type_name(dst)) {
    cast_trap = (oc_string){ .ptr = (const uint8_t*)("__trap_cast_unsigned"), .len = sizeof("__trap_cast_unsigned") - 1 };
    }
    if (str_eq(dst, (oc_string){ .ptr = (const uint8_t*)("u8"), .len = sizeof("u8") - 1 })) {
    ng_emit_range_check(ng, code, dst, cast_trap);
    return;
    }
    if (str_eq(dst, (oc_string){ .ptr = (const uint8_t*)("u16"), .len = sizeof("u16") - 1 })) {
    ng_emit_range_check(ng, code, dst, cast_trap);
    return;
    }
    if (str_eq(dst, (oc_string){ .ptr = (const uint8_t*)("u32"), .len = sizeof("u32") - 1 })) {
    ng_emit_range_check(ng, code, dst, cast_trap);
    return;
    }
    bool is_u64_dst = false;
    if (str_eq(dst, (oc_string){ .ptr = (const uint8_t*)("u64"), .len = sizeof("u64") - 1 })) {
    is_u64_dst = true;
    }
    if (str_eq(dst, (oc_string){ .ptr = (const uint8_t*)("usize"), .len = sizeof("usize") - 1 })) {
    is_u64_dst = true;
    }
    if (is_u64_dst) {
    if ((!src_uns)) {
    buf_append_u8(code, 72);
    buf_append_u8(code, 133);
    buf_append_u8(code, 192);
    ng_add_fixup(ng, ((code)->len), cast_trap, true);
    buf_append_u8(code, 15);
    buf_append_u8(code, 136);
    elf_append_u32(code, 0);
    }
    return;
    }
    bool is_narrow_signed = false;
    if (str_eq(dst, (oc_string){ .ptr = (const uint8_t*)("i8"), .len = sizeof("i8") - 1 })) {
    is_narrow_signed = true;
    }
    if (str_eq(dst, (oc_string){ .ptr = (const uint8_t*)("i16"), .len = sizeof("i16") - 1 })) {
    is_narrow_signed = true;
    }
    if (str_eq(dst, (oc_string){ .ptr = (const uint8_t*)("i32"), .len = sizeof("i32") - 1 })) {
    is_narrow_signed = true;
    }
    if (is_narrow_signed) {
    if (src_uns) {
    uint64_t max_v = 127;
    if (str_eq(dst, (oc_string){ .ptr = (const uint8_t*)("i16"), .len = sizeof("i16") - 1 })) {
    max_v = 32767;
    }
    else {
    if (str_eq(dst, (oc_string){ .ptr = (const uint8_t*)("i32"), .len = sizeof("i32") - 1 })) {
    max_v = 2147483647;
    }
    }
    buf_append_u8(code, 72);
    buf_append_u8(code, 185);
    elf_append_u64(code, max_v);
    buf_append_u8(code, 72);
    buf_append_u8(code, 57);
    buf_append_u8(code, 200);
    ng_add_fixup(ng, ((code)->len), cast_trap, true);
    buf_append_u8(code, 15);
    buf_append_u8(code, 135);
    elf_append_u32(code, 0);
    }
    else {
    ng_emit_range_check(ng, code, dst, cast_trap);
    }
    return;
    }
    if (str_eq(dst, (oc_string){ .ptr = (const uint8_t*)("i64"), .len = sizeof("i64") - 1 })) {
    if (src_uns) {
    buf_append_u8(code, 72);
    buf_append_u8(code, 133);
    buf_append_u8(code, 192);
    ng_add_fixup(ng, ((code)->len), cast_trap, true);
    buf_append_u8(code, 15);
    buf_append_u8(code, 136);
    elf_append_u32(code, 0);
    }
    return;
    }
    return;
    }
    if ((((expr)->tag) == NODE_UNARY)) {
    if ((((((expr)->tok)).tag) == TOK_MINUS)) {
    if ((((expr)->left) != ((AstNode*)(0)))) {
    if ((((((expr)->left))->tag) == NODE_INT)) {
    uint64_t mag = parse_literal_u64(((((((expr)->left))->tok)).slice));
    if ((mag == 2147483648)) {
    buf_append_u8(code, 72);
    buf_append_u8(code, 184);
    elf_append_u32(code, 2147483648);
    elf_append_u32(code, 4294967295);
    return;
    }
    if ((mag > 9223372036854775807)) {
    if ((__OC_SUB(mag, 1) == 9223372036854775807)) {
    buf_append_u8(code, 72);
    buf_append_u8(code, 184);
    elf_append_u32(code, 0);
    elf_append_u32(code, 2147483648);
    return;
    }
    }
    }
    }
    ng_emit_expr(ng, code, data, ((expr)->left));
    oc_string neg_t = ng_expr_type(ng, ((expr)->left));
    if (is_float_type(neg_t)) {
    if (str_eq(neg_t, (oc_string){ .ptr = (const uint8_t*)("f64"), .len = sizeof("f64") - 1 })) {
    buf_append_u8(code, 72);
    buf_append_u8(code, 15);
    buf_append_u8(code, 186);
    buf_append_u8(code, 248);
    buf_append_u8(code, 63);
    }
    else {
    buf_append_u8(code, 15);
    buf_append_u8(code, 186);
    buf_append_u8(code, 248);
    buf_append_u8(code, 31);
    }
    return;
    }
    buf_append_u8(code, 72);
    buf_append_u8(code, 247);
    buf_append_u8(code, 216);
    ng_add_fixup(ng, ((code)->len), (oc_string){ .ptr = (const uint8_t*)("__trap_overflow"), .len = sizeof("__trap_overflow") - 1 }, true);
    buf_append_u8(code, 15);
    if (is_unsigned_type_name(neg_t)) {
    buf_append_u8(code, 130);
    }
    else {
    buf_append_u8(code, 128);
    }
    elf_append_u32(code, 0);
    ng_emit_range_check(ng, code, neg_t, (oc_string){ .ptr = (const uint8_t*)("__trap_overflow"), .len = sizeof("__trap_overflow") - 1 });
    return;
    }
    ng_emit_expr(ng, code, data, ((expr)->left));
    if ((((((expr)->tok)).tag) == TOK_TILDE)) {
    buf_append_u8(code, 72);
    buf_append_u8(code, 247);
    buf_append_u8(code, 208);
    ng_emit_truncate(code, ng_expr_type(ng, ((expr)->left)));
    }
    if ((((((expr)->tok)).tag) == TOK_BANG)) {
    buf_append_u8(code, 72);
    buf_append_u8(code, 133);
    buf_append_u8(code, 192);
    buf_append_u8(code, 15);
    buf_append_u8(code, 148);
    buf_append_u8(code, 192);
    buf_append_u8(code, 15);
    buf_append_u8(code, 182);
    buf_append_u8(code, 192);
    }
    return;
    }
    if ((((expr)->tag) == NODE_INDEX)) {
    if (ng_is_expr_slice(ng, ((expr)->left))) {
    ng_emit_expr(ng, code, data, ((expr)->left));
    buf_append_u8(code, 80);
    buf_append_u8(code, 82);
    ng_emit_expr(ng, code, data, ((expr)->right));
    buf_append_u8(code, 90);
    buf_append_u8(code, 89);
    buf_append_u8(code, 72);
    buf_append_u8(code, 57);
    buf_append_u8(code, 208);
    ng_add_fixup(ng, ((code)->len), (oc_string){ .ptr = (const uint8_t*)("__trap_slice_bounds"), .len = sizeof("__trap_slice_bounds") - 1 }, true);
    buf_append_u8(code, 15);
    buf_append_u8(code, 131);
    elf_append_u32(code, 0);
    buf_append_u8(code, 15);
    buf_append_u8(code, 182);
    buf_append_u8(code, 4);
    buf_append_u8(code, 1);
    return;
    }
    oc_string elem_t = ng_expr_type(ng, expr);
    ng_emit_lvalue_addr(ng, code, data, expr);
    if (ng_is_byte_type(elem_t)) {
    buf_append_u8(code, 15);
    buf_append_u8(code, 182);
    buf_append_u8(code, 0);
    return;
    }
    if (str_eq(elem_t, (oc_string){ .ptr = (const uint8_t*)("i8"), .len = sizeof("i8") - 1 })) {
    buf_append_u8(code, 72);
    buf_append_u8(code, 15);
    buf_append_u8(code, 190);
    buf_append_u8(code, 0);
    return;
    }
    if (str_eq(elem_t, (oc_string){ .ptr = (const uint8_t*)("string"), .len = sizeof("string") - 1 })) {
    buf_append_u8(code, 72);
    buf_append_u8(code, 139);
    buf_append_u8(code, 80);
    buf_append_u8(code, 8);
    buf_append_u8(code, 72);
    buf_append_u8(code, 139);
    buf_append_u8(code, 0);
    return;
    }
    buf_append_u8(code, 72);
    buf_append_u8(code, 139);
    buf_append_u8(code, 0);
    return;
    }
    if ((((expr)->tag) == NODE_DEREF)) {
    oc_string elem_t = ng_expr_type(ng, expr);
    ng_emit_expr(ng, code, data, ((expr)->left));
    buf_append_u8(code, 72);
    buf_append_u8(code, 133);
    buf_append_u8(code, 192);
    ng_add_fixup(ng, ((code)->len), (oc_string){ .ptr = (const uint8_t*)("__trap_null_deref"), .len = sizeof("__trap_null_deref") - 1 }, true);
    buf_append_u8(code, 15);
    buf_append_u8(code, 132);
    elf_append_u32(code, 0);
    if (ng_emit_load_struct_at_rax(ng, code, elem_t)) {
    return;
    }
    if (ng_is_byte_type(elem_t)) {
    buf_append_u8(code, 15);
    buf_append_u8(code, 182);
    buf_append_u8(code, 0);
    return;
    }
    if (str_eq(elem_t, (oc_string){ .ptr = (const uint8_t*)("i8"), .len = sizeof("i8") - 1 })) {
    buf_append_u8(code, 72);
    buf_append_u8(code, 15);
    buf_append_u8(code, 190);
    buf_append_u8(code, 0);
    return;
    }
    buf_append_u8(code, 72);
    buf_append_u8(code, 139);
    buf_append_u8(code, 0);
    return;
    }
    if ((((expr)->tag) == NODE_SLICE)) {
    oc_string base_t = ng_expr_type(ng, ((expr)->left));
    bool is_array = false;
    if ((((base_t).len) > 0)) {
    if ((__oc_index_str(base_t, 0) == 91)) {
    is_array = true;
    }
    if ((__oc_index_str(base_t, 0) == 42)) {
    ng_fail(ng, (oc_string){ .ptr = (const uint8_t*)("native codegen error: cannot slice a raw pointer (it has no length)"), .len = sizeof("native codegen error: cannot slice a raw pointer (it has no length)") - 1 });
    }
    if ((__oc_index_str(base_t, 0) == 38)) {
    ng_fail(ng, (oc_string){ .ptr = (const uint8_t*)("native codegen error: cannot slice a raw pointer (it has no length)"), .len = sizeof("native codegen error: cannot slice a raw pointer (it has no length)") - 1 });
    }
    }
    if (is_array) {
    size_t arr_bytes = __OC_MUL(ng_parse_array_len(base_t), ({ __auto_type _v = (ng_type_stride(ng_parse_array_elem(base_t))); if (_v < 0) { fprintf(stderr, "panic: invalid unsigned cast\n"); exit(1); } (size_t)_v; }));
    ng_emit_lvalue_addr(ng, code, data, ((expr)->left));
    buf_append_u8(code, 72);
    buf_append_u8(code, 199);
    buf_append_u8(code, 194);
    elf_append_u32(code, ({ __auto_type _v = (arr_bytes); if (_v < 0 || (uint64_t)_v > 4294967295ULL) { fprintf(stderr, "panic: invalid unsigned cast\n"); exit(1); } (uint32_t)_v; }));
    }
    else {
    ng_emit_expr(ng, code, data, ((expr)->left));
    }
    buf_append_u8(code, 80);
    buf_append_u8(code, 82);
    AstNode* range_node = ((expr)->right);
    if ((range_node != ((AstNode*)(0)))) {
    if ((((range_node)->left) != ((AstNode*)(0)))) {
    ng_emit_expr(ng, code, data, ((range_node)->left));
    }
    else {
    buf_append_u8(code, 72);
    buf_append_u8(code, 49);
    buf_append_u8(code, 192);
    }
    }
    else {
    buf_append_u8(code, 72);
    buf_append_u8(code, 49);
    buf_append_u8(code, 192);
    }
    buf_append_u8(code, 80);
    if ((range_node != ((AstNode*)(0)))) {
    if ((((range_node)->right) != ((AstNode*)(0)))) {
    ng_emit_expr(ng, code, data, ((range_node)->right));
    }
    else {
    buf_append_u8(code, 72);
    buf_append_u8(code, 139);
    buf_append_u8(code, 68);
    buf_append_u8(code, 36);
    buf_append_u8(code, 8);
    }
    }
    else {
    buf_append_u8(code, 72);
    buf_append_u8(code, 139);
    buf_append_u8(code, 68);
    buf_append_u8(code, 36);
    buf_append_u8(code, 8);
    }
    buf_append_u8(code, 91);
    buf_append_u8(code, 72);
    buf_append_u8(code, 57);
    buf_append_u8(code, 195);
    ng_add_fixup(ng, ((code)->len), (oc_string){ .ptr = (const uint8_t*)("__trap_slice_bounds"), .len = sizeof("__trap_slice_bounds") - 1 }, true);
    buf_append_u8(code, 15);
    buf_append_u8(code, 135);
    elf_append_u32(code, 0);
    buf_append_u8(code, 72);
    buf_append_u8(code, 139);
    buf_append_u8(code, 12);
    buf_append_u8(code, 36);
    buf_append_u8(code, 72);
    buf_append_u8(code, 57);
    buf_append_u8(code, 200);
    ng_add_fixup(ng, ((code)->len), (oc_string){ .ptr = (const uint8_t*)("__trap_slice_bounds"), .len = sizeof("__trap_slice_bounds") - 1 }, true);
    buf_append_u8(code, 15);
    buf_append_u8(code, 135);
    elf_append_u32(code, 0);
    buf_append_u8(code, 72);
    buf_append_u8(code, 41);
    buf_append_u8(code, 216);
    buf_append_u8(code, 72);
    buf_append_u8(code, 137);
    buf_append_u8(code, 194);
    buf_append_u8(code, 72);
    buf_append_u8(code, 131);
    buf_append_u8(code, 196);
    buf_append_u8(code, 8);
    buf_append_u8(code, 88);
    buf_append_u8(code, 72);
    buf_append_u8(code, 1);
    buf_append_u8(code, 216);
    return;
    }
    if ((((expr)->tag) == NODE_BINARY)) {
    oc_string float_t = ng_float_result_type(ng_expr_type(ng, ((expr)->left)), ng_expr_type(ng, ((expr)->right)));
    if ((((float_t).len) > 0)) {
    bool wide = str_eq(float_t, (oc_string){ .ptr = (const uint8_t*)("f64"), .len = sizeof("f64") - 1 });
    ng_emit_expr_as(ng, code, data, ((expr)->left), float_t);
    buf_append_u8(code, 80);
    ng_emit_expr_as(ng, code, data, ((expr)->right), float_t);
    buf_append_u8(code, 72);
    buf_append_u8(code, 137);
    buf_append_u8(code, 195);
    buf_append_u8(code, 88);
    ng_emit_rax_to_xmm0(code, wide);
    ng_emit_rbx_to_xmm1(code, wide);
    uint32_t op = ((((expr)->tok)).tag);
    uint8_t arith = 0;
    if ((op == TOK_PLUS)) {
    arith = 88;
    }
    if ((op == TOK_MINUS)) {
    arith = 92;
    }
    if ((op == TOK_STAR)) {
    arith = 89;
    }
    if ((op == TOK_SLASH)) {
    arith = 94;
    }
    if ((arith != 0)) {
    buf_append_u8(code, ng_sse_prefix(wide));
    buf_append_u8(code, 15);
    buf_append_u8(code, arith);
    buf_append_u8(code, 193);
    ng_emit_xmm0_to_rax(code, wide);
    ng_emit_fp_check(ng, code, data, wide, ((expr)->tok));
    return;
    }
    bool swapped = (op == TOK_LT);
    if ((op == TOK_LTE)) {
    swapped = true;
    }
    bool strict = (op == TOK_LT);
    if ((op == TOK_GT)) {
    strict = true;
    }
    bool is_cmp = swapped;
    if ((op == TOK_GT)) {
    is_cmp = true;
    }
    if ((op == TOK_GTE)) {
    is_cmp = true;
    }
    if ((!is_cmp)) {
    ng_fail(ng, (oc_string){ .ptr = (const uint8_t*)("native codegen error: operator is not defined for floats (use + - * / and < <= > >=)"), .len = sizeof("native codegen error: operator is not defined for floats (use + - * / and < <= > >=)") - 1 });
    return;
    }
    if (wide) {
    buf_append_u8(code, 102);
    }
    buf_append_u8(code, 15);
    buf_append_u8(code, 46);
    if (swapped) {
    buf_append_u8(code, 200);
    }
    else {
    buf_append_u8(code, 193);
    }
    buf_append_u8(code, 15);
    if (strict) {
    buf_append_u8(code, 151);
    }
    else {
    buf_append_u8(code, 147);
    }
    buf_append_u8(code, 192);
    buf_append_u8(code, 15);
    buf_append_u8(code, 182);
    buf_append_u8(code, 192);
    return;
    }
    bool is_uns = ng_binary_unsigned(ng, ((expr)->left), ((expr)->right));
    if ((((((expr)->tok)).tag) == TOK_SHR)) {
    is_uns = is_unsigned_type_name(ng_expr_type(ng, ((expr)->left)));
    }
    oc_string res_type = ng_expr_type(ng, expr);
    oc_string lt = ng_expr_type(ng, ((expr)->left));
    bool is_ptr_arith = false;
    if ((((lt).len) > 0)) {
    if ((__oc_index_str(lt, 0) == 42)) {
    is_ptr_arith = true;
    }
    if ((__oc_index_str(lt, 0) == 38)) {
    is_ptr_arith = true;
    }
    }
    ng_emit_expr(ng, code, data, ((expr)->left));
    buf_append_u8(code, 80);
    ng_emit_expr(ng, code, data, ((expr)->right));
    buf_append_u8(code, 72);
    buf_append_u8(code, 137);
    buf_append_u8(code, 195);
    buf_append_u8(code, 88);
    if ((((((expr)->tok)).tag) == TOK_PLUS)) {
    buf_append_u8(code, 72);
    buf_append_u8(code, 1);
    buf_append_u8(code, 216);
    if ((!is_ptr_arith)) {
    ng_add_fixup(ng, ((code)->len), (oc_string){ .ptr = (const uint8_t*)("__trap_overflow"), .len = sizeof("__trap_overflow") - 1 }, true);
    buf_append_u8(code, 15);
    if (is_uns) {
    buf_append_u8(code, 130);
    }
    else {
    buf_append_u8(code, 128);
    }
    elf_append_u32(code, 0);
    ng_emit_range_check(ng, code, res_type, (oc_string){ .ptr = (const uint8_t*)("__trap_overflow"), .len = sizeof("__trap_overflow") - 1 });
    }
    return;
    }
    if ((((((expr)->tok)).tag) == TOK_MINUS)) {
    buf_append_u8(code, 72);
    buf_append_u8(code, 41);
    buf_append_u8(code, 216);
    if ((!is_ptr_arith)) {
    ng_add_fixup(ng, ((code)->len), (oc_string){ .ptr = (const uint8_t*)("__trap_overflow"), .len = sizeof("__trap_overflow") - 1 }, true);
    buf_append_u8(code, 15);
    if (is_uns) {
    buf_append_u8(code, 130);
    }
    else {
    buf_append_u8(code, 128);
    }
    elf_append_u32(code, 0);
    ng_emit_range_check(ng, code, res_type, (oc_string){ .ptr = (const uint8_t*)("__trap_overflow"), .len = sizeof("__trap_overflow") - 1 });
    }
    return;
    }
    if ((((((expr)->tok)).tag) == TOK_STAR)) {
    if (is_uns) {
    buf_append_u8(code, 72);
    buf_append_u8(code, 247);
    buf_append_u8(code, 227);
    }
    else {
    buf_append_u8(code, 72);
    buf_append_u8(code, 15);
    buf_append_u8(code, 175);
    buf_append_u8(code, 195);
    }
    ng_add_fixup(ng, ((code)->len), (oc_string){ .ptr = (const uint8_t*)("__trap_overflow"), .len = sizeof("__trap_overflow") - 1 }, true);
    buf_append_u8(code, 15);
    buf_append_u8(code, 128);
    elf_append_u32(code, 0);
    ng_emit_range_check(ng, code, res_type, (oc_string){ .ptr = (const uint8_t*)("__trap_overflow"), .len = sizeof("__trap_overflow") - 1 });
    return;
    }
    if ((((((expr)->tok)).tag) == TOK_LT)) {
    buf_append_u8(code, 72);
    buf_append_u8(code, 57);
    buf_append_u8(code, 216);
    buf_append_u8(code, 15);
    if (is_uns) {
    buf_append_u8(code, 146);
    }
    else {
    buf_append_u8(code, 156);
    }
    buf_append_u8(code, 192);
    buf_append_u8(code, 15);
    buf_append_u8(code, 182);
    buf_append_u8(code, 192);
    return;
    }
    if ((((((expr)->tok)).tag) == TOK_LTE)) {
    buf_append_u8(code, 72);
    buf_append_u8(code, 57);
    buf_append_u8(code, 216);
    buf_append_u8(code, 15);
    if (is_uns) {
    buf_append_u8(code, 150);
    }
    else {
    buf_append_u8(code, 158);
    }
    buf_append_u8(code, 192);
    buf_append_u8(code, 15);
    buf_append_u8(code, 182);
    buf_append_u8(code, 192);
    return;
    }
    if ((((((expr)->tok)).tag) == TOK_GT)) {
    buf_append_u8(code, 72);
    buf_append_u8(code, 57);
    buf_append_u8(code, 216);
    buf_append_u8(code, 15);
    if (is_uns) {
    buf_append_u8(code, 151);
    }
    else {
    buf_append_u8(code, 159);
    }
    buf_append_u8(code, 192);
    buf_append_u8(code, 15);
    buf_append_u8(code, 182);
    buf_append_u8(code, 192);
    return;
    }
    if ((((((expr)->tok)).tag) == TOK_GTE)) {
    buf_append_u8(code, 72);
    buf_append_u8(code, 57);
    buf_append_u8(code, 216);
    buf_append_u8(code, 15);
    if (is_uns) {
    buf_append_u8(code, 147);
    }
    else {
    buf_append_u8(code, 157);
    }
    buf_append_u8(code, 192);
    buf_append_u8(code, 15);
    buf_append_u8(code, 182);
    buf_append_u8(code, 192);
    return;
    }
    if ((((((expr)->tok)).tag) == TOK_EQEQ)) {
    buf_append_u8(code, 72);
    buf_append_u8(code, 57);
    buf_append_u8(code, 216);
    buf_append_u8(code, 15);
    buf_append_u8(code, 148);
    buf_append_u8(code, 192);
    buf_append_u8(code, 15);
    buf_append_u8(code, 182);
    buf_append_u8(code, 192);
    return;
    }
    if ((((((expr)->tok)).tag) == TOK_NEQ)) {
    buf_append_u8(code, 72);
    buf_append_u8(code, 57);
    buf_append_u8(code, 216);
    buf_append_u8(code, 15);
    buf_append_u8(code, 149);
    buf_append_u8(code, 192);
    buf_append_u8(code, 15);
    buf_append_u8(code, 182);
    buf_append_u8(code, 192);
    return;
    }
    if ((((((expr)->tok)).tag) == TOK_SLASH)) {
    buf_append_u8(code, 72);
    buf_append_u8(code, 133);
    buf_append_u8(code, 219);
    ng_add_fixup(ng, ((code)->len), (oc_string){ .ptr = (const uint8_t*)("__trap_div_zero"), .len = sizeof("__trap_div_zero") - 1 }, true);
    buf_append_u8(code, 15);
    buf_append_u8(code, 132);
    elf_append_u32(code, 0);
    if (is_uns) {
    buf_append_u8(code, 49);
    buf_append_u8(code, 210);
    buf_append_u8(code, 72);
    buf_append_u8(code, 247);
    buf_append_u8(code, 243);
    }
    else {
    buf_append_u8(code, 72);
    buf_append_u8(code, 153);
    buf_append_u8(code, 72);
    buf_append_u8(code, 247);
    buf_append_u8(code, 251);
    }
    return;
    }
    if ((((((expr)->tok)).tag) == TOK_PERCENT)) {
    buf_append_u8(code, 72);
    buf_append_u8(code, 133);
    buf_append_u8(code, 219);
    ng_add_fixup(ng, ((code)->len), (oc_string){ .ptr = (const uint8_t*)("__trap_div_zero"), .len = sizeof("__trap_div_zero") - 1 }, true);
    buf_append_u8(code, 15);
    buf_append_u8(code, 132);
    elf_append_u32(code, 0);
    if (is_uns) {
    buf_append_u8(code, 49);
    buf_append_u8(code, 210);
    buf_append_u8(code, 72);
    buf_append_u8(code, 247);
    buf_append_u8(code, 243);
    }
    else {
    buf_append_u8(code, 72);
    buf_append_u8(code, 153);
    buf_append_u8(code, 72);
    buf_append_u8(code, 247);
    buf_append_u8(code, 251);
    }
    buf_append_u8(code, 72);
    buf_append_u8(code, 137);
    buf_append_u8(code, 208);
    return;
    }
    if ((((((expr)->tok)).tag) == TOK_AMP)) {
    buf_append_u8(code, 72);
    buf_append_u8(code, 33);
    buf_append_u8(code, 216);
    return;
    }
    if ((((((expr)->tok)).tag) == TOK_PIPE)) {
    buf_append_u8(code, 72);
    buf_append_u8(code, 9);
    buf_append_u8(code, 216);
    return;
    }
    if ((((((expr)->tok)).tag) == TOK_CARET)) {
    buf_append_u8(code, 72);
    buf_append_u8(code, 49);
    buf_append_u8(code, 216);
    return;
    }
    if ((((((expr)->tok)).tag) == TOK_SHL)) {
    buf_append_u8(code, 136);
    buf_append_u8(code, 217);
    buf_append_u8(code, 72);
    buf_append_u8(code, 211);
    buf_append_u8(code, 224);
    return;
    }
    if ((((((expr)->tok)).tag) == TOK_SHR)) {
    buf_append_u8(code, 136);
    buf_append_u8(code, 217);
    buf_append_u8(code, 72);
    buf_append_u8(code, 211);
    if (is_uns) {
    buf_append_u8(code, 232);
    }
    else {
    buf_append_u8(code, 248);
    }
    return;
    }
    return;
    }
    if ((((expr)->tag) == NODE_CALL)) {
    if (str_eq(((((expr)->tok)).slice), (oc_string){ .ptr = (const uint8_t*)("f64_to_bits"), .len = sizeof("f64_to_bits") - 1 })) {
    ng_emit_expr_as(ng, code, data, ((expr)->left), (oc_string){ .ptr = (const uint8_t*)("f64"), .len = sizeof("f64") - 1 });
    return;
    }
    if (str_eq(((((expr)->tok)).slice), (oc_string){ .ptr = (const uint8_t*)("sqrt_f64"), .len = sizeof("sqrt_f64") - 1 })) {
    ng_emit_expr_as(ng, code, data, ((expr)->left), (oc_string){ .ptr = (const uint8_t*)("f64"), .len = sizeof("f64") - 1 });
    ng_emit_rax_to_xmm0(code, true);
    buf_append_u8(code, 242);
    buf_append_u8(code, 15);
    buf_append_u8(code, 81);
    buf_append_u8(code, 192);
    ng_emit_xmm0_to_rax(code, true);
    ng_emit_fp_check(ng, code, data, true, ((expr)->tok));
    return;
    }
    oc_string w_op = wrapping_op(((((expr)->tok)).slice));
    if ((((w_op).len) > 0)) {
    size_t w_argc = 0;
    AstNode* w_arg = ((expr)->left);
    while ((w_arg != ((AstNode*)(0)))) {
    w_argc = __OC_ADD(w_argc, 1);
    w_arg = ((w_arg)->next);
    }
    if ((w_argc != 2)) {
    ng_fail(ng, (oc_string){ .ptr = (const uint8_t*)("native codegen error: wrapping_add/wrapping_sub/wrapping_mul take exactly 2 arguments"), .len = sizeof("native codegen error: wrapping_add/wrapping_sub/wrapping_mul take exactly 2 arguments") - 1 });
    return;
    }
    ng_emit_expr(ng, code, data, ((expr)->left));
    buf_append_u8(code, 80);
    ng_emit_expr(ng, code, data, ((((expr)->left))->next));
    buf_append_u8(code, 72);
    buf_append_u8(code, 137);
    buf_append_u8(code, 195);
    buf_append_u8(code, 88);
    if (str_eq(w_op, (oc_string){ .ptr = (const uint8_t*)("add"), .len = sizeof("add") - 1 })) {
    buf_append_u8(code, 72);
    buf_append_u8(code, 1);
    buf_append_u8(code, 216);
    }
    else {
    if (str_eq(w_op, (oc_string){ .ptr = (const uint8_t*)("sub"), .len = sizeof("sub") - 1 })) {
    buf_append_u8(code, 72);
    buf_append_u8(code, 41);
    buf_append_u8(code, 216);
    }
    else {
    buf_append_u8(code, 72);
    buf_append_u8(code, 15);
    buf_append_u8(code, 175);
    buf_append_u8(code, 195);
    }
    }
    ng_emit_truncate(code, ng_expr_type(ng, expr));
    return;
    }
    size_t reg_count = 0;
    AstNode* a = ((expr)->left);
    size_t a_idx = 0;
    while ((a != ((AstNode*)(0)))) {
    ng_emit_expr_as(ng, code, data, a, ng_fn_param_type(ng, ((((expr)->tok)).slice), a_idx));
    a_idx = __OC_ADD(a_idx, 1);
    int32_t a_sz = ng_find_struct_size(ng, ng_expr_type(ng, a));
    if (ng_is_expr_slice(ng, a)) {
    buf_append_u8(code, 80);
    buf_append_u8(code, 82);
    reg_count = __OC_ADD(reg_count, 2);
    }
    else {
    if ((a_sz > 8)) {
    bool whole_value = (((a)->tag) == NODE_IDENT);
    if ((((a)->tag) == NODE_STRUCT_INIT)) {
    whole_value = true;
    }
    if ((((a)->tag) == NODE_CALL)) {
    whole_value = true;
    }
    if ((((a)->tag) == NODE_FIELD_ACCESS)) {
    whole_value = true;
    }
    if ((((a)->tag) == NODE_DEREF)) {
    whole_value = true;
    }
    if ((a_sz > 32)) {
    ng_fail(ng, (oc_string){ .ptr = (const uint8_t*)("native codegen error: a struct argument larger than 32 bytes cannot be passed by value; pass a pointer"), .len = sizeof("native codegen error: a struct argument larger than 32 bytes cannot be passed by value; pass a pointer") - 1 });
    }
    else {
    if ((!whole_value)) {
    ng_fail(ng, (oc_string){ .ptr = (const uint8_t*)("native codegen error: a struct passed by value must be a variable, a field, a dereference, a struct literal or a call result"), .len = sizeof("native codegen error: a struct passed by value must be a variable, a field, a dereference, a struct literal or a call result") - 1 });
    }
    }
    buf_append_u8(code, 80);
    buf_append_u8(code, 82);
    if ((a_sz >= 24)) {
    buf_append_u8(code, 81);
    }
    if ((a_sz >= 32)) {
    buf_append_u8(code, 65);
    buf_append_u8(code, 80);
    }
    reg_count = __OC_ADD(reg_count, ({ __auto_type _v = (__OC_DIV(a_sz, 8)); if (_v < 0) { fprintf(stderr, "panic: invalid unsigned cast\n"); exit(1); } (size_t)_v; }));
    }
    else {
    buf_append_u8(code, 80);
    reg_count = __OC_ADD(reg_count, 1);
    }
    }
    a = ((a)->next);
    }
    bool stack_args = (reg_count > 6);
    if (stack_args) {
    size_t r = 0;
    while ((r < 6)) {
    ng_emit_load_arg_reg(code, r, ({ __auto_type _v = (__OC_MUL(__OC_SUB(__OC_SUB(reg_count, 1), r), 8)); if (_v < 0 || (uint64_t)_v > 4294967295ULL) { fprintf(stderr, "panic: invalid unsigned cast\n"); exit(1); } (uint32_t)_v; }));
    r = __OC_ADD(r, 1);
    }
    }
    size_t idx = reg_count;
    if (stack_args) {
    idx = 0;
    }
    while ((idx > 0)) {
    idx = __OC_SUB(idx, 1);
    if ((idx == 0)) {
    buf_append_u8(code, 95);
    }
    else {
    if ((idx == 1)) {
    buf_append_u8(code, 94);
    }
    else {
    if ((idx == 2)) {
    buf_append_u8(code, 90);
    }
    else {
    if ((idx == 3)) {
    buf_append_u8(code, 89);
    }
    else {
    if ((idx == 4)) {
    buf_append_u8(code, 65);
    buf_append_u8(code, 88);
    }
    else {
    if ((idx == 5)) {
    buf_append_u8(code, 65);
    buf_append_u8(code, 89);
    }
    else {
    buf_append_u8(code, 72);
    buf_append_u8(code, 131);
    buf_append_u8(code, 196);
    buf_append_u8(code, 8);
    }
    }
    }
    }
    }
    }
    }
    oc_string fn_name = ((((expr)->tok)).slice);
    bool is_sys = false;
    uint32_t sys_nr = 0;
    if (str_eq(fn_name, (oc_string){ .ptr = (const uint8_t*)("sys_read"), .len = sizeof("sys_read") - 1 })) {
    is_sys = true;
    sys_nr = 0;
    }
    else {
    if (str_eq(fn_name, (oc_string){ .ptr = (const uint8_t*)("sys_write"), .len = sizeof("sys_write") - 1 })) {
    is_sys = true;
    sys_nr = 1;
    }
    else {
    if (str_eq(fn_name, (oc_string){ .ptr = (const uint8_t*)("sys_open"), .len = sizeof("sys_open") - 1 })) {
    is_sys = true;
    sys_nr = 2;
    }
    else {
    if (str_eq(fn_name, (oc_string){ .ptr = (const uint8_t*)("sys_close"), .len = sizeof("sys_close") - 1 })) {
    is_sys = true;
    sys_nr = 3;
    }
    else {
    if (str_eq(fn_name, (oc_string){ .ptr = (const uint8_t*)("sys_mmap"), .len = sizeof("sys_mmap") - 1 })) {
    is_sys = true;
    sys_nr = 9;
    }
    else {
    if (str_eq(fn_name, (oc_string){ .ptr = (const uint8_t*)("sys_munmap"), .len = sizeof("sys_munmap") - 1 })) {
    is_sys = true;
    sys_nr = 11;
    }
    else {
    if (str_eq(fn_name, (oc_string){ .ptr = (const uint8_t*)("sys_mprotect"), .len = sizeof("sys_mprotect") - 1 })) {
    is_sys = true;
    sys_nr = 10;
    }
    else {
    if (str_eq(fn_name, (oc_string){ .ptr = (const uint8_t*)("sys_socket"), .len = sizeof("sys_socket") - 1 })) {
    is_sys = true;
    sys_nr = 41;
    }
    else {
    if (str_eq(fn_name, (oc_string){ .ptr = (const uint8_t*)("sys_connect"), .len = sizeof("sys_connect") - 1 })) {
    is_sys = true;
    sys_nr = 42;
    }
    else {
    if (str_eq(fn_name, (oc_string){ .ptr = (const uint8_t*)("sys_exit"), .len = sizeof("sys_exit") - 1 })) {
    is_sys = true;
    sys_nr = 60;
    }
    else {
    if (str_eq(fn_name, (oc_string){ .ptr = (const uint8_t*)("sys_unlink"), .len = sizeof("sys_unlink") - 1 })) {
    is_sys = true;
    sys_nr = 87;
    }
    else {
    if (str_eq(fn_name, (oc_string){ .ptr = (const uint8_t*)("sys_chmod"), .len = sizeof("sys_chmod") - 1 })) {
    is_sys = true;
    sys_nr = 90;
    }
    }
    }
    }
    }
    }
    }
    }
    }
    }
    }
    }
    if (stack_args) {
    bool direct = (!is_sys);
    if ((((fn_name).len) > 6)) {
    if (str_eq(__oc_slice_str(fn_name, 0, 6), (oc_string){ .ptr = (const uint8_t*)("win32_"), .len = sizeof("win32_") - 1 })) {
    direct = false;
    }
    }
    if ((!direct)) {
    ng_fail(ng, (oc_string){ .ptr = (const uint8_t*)("native codegen error: system calls and win32_* imports take at most 6 register words"), .len = sizeof("native codegen error: system calls and win32_* imports take at most 6 register words") - 1 });
    return;
    }
    }
    if (str_eq(fn_name, (oc_string){ .ptr = (const uint8_t*)("sys_arena_poison"), .len = sizeof("sys_arena_poison") - 1 })) {
    buf_append_u8(code, 72);
    buf_append_u8(code, 199);
    buf_append_u8(code, 192);
    if (((ng)->arena_poison)) {
    elf_append_u32(code, 1);
    }
    else {
    elf_append_u32(code, 0);
    }
    return;
    }
    if (str_eq(fn_name, (oc_string){ .ptr = (const uint8_t*)("sys_host_target"), .len = sizeof("sys_host_target") - 1 })) {
    buf_append_u8(code, 72);
    buf_append_u8(code, 199);
    buf_append_u8(code, 192);
    elf_append_u32(code, ({ __auto_type _v = (((ng)->target)); if (_v < 0 || (uint64_t)_v > 4294967295ULL) { fprintf(stderr, "panic: invalid unsigned cast\n"); exit(1); } (uint32_t)_v; }));
    return;
    }
    if (is_sys) {
    if ((((ng)->target) == OC_TARGET_WINDOWS_X86_64)) {
    if ((sys_nr == 60)) {
    ng_emit_call(ng, code, (oc_string){ .ptr = (const uint8_t*)("__win_exit"), .len = sizeof("__win_exit") - 1 });
    return;
    }
    }
    else {
    buf_append_u8(code, 73);
    buf_append_u8(code, 137);
    buf_append_u8(code, 202);
    buf_append_u8(code, 72);
    buf_append_u8(code, 199);
    buf_append_u8(code, 192);
    elf_append_u32(code, sys_nr);
    buf_append_u8(code, 15);
    buf_append_u8(code, 5);
    return;
    }
    }
    bool is_win32 = false;
    if ((((fn_name).len) > 6)) {
    if (str_eq(__oc_slice_str(fn_name, 0, 6), (oc_string){ .ptr = (const uint8_t*)("win32_"), .len = sizeof("win32_") - 1 })) {
    is_win32 = true;
    }
    }
    if (is_win32) {
    int32_t slot = ng_imp_slot(__oc_slice_str(fn_name, 6, ((fn_name).len)));
    if ((((ng)->target) != OC_TARGET_WINDOWS_X86_64)) {
    ng_fail(ng, (oc_string){ .ptr = (const uint8_t*)("native codegen error: win32_* imports exist only on target windows-x86_64"), .len = sizeof("native codegen error: win32_* imports exist only on target windows-x86_64") - 1 });
    return;
    }
    if ((slot < 0)) {
    if ((!((ng)->has_error))) {
    fwrite("native codegen error: not a KERNEL32 import known to the PE emitter:", 1, sizeof("native codegen error: not a KERNEL32 import known to the PE emitter:") - 1, stdout); fputc('\n', stdout);
    __oc_print(fn_name);
    }
    ((ng)->has_error) = true;
    return;
    }
    buf_append_u8(code, 72);
    buf_append_u8(code, 141);
    buf_append_u8(code, 5);
    ng_add_iat_fixup(ng, ((code)->len), ({ __auto_type _v = (slot); if (_v < 0) { fprintf(stderr, "panic: invalid unsigned cast\n"); exit(1); } (size_t)_v; }));
    elf_append_u32(code, 0);
    ng_emit_call(ng, code, (oc_string){ .ptr = (const uint8_t*)("__win_call"), .len = sizeof("__win_call") - 1 });
    oc_string ret_t = ng_find_fn_ret_type(ng, fn_name);
    if (str_eq(ret_t, (oc_string){ .ptr = (const uint8_t*)("i32"), .len = sizeof("i32") - 1 })) {
    buf_append_u8(code, 72);
    buf_append_u8(code, 99);
    buf_append_u8(code, 192);
    }
    else {
    if (str_eq(ret_t, (oc_string){ .ptr = (const uint8_t*)("u32"), .len = sizeof("u32") - 1 })) {
    buf_append_u8(code, 137);
    buf_append_u8(code, 192);
    }
    }
    return;
    }
    ng_add_fixup(ng, ((code)->len), ((((expr)->tok)).slice), false);
    buf_append_u8(code, 232);
    elf_append_u32(code, 0);
    if (stack_args) {
    buf_append_u8(code, 72);
    buf_append_u8(code, 129);
    buf_append_u8(code, 196);
    elf_append_u32(code, ({ __auto_type _v = (__OC_MUL(reg_count, 8)); if (_v < 0 || (uint64_t)_v > 4294967295ULL) { fprintf(stderr, "panic: invalid unsigned cast\n"); exit(1); } (uint32_t)_v; }));
    }
    return;
    }
}

void ng_emit_print_lit(NativeGen* ng, CodeBuffer* code, CodeBuffer* data, oc_string raw) {
    size_t data_off = ((data)->len);
    size_t emitted_len = ng_append_unescaped_str(data, raw);
    buf_append_u8(data, 10);
    bool is_win = (((ng)->target) == OC_TARGET_WINDOWS_X86_64);
    if ((!is_win)) {
    buf_append_u8(code, 72);
    buf_append_u8(code, 199);
    buf_append_u8(code, 192);
    elf_append_u32(code, 1);
    }
    buf_append_u8(code, 72);
    buf_append_u8(code, 199);
    buf_append_u8(code, 199);
    elf_append_u32(code, 1);
    buf_append_u8(code, 72);
    buf_append_u8(code, 190);
    size_t patch_pos = ((code)->len);
    elf_append_u64(code, 0);
    ng_add_data_reloc(ng, patch_pos, data_off);
    buf_append_u8(code, 72);
    buf_append_u8(code, 199);
    buf_append_u8(code, 194);
    elf_append_u32(code, ({ __auto_type _v = (__OC_ADD(emitted_len, 1)); if (_v < 0 || (uint64_t)_v > 4294967295ULL) { fprintf(stderr, "panic: invalid unsigned cast\n"); exit(1); } (uint32_t)_v; }));
    if (is_win) {
    ng_emit_call(ng, code, (oc_string){ .ptr = (const uint8_t*)("__win_write"), .len = sizeof("__win_write") - 1 });
    }
    else {
    buf_append_u8(code, 15);
    buf_append_u8(code, 5);
    }
}

void ng_emit_stmt(NativeGen* ng, CodeBuffer* code, CodeBuffer* data, AstNode* stmt) {
    if ((stmt == ((AstNode*)(0)))) {
    return;
    }
    if ((((stmt)->left) == ((AstNode*)(0)))) {
    if (str_eq(((((stmt)->tok)).slice), (oc_string){ .ptr = (const uint8_t*)("break"), .len = sizeof("break") - 1 })) {
    buf_append_u8(code, 233);
    if ((((ng)->break_count) < 128)) {
    __OC_AT((((ng)->break_patches)), (((ng)->break_count))) = ((code)->len);
    __OC_AT((((ng)->break_depths)), (((ng)->break_count))) = ((ng)->loop_depth);
    ((ng)->break_count) = __OC_ADD(((ng)->break_count), 1);
    }
    elf_append_u32(code, 0);
    return;
    }
    if (str_eq(((((stmt)->tok)).slice), (oc_string){ .ptr = (const uint8_t*)("continue"), .len = sizeof("continue") - 1 })) {
    buf_append_u8(code, 233);
    size_t jmp_pos = ((code)->len);
    int32_t rel_back = ({ __auto_type _v = (__OC_SUB(({ __auto_type _v = (((ng)->loop_start)); if (!(_v < 0) && (uint64_t)_v > 9223372036854775807ULL) { fprintf(stderr, "panic: invalid signed cast\n"); exit(1); } (int64_t)_v; }), ({ __auto_type _v = (__OC_ADD(jmp_pos, 4)); if (!(_v < 0) && (uint64_t)_v > 9223372036854775807ULL) { fprintf(stderr, "panic: invalid signed cast\n"); exit(1); } (int64_t)_v; }))); if ((_v < 0 && (int64_t)_v < -2147483648LL) || (!(_v < 0) && (uint64_t)_v > 2147483647ULL)) { fprintf(stderr, "panic: invalid signed cast\n"); exit(1); } (int32_t)_v; });
    elf_append_i32(code, rel_back);
    return;
    }
    }
    if ((((stmt)->tag) == NODE_LET)) {
    oc_string t_name = ((stmt)->type_name);
    if ((((t_name).len) == 0)) {
    t_name = ng_expr_type(ng, ((stmt)->left));
    }
    bool is_slice = false;
    if (str_eq(t_name, (oc_string){ .ptr = (const uint8_t*)("string"), .len = sizeof("string") - 1 })) {
    is_slice = true;
    }
    else {
    if ((((stmt)->left) != ((AstNode*)(0)))) {
    if ((((((stmt)->left))->tag) == NODE_STRING)) {
    is_slice = true;
    t_name = (oc_string){ .ptr = (const uint8_t*)("string"), .len = sizeof("string") - 1 };
    }
    else {
    if ((((((stmt)->left))->tag) == NODE_SLICE)) {
    is_slice = true;
    t_name = (oc_string){ .ptr = (const uint8_t*)("string"), .len = sizeof("string") - 1 };
    }
    }
    }
    }
    if ((((t_name).len) > 0)) {
    if ((__oc_index_str(t_name, 0) == 91)) {
    size_t arr_len = ng_parse_array_len(t_name);
    oc_string elem_t = ng_parse_array_elem(t_name);
    int32_t stride = ng_type_stride(elem_t);
    int32_t raw_bytes = __OC_MUL(({ __auto_type _v = (arr_len); if ((_v < 0 && (int64_t)_v < -2147483648LL) || (!(_v < 0) && (uint64_t)_v > 2147483647ULL)) { fprintf(stderr, "panic: invalid signed cast\n"); exit(1); } (int32_t)_v; }), stride);
    int32_t qwords = __OC_DIV(__OC_ADD(raw_bytes, 7), 8);
    int32_t aligned_bytes = __OC_MUL(qwords, 8);
    ((ng)->curr_off) = __OC_SUB(((ng)->curr_off), aligned_bytes);
    int32_t base_off = ((ng)->curr_off);
    if ((((ng)->var_count) < 256)) {
    __OC_AT((((ng)->var_names)), (((ng)->var_count))) = ((((stmt)->tok)).slice);
    __OC_AT((((ng)->var_offsets)), (((ng)->var_count))) = base_off;
    __OC_AT((((ng)->var_is_slice)), (((ng)->var_count))) = false;
    __OC_AT((((ng)->var_types)), (((ng)->var_count))) = t_name;
    __OC_AT((((ng)->var_active)), (((ng)->var_count))) = true;
    __OC_AT((((ng)->var_scope)), (((ng)->var_count))) = ((ng)->scope_depth);
    __OC_AT((((ng)->var_arr_len)), (((ng)->var_count))) = arr_len;
    ((ng)->var_count) = __OC_ADD(((ng)->var_count), 1);
    }
    else {
    ng_fail(ng, (oc_string){ .ptr = (const uint8_t*)("native codegen error: too many variables in one function (256)"), .len = sizeof("native codegen error: too many variables in one function (256)") - 1 });
    }
    buf_append_u8(code, 72);
    buf_append_u8(code, 141);
    buf_append_u8(code, 189);
    elf_append_i32(code, base_off);
    buf_append_u8(code, 72);
    buf_append_u8(code, 199);
    buf_append_u8(code, 193);
    elf_append_u32(code, ({ __auto_type _v = (qwords); if (_v < 0 || (uint64_t)_v > 4294967295ULL) { fprintf(stderr, "panic: invalid unsigned cast\n"); exit(1); } (uint32_t)_v; }));
    buf_append_u8(code, 49);
    buf_append_u8(code, 192);
    buf_append_u8(code, 243);
    buf_append_u8(code, 72);
    buf_append_u8(code, 171);
    if ((((stmt)->left) != ((AstNode*)(0)))) {
    if ((((((stmt)->left))->tag) == NODE_ARRAY_INIT)) {
    AstNode* first_el = ((((stmt)->left))->left);
    bool skip_init = false;
    if ((first_el != ((AstNode*)(0)))) {
    if ((((first_el)->next) == ((AstNode*)(0)))) {
    if ((((first_el)->tag) == NODE_INT)) {
    if (str_eq(((((first_el)->tok)).slice), (oc_string){ .ptr = (const uint8_t*)("0"), .len = sizeof("0") - 1 })) {
    skip_init = true;
    }
    }
    if ((((first_el)->tag) == NODE_STRING)) {
    if (str_eq(((((first_el)->tok)).slice), (oc_string){ .ptr = (const uint8_t*)("\042\042"), .len = sizeof("\042\042") - 1 })) {
    skip_init = true;
    }
    }
    if ((((first_el)->tag) == NODE_BOOL)) {
    if (str_eq(((((first_el)->tok)).slice), (oc_string){ .ptr = (const uint8_t*)("false"), .len = sizeof("false") - 1 })) {
    skip_init = true;
    }
    }
    }
    }
    if ((!skip_init)) {
    AstNode* el = first_el;
    int32_t k = 0;
    while ((el != ((AstNode*)(0)))) {
    ng_emit_expr_as(ng, code, data, el, elem_t);
    int32_t el_off = __OC_ADD(base_off, __OC_MUL(k, stride));
    if ((stride == 1)) {
    buf_append_u8(code, 136);
    buf_append_u8(code, 133);
    elf_append_i32(code, el_off);
    }
    else {
    if ((stride == 16)) {
    ng_emit_disp_store_rax(code, el_off);
    ng_emit_disp_store_rdx(code, __OC_ADD(el_off, 8));
    }
    else {
    ng_emit_disp_store_rax(code, el_off);
    }
    }
    k = __OC_ADD(k, 1);
    el = ((el)->next);
    }
    }
    }
    }
    return;
    }
    }
    int32_t struct_sz = ng_find_struct_size(ng, t_name);
    if ((struct_sz > 0)) {
    ((ng)->curr_off) = __OC_SUB(((ng)->curr_off), struct_sz);
    int32_t base_off = ((ng)->curr_off);
    if ((((ng)->var_count) < 256)) {
    __OC_AT((((ng)->var_names)), (((ng)->var_count))) = ((((stmt)->tok)).slice);
    __OC_AT((((ng)->var_offsets)), (((ng)->var_count))) = base_off;
    __OC_AT((((ng)->var_is_slice)), (((ng)->var_count))) = false;
    __OC_AT((((ng)->var_types)), (((ng)->var_count))) = t_name;
    __OC_AT((((ng)->var_active)), (((ng)->var_count))) = true;
    __OC_AT((((ng)->var_scope)), (((ng)->var_count))) = ((ng)->scope_depth);
    __OC_AT((((ng)->var_arr_len)), (((ng)->var_count))) = 0;
    ((ng)->var_count) = __OC_ADD(((ng)->var_count), 1);
    }
    else {
    ng_fail(ng, (oc_string){ .ptr = (const uint8_t*)("native codegen error: too many variables in one function (256)"), .len = sizeof("native codegen error: too many variables in one function (256)") - 1 });
    }
    if ((((stmt)->left) != ((AstNode*)(0)))) {
    if ((((((stmt)->left))->tag) == NODE_STRUCT_INIT)) {
    buf_append_u8(code, 72);
    buf_append_u8(code, 49);
    buf_append_u8(code, 192);
    int32_t zb = 0;
    while ((__OC_ADD(zb, 8) <= struct_sz)) {
    ng_emit_store_disp(code, 0, 8, __OC_ADD(base_off, zb), false);
    zb = __OC_ADD(zb, 8);
    }
    while ((zb < struct_sz)) {
    ng_emit_store_disp(code, 0, 1, __OC_ADD(base_off, zb), false);
    zb = __OC_ADD(zb, 1);
    }
    AstNode* f = ((((stmt)->left))->left);
    while ((f != ((AstNode*)(0)))) {
    int32_t f_off = ng_find_field_offset(ng, t_name, ((((f)->tok)).slice));
    oc_string f_type = ng_find_field_type(ng, t_name, ((((f)->tok)).slice));
    if ((((f_type).len) > 0)) {
    if ((__oc_index_str(f_type, 0) == 91)) {
    if ((((((f)->left))->tag) == NODE_ARRAY_INIT)) {
    ng_emit_array_init_at(ng, code, data, __OC_ADD(base_off, f_off), f_type, ((f)->left), false);
    f = ((f)->next);
    continue;
    }
    }
    }
    ng_emit_expr_as(ng, code, data, ((f)->left), f_type);
    int32_t f_words = ng_value_words(ng, f_type);
    if ((f_words > 1)) {
    ng_emit_disp_store_rax(code, __OC_ADD(base_off, f_off));
    ng_emit_disp_store_rdx(code, __OC_ADD(__OC_ADD(base_off, f_off), 8));
    if ((f_words >= 3)) {
    ng_emit_disp_store_rcx(code, __OC_ADD(__OC_ADD(base_off, f_off), 16));
    }
    if ((f_words >= 4)) {
    ng_emit_disp_store_r8(code, __OC_ADD(__OC_ADD(base_off, f_off), 24));
    }
    }
    else {
    buf_append_u8(code, 72);
    buf_append_u8(code, 141);
    buf_append_u8(code, 141);
    elf_append_i32(code, __OC_ADD(base_off, f_off));
    ng_emit_store_at_rcx(code, ng_field_width(ng, t_name, ((((f)->tok)).slice)));
    }
    f = ((f)->next);
    }
    return;
    }
    ng_emit_expr(ng, code, data, ((stmt)->left));
    ng_emit_disp_store_rax(code, base_off);
    if ((struct_sz >= 16)) {
    ng_emit_disp_store_rdx(code, __OC_ADD(base_off, 8));
    }
    if ((struct_sz >= 24)) {
    ng_emit_disp_store_rcx(code, __OC_ADD(base_off, 16));
    }
    if ((struct_sz >= 32)) {
    ng_emit_disp_store_r8(code, __OC_ADD(base_off, 24));
    }
    }
    return;
    }
    ng_emit_expr_as(ng, code, data, ((stmt)->left), t_name);
    ng_emit_range_check(ng, code, t_name, (oc_string){ .ptr = (const uint8_t*)("__trap_overflow"), .len = sizeof("__trap_overflow") - 1 });
    ((ng)->curr_off) = __OC_SUB(((ng)->curr_off), 8);
    int32_t off = ((ng)->curr_off);
    if (is_slice) {
    ((ng)->curr_off) = __OC_SUB(((ng)->curr_off), 8);
    }
    if ((((ng)->var_count) < 256)) {
    __OC_AT((((ng)->var_names)), (((ng)->var_count))) = ((((stmt)->tok)).slice);
    __OC_AT((((ng)->var_offsets)), (((ng)->var_count))) = off;
    __OC_AT((((ng)->var_is_slice)), (((ng)->var_count))) = is_slice;
    __OC_AT((((ng)->var_types)), (((ng)->var_count))) = t_name;
    __OC_AT((((ng)->var_active)), (((ng)->var_count))) = true;
    __OC_AT((((ng)->var_scope)), (((ng)->var_count))) = ((ng)->scope_depth);
    __OC_AT((((ng)->var_arr_len)), (((ng)->var_count))) = 0;
    ((ng)->var_count) = __OC_ADD(((ng)->var_count), 1);
    }
    else {
    ng_fail(ng, (oc_string){ .ptr = (const uint8_t*)("native codegen error: too many variables in one function (256)"), .len = sizeof("native codegen error: too many variables in one function (256)") - 1 });
    }
    ng_emit_disp_store_rax(code, off);
    if (is_slice) {
    ng_emit_disp_store_rdx(code, __OC_SUB(off, 8));
    }
    return;
    }
    if ((((stmt)->tag) == NODE_ASSIGN)) {
    if ((((stmt)->left) != ((AstNode*)(0)))) {
    if ((((((stmt)->left))->tag) == NODE_IDENT)) {
    oc_string vt = ng_find_var_type(ng, ((((((stmt)->left))->tok)).slice));
    int32_t s_sz = ng_find_struct_size(ng, vt);
    int32_t off = ng_find_var_offset(ng, ((((((stmt)->left))->tok)).slice));
    ng_emit_expr_as(ng, code, data, ((stmt)->right), vt);
    if ((s_sz > 0)) {
    ng_emit_disp_store_rax(code, off);
    if ((s_sz >= 16)) {
    ng_emit_disp_store_rdx(code, __OC_ADD(off, 8));
    }
    if ((s_sz >= 24)) {
    ng_emit_disp_store_rcx(code, __OC_ADD(off, 16));
    }
    if ((s_sz >= 32)) {
    ng_emit_disp_store_r8(code, __OC_ADD(off, 24));
    }
    return;
    }
    ng_emit_range_check(ng, code, vt, (oc_string){ .ptr = (const uint8_t*)("__trap_overflow"), .len = sizeof("__trap_overflow") - 1 });
    ng_emit_disp_store_rax(code, off);
    if (ng_is_var_slice(ng, ((((((stmt)->left))->tok)).slice))) {
    ng_emit_disp_store_rdx(code, __OC_SUB(off, 8));
    }
    return;
    }
    oc_string lhs_t = ng_expr_type(ng, ((stmt)->left));
    ng_emit_expr_as(ng, code, data, ((stmt)->right), lhs_t);
    int32_t lhs_words = ng_value_words(ng, lhs_t);
    if ((lhs_words > 2)) {
    if ((lhs_words == 4)) {
    buf_append_u8(code, 65);
    buf_append_u8(code, 80);
    }
    buf_append_u8(code, 81);
    buf_append_u8(code, 82);
    buf_append_u8(code, 80);
    ng_emit_lvalue_addr(ng, code, data, ((stmt)->left));
    int32_t w = 0;
    while ((w < lhs_words)) {
    buf_append_u8(code, 90);
    buf_append_u8(code, 72);
    buf_append_u8(code, 137);
    buf_append_u8(code, 80);
    buf_append_u8(code, ({ __auto_type _v = (__OC_MUL(w, 8)); if (_v < 0 || (uint64_t)_v > 255ULL) { fprintf(stderr, "panic: invalid unsigned cast\n"); exit(1); } (uint8_t)_v; }));
    w = __OC_ADD(w, 1);
    }
    return;
    }
    ng_emit_range_check(ng, code, lhs_t, (oc_string){ .ptr = (const uint8_t*)("__trap_overflow"), .len = sizeof("__trap_overflow") - 1 });
    buf_append_u8(code, 80);
    if ((lhs_words == 2)) {
    buf_append_u8(code, 82);
    }
    ng_emit_lvalue_addr(ng, code, data, ((stmt)->left));
    buf_append_u8(code, 72);
    buf_append_u8(code, 137);
    buf_append_u8(code, 193);
    if ((lhs_words == 2)) {
    buf_append_u8(code, 90);
    buf_append_u8(code, 88);
    buf_append_u8(code, 72);
    buf_append_u8(code, 137);
    buf_append_u8(code, 1);
    buf_append_u8(code, 72);
    buf_append_u8(code, 137);
    buf_append_u8(code, 81);
    buf_append_u8(code, 8);
    return;
    }
    buf_append_u8(code, 88);
    int32_t width = 8;
    if ((((((stmt)->left))->tag) == NODE_FIELD_ACCESS)) {
    oc_string base_t = ng_strip_ptr_type(ng_expr_type(ng, ((((stmt)->left))->left)));
    width = ng_field_width(ng, base_t, ((((((stmt)->left))->tok)).slice));
    }
    else {
    if ((ng_scalar_width(lhs_t) == 1)) {
    width = 1;
    }
    }
    ng_emit_store_at_rcx(code, width);
    return;
    }
    return;
    }
    if ((((stmt)->tag) == NODE_RETURN)) {
    if ((((stmt)->left) != ((AstNode*)(0)))) {
    ng_emit_expr_as(ng, code, data, ((stmt)->left), ((ng)->cur_ret));
    }
    else {
    buf_append_u8(code, 72);
    buf_append_u8(code, 49);
    buf_append_u8(code, 192);
    }
    buf_append_u8(code, 72);
    buf_append_u8(code, 137);
    buf_append_u8(code, 236);
    buf_append_u8(code, 93);
    buf_append_u8(code, 195);
    return;
    }
    if ((((stmt)->tag) == NODE_IF)) {
    ng_emit_expr(ng, code, data, ((stmt)->left));
    buf_append_u8(code, 72);
    buf_append_u8(code, 133);
    buf_append_u8(code, 192);
    buf_append_u8(code, 15);
    buf_append_u8(code, 132);
    size_t jz_pos = ((code)->len);
    elf_append_u32(code, 0);
    ng_emit_stmt(ng, code, data, ((stmt)->right));
    if ((((stmt)->right) != ((AstNode*)(0)))) {
    if ((((((stmt)->right))->next) != ((AstNode*)(0)))) {
    buf_append_u8(code, 233);
    size_t jmp_pos = ((code)->len);
    elf_append_u32(code, 0);
    size_t else_target = ((code)->len);
    int32_t jz_rel = ({ __auto_type _v = (__OC_SUB(({ __auto_type _v = (else_target); if (!(_v < 0) && (uint64_t)_v > 9223372036854775807ULL) { fprintf(stderr, "panic: invalid signed cast\n"); exit(1); } (int64_t)_v; }), ({ __auto_type _v = (__OC_ADD(jz_pos, 4)); if (!(_v < 0) && (uint64_t)_v > 9223372036854775807ULL) { fprintf(stderr, "panic: invalid signed cast\n"); exit(1); } (int64_t)_v; }))); if ((_v < 0 && (int64_t)_v < -2147483648LL) || (!(_v < 0) && (uint64_t)_v > 2147483647ULL)) { fprintf(stderr, "panic: invalid signed cast\n"); exit(1); } (int32_t)_v; });
    elf_patch_i32(code, jz_pos, jz_rel);
    ng_emit_stmt(ng, code, data, ((((stmt)->right))->next));
    size_t if_end = ((code)->len);
    int32_t jmp_rel = ({ __auto_type _v = (__OC_SUB(({ __auto_type _v = (if_end); if (!(_v < 0) && (uint64_t)_v > 9223372036854775807ULL) { fprintf(stderr, "panic: invalid signed cast\n"); exit(1); } (int64_t)_v; }), ({ __auto_type _v = (__OC_ADD(jmp_pos, 4)); if (!(_v < 0) && (uint64_t)_v > 9223372036854775807ULL) { fprintf(stderr, "panic: invalid signed cast\n"); exit(1); } (int64_t)_v; }))); if ((_v < 0 && (int64_t)_v < -2147483648LL) || (!(_v < 0) && (uint64_t)_v > 2147483647ULL)) { fprintf(stderr, "panic: invalid signed cast\n"); exit(1); } (int32_t)_v; });
    elf_patch_i32(code, jmp_pos, jmp_rel);
    }
    else {
    size_t if_end = ((code)->len);
    int32_t jz_rel = ({ __auto_type _v = (__OC_SUB(({ __auto_type _v = (if_end); if (!(_v < 0) && (uint64_t)_v > 9223372036854775807ULL) { fprintf(stderr, "panic: invalid signed cast\n"); exit(1); } (int64_t)_v; }), ({ __auto_type _v = (__OC_ADD(jz_pos, 4)); if (!(_v < 0) && (uint64_t)_v > 9223372036854775807ULL) { fprintf(stderr, "panic: invalid signed cast\n"); exit(1); } (int64_t)_v; }))); if ((_v < 0 && (int64_t)_v < -2147483648LL) || (!(_v < 0) && (uint64_t)_v > 2147483647ULL)) { fprintf(stderr, "panic: invalid signed cast\n"); exit(1); } (int32_t)_v; });
    elf_patch_i32(code, jz_pos, jz_rel);
    }
    }
    else {
    size_t if_end = ((code)->len);
    int32_t jz_rel = ({ __auto_type _v = (__OC_SUB(({ __auto_type _v = (if_end); if (!(_v < 0) && (uint64_t)_v > 9223372036854775807ULL) { fprintf(stderr, "panic: invalid signed cast\n"); exit(1); } (int64_t)_v; }), ({ __auto_type _v = (__OC_ADD(jz_pos, 4)); if (!(_v < 0) && (uint64_t)_v > 9223372036854775807ULL) { fprintf(stderr, "panic: invalid signed cast\n"); exit(1); } (int64_t)_v; }))); if ((_v < 0 && (int64_t)_v < -2147483648LL) || (!(_v < 0) && (uint64_t)_v > 2147483647ULL)) { fprintf(stderr, "panic: invalid signed cast\n"); exit(1); } (int32_t)_v; });
    elf_patch_i32(code, jz_pos, jz_rel);
    }
    return;
    }
    if ((((stmt)->tag) == NODE_WHILE)) {
    size_t prev_loop_start = ((ng)->loop_start);
    ((ng)->loop_depth) = __OC_ADD(((ng)->loop_depth), 1);
    size_t loop_start = ((code)->len);
    ((ng)->loop_start) = loop_start;
    ng_emit_expr(ng, code, data, ((stmt)->left));
    buf_append_u8(code, 72);
    buf_append_u8(code, 133);
    buf_append_u8(code, 192);
    buf_append_u8(code, 15);
    buf_append_u8(code, 132);
    size_t jz_pos = ((code)->len);
    elf_append_u32(code, 0);
    ng_emit_stmt(ng, code, data, ((stmt)->right));
    buf_append_u8(code, 233);
    size_t jmp_pos = ((code)->len);
    int32_t rel_back = ({ __auto_type _v = (__OC_SUB(({ __auto_type _v = (loop_start); if (!(_v < 0) && (uint64_t)_v > 9223372036854775807ULL) { fprintf(stderr, "panic: invalid signed cast\n"); exit(1); } (int64_t)_v; }), ({ __auto_type _v = (__OC_ADD(jmp_pos, 4)); if (!(_v < 0) && (uint64_t)_v > 9223372036854775807ULL) { fprintf(stderr, "panic: invalid signed cast\n"); exit(1); } (int64_t)_v; }))); if ((_v < 0 && (int64_t)_v < -2147483648LL) || (!(_v < 0) && (uint64_t)_v > 2147483647ULL)) { fprintf(stderr, "panic: invalid signed cast\n"); exit(1); } (int32_t)_v; });
    elf_append_i32(code, rel_back);
    size_t loop_end = ((code)->len);
    int32_t jz_rel = ({ __auto_type _v = (__OC_SUB(({ __auto_type _v = (loop_end); if (!(_v < 0) && (uint64_t)_v > 9223372036854775807ULL) { fprintf(stderr, "panic: invalid signed cast\n"); exit(1); } (int64_t)_v; }), ({ __auto_type _v = (__OC_ADD(jz_pos, 4)); if (!(_v < 0) && (uint64_t)_v > 9223372036854775807ULL) { fprintf(stderr, "panic: invalid signed cast\n"); exit(1); } (int64_t)_v; }))); if ((_v < 0 && (int64_t)_v < -2147483648LL) || (!(_v < 0) && (uint64_t)_v > 2147483647ULL)) { fprintf(stderr, "panic: invalid signed cast\n"); exit(1); } (int32_t)_v; });
    elf_patch_i32(code, jz_pos, jz_rel);
    size_t bi = 0;
    while ((bi < ((ng)->break_count))) {
    if ((__OC_AT((((ng)->break_depths)), (bi)) == ((ng)->loop_depth))) {
    size_t b_pos = __OC_AT((((ng)->break_patches)), (bi));
    int32_t b_rel = ({ __auto_type _v = (__OC_SUB(({ __auto_type _v = (loop_end); if (!(_v < 0) && (uint64_t)_v > 9223372036854775807ULL) { fprintf(stderr, "panic: invalid signed cast\n"); exit(1); } (int64_t)_v; }), ({ __auto_type _v = (__OC_ADD(b_pos, 4)); if (!(_v < 0) && (uint64_t)_v > 9223372036854775807ULL) { fprintf(stderr, "panic: invalid signed cast\n"); exit(1); } (int64_t)_v; }))); if ((_v < 0 && (int64_t)_v < -2147483648LL) || (!(_v < 0) && (uint64_t)_v > 2147483647ULL)) { fprintf(stderr, "panic: invalid signed cast\n"); exit(1); } (int32_t)_v; });
    elf_patch_i32(code, b_pos, b_rel);
    __OC_AT((((ng)->break_depths)), (bi)) = (-1);
    }
    bi = __OC_ADD(bi, 1);
    }
    ((ng)->loop_depth) = __OC_SUB(((ng)->loop_depth), 1);
    ((ng)->loop_start) = prev_loop_start;
    return;
    }
    if ((((stmt)->tag) == NODE_BLOCK)) {
    ((ng)->scope_depth) = __OC_ADD(((ng)->scope_depth), 1);
    AstNode* s = ((stmt)->left);
    while ((s != ((AstNode*)(0)))) {
    ng_emit_stmt(ng, code, data, s);
    s = ((s)->next);
    }
    size_t vi = 0;
    while ((vi < ((ng)->var_count))) {
    if (__OC_AT((((ng)->var_active)), (vi))) {
    if ((__OC_AT((((ng)->var_scope)), (vi)) == ((ng)->scope_depth))) {
    __OC_AT((((ng)->var_active)), (vi)) = false;
    }
    }
    vi = __OC_ADD(vi, 1);
    }
    ((ng)->scope_depth) = __OC_SUB(((ng)->scope_depth), 1);
    return;
    }
    if ((((stmt)->tag) == NODE_PRINT)) {
    AstNode* arg = ((stmt)->left);
    if ((arg != ((AstNode*)(0)))) {
    if ((((arg)->tag) == NODE_STRING)) {
    oc_string raw = ((((arg)->tok)).slice);
    if ((((raw).len) >= 2)) {
    if ((__oc_index_str(raw, 0) == 34)) {
    raw = __oc_slice_str(raw, 1, __OC_SUB(((raw).len), 1));
    }
    }
    ng_emit_print_lit(ng, code, data, raw);
    return;
    }
    else {
    if (ng_is_expr_slice(ng, arg)) {
    ng_emit_expr(ng, code, data, arg);
    buf_append_u8(code, 72);
    buf_append_u8(code, 137);
    buf_append_u8(code, 199);
    buf_append_u8(code, 72);
    buf_append_u8(code, 137);
    buf_append_u8(code, 214);
    ng_add_fixup(ng, ((code)->len), (oc_string){ .ptr = (const uint8_t*)("__print_slice"), .len = sizeof("__print_slice") - 1 }, false);
    buf_append_u8(code, 232);
    elf_append_u32(code, 0);
    return;
    }
    else {
    oc_string arg_t = ng_expr_type(ng, arg);
    if (is_float_type(arg_t)) {
    ng_emit_expr_as(ng, code, data, arg, (oc_string){ .ptr = (const uint8_t*)("f64"), .len = sizeof("f64") - 1 });
    buf_append_u8(code, 72);
    buf_append_u8(code, 137);
    buf_append_u8(code, 199);
    ng_emit_call(ng, code, (oc_string){ .ptr = (const uint8_t*)("fmt_print_f64"), .len = sizeof("fmt_print_f64") - 1 });
    return;
    }
    if (str_eq(arg_t, (oc_string){ .ptr = (const uint8_t*)("bool"), .len = sizeof("bool") - 1 })) {
    ng_emit_expr(ng, code, data, arg);
    buf_append_u8(code, 72);
    buf_append_u8(code, 133);
    buf_append_u8(code, 192);
    buf_append_u8(code, 15);
    buf_append_u8(code, 132);
    size_t jz_pos = ((code)->len);
    elf_append_u32(code, 0);
    ng_emit_print_lit(ng, code, data, (oc_string){ .ptr = (const uint8_t*)("true"), .len = sizeof("true") - 1 });
    buf_append_u8(code, 233);
    size_t jmp_pos = ((code)->len);
    elf_append_u32(code, 0);
    elf_patch_i32(code, jz_pos, ({ __auto_type _v = (__OC_SUB(({ __auto_type _v = (((code)->len)); if (!(_v < 0) && (uint64_t)_v > 9223372036854775807ULL) { fprintf(stderr, "panic: invalid signed cast\n"); exit(1); } (int64_t)_v; }), ({ __auto_type _v = (__OC_ADD(jz_pos, 4)); if (!(_v < 0) && (uint64_t)_v > 9223372036854775807ULL) { fprintf(stderr, "panic: invalid signed cast\n"); exit(1); } (int64_t)_v; }))); if ((_v < 0 && (int64_t)_v < -2147483648LL) || (!(_v < 0) && (uint64_t)_v > 2147483647ULL)) { fprintf(stderr, "panic: invalid signed cast\n"); exit(1); } (int32_t)_v; }));
    ng_emit_print_lit(ng, code, data, (oc_string){ .ptr = (const uint8_t*)("false"), .len = sizeof("false") - 1 });
    elf_patch_i32(code, jmp_pos, ({ __auto_type _v = (__OC_SUB(({ __auto_type _v = (((code)->len)); if (!(_v < 0) && (uint64_t)_v > 9223372036854775807ULL) { fprintf(stderr, "panic: invalid signed cast\n"); exit(1); } (int64_t)_v; }), ({ __auto_type _v = (__OC_ADD(jmp_pos, 4)); if (!(_v < 0) && (uint64_t)_v > 9223372036854775807ULL) { fprintf(stderr, "panic: invalid signed cast\n"); exit(1); } (int64_t)_v; }))); if ((_v < 0 && (int64_t)_v < -2147483648LL) || (!(_v < 0) && (uint64_t)_v > 2147483647ULL)) { fprintf(stderr, "panic: invalid signed cast\n"); exit(1); } (int32_t)_v; }));
    return;
    }
    if (is_u8_array_type(arg_t)) {
    ng_emit_lvalue_addr(ng, code, data, arg);
    buf_append_u8(code, 72);
    buf_append_u8(code, 137);
    buf_append_u8(code, 199);
    buf_append_u8(code, 49);
    buf_append_u8(code, 246);
    buf_append_u8(code, 72);
    buf_append_u8(code, 129);
    buf_append_u8(code, 254);
    elf_append_u32(code, ({ __auto_type _v = (ng_parse_array_len(arg_t)); if (_v < 0 || (uint64_t)_v > 4294967295ULL) { fprintf(stderr, "panic: invalid unsigned cast\n"); exit(1); } (uint32_t)_v; }));
    buf_append_u8(code, 115);
    buf_append_u8(code, 11);
    buf_append_u8(code, 128);
    buf_append_u8(code, 60);
    buf_append_u8(code, 55);
    buf_append_u8(code, 0);
    buf_append_u8(code, 116);
    buf_append_u8(code, 5);
    buf_append_u8(code, 72);
    buf_append_u8(code, 255);
    buf_append_u8(code, 198);
    buf_append_u8(code, 235);
    buf_append_u8(code, 236);
    ng_emit_call(ng, code, (oc_string){ .ptr = (const uint8_t*)("__print_slice"), .len = sizeof("__print_slice") - 1 });
    return;
    }
    ng_emit_expr(ng, code, data, arg);
    buf_append_u8(code, 72);
    buf_append_u8(code, 137);
    buf_append_u8(code, 199);
    bool use_i64_print = false;
    if ((!ng_is_unsigned_expr(ng, arg))) {
    if ((!str_eq(arg_t, (oc_string){ .ptr = (const uint8_t*)("bool"), .len = sizeof("bool") - 1 }))) {
    use_i64_print = true;
    }
    }
    if (use_i64_print) {
    ng_add_fixup(ng, ((code)->len), (oc_string){ .ptr = (const uint8_t*)("__print_i64"), .len = sizeof("__print_i64") - 1 }, false);
    }
    else {
    ng_add_fixup(ng, ((code)->len), (oc_string){ .ptr = (const uint8_t*)("__print_u64"), .len = sizeof("__print_u64") - 1 }, false);
    }
    buf_append_u8(code, 232);
    elf_append_u32(code, 0);
    return;
    }
    }
    }
    return;
    }
    ng_emit_expr(ng, code, data, stmt);
}

int32_t calc_locals_bytes_stmt(NativeGen* ng, AstNode* stmt) {
    if ((stmt == ((AstNode*)(0)))) {
    return 0;
    }
    if ((((stmt)->tag) == NODE_LET)) {
    oc_string t_name = ((stmt)->type_name);
    if (str_eq(t_name, (oc_string){ .ptr = (const uint8_t*)("string"), .len = sizeof("string") - 1 })) {
    return 16;
    }
    if ((((stmt)->left) != ((AstNode*)(0)))) {
    if ((((((stmt)->left))->tag) == NODE_STRING)) {
    return 16;
    }
    if ((((((stmt)->left))->tag) == NODE_SLICE)) {
    return 16;
    }
    }
    if ((((t_name).len) > 0)) {
    if ((__oc_index_str(t_name, 0) == 91)) {
    size_t arr_len = ng_parse_array_len(t_name);
    oc_string elem_t = ng_parse_array_elem(t_name);
    int32_t stride = ng_type_stride(elem_t);
    int32_t raw_bytes = __OC_MUL(({ __auto_type _v = (arr_len); if ((_v < 0 && (int64_t)_v < -2147483648LL) || (!(_v < 0) && (uint64_t)_v > 2147483647ULL)) { fprintf(stderr, "panic: invalid signed cast\n"); exit(1); } (int32_t)_v; }), stride);
    return __OC_MUL(__OC_DIV(__OC_ADD(raw_bytes, 7), 8), 8);
    }
    }
    int32_t s_sz = ng_find_struct_size(ng, t_name);
    if ((s_sz > 0)) {
    return s_sz;
    }
    return 8;
    }
    if ((((stmt)->tag) == NODE_BLOCK)) {
    int32_t total = 0;
    AstNode* s = ((stmt)->left);
    while ((s != ((AstNode*)(0)))) {
    total = __OC_ADD(total, calc_locals_bytes_stmt(ng, s));
    s = ((s)->next);
    }
    return total;
    }
    if ((((stmt)->tag) == NODE_IF)) {
    int32_t total = calc_locals_bytes_stmt(ng, ((stmt)->right));
    if ((((stmt)->right) != ((AstNode*)(0)))) {
    if ((((((stmt)->right))->next) != ((AstNode*)(0)))) {
    total = __OC_ADD(total, calc_locals_bytes_stmt(ng, ((((stmt)->right))->next)));
    }
    }
    return total;
    }
    if ((((stmt)->tag) == NODE_WHILE)) {
    return calc_locals_bytes_stmt(ng, ((stmt)->right));
    }
    return 0;
}

void ng_emit_store_param_reg(CodeBuffer* code, size_t reg_idx, int32_t off) {
    if ((reg_idx == 0)) {
    buf_append_u8(code, 72);
    buf_append_u8(code, 137);
    buf_append_u8(code, 189);
    elf_append_i32(code, off);
    }
    else {
    if ((reg_idx == 1)) {
    buf_append_u8(code, 72);
    buf_append_u8(code, 137);
    buf_append_u8(code, 181);
    elf_append_i32(code, off);
    }
    else {
    if ((reg_idx == 2)) {
    buf_append_u8(code, 72);
    buf_append_u8(code, 137);
    buf_append_u8(code, 149);
    elf_append_i32(code, off);
    }
    else {
    if ((reg_idx == 3)) {
    buf_append_u8(code, 72);
    buf_append_u8(code, 137);
    buf_append_u8(code, 141);
    elf_append_i32(code, off);
    }
    else {
    if ((reg_idx == 4)) {
    buf_append_u8(code, 76);
    buf_append_u8(code, 137);
    buf_append_u8(code, 133);
    elf_append_i32(code, off);
    }
    else {
    if ((reg_idx == 5)) {
    buf_append_u8(code, 76);
    buf_append_u8(code, 137);
    buf_append_u8(code, 141);
    elf_append_i32(code, off);
    }
    }
    }
    }
    }
    }
}

void ng_emit_load_arg_reg(CodeBuffer* code, size_t reg_idx, uint32_t disp) {
    uint8_t rex = 72;
    uint8_t modrm = 188;
    if ((reg_idx == 1)) {
    modrm = 180;
    }
    else {
    if ((reg_idx == 2)) {
    modrm = 148;
    }
    else {
    if ((reg_idx == 3)) {
    modrm = 140;
    }
    else {
    if ((reg_idx == 4)) {
    rex = 76;
    modrm = 132;
    }
    else {
    if ((reg_idx == 5)) {
    rex = 76;
    modrm = 140;
    }
    }
    }
    }
    }
    buf_append_u8(code, rex);
    buf_append_u8(code, 139);
    buf_append_u8(code, modrm);
    buf_append_u8(code, 36);
    elf_append_u32(code, disp);
}

void ng_emit_store_param(CodeBuffer* code, size_t reg_idx, size_t total, int32_t off) {
    if ((reg_idx < 6)) {
    ng_emit_store_param_reg(code, reg_idx, off);
    return;
    }
    ng_emit_disp_load_rax(code, ({ __auto_type _v = (__OC_ADD(16, __OC_MUL(__OC_SUB(__OC_SUB(total, 1), reg_idx), 8))); if ((_v < 0 && (int64_t)_v < -2147483648LL) || (!(_v < 0) && (uint64_t)_v > 2147483647ULL)) { fprintf(stderr, "panic: invalid signed cast\n"); exit(1); } (int32_t)_v; }));
    ng_emit_disp_store_rax(code, off);
}

void ng_define_helper(NativeGen* ng, bool* has_offset, oc_string name, size_t off) {
    if ((((ng)->fn_count) < 1024)) {
    __OC_AT((((ng)->fn_names)), (((ng)->fn_count))) = name;
    __OC_AT((has_offset), (((ng)->fn_count))) = true;
    __OC_AT((((ng)->fn_offsets)), (((ng)->fn_count))) = off;
    ((ng)->fn_count) = __OC_ADD(((ng)->fn_count), 1);
    }
    else {
    ng_fail(ng, (oc_string){ .ptr = (const uint8_t*)("native codegen error: too many functions (1024)"), .len = sizeof("native codegen error: too many functions (1024)") - 1 });
    }
}

void ng_emit_write_fd(NativeGen* ng, CodeBuffer* code) {
    if ((((ng)->target) == OC_TARGET_WINDOWS_X86_64)) {
    ng_emit_call(ng, code, (oc_string){ .ptr = (const uint8_t*)("__win_write"), .len = sizeof("__win_write") - 1 });
    return;
    }
    buf_append_u8(code, 72);
    buf_append_u8(code, 199);
    buf_append_u8(code, 192);
    elf_append_u32(code, 1);
    buf_append_u8(code, 15);
    buf_append_u8(code, 5);
}

void ng_emit_win_runtime(NativeGen* ng, CodeBuffer* code, bool* has_offset) {
    ng_define_helper(ng, has_offset, (oc_string){ .ptr = (const uint8_t*)("__win_write"), .len = sizeof("__win_write") - 1 }, ((code)->len));
    buf_append_u8(code, 85);
    buf_append_u8(code, 72);
    buf_append_u8(code, 137);
    buf_append_u8(code, 229);
    buf_append_u8(code, 72);
    buf_append_u8(code, 131);
    buf_append_u8(code, 228);
    buf_append_u8(code, 240);
    buf_append_u8(code, 72);
    buf_append_u8(code, 131);
    buf_append_u8(code, 236);
    buf_append_u8(code, 64);
    buf_append_u8(code, 72);
    buf_append_u8(code, 137);
    buf_append_u8(code, 84);
    buf_append_u8(code, 36);
    buf_append_u8(code, 56);
    buf_append_u8(code, 72);
    buf_append_u8(code, 199);
    buf_append_u8(code, 193);
    elf_append_i32(code, (-10));
    buf_append_u8(code, 72);
    buf_append_u8(code, 41);
    buf_append_u8(code, 249);
    ng_emit_iat_call(ng, code, NG_IMP_GET_STD_HANDLE);
    buf_append_u8(code, 72);
    buf_append_u8(code, 137);
    buf_append_u8(code, 193);
    buf_append_u8(code, 72);
    buf_append_u8(code, 137);
    buf_append_u8(code, 242);
    buf_append_u8(code, 76);
    buf_append_u8(code, 139);
    buf_append_u8(code, 68);
    buf_append_u8(code, 36);
    buf_append_u8(code, 56);
    buf_append_u8(code, 76);
    buf_append_u8(code, 141);
    buf_append_u8(code, 76);
    buf_append_u8(code, 36);
    buf_append_u8(code, 48);
    buf_append_u8(code, 72);
    buf_append_u8(code, 199);
    buf_append_u8(code, 68);
    buf_append_u8(code, 36);
    buf_append_u8(code, 32);
    elf_append_u32(code, 0);
    ng_emit_iat_call(ng, code, NG_IMP_WRITE_FILE);
    buf_append_u8(code, 72);
    buf_append_u8(code, 137);
    buf_append_u8(code, 236);
    buf_append_u8(code, 93);
    buf_append_u8(code, 195);
    ng_define_helper(ng, has_offset, (oc_string){ .ptr = (const uint8_t*)("__win_call"), .len = sizeof("__win_call") - 1 }, ((code)->len));
    buf_append_u8(code, 85);
    buf_append_u8(code, 72);
    buf_append_u8(code, 137);
    buf_append_u8(code, 229);
    buf_append_u8(code, 72);
    buf_append_u8(code, 131);
    buf_append_u8(code, 228);
    buf_append_u8(code, 240);
    buf_append_u8(code, 72);
    buf_append_u8(code, 131);
    buf_append_u8(code, 236);
    buf_append_u8(code, 64);
    buf_append_u8(code, 76);
    buf_append_u8(code, 137);
    buf_append_u8(code, 68);
    buf_append_u8(code, 36);
    buf_append_u8(code, 32);
    buf_append_u8(code, 76);
    buf_append_u8(code, 137);
    buf_append_u8(code, 76);
    buf_append_u8(code, 36);
    buf_append_u8(code, 40);
    buf_append_u8(code, 72);
    buf_append_u8(code, 199);
    buf_append_u8(code, 68);
    buf_append_u8(code, 36);
    buf_append_u8(code, 48);
    elf_append_u32(code, 0);
    buf_append_u8(code, 73);
    buf_append_u8(code, 137);
    buf_append_u8(code, 201);
    buf_append_u8(code, 73);
    buf_append_u8(code, 137);
    buf_append_u8(code, 208);
    buf_append_u8(code, 72);
    buf_append_u8(code, 137);
    buf_append_u8(code, 242);
    buf_append_u8(code, 72);
    buf_append_u8(code, 137);
    buf_append_u8(code, 249);
    buf_append_u8(code, 255);
    buf_append_u8(code, 16);
    buf_append_u8(code, 72);
    buf_append_u8(code, 137);
    buf_append_u8(code, 236);
    buf_append_u8(code, 93);
    buf_append_u8(code, 195);
    ng_define_helper(ng, has_offset, (oc_string){ .ptr = (const uint8_t*)("__win_exit"), .len = sizeof("__win_exit") - 1 }, ((code)->len));
    buf_append_u8(code, 72);
    buf_append_u8(code, 131);
    buf_append_u8(code, 228);
    buf_append_u8(code, 240);
    buf_append_u8(code, 72);
    buf_append_u8(code, 131);
    buf_append_u8(code, 236);
    buf_append_u8(code, 32);
    buf_append_u8(code, 137);
    buf_append_u8(code, 249);
    ng_emit_iat_call(ng, code, NG_IMP_EXIT_PROCESS);
    buf_append_u8(code, 204);
}

size_t pe_align(size_t v, size_t a) {
    return __OC_MUL(__OC_DIV(__OC_SUB(__OC_ADD(v, a), 1), a), a);
}

void pe_append_zeros(CodeBuffer* buf, size_t n) {
    size_t i = 0;
    while ((i < n)) {
    buf_append_u8(buf, 0);
    i = __OC_ADD(i, 1);
    }
}

void pe_append_section_header(CodeBuffer* pe, oc_string name, size_t vsize, size_t rva, size_t raw_size, size_t raw_off, uint32_t flags) {
    size_t i = 0;
    while ((i < 8)) {
    if ((i < ((name).len))) {
    buf_append_u8(pe, __oc_index_str(name, i));
    }
    else {
    buf_append_u8(pe, 0);
    }
    i = __OC_ADD(i, 1);
    }
    elf_append_u32(pe, ({ __auto_type _v = (vsize); if (_v < 0 || (uint64_t)_v > 4294967295ULL) { fprintf(stderr, "panic: invalid unsigned cast\n"); exit(1); } (uint32_t)_v; }));
    elf_append_u32(pe, ({ __auto_type _v = (rva); if (_v < 0 || (uint64_t)_v > 4294967295ULL) { fprintf(stderr, "panic: invalid unsigned cast\n"); exit(1); } (uint32_t)_v; }));
    elf_append_u32(pe, ({ __auto_type _v = (raw_size); if (_v < 0 || (uint64_t)_v > 4294967295ULL) { fprintf(stderr, "panic: invalid unsigned cast\n"); exit(1); } (uint32_t)_v; }));
    elf_append_u32(pe, ({ __auto_type _v = (raw_off); if (_v < 0 || (uint64_t)_v > 4294967295ULL) { fprintf(stderr, "panic: invalid unsigned cast\n"); exit(1); } (uint32_t)_v; }));
    elf_append_u32(pe, 0);
    elf_append_u32(pe, 0);
    elf_append_u16(pe, 0);
    elf_append_u16(pe, 0);
    elf_append_u32(pe, flags);
}

bool ng_write_pe(NativeGen* ng, CodeBuffer* code, CodeBuffer* data, oc_string out_file_str) {
    oc_string dll_name = (oc_string){ .ptr = (const uint8_t*)("KERNEL32.dll"), .len = sizeof("KERNEL32.dll") - 1 };
    size_t thunk_bytes = __OC_MUL(__OC_ADD(NG_IMP_COUNT, 1), 8);
    size_t iat_off = 0;
    size_t ilt_off = __OC_ADD(iat_off, thunk_bytes);
    size_t idt_off = __OC_ADD(ilt_off, thunk_bytes);
    size_t idt_bytes = __OC_MUL(2, 20);
    size_t hint_offs[10] = {0};
    size_t cur = __OC_ADD(idt_off, idt_bytes);
    size_t k = 0;
    while ((k < NG_IMP_COUNT)) {
    __OC_AT((hint_offs), (k)) = cur;
    cur = pe_align(__OC_ADD(__OC_ADD(__OC_ADD(cur, 2), ((ng_imp_name(k)).len)), 1), 2);
    k = __OC_ADD(k, 1);
    }
    size_t dll_off = cur;
    size_t udata_off = pe_align(__OC_ADD(__OC_ADD(dll_off, ((dll_name).len)), 1), 16);
    size_t rdata_len = __OC_ADD(udata_off, ((data)->len));
    size_t file_align = 512;
    size_t sect_align = 4096;
    size_t headers_size = 512;
    size_t text_rva = 4096;
    size_t text_raw_off = headers_size;
    size_t text_raw_size = pe_align(((code)->len), file_align);
    size_t rdata_rva = __OC_ADD(text_rva, pe_align(((code)->len), sect_align));
    size_t rdata_raw_off = __OC_ADD(text_raw_off, text_raw_size);
    size_t rdata_raw_size = pe_align(rdata_len, file_align);
    size_t image_size = __OC_ADD(rdata_rva, pe_align(rdata_len, sect_align));
    size_t file_size = __OC_ADD(rdata_raw_off, rdata_raw_size);
    size_t fi = 0;
    while ((fi < ((ng)->iat_fix_count))) {
    size_t pos = __OC_AT((((ng)->iat_fix_code)), (fi));
    size_t slot_rva = __OC_ADD(__OC_ADD(rdata_rva, iat_off), __OC_MUL(__OC_AT((((ng)->iat_fix_slot)), (fi)), 8));
    int64_t rel = __OC_SUB(({ __auto_type _v = (slot_rva); if (!(_v < 0) && (uint64_t)_v > 9223372036854775807ULL) { fprintf(stderr, "panic: invalid signed cast\n"); exit(1); } (int64_t)_v; }), ({ __auto_type _v = (__OC_ADD(__OC_ADD(text_rva, pos), 4)); if (!(_v < 0) && (uint64_t)_v > 9223372036854775807ULL) { fprintf(stderr, "panic: invalid signed cast\n"); exit(1); } (int64_t)_v; }));
    elf_patch_i32(code, pos, ({ __auto_type _v = (rel); if ((_v < 0 && (int64_t)_v < -2147483648LL) || (!(_v < 0) && (uint64_t)_v > 2147483647ULL)) { fprintf(stderr, "panic: invalid signed cast\n"); exit(1); } (int32_t)_v; }));
    fi = __OC_ADD(fi, 1);
    }
    size_t ri = 0;
    while ((ri < ((ng)->data_reloc_count))) {
    size_t c_pos = __OC_AT((((ng)->data_reloc_code)), (ri));
    bool form_ok = false;
    if ((c_pos >= 2)) {
    if ((__OC_AT((((code)->buffer)), (__OC_SUB(c_pos, 2))) == 72)) {
    if ((__OC_AT((((code)->buffer)), (__OC_SUB(c_pos, 1))) >= 184)) {
    if ((__OC_AT((((code)->buffer)), (__OC_SUB(c_pos, 1))) <= 191)) {
    form_ok = true;
    }
    }
    }
    }
    if ((!form_ok)) {
    fwrite("native codegen error: unexpected data relocation form for PE32+", 1, sizeof("native codegen error: unexpected data relocation form for PE32+") - 1, stdout); fputc('\n', stdout);
    return false;
    }
    uint8_t reg = __OC_SUB(__OC_AT((((code)->buffer)), (__OC_SUB(c_pos, 1))), 184);
    size_t target_rva = __OC_ADD(__OC_ADD(rdata_rva, udata_off), __OC_AT((((ng)->data_reloc_data)), (ri)));
    int64_t rel = __OC_SUB(({ __auto_type _v = (target_rva); if (!(_v < 0) && (uint64_t)_v > 9223372036854775807ULL) { fprintf(stderr, "panic: invalid signed cast\n"); exit(1); } (int64_t)_v; }), ({ __auto_type _v = (__OC_ADD(__OC_ADD(text_rva, c_pos), 5)); if (!(_v < 0) && (uint64_t)_v > 9223372036854775807ULL) { fprintf(stderr, "panic: invalid signed cast\n"); exit(1); } (int64_t)_v; }));
    __OC_AT((((code)->buffer)), (__OC_SUB(c_pos, 1))) = 141;
    __OC_AT((((code)->buffer)), (c_pos)) = __OC_ADD(5, __OC_MUL(reg, 8));
    elf_patch_i32(code, __OC_ADD(c_pos, 1), ({ __auto_type _v = (rel); if ((_v < 0 && (int64_t)_v < -2147483648LL) || (!(_v < 0) && (uint64_t)_v > 2147483647ULL)) { fprintf(stderr, "panic: invalid signed cast\n"); exit(1); } (int32_t)_v; }));
    __OC_AT((((code)->buffer)), (__OC_ADD(c_pos, 5))) = 15;
    __OC_AT((((code)->buffer)), (__OC_ADD(c_pos, 6))) = 31;
    __OC_AT((((code)->buffer)), (__OC_ADD(c_pos, 7))) = 0;
    ri = __OC_ADD(ri, 1);
    }
    CodeBuffer pe_buf = buf_init(__OC_ADD(file_size, 1));
    CodeBuffer* pe = (&(pe_buf));
    elf_append_u16(pe, 23117);
    elf_append_u16(pe, 144);
    elf_append_u16(pe, 3);
    elf_append_u16(pe, 0);
    elf_append_u16(pe, 4);
    elf_append_u16(pe, 0);
    elf_append_u16(pe, 65535);
    elf_append_u16(pe, 0);
    elf_append_u16(pe, 184);
    pe_append_zeros(pe, 6);
    elf_append_u16(pe, 64);
    pe_append_zeros(pe, 34);
    elf_append_u32(pe, 64);
    elf_append_u32(pe, 17744);
    elf_append_u16(pe, 34404);
    elf_append_u16(pe, 2);
    elf_append_u32(pe, 0);
    elf_append_u32(pe, 0);
    elf_append_u32(pe, 0);
    elf_append_u16(pe, 240);
    elf_append_u16(pe, 34);
    elf_append_u16(pe, 523);
    buf_append_u8(pe, 1);
    buf_append_u8(pe, 0);
    elf_append_u32(pe, ({ __auto_type _v = (text_raw_size); if (_v < 0 || (uint64_t)_v > 4294967295ULL) { fprintf(stderr, "panic: invalid unsigned cast\n"); exit(1); } (uint32_t)_v; }));
    elf_append_u32(pe, ({ __auto_type _v = (rdata_raw_size); if (_v < 0 || (uint64_t)_v > 4294967295ULL) { fprintf(stderr, "panic: invalid unsigned cast\n"); exit(1); } (uint32_t)_v; }));
    elf_append_u32(pe, 0);
    elf_append_u32(pe, ({ __auto_type _v = (text_rva); if (_v < 0 || (uint64_t)_v > 4294967295ULL) { fprintf(stderr, "panic: invalid unsigned cast\n"); exit(1); } (uint32_t)_v; }));
    elf_append_u32(pe, ({ __auto_type _v = (text_rva); if (_v < 0 || (uint64_t)_v > 4294967295ULL) { fprintf(stderr, "panic: invalid unsigned cast\n"); exit(1); } (uint32_t)_v; }));
    elf_append_u64(pe, 5368709120);
    elf_append_u32(pe, ({ __auto_type _v = (sect_align); if (_v < 0 || (uint64_t)_v > 4294967295ULL) { fprintf(stderr, "panic: invalid unsigned cast\n"); exit(1); } (uint32_t)_v; }));
    elf_append_u32(pe, ({ __auto_type _v = (file_align); if (_v < 0 || (uint64_t)_v > 4294967295ULL) { fprintf(stderr, "panic: invalid unsigned cast\n"); exit(1); } (uint32_t)_v; }));
    elf_append_u16(pe, 6);
    elf_append_u16(pe, 0);
    elf_append_u16(pe, 0);
    elf_append_u16(pe, 0);
    elf_append_u16(pe, 6);
    elf_append_u16(pe, 0);
    elf_append_u32(pe, 0);
    elf_append_u32(pe, ({ __auto_type _v = (image_size); if (_v < 0 || (uint64_t)_v > 4294967295ULL) { fprintf(stderr, "panic: invalid unsigned cast\n"); exit(1); } (uint32_t)_v; }));
    elf_append_u32(pe, ({ __auto_type _v = (headers_size); if (_v < 0 || (uint64_t)_v > 4294967295ULL) { fprintf(stderr, "panic: invalid unsigned cast\n"); exit(1); } (uint32_t)_v; }));
    elf_append_u32(pe, 0);
    elf_append_u16(pe, 3);
    elf_append_u16(pe, 33120);
    elf_append_u64(pe, 1048576);
    elf_append_u64(pe, 1048576);
    elf_append_u64(pe, 1048576);
    elf_append_u64(pe, 4096);
    elf_append_u32(pe, 0);
    elf_append_u32(pe, 16);
    size_t d = 0;
    while ((d < 16)) {
    if ((d == 1)) {
    elf_append_u32(pe, ({ __auto_type _v = (__OC_ADD(rdata_rva, idt_off)); if (_v < 0 || (uint64_t)_v > 4294967295ULL) { fprintf(stderr, "panic: invalid unsigned cast\n"); exit(1); } (uint32_t)_v; }));
    elf_append_u32(pe, ({ __auto_type _v = (idt_bytes); if (_v < 0 || (uint64_t)_v > 4294967295ULL) { fprintf(stderr, "panic: invalid unsigned cast\n"); exit(1); } (uint32_t)_v; }));
    }
    else {
    if ((d == 12)) {
    elf_append_u32(pe, ({ __auto_type _v = (__OC_ADD(rdata_rva, iat_off)); if (_v < 0 || (uint64_t)_v > 4294967295ULL) { fprintf(stderr, "panic: invalid unsigned cast\n"); exit(1); } (uint32_t)_v; }));
    elf_append_u32(pe, ({ __auto_type _v = (thunk_bytes); if (_v < 0 || (uint64_t)_v > 4294967295ULL) { fprintf(stderr, "panic: invalid unsigned cast\n"); exit(1); } (uint32_t)_v; }));
    }
    else {
    elf_append_u64(pe, 0);
    }
    }
    d = __OC_ADD(d, 1);
    }
    pe_append_section_header(pe, (oc_string){ .ptr = (const uint8_t*)(".text"), .len = sizeof(".text") - 1 }, ((code)->len), text_rva, text_raw_size, text_raw_off, 1610612768);
    pe_append_section_header(pe, (oc_string){ .ptr = (const uint8_t*)(".rdata"), .len = sizeof(".rdata") - 1 }, rdata_len, rdata_rva, rdata_raw_size, rdata_raw_off, 1073741888);
    pe_append_zeros(pe, __OC_SUB(headers_size, ((pe)->len)));
    size_t p = 0;
    while ((p < ((code)->len))) {
    buf_append_u8(pe, __OC_AT((((code)->buffer)), (p)));
    p = __OC_ADD(p, 1);
    }
    pe_append_zeros(pe, __OC_SUB(text_raw_size, ((code)->len)));
    size_t t = 0;
    while ((t < 2)) {
    k = 0;
    while ((k < NG_IMP_COUNT)) {
    elf_append_u64(pe, ({ __auto_type _v = (__OC_ADD(rdata_rva, __OC_AT((hint_offs), (k)))); if (_v < 0) { fprintf(stderr, "panic: invalid unsigned cast\n"); exit(1); } (uint64_t)_v; }));
    k = __OC_ADD(k, 1);
    }
    elf_append_u64(pe, 0);
    t = __OC_ADD(t, 1);
    }
    elf_append_u32(pe, ({ __auto_type _v = (__OC_ADD(rdata_rva, ilt_off)); if (_v < 0 || (uint64_t)_v > 4294967295ULL) { fprintf(stderr, "panic: invalid unsigned cast\n"); exit(1); } (uint32_t)_v; }));
    elf_append_u32(pe, 0);
    elf_append_u32(pe, 0);
    elf_append_u32(pe, ({ __auto_type _v = (__OC_ADD(rdata_rva, dll_off)); if (_v < 0 || (uint64_t)_v > 4294967295ULL) { fprintf(stderr, "panic: invalid unsigned cast\n"); exit(1); } (uint32_t)_v; }));
    elf_append_u32(pe, ({ __auto_type _v = (__OC_ADD(rdata_rva, iat_off)); if (_v < 0 || (uint64_t)_v > 4294967295ULL) { fprintf(stderr, "panic: invalid unsigned cast\n"); exit(1); } (uint32_t)_v; }));
    pe_append_zeros(pe, 20);
    k = 0;
    while ((k < NG_IMP_COUNT)) {
    pe_append_zeros(pe, __OC_SUB(__OC_ADD(rdata_raw_off, __OC_AT((hint_offs), (k))), ((pe)->len)));
    elf_append_u16(pe, 0);
    buf_append(pe, ng_imp_name(k));
    buf_append_u8(pe, 0);
    k = __OC_ADD(k, 1);
    }
    pe_append_zeros(pe, __OC_SUB(__OC_ADD(rdata_raw_off, dll_off), ((pe)->len)));
    buf_append(pe, dll_name);
    buf_append_u8(pe, 0);
    pe_append_zeros(pe, __OC_SUB(__OC_ADD(rdata_raw_off, udata_off), ((pe)->len)));
    p = 0;
    while ((p < ((data)->len))) {
    buf_append_u8(pe, __OC_AT((((data)->buffer)), (p)));
    p = __OC_ADD(p, 1);
    }
    pe_append_zeros(pe, __OC_SUB(file_size, ((pe)->len)));
    if ((((pe)->len) != file_size)) {
    fwrite("native codegen error: PE32+ image size mismatch", 1, sizeof("native codegen error: PE32+ image size mismatch") - 1, stdout); fputc('\n', stdout);
    buf_deinit(pe);
    return false;
    }
    oc_string pe_str = buf_to_string(pe);
    bool ok = write_file(out_file_str, pe_str);
    buf_deinit(pe);
    return ok;
}

bool emit_native_image(AstNode* root, oc_string out_file_str, int32_t target, bool fp_trap, bool arena_poison, SourceMap* m) {
    CodeBuffer code_buf = buf_init(262144);
    CodeBuffer data_buf = buf_init(65536);
    CodeBuffer* code = (&(code_buf));
    CodeBuffer* data = (&(data_buf));
    NativeGen ng_state = (NativeGen){ .fn_names = { (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 } }, .fn_offsets = { 0 }, .fn_ret_types = { (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 } }, .fn_count = 0, .fixup_offsets = { 0 }, .fixup_names = { (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 } }, .fixup_is_6byte = { false }, .fixup_count = 0, .data_reloc_code = { 0 }, .data_reloc_data = { 0 }, .data_reloc_count = 0, .var_names = { (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 } }, .var_offsets = { 0 }, .var_is_slice = { false }, .var_types = { (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 } }, .var_active = { false }, .var_scope = { 0 }, .var_arr_len = { 0 }, .var_count = 0, .curr_off = 0, .scope_depth = 0, .struct_names = { (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 } }, .struct_sizes = { 0 }, .struct_count = 0, .field_struct = { (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 } }, .field_names = { (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 } }, .field_offsets = { 0 }, .field_types = { (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 } }, .field_packed = { false }, .field_count = 0, .root = root, .fp_trap = fp_trap, .arena_poison = arena_poison, .srcmap = m, .cur_ret = (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 }, .const_types = { (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 } }, .const_names = { (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 } }, .const_vals = { 0 }, .const_count = 0, .loop_start = 0, .loop_depth = 0, .break_patches = { 0 }, .break_depths = { 0 }, .break_count = 0, .target = target, .has_error = false, .iat_fix_code = { 0 }, .iat_fix_slot = { 0 }, .iat_fix_count = 0 };
    NativeGen* ng = (&(ng_state));
    bool is_win = (target == OC_TARGET_WINDOWS_X86_64);
    AstNode* scan = ((root)->left);
    while ((scan != ((AstNode*)(0)))) {
    if ((((scan)->tag) == NODE_STRUCT_DECL)) {
    oc_string s_name = ((((scan)->tok)).slice);
    bool is_packed = str_eq(((scan)->type_name), (oc_string){ .ptr = (const uint8_t*)("packed"), .len = sizeof("packed") - 1 });
    int32_t f_off = 0;
    AstNode* sf = ((scan)->left);
    while ((sf != ((AstNode*)(0)))) {
    oc_string ft = ((sf)->type_name);
    int32_t f_sz = 8;
    if (is_packed) {
    f_sz = ng_scalar_width(ft);
    }
    if (str_eq(ft, (oc_string){ .ptr = (const uint8_t*)("string"), .len = sizeof("string") - 1 })) {
    f_sz = 16;
    }
    else {
    if ((((ft).len) > 0)) {
    if ((__oc_index_str(ft, 0) == 91)) {
    size_t alen = ng_parse_array_len(ft);
    oc_string ael = ng_parse_array_elem(ft);
    int32_t astr = ng_type_stride(ael);
    f_sz = __OC_MUL(({ __auto_type _v = (alen); if ((_v < 0 && (int64_t)_v < -2147483648LL) || (!(_v < 0) && (uint64_t)_v > 2147483647ULL)) { fprintf(stderr, "panic: invalid signed cast\n"); exit(1); } (int32_t)_v; }), astr);
    if ((!is_packed)) {
    f_sz = __OC_MUL(__OC_DIV(__OC_ADD(f_sz, 7), 8), 8);
    }
    }
    else {
    if (is_packed) {
    int32_t raw_sz = ng_struct_raw_size(ng, ft);
    if ((raw_sz > 0)) {
    f_sz = raw_sz;
    }
    }
    else {
    int32_t sub_sz = ng_find_struct_size(ng, ft);
    if ((sub_sz > 0)) {
    f_sz = sub_sz;
    }
    }
    }
    }
    }
    if ((((ng)->field_count) < 1024)) {
    __OC_AT((((ng)->field_struct)), (((ng)->field_count))) = s_name;
    __OC_AT((((ng)->field_names)), (((ng)->field_count))) = ((((sf)->tok)).slice);
    __OC_AT((((ng)->field_offsets)), (((ng)->field_count))) = f_off;
    __OC_AT((((ng)->field_types)), (((ng)->field_count))) = ft;
    __OC_AT((((ng)->field_packed)), (((ng)->field_count))) = is_packed;
    ((ng)->field_count) = __OC_ADD(((ng)->field_count), 1);
    }
    else {
    ng_fail(ng, (oc_string){ .ptr = (const uint8_t*)("native codegen error: too many struct fields in the program (1024)"), .len = sizeof("native codegen error: too many struct fields in the program (1024)") - 1 });
    }
    f_off = __OC_ADD(f_off, f_sz);
    sf = ((sf)->next);
    }
    if ((((ng)->struct_count) < 256)) {
    __OC_AT((((ng)->struct_names)), (((ng)->struct_count))) = s_name;
    __OC_AT((((ng)->struct_sizes)), (((ng)->struct_count))) = f_off;
    ((ng)->struct_count) = __OC_ADD(((ng)->struct_count), 1);
    }
    else {
    ng_fail(ng, (oc_string){ .ptr = (const uint8_t*)("native codegen error: too many structs in the program (256)"), .len = sizeof("native codegen error: too many structs in the program (256)") - 1 });
    }
    }
    else {
    if ((((scan)->tag) == NODE_LET)) {
    if ((((scan)->left) != ((AstNode*)(0)))) {
    if ((((ng)->const_count) < 128)) {
    AstNode* lit = ((scan)->left);
    bool negative = false;
    if ((((lit)->tag) == NODE_UNARY)) {
    if ((((((lit)->tok)).tag) == TOK_MINUS)) {
    if ((((lit)->left) != ((AstNode*)(0)))) {
    negative = true;
    lit = ((lit)->left);
    }
    }
    }
    oc_string c_type = ((scan)->type_name);
    if ((((c_type).len) == 0)) {
    if ((((lit)->tag) == NODE_FLOAT)) {
    c_type = (oc_string){ .ptr = (const uint8_t*)("f64"), .len = sizeof("f64") - 1 };
    }
    }
    uint64_t c_val = 0;
    if (str_eq(c_type, (oc_string){ .ptr = (const uint8_t*)("f64"), .len = sizeof("f64") - 1 })) {
    c_val = ng_literal_bits(ng, ((((lit)->tok)).slice), false);
    if (negative) {
    c_val = (c_val ^ 0x8000000000000000);
    }
    }
    else {
    if (str_eq(c_type, (oc_string){ .ptr = (const uint8_t*)("f32"), .len = sizeof("f32") - 1 })) {
    c_val = ng_literal_bits(ng, ((((lit)->tok)).slice), true);
    if (negative) {
    c_val = (c_val ^ 0x80000000);
    }
    }
    else {
    c_val = parse_literal_u64(((((lit)->tok)).slice));
    if (negative) {
    c_val = __OC_WRAP(sub, (0), (c_val));
    }
    }
    }
    __OC_AT((((ng)->const_names)), (((ng)->const_count))) = ((((scan)->tok)).slice);
    __OC_AT((((ng)->const_types)), (((ng)->const_count))) = c_type;
    __OC_AT((((ng)->const_vals)), (((ng)->const_count))) = c_val;
    ((ng)->const_count) = __OC_ADD(((ng)->const_count), 1);
    }
    else {
    ng_fail(ng, (oc_string){ .ptr = (const uint8_t*)("native codegen error: too many top-level constants (128)"), .len = sizeof("native codegen error: too many top-level constants (128)") - 1 });
    }
    }
    }
    else {
    if ((((scan)->tag) == NODE_FN)) {
    if ((((ng)->fn_count) < 1024)) {
    __OC_AT((((ng)->fn_names)), (((ng)->fn_count))) = ((((scan)->tok)).slice);
    __OC_AT((((ng)->fn_offsets)), (((ng)->fn_count))) = 0;
    __OC_AT((((ng)->fn_ret_types)), (((ng)->fn_count))) = ((scan)->type_name);
    ((ng)->fn_count) = __OC_ADD(((ng)->fn_count), 1);
    }
    else {
    ng_fail(ng, (oc_string){ .ptr = (const uint8_t*)("native codegen error: too many functions (1024)"), .len = sizeof("native codegen error: too many functions (1024)") - 1 });
    }
    }
    }
    }
    scan = ((scan)->next);
    }
    bool fn_has_offset[1024] = {0};
    if (is_win) {
    buf_append_u8(code, 72);
    buf_append_u8(code, 131);
    buf_append_u8(code, 236);
    buf_append_u8(code, 40);
    }
    ng_add_fixup(ng, ((code)->len), (oc_string){ .ptr = (const uint8_t*)("main"), .len = sizeof("main") - 1 }, false);
    buf_append_u8(code, 232);
    elf_append_u32(code, 0);
    buf_append_u8(code, 72);
    buf_append_u8(code, 137);
    buf_append_u8(code, 199);
    if (is_win) {
    ng_emit_call(ng, code, (oc_string){ .ptr = (const uint8_t*)("__win_exit"), .len = sizeof("__win_exit") - 1 });
    }
    else {
    buf_append_u8(code, 72);
    buf_append_u8(code, 199);
    buf_append_u8(code, 192);
    elf_append_u32(code, 60);
    buf_append_u8(code, 15);
    buf_append_u8(code, 5);
    }
    AstNode* decl = ((root)->left);
    while ((decl != ((AstNode*)(0)))) {
    if ((((decl)->tag) == NODE_FN)) {
    if ((((decl)->right) != ((AstNode*)(0)))) {
    bool found_fn = false;
    size_t fi_idx = 0;
    while ((fi_idx < ((ng)->fn_count))) {
    if (str_eq(__OC_AT((((ng)->fn_names)), (fi_idx)), ((((decl)->tok)).slice))) {
    __OC_AT((((ng)->fn_offsets)), (fi_idx)) = ((code)->len);
    __OC_AT((fn_has_offset), (fi_idx)) = true;
    found_fn = true;
    break;
    }
    fi_idx = __OC_ADD(fi_idx, 1);
    }
    if ((!found_fn)) {
    if ((((ng)->fn_count) < 1024)) {
    __OC_AT((((ng)->fn_names)), (((ng)->fn_count))) = ((((decl)->tok)).slice);
    __OC_AT((((ng)->fn_offsets)), (((ng)->fn_count))) = ((code)->len);
    __OC_AT((((ng)->fn_ret_types)), (((ng)->fn_count))) = ((decl)->type_name);
    __OC_AT((fn_has_offset), (((ng)->fn_count))) = true;
    ((ng)->fn_count) = __OC_ADD(((ng)->fn_count), 1);
    }
    }
    ((ng)->var_count) = 0;
    ((ng)->curr_off) = 0;
    ((ng)->scope_depth) = 0;
    ((ng)->cur_ret) = ((decl)->type_name);
    if ((ng_find_struct_size(ng, ((decl)->type_name)) > 32)) {
    ng_fail(ng, (oc_string){ .ptr = (const uint8_t*)("native codegen error: a function cannot return a struct larger than 32 bytes; return it through a pointer parameter"), .len = sizeof("native codegen error: a function cannot return a struct larger than 32 bytes; return it through a pointer parameter") - 1 });
    }
    AstNode* p = ((decl)->left);
    int32_t param_bytes = 0;
    while ((p != ((AstNode*)(0)))) {
    bool is_slice = str_eq(((p)->type_name), (oc_string){ .ptr = (const uint8_t*)("string"), .len = sizeof("string") - 1 });
    if ((((ng)->var_count) < 256)) {
    ((ng)->curr_off) = __OC_SUB(((ng)->curr_off), 8);
    param_bytes = __OC_ADD(param_bytes, 8);
    __OC_AT((((ng)->var_names)), (((ng)->var_count))) = ((((p)->tok)).slice);
    __OC_AT((((ng)->var_offsets)), (((ng)->var_count))) = ((ng)->curr_off);
    __OC_AT((((ng)->var_is_slice)), (((ng)->var_count))) = is_slice;
    __OC_AT((((ng)->var_types)), (((ng)->var_count))) = ((p)->type_name);
    __OC_AT((((ng)->var_active)), (((ng)->var_count))) = true;
    __OC_AT((((ng)->var_scope)), (((ng)->var_count))) = 0;
    __OC_AT((((ng)->var_arr_len)), (((ng)->var_count))) = 0;
    ((ng)->var_count) = __OC_ADD(((ng)->var_count), 1);
    if (is_slice) {
    ((ng)->curr_off) = __OC_SUB(((ng)->curr_off), 8);
    param_bytes = __OC_ADD(param_bytes, 8);
    }
    int32_t p_sz = ng_find_struct_size(ng, ((p)->type_name));
    if ((p_sz > 8)) {
    ((ng)->curr_off) = __OC_SUB(((ng)->curr_off), __OC_SUB(p_sz, 8));
    param_bytes = __OC_ADD(param_bytes, __OC_SUB(p_sz, 8));
    __OC_AT((((ng)->var_offsets)), (__OC_SUB(((ng)->var_count), 1))) = ((ng)->curr_off);
    }
    }
    else {
    ng_fail(ng, (oc_string){ .ptr = (const uint8_t*)("native codegen error: too many variables in one function (256)"), .len = sizeof("native codegen error: too many variables in one function (256)") - 1 });
    }
    p = ((p)->next);
    }
    AstNode* body_block = ((decl)->right);
    int32_t body_bytes = 0;
    if ((body_block != ((AstNode*)(0)))) {
    AstNode* s = ((body_block)->left);
    while ((s != ((AstNode*)(0)))) {
    body_bytes = __OC_ADD(body_bytes, calc_locals_bytes_stmt(ng, s));
    s = ((s)->next);
    }
    }
    int32_t raw_sz = __OC_ADD(param_bytes, body_bytes);
    uint32_t stack_sz = ({ __auto_type _v = (__OC_MUL(__OC_DIV(__OC_ADD(raw_sz, 15), 16), 16)); if (_v < 0 || (uint64_t)_v > 4294967295ULL) { fprintf(stderr, "panic: invalid unsigned cast\n"); exit(1); } (uint32_t)_v; });
    buf_append_u8(code, 85);
    buf_append_u8(code, 72);
    buf_append_u8(code, 137);
    buf_append_u8(code, 229);
    size_t frame_patch = 0;
    bool has_frame = false;
    if ((stack_sz > 0)) {
    buf_append_u8(code, 72);
    buf_append_u8(code, 129);
    buf_append_u8(code, 236);
    frame_patch = ((code)->len);
    has_frame = true;
    elf_append_u32(code, stack_sz);
    }
    size_t total_words = 0;
    size_t tw = 0;
    p = ((decl)->left);
    while ((p != ((AstNode*)(0)))) {
    total_words = __OC_ADD(total_words, 1);
    if (__OC_AT((((ng)->var_is_slice)), (tw))) {
    total_words = __OC_ADD(total_words, 1);
    }
    int32_t tw_sz = ng_find_struct_size(ng, __OC_AT((((ng)->var_types)), (tw)));
    if ((tw_sz > 8)) {
    total_words = __OC_ADD(total_words, ({ __auto_type _v = (__OC_DIV(__OC_SUB(tw_sz, 1), 8)); if (_v < 0) { fprintf(stderr, "panic: invalid unsigned cast\n"); exit(1); } (size_t)_v; }));
    }
    tw = __OC_ADD(tw, 1);
    p = ((p)->next);
    }
    p = ((decl)->left);
    size_t reg_idx = 0;
    size_t p_var_idx = 0;
    while ((p != ((AstNode*)(0)))) {
    int32_t off = __OC_AT((((ng)->var_offsets)), (p_var_idx));
    ng_emit_store_param(code, reg_idx, total_words, off);
    reg_idx = __OC_ADD(reg_idx, 1);
    if (__OC_AT((((ng)->var_is_slice)), (p_var_idx))) {
    ng_emit_store_param(code, reg_idx, total_words, __OC_SUB(off, 8));
    reg_idx = __OC_ADD(reg_idx, 1);
    }
    int32_t p_word = 8;
    while ((p_word < ng_find_struct_size(ng, __OC_AT((((ng)->var_types)), (p_var_idx))))) {
    ng_emit_store_param(code, reg_idx, total_words, __OC_ADD(off, p_word));
    reg_idx = __OC_ADD(reg_idx, 1);
    p_word = __OC_ADD(p_word, 8);
    }
    p_var_idx = __OC_ADD(p_var_idx, 1);
    p = ((p)->next);
    }
    if ((body_block != ((AstNode*)(0)))) {
    AstNode* stmt = ((body_block)->left);
    while ((stmt != ((AstNode*)(0)))) {
    ng_emit_stmt(ng, code, data, stmt);
    stmt = ((stmt)->next);
    }
    }
    uint32_t used_sz = ({ __auto_type _v = (__OC_MUL(__OC_DIV(__OC_ADD(__OC_SUB(0, ((ng)->curr_off)), 15), 16), 16)); if (_v < 0 || (uint64_t)_v > 4294967295ULL) { fprintf(stderr, "panic: invalid unsigned cast\n"); exit(1); } (uint32_t)_v; });
    if ((used_sz > stack_sz)) {
    if (has_frame) {
    elf_patch_u32(code, frame_patch, used_sz);
    }
    else {
    ng_fail(ng, (oc_string){ .ptr = (const uint8_t*)("native codegen error: stack frame size underestimated"), .len = sizeof("native codegen error: stack frame size underestimated") - 1 });
    }
    }
    buf_append_u8(code, 72);
    buf_append_u8(code, 49);
    buf_append_u8(code, 192);
    buf_append_u8(code, 72);
    buf_append_u8(code, 137);
    buf_append_u8(code, 236);
    buf_append_u8(code, 93);
    buf_append_u8(code, 195);
    }
    }
    decl = ((decl)->next);
    }
    if (((ng)->has_error)) {
    buf_deinit(code);
    buf_deinit(data);
    return false;
    }
    bool* has_offset = ((bool*)((&(fn_has_offset))));
    if (is_win) {
    ng_emit_win_runtime(ng, code, has_offset);
    }
    ng_define_helper(ng, has_offset, (oc_string){ .ptr = (const uint8_t*)("__print_i64"), .len = sizeof("__print_i64") - 1 }, ((code)->len));
    buf_append_u8(code, 72);
    buf_append_u8(code, 133);
    buf_append_u8(code, 255);
    ng_add_fixup(ng, ((code)->len), (oc_string){ .ptr = (const uint8_t*)("__print_u64"), .len = sizeof("__print_u64") - 1 }, true);
    buf_append_u8(code, 15);
    buf_append_u8(code, 137);
    elf_append_u32(code, 0);
    buf_append_u8(code, 87);
    buf_append_u8(code, 106);
    buf_append_u8(code, 45);
    buf_append_u8(code, 72);
    buf_append_u8(code, 137);
    buf_append_u8(code, 230);
    buf_append_u8(code, 72);
    buf_append_u8(code, 199);
    buf_append_u8(code, 194);
    elf_append_u32(code, 1);
    buf_append_u8(code, 72);
    buf_append_u8(code, 199);
    buf_append_u8(code, 199);
    elf_append_u32(code, 1);
    ng_emit_write_fd(ng, code);
    buf_append_u8(code, 88);
    buf_append_u8(code, 95);
    buf_append_u8(code, 72);
    buf_append_u8(code, 247);
    buf_append_u8(code, 223);
    ng_define_helper(ng, has_offset, (oc_string){ .ptr = (const uint8_t*)("__print_u64"), .len = sizeof("__print_u64") - 1 }, ((code)->len));
    buf_append_u8(code, 85);
    buf_append_u8(code, 72);
    buf_append_u8(code, 137);
    buf_append_u8(code, 229);
    buf_append_u8(code, 72);
    buf_append_u8(code, 131);
    buf_append_u8(code, 236);
    buf_append_u8(code, 32);
    buf_append_u8(code, 72);
    buf_append_u8(code, 141);
    buf_append_u8(code, 116);
    buf_append_u8(code, 36);
    buf_append_u8(code, 31);
    buf_append_u8(code, 198);
    buf_append_u8(code, 6);
    buf_append_u8(code, 10);
    buf_append_u8(code, 72);
    buf_append_u8(code, 199);
    buf_append_u8(code, 193);
    elf_append_u32(code, 1);
    buf_append_u8(code, 72);
    buf_append_u8(code, 199);
    buf_append_u8(code, 195);
    elf_append_u32(code, 10);
    buf_append_u8(code, 72);
    buf_append_u8(code, 137);
    buf_append_u8(code, 248);
    size_t loop_s = ((code)->len);
    buf_append_u8(code, 72);
    buf_append_u8(code, 49);
    buf_append_u8(code, 210);
    buf_append_u8(code, 72);
    buf_append_u8(code, 247);
    buf_append_u8(code, 243);
    buf_append_u8(code, 128);
    buf_append_u8(code, 194);
    buf_append_u8(code, 48);
    buf_append_u8(code, 72);
    buf_append_u8(code, 255);
    buf_append_u8(code, 206);
    buf_append_u8(code, 136);
    buf_append_u8(code, 22);
    buf_append_u8(code, 72);
    buf_append_u8(code, 255);
    buf_append_u8(code, 193);
    buf_append_u8(code, 72);
    buf_append_u8(code, 133);
    buf_append_u8(code, 192);
    size_t loop_e = __OC_ADD(((code)->len), 2);
    int32_t rel8 = ({ __auto_type _v = (__OC_SUB(({ __auto_type _v = (loop_s); if (!(_v < 0) && (uint64_t)_v > 9223372036854775807ULL) { fprintf(stderr, "panic: invalid signed cast\n"); exit(1); } (int64_t)_v; }), ({ __auto_type _v = (loop_e); if (!(_v < 0) && (uint64_t)_v > 9223372036854775807ULL) { fprintf(stderr, "panic: invalid signed cast\n"); exit(1); } (int64_t)_v; }))); if ((_v < 0 && (int64_t)_v < -2147483648LL) || (!(_v < 0) && (uint64_t)_v > 2147483647ULL)) { fprintf(stderr, "panic: invalid signed cast\n"); exit(1); } (int32_t)_v; });
    uint32_t u8_val = 0;
    if ((rel8 < 0)) {
    int32_t neg = __OC_SUB(0, rel8);
    u8_val = __OC_SUB(256, __OC_MOD(({ __auto_type _v = (neg); if (_v < 0 || (uint64_t)_v > 4294967295ULL) { fprintf(stderr, "panic: invalid unsigned cast\n"); exit(1); } (uint32_t)_v; }), 256));
    }
    else {
    u8_val = __OC_MOD(({ __auto_type _v = (rel8); if (_v < 0 || (uint64_t)_v > 4294967295ULL) { fprintf(stderr, "panic: invalid unsigned cast\n"); exit(1); } (uint32_t)_v; }), 256);
    }
    buf_append_u8(code, 117);
    buf_append_u8(code, ({ __auto_type _v = (__OC_MOD(u8_val, 256)); if (_v < 0 || (uint64_t)_v > 255ULL) { fprintf(stderr, "panic: invalid unsigned cast\n"); exit(1); } (uint8_t)_v; }));
    if ((!is_win)) {
    buf_append_u8(code, 72);
    buf_append_u8(code, 199);
    buf_append_u8(code, 192);
    elf_append_u32(code, 1);
    }
    buf_append_u8(code, 72);
    buf_append_u8(code, 199);
    buf_append_u8(code, 199);
    elf_append_u32(code, 1);
    buf_append_u8(code, 72);
    buf_append_u8(code, 137);
    buf_append_u8(code, 202);
    if (is_win) {
    ng_emit_call(ng, code, (oc_string){ .ptr = (const uint8_t*)("__win_write"), .len = sizeof("__win_write") - 1 });
    }
    else {
    buf_append_u8(code, 15);
    buf_append_u8(code, 5);
    }
    buf_append_u8(code, 72);
    buf_append_u8(code, 137);
    buf_append_u8(code, 236);
    buf_append_u8(code, 93);
    buf_append_u8(code, 195);
    ng_define_helper(ng, has_offset, (oc_string){ .ptr = (const uint8_t*)("__print_slice"), .len = sizeof("__print_slice") - 1 }, ((code)->len));
    buf_append_u8(code, 85);
    buf_append_u8(code, 72);
    buf_append_u8(code, 137);
    buf_append_u8(code, 229);
    buf_append_u8(code, 87);
    buf_append_u8(code, 86);
    buf_append_u8(code, 72);
    buf_append_u8(code, 139);
    buf_append_u8(code, 85);
    buf_append_u8(code, 240);
    buf_append_u8(code, 72);
    buf_append_u8(code, 139);
    buf_append_u8(code, 117);
    buf_append_u8(code, 248);
    buf_append_u8(code, 72);
    buf_append_u8(code, 199);
    buf_append_u8(code, 199);
    elf_append_u32(code, 1);
    ng_emit_write_fd(ng, code);
    buf_append_u8(code, 106);
    buf_append_u8(code, 10);
    buf_append_u8(code, 72);
    buf_append_u8(code, 137);
    buf_append_u8(code, 230);
    buf_append_u8(code, 72);
    buf_append_u8(code, 199);
    buf_append_u8(code, 194);
    elf_append_u32(code, 1);
    buf_append_u8(code, 72);
    buf_append_u8(code, 199);
    buf_append_u8(code, 199);
    elf_append_u32(code, 1);
    ng_emit_write_fd(ng, code);
    buf_append_u8(code, 72);
    buf_append_u8(code, 137);
    buf_append_u8(code, 236);
    buf_append_u8(code, 93);
    buf_append_u8(code, 195);
    oc_string traps[7] = {0};
    __OC_AT((traps), (0)) = (oc_string){ .ptr = (const uint8_t*)("__trap_null_deref"), .len = sizeof("__trap_null_deref") - 1 };
    __OC_AT((traps), (1)) = (oc_string){ .ptr = (const uint8_t*)("__trap_slice_bounds"), .len = sizeof("__trap_slice_bounds") - 1 };
    __OC_AT((traps), (2)) = (oc_string){ .ptr = (const uint8_t*)("__trap_overflow"), .len = sizeof("__trap_overflow") - 1 };
    __OC_AT((traps), (3)) = (oc_string){ .ptr = (const uint8_t*)("__trap_div_zero"), .len = sizeof("__trap_div_zero") - 1 };
    __OC_AT((traps), (4)) = (oc_string){ .ptr = (const uint8_t*)("__trap_cast_signed"), .len = sizeof("__trap_cast_signed") - 1 };
    __OC_AT((traps), (5)) = (oc_string){ .ptr = (const uint8_t*)("__trap_cast_unsigned"), .len = sizeof("__trap_cast_unsigned") - 1 };
    __OC_AT((traps), (6)) = (oc_string){ .ptr = (const uint8_t*)("__trap_fp"), .len = sizeof("__trap_fp") - 1 };
    oc_string trap_msgs[7] = {0};
    __OC_AT((trap_msgs), (0)) = (oc_string){ .ptr = (const uint8_t*)("panic: null pointer dereference\012"), .len = sizeof("panic: null pointer dereference\012") - 1 };
    __OC_AT((trap_msgs), (1)) = (oc_string){ .ptr = (const uint8_t*)("panic: slice index out of bounds\012"), .len = sizeof("panic: slice index out of bounds\012") - 1 };
    __OC_AT((trap_msgs), (2)) = (oc_string){ .ptr = (const uint8_t*)("panic: integer overflow\012"), .len = sizeof("panic: integer overflow\012") - 1 };
    __OC_AT((trap_msgs), (3)) = (oc_string){ .ptr = (const uint8_t*)("panic: division by zero\012"), .len = sizeof("panic: division by zero\012") - 1 };
    __OC_AT((trap_msgs), (4)) = (oc_string){ .ptr = (const uint8_t*)("panic: invalid signed cast\012"), .len = sizeof("panic: invalid signed cast\012") - 1 };
    __OC_AT((trap_msgs), (5)) = (oc_string){ .ptr = (const uint8_t*)("panic: invalid unsigned cast\012"), .len = sizeof("panic: invalid unsigned cast\012") - 1 };
    __OC_AT((trap_msgs), (6)) = (oc_string){ .ptr = (const uint8_t*)("panic: floating-point result is NaN or infinite at "), .len = sizeof("panic: floating-point result is NaN or infinite at ") - 1 };
    size_t ti = 0;
    while ((ti < 7)) {
    ng_define_helper(ng, has_offset, __OC_AT((traps), (ti)), ((code)->len));
    oc_string tmsg = __OC_AT((trap_msgs), (ti));
    size_t t_data_off = ((data)->len);
    size_t tk = 0;
    while ((tk < ((tmsg).len))) {
    buf_append_u8(data, __oc_index_str(tmsg, tk));
    tk = __OC_ADD(tk, 1);
    }
    bool with_loc = (ti == 6);
    if (with_loc) {
    buf_append_u8(code, 86);
    buf_append_u8(code, 82);
    }
    buf_append_u8(code, 72);
    buf_append_u8(code, 190);
    size_t t_patch = ((code)->len);
    elf_append_u64(code, 0);
    ng_add_data_reloc(ng, t_patch, t_data_off);
    buf_append_u8(code, 72);
    buf_append_u8(code, 199);
    buf_append_u8(code, 194);
    elf_append_u32(code, ({ __auto_type _v = (((tmsg).len)); if (_v < 0 || (uint64_t)_v > 4294967295ULL) { fprintf(stderr, "panic: invalid unsigned cast\n"); exit(1); } (uint32_t)_v; }));
    ng_emit_stderr_write(ng, code, is_win);
    if (with_loc) {
    buf_append_u8(code, 90);
    buf_append_u8(code, 94);
    ng_emit_stderr_write(ng, code, is_win);
    buf_append_u8(code, 106);
    buf_append_u8(code, 10);
    buf_append_u8(code, 106);
    buf_append_u8(code, 10);
    buf_append_u8(code, 72);
    buf_append_u8(code, 137);
    buf_append_u8(code, 230);
    buf_append_u8(code, 72);
    buf_append_u8(code, 199);
    buf_append_u8(code, 194);
    elf_append_u32(code, 1);
    ng_emit_stderr_write(ng, code, is_win);
    }
    if (is_win) {
    buf_append_u8(code, 72);
    buf_append_u8(code, 199);
    buf_append_u8(code, 199);
    elf_append_u32(code, 1);
    ng_emit_call(ng, code, (oc_string){ .ptr = (const uint8_t*)("__win_exit"), .len = sizeof("__win_exit") - 1 });
    }
    else {
    buf_append_u8(code, 72);
    buf_append_u8(code, 199);
    buf_append_u8(code, 192);
    elf_append_u32(code, 60);
    buf_append_u8(code, 72);
    buf_append_u8(code, 199);
    buf_append_u8(code, 199);
    elf_append_u32(code, 1);
    buf_append_u8(code, 15);
    buf_append_u8(code, 5);
    }
    ti = __OC_ADD(ti, 1);
    }
    size_t fi = 0;
    while ((fi < ((ng)->fixup_count))) {
    size_t f_off = __OC_AT((((ng)->fixup_offsets)), (fi));
    oc_string f_name = __OC_AT((((ng)->fixup_names)), (fi));
    bool is_6 = __OC_AT((((ng)->fixup_is_6byte)), (fi));
    size_t target_off = 0;
    bool found = false;
    size_t k = 0;
    while ((k < ((ng)->fn_count))) {
    if (__OC_AT((fn_has_offset), (k))) {
    if (str_eq(__OC_AT((((ng)->fn_names)), (k)), f_name)) {
    target_off = __OC_AT((((ng)->fn_offsets)), (k));
    found = true;
    break;
    }
    }
    k = __OC_ADD(k, 1);
    }
    if ((!found)) {
    CodeBuffer msg = buf_init(4096);
    buf_append((&(msg)), (oc_string){ .ptr = (const uint8_t*)("native codegen error: unresolved symbol '"), .len = sizeof("native codegen error: unresolved symbol '") - 1 });
    buf_append((&(msg)), f_name);
    buf_append((&(msg)), (oc_string){ .ptr = (const uint8_t*)("'"), .len = sizeof("'") - 1 });
    __oc_print(buf_to_string((&(msg))));
    buf_deinit((&(msg)));
    buf_deinit(code);
    buf_deinit(data);
    return false;
    }
    size_t add_len = 5;
    size_t add_pos = 1;
    if (is_6) {
    add_len = 6;
    add_pos = 2;
    }
    size_t next_ip = __OC_ADD(f_off, add_len);
    int32_t rel32 = ({ __auto_type _v = (__OC_SUB(({ __auto_type _v = (target_off); if (!(_v < 0) && (uint64_t)_v > 9223372036854775807ULL) { fprintf(stderr, "panic: invalid signed cast\n"); exit(1); } (int64_t)_v; }), ({ __auto_type _v = (next_ip); if (!(_v < 0) && (uint64_t)_v > 9223372036854775807ULL) { fprintf(stderr, "panic: invalid signed cast\n"); exit(1); } (int64_t)_v; }))); if ((_v < 0 && (int64_t)_v < -2147483648LL) || (!(_v < 0) && (uint64_t)_v > 2147483647ULL)) { fprintf(stderr, "panic: invalid signed cast\n"); exit(1); } (int32_t)_v; });
    size_t patch_pos = __OC_ADD(f_off, add_pos);
    elf_patch_i32(code, patch_pos, rel32);
    fi = __OC_ADD(fi, 1);
    }
    if (((ng)->has_error)) {
    buf_deinit(code);
    buf_deinit(data);
    return false;
    }
    if ((((code)->len) >= ((code)->capacity))) {
    ng_fail(ng, (oc_string){ .ptr = (const uint8_t*)("native codegen error: code buffer capacity exceeded"), .len = sizeof("native codegen error: code buffer capacity exceeded") - 1 });
    }
    if ((((data)->len) >= ((data)->capacity))) {
    ng_fail(ng, (oc_string){ .ptr = (const uint8_t*)("native codegen error: data buffer capacity exceeded"), .len = sizeof("native codegen error: data buffer capacity exceeded") - 1 });
    }
    bool ok = false;
    if ((!((ng)->has_error))) {
    if (is_win) {
    ok = ng_write_pe(ng, code, data, out_file_str);
    }
    else {
    ok = ng_write_elf(ng, code, data, out_file_str);
    }
    }
    buf_deinit(code);
    buf_deinit(data);
    if ((!ok)) {
    return false;
    }
    chmod_exec(out_file_str);
    return true;
}

bool emit_native_elf(AstNode* root, oc_string out_file_str, bool fp_trap, bool arena_poison, SourceMap* m) {
    return emit_native_image(root, out_file_str, OC_TARGET_LINUX_X86_64, fp_trap, arena_poison, m);
}

bool emit_native_pe(AstNode* root, oc_string out_file_str, bool fp_trap, bool arena_poison, SourceMap* m) {
    return emit_native_image(root, out_file_str, OC_TARGET_WINDOWS_X86_64, fp_trap, arena_poison, m);
}

bool ng_write_elf(NativeGen* ng, CodeBuffer* code, CodeBuffer* data, oc_string out_file_str) {
    size_t rodata_end_off = __OC_ADD(256, ((data)->len));
    size_t code_file_off = __OC_MUL(__OC_DIV(__OC_ADD(rodata_end_off, 4095), 4096), 4096);
    uint64_t code_vaddr = __OC_ADD(4194304, ({ __auto_type _v = (code_file_off); if (_v < 0) { fprintf(stderr, "panic: invalid unsigned cast\n"); exit(1); } (uint64_t)_v; }));
    uint64_t data_base_vaddr = __OC_ADD(4194304, 256);
    size_t ri = 0;
    while ((ri < ((ng)->data_reloc_count))) {
    size_t c_pos = __OC_AT((((ng)->data_reloc_code)), (ri));
    size_t d_off = __OC_AT((((ng)->data_reloc_data)), (ri));
    uint64_t target_vaddr = __OC_ADD(data_base_vaddr, ({ __auto_type _v = (d_off); if (_v < 0) { fprintf(stderr, "panic: invalid unsigned cast\n"); exit(1); } (uint64_t)_v; }));
    elf_patch_u64(code, c_pos, target_vaddr);
    ri = __OC_ADD(ri, 1);
    }
    size_t total_code_len = ((code)->len);
    CodeBuffer elf_buf = buf_init(524288);
    CodeBuffer* elf = (&(elf_buf));
    buf_append_u8(elf, 127);
    buf_append_u8(elf, 69);
    buf_append_u8(elf, 76);
    buf_append_u8(elf, 70);
    buf_append_u8(elf, 2);
    buf_append_u8(elf, 1);
    buf_append_u8(elf, 1);
    buf_append_u8(elf, 0);
    size_t p = 0;
    while ((p < 8)) {
    buf_append_u8(elf, 0);
    p = __OC_ADD(p, 1);
    }
    elf_append_u16(elf, 2);
    elf_append_u16(elf, 62);
    elf_append_u32(elf, 1);
    elf_append_u64(elf, code_vaddr);
    elf_append_u64(elf, 64);
    elf_append_u64(elf, 0);
    elf_append_u32(elf, 0);
    elf_append_u16(elf, 64);
    elf_append_u16(elf, 56);
    elf_append_u16(elf, 3);
    elf_append_u16(elf, 0);
    elf_append_u16(elf, 0);
    elf_append_u16(elf, 0);
    elf_append_u32(elf, 1);
    elf_append_u32(elf, 4);
    elf_append_u64(elf, 0);
    elf_append_u64(elf, 4194304);
    elf_append_u64(elf, 4194304);
    elf_append_u64(elf, ({ __auto_type _v = (rodata_end_off); if (_v < 0) { fprintf(stderr, "panic: invalid unsigned cast\n"); exit(1); } (uint64_t)_v; }));
    elf_append_u64(elf, ({ __auto_type _v = (rodata_end_off); if (_v < 0) { fprintf(stderr, "panic: invalid unsigned cast\n"); exit(1); } (uint64_t)_v; }));
    elf_append_u64(elf, 4096);
    elf_append_u32(elf, 1);
    elf_append_u32(elf, 5);
    elf_append_u64(elf, ({ __auto_type _v = (code_file_off); if (_v < 0) { fprintf(stderr, "panic: invalid unsigned cast\n"); exit(1); } (uint64_t)_v; }));
    elf_append_u64(elf, code_vaddr);
    elf_append_u64(elf, code_vaddr);
    elf_append_u64(elf, ({ __auto_type _v = (total_code_len); if (_v < 0) { fprintf(stderr, "panic: invalid unsigned cast\n"); exit(1); } (uint64_t)_v; }));
    elf_append_u64(elf, ({ __auto_type _v = (total_code_len); if (_v < 0) { fprintf(stderr, "panic: invalid unsigned cast\n"); exit(1); } (uint64_t)_v; }));
    elf_append_u64(elf, 4096);
    elf_append_u32(elf, 1685382481);
    elf_append_u32(elf, 6);
    elf_append_u64(elf, 0);
    elf_append_u64(elf, 0);
    elf_append_u64(elf, 0);
    elf_append_u64(elf, 0);
    elf_append_u64(elf, 0);
    elf_append_u64(elf, 16);
    p = 0;
    while ((p < 24)) {
    buf_append_u8(elf, 0);
    p = __OC_ADD(p, 1);
    }
    p = 0;
    while ((p < ((data)->len))) {
    buf_append_u8(elf, __OC_AT((((data)->buffer)), (p)));
    p = __OC_ADD(p, 1);
    }
    size_t pad_to_code = __OC_SUB(code_file_off, rodata_end_off);
    p = 0;
    while ((p < pad_to_code)) {
    buf_append_u8(elf, 0);
    p = __OC_ADD(p, 1);
    }
    p = 0;
    while ((p < ((code)->len))) {
    buf_append_u8(elf, __OC_AT((((code)->buffer)), (p)));
    p = __OC_ADD(p, 1);
    }
    oc_string elf_str = buf_to_string(elf);
    bool ok = write_file(out_file_str, elf_str);
    buf_deinit(elf);
    return ok;
}

bool is_safe_path(oc_string s, bool allow_backslash) {
    if ((((s).len) == 0)) {
    return false;
    }
    size_t i = 0;
    while ((i < ((s).len))) {
    uint8_t c = __oc_index_str(s, i);
    if ((c <= 32)) {
    return false;
    }
    if ((c == 33)) {
    return false;
    }
    if ((c == 34)) {
    return false;
    }
    if ((c == 35)) {
    return false;
    }
    if ((c == 36)) {
    return false;
    }
    if ((c == 37)) {
    return false;
    }
    if ((c == 38)) {
    return false;
    }
    if ((c == 39)) {
    return false;
    }
    if ((c == 40)) {
    return false;
    }
    if ((c == 41)) {
    return false;
    }
    if ((c == 42)) {
    return false;
    }
    if ((c == 59)) {
    return false;
    }
    if ((c == 60)) {
    return false;
    }
    if ((c == 62)) {
    return false;
    }
    if ((c == 63)) {
    return false;
    }
    if ((c == 92)) {
    if ((!allow_backslash)) {
    return false;
    }
    }
    if ((c == 94)) {
    return false;
    }
    if ((c == 96)) {
    return false;
    }
    if ((c == 123)) {
    return false;
    }
    if ((c == 124)) {
    return false;
    }
    if ((c == 125)) {
    return false;
    }
    if ((c == 126)) {
    return false;
    }
    if ((c == 127)) {
    return false;
    }
    i = __OC_ADD(i, 1);
    }
    return true;
}

oc_string arena_dup_str(Arena* arena, oc_string s) {
    if ((((s).len) == 0)) {
    return (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 };
    }
    uint8_t* buf = arena_alloc(arena, ((s).len));
    size_t i = 0;
    while ((i < ((s).len))) {
    __OC_AT((buf), (i)) = __oc_index_str(s, i);
    i = __OC_ADD(i, 1);
    }
    return (oc_string){ .ptr = ((uint8_t*)(buf)), .len = ((s).len) };
}

bool is_path_sep(uint8_t c) {
    if ((c == 47)) {
    return true;
    }
    if ((c == 92)) {
    return true;
    }
    return false;
}

bool is_abs_path(oc_string p) {
    if ((((p).len) == 0)) {
    return false;
    }
    if (is_path_sep(__oc_index_str(p, 0))) {
    return true;
    }
    if ((((p).len) >= 2)) {
    if ((__oc_index_str(p, 1) == 58)) {
    uint8_t d = (__oc_index_str(p, 0) | 32);
    if ((d >= 97)) {
    if ((d <= 122)) {
    return true;
    }
    }
    }
    }
    return false;
}

oc_string dir_of_path(Arena* arena, oc_string path) {
    size_t i = ((path).len);
    while ((i > 0)) {
    i = __OC_SUB(i, 1);
    if (is_path_sep(__oc_index_str(path, i))) {
    return arena_dup_str(arena, __oc_slice_str(path, 0, __OC_ADD(i, 1)));
    }
    }
    return (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 };
}

bool ends_with_exe(oc_string p) {
    if ((((p).len) < 4)) {
    return false;
    }
    if ((__oc_index_str(p, __OC_SUB(((p).len), 4)) != 46)) {
    return false;
    }
    if (((__oc_index_str(p, __OC_SUB(((p).len), 3)) | 32) != 101)) {
    return false;
    }
    if (((__oc_index_str(p, __OC_SUB(((p).len), 2)) | 32) != 120)) {
    return false;
    }
    if (((__oc_index_str(p, __OC_SUB(((p).len), 1)) | 32) != 101)) {
    return false;
    }
    return true;
}

int32_t parse_target_name(oc_string name) {
    if (str_eq(name, (oc_string){ .ptr = (const uint8_t*)("linux-x86_64"), .len = sizeof("linux-x86_64") - 1 })) {
    return OC_TARGET_LINUX_X86_64;
    }
    if (str_eq(name, (oc_string){ .ptr = (const uint8_t*)("windows-x86_64"), .len = sizeof("windows-x86_64") - 1 })) {
    return OC_TARGET_WINDOWS_X86_64;
    }
    if (str_eq(name, (oc_string){ .ptr = (const uint8_t*)("windows"), .len = sizeof("windows") - 1 })) {
    return OC_TARGET_WINDOWS_X86_64;
    }
    if (str_eq(name, (oc_string){ .ptr = (const uint8_t*)("macos-x86_64"), .len = sizeof("macos-x86_64") - 1 })) {
    return OC_TARGET_MACOS_X86_64;
    }
    if (str_eq(name, (oc_string){ .ptr = (const uint8_t*)("macos-aarch64"), .len = sizeof("macos-aarch64") - 1 })) {
    return OC_TARGET_MACOS_AARCH64;
    }
    return (-1);
}

bool is_macos_target(int32_t t) {
    if ((t == OC_TARGET_MACOS_X86_64)) {
    return true;
    }
    if ((t == OC_TARGET_MACOS_AARCH64)) {
    return true;
    }
    return false;
}

oc_string c_compiler_for(int32_t target, int32_t host) {
    if ((target == host)) {
    if ((host == OC_TARGET_WINDOWS_X86_64)) {
    return (oc_string){ .ptr = (const uint8_t*)("gcc"), .len = sizeof("gcc") - 1 };
    }
    return (oc_string){ .ptr = (const uint8_t*)("cc"), .len = sizeof("cc") - 1 };
    }
    if ((target == OC_TARGET_WINDOWS_X86_64)) {
    return (oc_string){ .ptr = (const uint8_t*)("x86_64-w64-mingw32-gcc"), .len = sizeof("x86_64-w64-mingw32-gcc") - 1 };
    }
    if ((target == OC_TARGET_LINUX_X86_64)) {
    return (oc_string){ .ptr = (const uint8_t*)("x86_64-linux-gnu-gcc"), .len = sizeof("x86_64-linux-gnu-gcc") - 1 };
    }
    if (is_macos_target(host)) {
    if ((target == OC_TARGET_MACOS_X86_64)) {
    return (oc_string){ .ptr = (const uint8_t*)("cc -arch x86_64"), .len = sizeof("cc -arch x86_64") - 1 };
    }
    if ((target == OC_TARGET_MACOS_AARCH64)) {
    return (oc_string){ .ptr = (const uint8_t*)("cc -arch arm64"), .len = sizeof("cc -arch arm64") - 1 };
    }
    }
    return (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 };
}

bool load_module_recursive(oc_string path, oc_string base_dir, oc_string exe_dir, CodeBuffer* out, ModuleLoader* loader, Arena* arena) {
    size_t i = 0;
    while ((i < ((loader)->visited_count))) {
    if (str_eq(__OC_AT((((loader)->visited)), (i)), path)) {
    return true;
    }
    i = __OC_ADD(i, 1);
    }
    uint8_t* file_buf = arena_alloc(arena, 262144);
    int64_t bytes = (-1);
    oc_string resolved_path = path;
    oc_string shown_path = path;
    if ((((base_dir).len) > 0)) {
    if ((((path).len) > 0)) {
    if ((!is_abs_path(path))) {
    oc_string cand1 = str_concat(arena, base_dir, path);
    bytes = read_file(cand1, file_buf, 262143);
    if ((bytes >= 0)) {
    resolved_path = cand1;
    shown_path = cand1;
    }
    }
    }
    }
    if ((bytes < 0)) {
    bytes = read_file(path, file_buf, 262143);
    if ((bytes >= 0)) {
    resolved_path = path;
    }
    }
    if ((bytes < 0)) {
    if ((((exe_dir).len) > 0)) {
    oc_string cand2 = str_concat(arena, exe_dir, path);
    bytes = read_file(cand2, file_buf, 262143);
    if ((bytes >= 0)) {
    resolved_path = cand2;
    }
    }
    }
    if ((bytes < 0)) {
    if ((((exe_dir).len) > 0)) {
    oc_string up1 = str_concat(arena, exe_dir, (oc_string){ .ptr = (const uint8_t*)("../"), .len = sizeof("../") - 1 });
    oc_string cand3 = str_concat(arena, up1, path);
    bytes = read_file(cand3, file_buf, 262143);
    if ((bytes >= 0)) {
    resolved_path = cand3;
    }
    }
    }
    if ((bytes < 0)) {
    if ((((exe_dir).len) > 0)) {
    oc_string up2 = str_concat(arena, exe_dir, (oc_string){ .ptr = (const uint8_t*)("../../"), .len = sizeof("../../") - 1 });
    oc_string cand4 = str_concat(arena, up2, path);
    bytes = read_file(cand4, file_buf, 262143);
    if ((bytes >= 0)) {
    resolved_path = cand4;
    }
    }
    }
    if ((bytes < 0)) {
    fwrite("error: failed to read file: ", 1, sizeof("error: failed to read file: ") - 1, stdout); fputc('\n', stdout);
    __oc_print(path);
    return false;
    }
    i = 0;
    while ((i < ((loader)->visited_count))) {
    if (str_eq(__OC_AT((((loader)->visited)), (i)), resolved_path)) {
    return true;
    }
    i = __OC_ADD(i, 1);
    }
    if ((((loader)->visited_count) < 128)) {
    __OC_AT((((loader)->visited)), (((loader)->visited_count))) = arena_dup_str(arena, path);
    ((loader)->visited_count) = __OC_ADD(((loader)->visited_count), 1);
    }
    if ((!str_eq(resolved_path, path))) {
    if ((((loader)->visited_count) < 128)) {
    __OC_AT((((loader)->visited)), (((loader)->visited_count))) = arena_dup_str(arena, resolved_path);
    ((loader)->visited_count) = __OC_ADD(((loader)->visited_count), 1);
    }
    }
    oc_string cur_dir = dir_of_path(arena, resolved_path);
    oc_string src = (oc_string){ .ptr = ((uint8_t*)(file_buf)), .len = ({ __auto_type _v = (bytes); if (_v < 0) { fprintf(stderr, "panic: invalid unsigned cast\n"); exit(1); } (size_t)_v; }) };
    Lexer lex = lexer_init(src);
    bool scanning = true;
    while (scanning) {
    Token tok = lexer_next((&(lex)));
    if ((((tok).tag) == TOK_IMPORT)) {
    Token path_tok = lexer_next((&(lex)));
    if ((((path_tok).tag) == TOK_STRING)) {
    oc_string raw = ((path_tok).slice);
    if ((((raw).len) >= 2)) {
    if ((__oc_index_str(raw, 0) == 34)) {
    raw = __oc_slice_str(raw, 1, __OC_SUB(((raw).len), 1));
    }
    }
    CodeBuffer sub_path_buf = buf_init(256);
    buf_append((&(sub_path_buf)), raw);
    bool need_dot_oc = false;
    if ((((raw).len) < 3)) {
    need_dot_oc = true;
    }
    else {
    if ((__oc_index_str(raw, __OC_SUB(((raw).len), 3)) != 46)) {
    need_dot_oc = true;
    }
    else {
    if ((__oc_index_str(raw, __OC_SUB(((raw).len), 2)) != 111)) {
    need_dot_oc = true;
    }
    else {
    if ((__oc_index_str(raw, __OC_SUB(((raw).len), 1)) != 99)) {
    need_dot_oc = true;
    }
    }
    }
    }
    if (need_dot_oc) {
    buf_append((&(sub_path_buf)), (oc_string){ .ptr = (const uint8_t*)(".oc"), .len = sizeof(".oc") - 1 });
    }
    oc_string sub_path = buf_to_string((&(sub_path_buf)));
    bool ok_sub = load_module_recursive(sub_path, cur_dir, exe_dir, out, loader, arena);
    buf_deinit((&(sub_path_buf)));
    if ((!ok_sub)) {
    return false;
    }
    }
    }
    else {
    if ((((tok).tag) == TOK_SEMICOLON)) {
    }
    else {
    if ((((tok).tag) == TOK_EOF)) {
    scanning = false;
    }
    else {
    scanning = false;
    }
    }
    }
    }
    if ((((((loader)->map)).count) < 128)) {
    __OC_AT((((((loader)->map)).first_line)), (((((loader)->map)).count))) = __OC_ADD(((loader)->lines), 1);
    __OC_AT((((((loader)->map)).path)), (((((loader)->map)).count))) = arena_dup_str(arena, shown_path);
    ((((loader)->map)).count) = __OC_ADD(((((loader)->map)).count), 1);
    }
    size_t nl = 0;
    while ((nl < ((src).len))) {
    if ((__oc_index_str(src, nl) == 10)) {
    ((loader)->lines) = __OC_ADD(((loader)->lines), 1);
    }
    nl = __OC_ADD(nl, 1);
    }
    ((loader)->lines) = __OC_ADD(((loader)->lines), 1);
    buf_append(out, src);
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("\012"), .len = sizeof("\012") - 1 });
    return true;
}

oc_string env_str(oc_string name) {
    uint8_t* p = getenv(((name).ptr));
    if ((((size_t)(p)) == 0)) {
    return (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 };
    }
    return cstr_to_string(p);
}

oc_string make_private_tmp_dir(Arena* arena, bool allow_backslash) {
    oc_string bases[4] = {0};
    __OC_AT((bases), (0)) = env_str((oc_string){ .ptr = (const uint8_t*)("TMPDIR"), .len = sizeof("TMPDIR") - 1 });
    __OC_AT((bases), (1)) = env_str((oc_string){ .ptr = (const uint8_t*)("TEMP"), .len = sizeof("TEMP") - 1 });
    __OC_AT((bases), (2)) = (oc_string){ .ptr = (const uint8_t*)("/tmp"), .len = sizeof("/tmp") - 1 };
    __OC_AT((bases), (3)) = (oc_string){ .ptr = (const uint8_t*)("."), .len = sizeof(".") - 1 };
    size_t i = 0;
    while ((i < 4)) {
    if (is_safe_path(__OC_AT((bases), (i)), allow_backslash)) {
    oc_string tmpl = str_concat(arena, __OC_AT((bases), (i)), (oc_string){ .ptr = (const uint8_t*)("/occ_XXXXXX"), .len = sizeof("/occ_XXXXXX") - 1 });
    uint8_t* buf = arena_alloc(arena, __OC_ADD(((tmpl).len), 1));
    if ((((size_t)(buf)) != 0)) {
    size_t k = 0;
    while ((k < ((tmpl).len))) {
    __OC_AT((buf), (k)) = __oc_index_str(tmpl, k);
    k = __OC_ADD(k, 1);
    }
    __OC_AT((buf), (((tmpl).len))) = 0;
    if ((sys_mkdtemp(buf) == 0)) {
    return (oc_string){ .ptr = ((uint8_t*)(buf)), .len = ((tmpl).len) };
    }
    }
    }
    i = __OC_ADD(i, 1);
    }
    return (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 };
}

void remove_private_tmp_dir(Arena* arena, oc_string dir) {
    if ((((dir).len) == 0)) {
    return;
    }
    unlink_file(str_concat(arena, dir, (oc_string){ .ptr = (const uint8_t*)("/stage.c"), .len = sizeof("/stage.c") - 1 }));
    unlink_file(str_concat(arena, dir, (oc_string){ .ptr = (const uint8_t*)("/run"), .len = sizeof("/run") - 1 }));
    unlink_file(str_concat(arena, dir, (oc_string){ .ptr = (const uint8_t*)("/run.exe"), .len = sizeof("/run.exe") - 1 }));
    uint8_t pbuf[4096] = {0};
    sys_rmdir(cstr_path(dir, ((uint8_t*)((&(pbuf))))));
}

bool source_uses_floats(oc_string src) {
    size_t i = 0;
    while ((__OC_ADD(i, 2) < ((src).len))) {
    uint8_t c = __oc_index_str(src, i);
    if ((c == 102)) {
    if ((__oc_index_str(src, __OC_ADD(i, 1)) == 54)) {
    if ((__oc_index_str(src, __OC_ADD(i, 2)) == 52)) {
    return true;
    }
    }
    if ((__oc_index_str(src, __OC_ADD(i, 1)) == 51)) {
    if ((__oc_index_str(src, __OC_ADD(i, 2)) == 50)) {
    return true;
    }
    }
    }
    if (is_digit(c)) {
    if ((__oc_index_str(src, __OC_ADD(i, 1)) == 46)) {
    if (is_digit(__oc_index_str(src, __OC_ADD(i, 2)))) {
    return true;
    }
    }
    if (((__oc_index_str(src, __OC_ADD(i, 1)) | 32) == 101)) {
    if (is_digit(__oc_index_str(src, __OC_ADD(i, 2)))) {
    return true;
    }
    if ((__oc_index_str(src, __OC_ADD(i, 2)) == 45)) {
    return true;
    }
    }
    }
    i = __OC_ADD(i, 1);
    }
    return false;
}

void print_usage(void) {
    fwrite("Usage: occ [run|build] <input.oc> [-o <output>] [--backend c|native] [--target <target>] [-O1|-O2|-O3] [--emit-c <file.c>] [--fp-trap] [--arena-poison]", 1, sizeof("Usage: occ [run|build] <input.oc> [-o <output>] [--backend c|native] [--target <target>] [-O1|-O2|-O3] [--emit-c <file.c>] [--fp-trap] [--arena-poison]") - 1, stdout); fputc('\n', stdout);
    fwrite("       --fp-trap: panic at the first float operation whose result is NaN or infinite (debugging aid; off by default)", 1, sizeof("       --fp-trap: panic at the first float operation whose result is NaN or infinite (debugging aid; off by default)") - 1, stdout); fputc('\n', stdout);
    fwrite("       --arena-poison: freed and reset arena memory becomes inaccessible, so a dangling access faults (testing aid)", 1, sizeof("       --arena-poison: freed and reset arena memory becomes inaccessible, so a dangling access faults (testing aid)") - 1, stdout); fputc('\n', stdout);
    fwrite("       occ install | update | version | help   (also: --version, -v, --help, -h)", 1, sizeof("       occ install | update | version | help   (also: --version, -v, --help, -h)") - 1, stdout); fputc('\n', stdout);
    fwrite("install: make 'occ' runnable by name (symlink in ~/.local/bin; user PATH on Windows). update: git pull + rebuild this install.", 1, sizeof("install: make 'occ' runnable by name (symlink in ~/.local/bin; user PATH on Windows). update: git pull + rebuild this install.") - 1, stdout); fputc('\n', stdout);
    fwrite("Environment: OC_HOME = directory containing std/ (default: next to the occ binary's bin/ directory)", 1, sizeof("Environment: OC_HOME = directory containing std/ (default: next to the occ binary's bin/ directory)") - 1, stdout); fputc('\n', stdout);
    fwrite("Targets (default: host): linux-x86_64, windows-x86_64 (alias: windows), macos-x86_64, macos-aarch64", 1, sizeof("Targets (default: host): linux-x86_64, windows-x86_64 (alias: windows), macos-x86_64, macos-aarch64") - 1, stdout); fputc('\n', stdout);
}

int32_t run_sys_cmd(oc_string cmd) {
    CodeBuffer b = buf_init(512);
    buf_append((&(b)), cmd);
    buf_append_u8((&(b)), 0);
    int32_t ret = system(((uint8_t*)(((b).buffer))));
    buf_deinit((&(b)));
    return ret;
}

oc_string exe_path(Arena* arena, oc_string argv0) {
    uint8_t* buf = arena_alloc(arena, 4096);
    if ((((size_t)(buf)) != 0)) {
    int64_t n = sys_exe_path(buf, 4096);
    if ((n > 0)) {
    return (oc_string){ .ptr = ((uint8_t*)(buf)), .len = ({ __auto_type _v = (n); if (_v < 0) { fprintf(stderr, "panic: invalid unsigned cast\n"); exit(1); } (size_t)_v; }) };
    }
    }
    return argv0;
}

int32_t handle_update(int32_t host, oc_string repo) {
    CodeBuffer cmd = buf_init(8192);
    fwrite("Updating oC from git...", 1, sizeof("Updating oC from git...") - 1, stdout); fputc('\n', stdout);
    buf_append((&(cmd)), (oc_string){ .ptr = (const uint8_t*)("git "), .len = sizeof("git ") - 1 });
    if ((((repo).len) > 0)) {
    buf_append((&(cmd)), (oc_string){ .ptr = (const uint8_t*)("-C "), .len = sizeof("-C ") - 1 });
    buf_append((&(cmd)), repo);
    buf_append((&(cmd)), (oc_string){ .ptr = (const uint8_t*)(" "), .len = sizeof(" ") - 1 });
    }
    buf_append((&(cmd)), (oc_string){ .ptr = (const uint8_t*)("pull"), .len = sizeof("pull") - 1 });
    int32_t pull_ret = run_sys_cmd(buf_to_string((&(cmd))));
    if ((pull_ret != 0)) {
    fwrite("error: 'git pull' failed. Make sure git is installed and occ lives in <oC checkout>/bin (checkout used:)", 1, sizeof("error: 'git pull' failed. Make sure git is installed and occ lives in <oC checkout>/bin (checkout used:)") - 1, stdout); fputc('\n', stdout);
    __oc_print(repo);
    buf_deinit((&(cmd)));
    return 1;
    }
    fwrite("Rebuilding oC compiler...", 1, sizeof("Rebuilding oC compiler...") - 1, stdout); fputc('\n', stdout);
    ((cmd).len) = 0;
    if ((host == OC_TARGET_WINDOWS_X86_64)) {
    buf_append((&(cmd)), (oc_string){ .ptr = (const uint8_t*)("if exist "), .len = sizeof("if exist ") - 1 });
    buf_append((&(cmd)), repo);
    buf_append((&(cmd)), (oc_string){ .ptr = (const uint8_t*)("bin\134occ.exe move /y "), .len = sizeof("bin\134occ.exe move /y ") - 1 });
    buf_append((&(cmd)), repo);
    buf_append((&(cmd)), (oc_string){ .ptr = (const uint8_t*)("bin\134occ.exe "), .len = sizeof("bin\134occ.exe ") - 1 });
    buf_append((&(cmd)), repo);
    buf_append((&(cmd)), (oc_string){ .ptr = (const uint8_t*)("bin\134occ.exe.old >nul 2>&1 & gcc -O2 "), .len = sizeof("bin\134occ.exe.old >nul 2>&1 & gcc -O2 ") - 1 });
    buf_append((&(cmd)), repo);
    buf_append((&(cmd)), (oc_string){ .ptr = (const uint8_t*)("bootstrap\134occ_bootstrap.c -o "), .len = sizeof("bootstrap\134occ_bootstrap.c -o ") - 1 });
    buf_append((&(cmd)), repo);
    buf_append((&(cmd)), (oc_string){ .ptr = (const uint8_t*)("bin\134occ.exe"), .len = sizeof("bin\134occ.exe") - 1 });
    }
    else {
    buf_append((&(cmd)), (oc_string){ .ptr = (const uint8_t*)("cc -O2 "), .len = sizeof("cc -O2 ") - 1 });
    buf_append((&(cmd)), repo);
    buf_append((&(cmd)), (oc_string){ .ptr = (const uint8_t*)("bootstrap/occ_bootstrap.c -o "), .len = sizeof("bootstrap/occ_bootstrap.c -o ") - 1 });
    buf_append((&(cmd)), repo);
    buf_append((&(cmd)), (oc_string){ .ptr = (const uint8_t*)("bin/occ"), .len = sizeof("bin/occ") - 1 });
    }
    int32_t build_ret = run_sys_cmd(buf_to_string((&(cmd))));
    buf_deinit((&(cmd)));
    if ((build_ret != 0)) {
    fwrite("error: failed to rebuild compiler from bootstrap/occ_bootstrap.c", 1, sizeof("error: failed to rebuild compiler from bootstrap/occ_bootstrap.c") - 1, stdout); fputc('\n', stdout);
    return 1;
    }
    fwrite("Successfully updated oC!", 1, sizeof("Successfully updated oC!") - 1, stdout); fputc('\n', stdout);
    return 0;
}

int32_t handle_install(int32_t host, oc_string exe, oc_string exe_dir) {
    bool host_is_windows = (host == OC_TARGET_WINDOWS_X86_64);
    if ((!is_safe_path(exe, host_is_windows))) {
    fwrite("error: cannot install: the path of this occ binary has characters that are unsafe on a command line:", 1, sizeof("error: cannot install: the path of this occ binary has characters that are unsafe on a command line:") - 1, stdout); fputc('\n', stdout);
    __oc_print(exe);
    return 1;
    }
    CodeBuffer cmd = buf_init(8192);
    if (host_is_windows) {
    oc_string bin_dir = __oc_slice_str(exe_dir, 0, __OC_SUB(((exe_dir).len), 1));
    buf_append((&(cmd)), (oc_string){ .ptr = (const uint8_t*)("powershell -NoProfile -Command \042$d='"), .len = sizeof("powershell -NoProfile -Command \042$d='") - 1 });
    buf_append((&(cmd)), bin_dir);
    buf_append((&(cmd)), (oc_string){ .ptr = (const uint8_t*)("'; $p=[Environment]::GetEnvironmentVariable('Path','User'); if (-not $p) { $p='' }; if (($p -split ';') -notcontains $d) { [Environment]::SetEnvironmentVariable('Path', ($p.TrimEnd(';') + ';' + $d).TrimStart(';'), 'User') }\042"), .len = sizeof("'; $p=[Environment]::GetEnvironmentVariable('Path','User'); if (-not $p) { $p='' }; if (($p -split ';') -notcontains $d) { [Environment]::SetEnvironmentVariable('Path', ($p.TrimEnd(';') + ';' + $d).TrimStart(';'), 'User') }\042") - 1 });
    }
    else {
    buf_append((&(cmd)), (oc_string){ .ptr = (const uint8_t*)("mkdir -p \042$HOME/.local/bin\042 && ln -sf "), .len = sizeof("mkdir -p \042$HOME/.local/bin\042 && ln -sf ") - 1 });
    buf_append((&(cmd)), exe);
    buf_append((&(cmd)), (oc_string){ .ptr = (const uint8_t*)(" \042$HOME/.local/bin/occ\042"), .len = sizeof(" \042$HOME/.local/bin/occ\042") - 1 });
    }
    int32_t ret = run_sys_cmd(buf_to_string((&(cmd))));
    buf_deinit((&(cmd)));
    if ((ret != 0)) {
    fwrite("error: install failed", 1, sizeof("error: install failed") - 1, stdout); fputc('\n', stdout);
    return 1;
    }
    if (host_is_windows) {
    fwrite("Added the oC bin directory to your user PATH. Open a new terminal, then run: occ --version", 1, sizeof("Added the oC bin directory to your user PATH. Open a new terminal, then run: occ --version") - 1, stdout); fputc('\n', stdout);
    }
    else {
    fwrite("Linked ~/.local/bin/occ. If 'occ' is not found, add ~/.local/bin to your PATH.", 1, sizeof("Linked ~/.local/bin/occ. If 'occ' is not found, add ~/.local/bin to your PATH.") - 1, stdout); fputc('\n', stdout);
    }
    return 0;
}

int32_t handle_self_cmd(bool is_install, oc_string argv0) {
    int32_t host = sys_host_target();
    bool host_is_windows = (host == OC_TARGET_WINDOWS_X86_64);
    Arena arena = arena_init(65536);
    oc_string exe = exe_path((&(arena)), argv0);
    oc_string exe_dir = dir_of_path((&(arena)), exe);
    int32_t ret = 1;
    if (is_install) {
    if (is_abs_path(exe)) {
    ret = handle_install(host, exe, exe_dir);
    }
    else {
    fwrite("error: cannot install: could not determine the absolute path of this occ binary", 1, sizeof("error: cannot install: could not determine the absolute path of this occ binary") - 1, stdout); fputc('\n', stdout);
    }
    }
    else {
    oc_string up = (oc_string){ .ptr = (const uint8_t*)("../"), .len = sizeof("../") - 1 };
    if (host_is_windows) {
    up = (oc_string){ .ptr = (const uint8_t*)("..\134"), .len = sizeof("..\134") - 1 };
    }
    oc_string cand = str_concat((&(arena)), exe_dir, up);
    oc_string repo = (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 };
    if (is_abs_path(exe)) {
    if (is_safe_path(cand, host_is_windows)) {
    repo = cand;
    }
    }
    ret = handle_update(host, repo);
    }
    arena_deinit((&(arena)));
    return ret;
}

int main(int argc, char** argv) {
    setvbuf(stdout, NULL, __OC_STDOUT_BUF, 0);
    if ((argc < 2)) {
    print_usage();
    return 0;
    }
    oc_string in_file_str = (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 };
    oc_string out_file_str = (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 };
    oc_string emit_c_str = (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 };
    oc_string opt_flag = (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 };
    bool is_run = false;
    bool is_native = false;
    bool fp_trap = false;
    bool arena_poison = false;
    oc_string target_name = (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 };
    bool target_explicit = false;
    int32_t i = 1;
    if ((i < argc)) {
    oc_string first = cstr_to_string(__OC_AT((argv), (i)));
    if (str_eq(first, (oc_string){ .ptr = (const uint8_t*)("version"), .len = sizeof("version") - 1 })) {
    fwrite("oC 1.0.2", 1, sizeof("oC 1.0.2") - 1, stdout); fputc('\n', stdout);
    return 0;
    }
    else {
    if (str_eq(first, (oc_string){ .ptr = (const uint8_t*)("--version"), .len = sizeof("--version") - 1 })) {
    fwrite("oC 1.0.2", 1, sizeof("oC 1.0.2") - 1, stdout); fputc('\n', stdout);
    return 0;
    }
    else {
    if (str_eq(first, (oc_string){ .ptr = (const uint8_t*)("-v"), .len = sizeof("-v") - 1 })) {
    fwrite("oC 1.0.2", 1, sizeof("oC 1.0.2") - 1, stdout); fputc('\n', stdout);
    return 0;
    }
    else {
    if (str_eq(first, (oc_string){ .ptr = (const uint8_t*)("help"), .len = sizeof("help") - 1 })) {
    print_usage();
    return 0;
    }
    else {
    if (str_eq(first, (oc_string){ .ptr = (const uint8_t*)("update"), .len = sizeof("update") - 1 })) {
    return handle_self_cmd(false, cstr_to_string(__OC_AT((argv), (0))));
    }
    else {
    if (str_eq(first, (oc_string){ .ptr = (const uint8_t*)("install"), .len = sizeof("install") - 1 })) {
    return handle_self_cmd(true, cstr_to_string(__OC_AT((argv), (0))));
    }
    else {
    if (str_eq(first, (oc_string){ .ptr = (const uint8_t*)("run"), .len = sizeof("run") - 1 })) {
    is_run = true;
    i = __OC_ADD(i, 1);
    }
    else {
    if (str_eq(first, (oc_string){ .ptr = (const uint8_t*)("build"), .len = sizeof("build") - 1 })) {
    i = __OC_ADD(i, 1);
    }
    }
    }
    }
    }
    }
    }
    }
    }
    while ((i < argc)) {
    oc_string arg = cstr_to_string(__OC_AT((argv), (i)));
    if (str_eq(arg, (oc_string){ .ptr = (const uint8_t*)("-o"), .len = sizeof("-o") - 1 })) {
    if ((__OC_ADD(i, 1) < argc)) {
    out_file_str = cstr_to_string(__OC_AT((argv), (__OC_ADD(i, 1))));
    i = __OC_ADD(i, 1);
    }
    }
    else {
    if (str_eq(arg, (oc_string){ .ptr = (const uint8_t*)("--emit-c"), .len = sizeof("--emit-c") - 1 })) {
    if ((__OC_ADD(i, 1) < argc)) {
    emit_c_str = cstr_to_string(__OC_AT((argv), (__OC_ADD(i, 1))));
    i = __OC_ADD(i, 1);
    }
    }
    else {
    if (str_eq(arg, (oc_string){ .ptr = (const uint8_t*)("--backend"), .len = sizeof("--backend") - 1 })) {
    if ((__OC_ADD(i, 1) < argc)) {
    oc_string backend_name = cstr_to_string(__OC_AT((argv), (__OC_ADD(i, 1))));
    if (str_eq(backend_name, (oc_string){ .ptr = (const uint8_t*)("native"), .len = sizeof("native") - 1 })) {
    is_native = true;
    }
    i = __OC_ADD(i, 1);
    }
    }
    else {
    if (str_eq(arg, (oc_string){ .ptr = (const uint8_t*)("--target"), .len = sizeof("--target") - 1 })) {
    if ((__OC_ADD(i, 1) < argc)) {
    target_name = cstr_to_string(__OC_AT((argv), (__OC_ADD(i, 1))));
    target_explicit = true;
    i = __OC_ADD(i, 1);
    }
    else {
    fwrite("error: --target requires a value", 1, sizeof("error: --target requires a value") - 1, stdout); fputc('\n', stdout);
    return 1;
    }
    }
    else {
    if (str_eq(arg, (oc_string){ .ptr = (const uint8_t*)("-O1"), .len = sizeof("-O1") - 1 })) {
    opt_flag = (oc_string){ .ptr = (const uint8_t*)(" -O1"), .len = sizeof(" -O1") - 1 };
    }
    else {
    if (str_eq(arg, (oc_string){ .ptr = (const uint8_t*)("-O2"), .len = sizeof("-O2") - 1 })) {
    opt_flag = (oc_string){ .ptr = (const uint8_t*)(" -O2"), .len = sizeof(" -O2") - 1 };
    }
    else {
    if (str_eq(arg, (oc_string){ .ptr = (const uint8_t*)("-O3"), .len = sizeof("-O3") - 1 })) {
    opt_flag = (oc_string){ .ptr = (const uint8_t*)(" -O3"), .len = sizeof(" -O3") - 1 };
    }
    else {
    if (str_eq(arg, (oc_string){ .ptr = (const uint8_t*)("--fp-trap"), .len = sizeof("--fp-trap") - 1 })) {
    fp_trap = true;
    }
    else {
    if (str_eq(arg, (oc_string){ .ptr = (const uint8_t*)("--arena-poison"), .len = sizeof("--arena-poison") - 1 })) {
    arena_poison = true;
    }
    else {
    if (str_eq(arg, (oc_string){ .ptr = (const uint8_t*)("--help"), .len = sizeof("--help") - 1 })) {
    print_usage();
    return 0;
    }
    else {
    if (str_eq(arg, (oc_string){ .ptr = (const uint8_t*)("-h"), .len = sizeof("-h") - 1 })) {
    print_usage();
    return 0;
    }
    else {
    if ((((arg).len) > 0)) {
    if ((__oc_index_str(arg, 0) == 45)) {
    fwrite("error: unknown option:", 1, sizeof("error: unknown option:") - 1, stdout); fputc('\n', stdout);
    __oc_print(arg);
    print_usage();
    return 1;
    }
    in_file_str = arg;
    }
    }
    }
    }
    }
    }
    }
    }
    }
    }
    }
    }
    i = __OC_ADD(i, 1);
    }
    if ((((in_file_str).len) == 0)) {
    print_usage();
    return 1;
    }
    int32_t host = sys_host_target();
    bool host_is_windows = (host == OC_TARGET_WINDOWS_X86_64);
    int32_t target = host;
    if (target_explicit) {
    target = parse_target_name(target_name);
    if ((target < 0)) {
    fwrite("error: unknown --target (expected linux-x86_64, windows-x86_64, macos-x86_64 or macos-aarch64):", 1, sizeof("error: unknown --target (expected linux-x86_64, windows-x86_64, macos-x86_64 or macos-aarch64):") - 1, stdout); fputc('\n', stdout);
    __oc_print(target_name);
    return 1;
    }
    }
    int32_t out_target = target;
    if (is_native) {
    if ((target != OC_TARGET_WINDOWS_X86_64)) {
    if ((target != OC_TARGET_LINUX_X86_64)) {
    if (target_explicit) {
    fwrite("error: --backend native emits linux-x86_64 (ELF64) or windows-x86_64 (PE32+); use --backend c for target:", 1, sizeof("error: --backend native emits linux-x86_64 (ELF64) or windows-x86_64 (PE32+); use --backend c for target:") - 1, stdout); fputc('\n', stdout);
    __oc_print(target_name);
    return 1;
    }
    out_target = OC_TARGET_LINUX_X86_64;
    }
    }
    }
    oc_string cc_cmd = (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 };
    if ((!is_native)) {
    cc_cmd = c_compiler_for(target, host);
    if ((((cc_cmd).len) == 0)) {
    fwrite("error: no C cross-compiler convention for this host and target (macOS targets need a macOS host):", 1, sizeof("error: no C cross-compiler convention for this host and target (macOS targets need a macOS host):") - 1, stdout); fputc('\n', stdout);
    __oc_print(target_name);
    return 1;
    }
    }
    if (is_run) {
    if (target_explicit) {
    if ((out_target != host)) {
    fwrite("error: cannot run a binary for another platform on this host; use 'occ build' for target:", 1, sizeof("error: cannot run a binary for another platform on this host; use 'occ build' for target:") - 1, stdout); fputc('\n', stdout);
    __oc_print(target_name);
    return 1;
    }
    }
    }
    if ((!is_safe_path(in_file_str, host_is_windows))) {
    fwrite("error: unsafe characters in file path", 1, sizeof("error: unsafe characters in file path") - 1, stdout); fputc('\n', stdout);
    return 1;
    }
    bool is_temp_out = false;
    if (is_run) {
    if ((((out_file_str).len) == 0)) {
    is_temp_out = true;
    }
    }
    if ((!is_temp_out)) {
    if ((((out_file_str).len) == 0)) {
    if ((out_target == OC_TARGET_WINDOWS_X86_64)) {
    out_file_str = (oc_string){ .ptr = (const uint8_t*)("a.exe"), .len = sizeof("a.exe") - 1 };
    }
    else {
    out_file_str = (oc_string){ .ptr = (const uint8_t*)("a.out"), .len = sizeof("a.out") - 1 };
    }
    }
    if ((!is_safe_path(out_file_str, host_is_windows))) {
    fwrite("error: unsafe characters in file path", 1, sizeof("error: unsafe characters in file path") - 1, stdout); fputc('\n', stdout);
    return 1;
    }
    }
    Arena arena = arena_init(33554432);
    if ((!is_temp_out)) {
    if ((out_target == OC_TARGET_WINDOWS_X86_64)) {
    if ((!ends_with_exe(out_file_str))) {
    out_file_str = str_concat((&(arena)), out_file_str, (oc_string){ .ptr = (const uint8_t*)(".exe"), .len = sizeof(".exe") - 1 });
    }
    }
    }
    oc_string std_root = dir_of_path((&(arena)), exe_path((&(arena)), cstr_to_string(__OC_AT((argv), (0)))));
    oc_string oc_home = env_str((oc_string){ .ptr = (const uint8_t*)("OC_HOME"), .len = sizeof("OC_HOME") - 1 });
    if ((((oc_home).len) > 0)) {
    std_root = str_concat((&(arena)), oc_home, (oc_string){ .ptr = (const uint8_t*)("/"), .len = sizeof("/") - 1 });
    }
    ModuleLoader loader = (ModuleLoader){ .visited = { (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 } }, .visited_count = 0, .map = (SourceMap){ .count = 0, .first_line = { 0 }, .path = { (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 } } }, .lines = 0 };
    CodeBuffer combined_buf = buf_init(4194304);
    bool load_ok = load_module_recursive(in_file_str, (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 }, std_root, (&(combined_buf)), (&(loader)), (&(arena)));
    if ((!load_ok)) {
    buf_deinit((&(combined_buf)));
    arena_deinit((&(arena)));
    return 1;
    }
    if (source_uses_floats(buf_to_string((&(combined_buf))))) {
    bool fmt_ok = load_module_recursive((oc_string){ .ptr = (const uint8_t*)("std/sys/float.oc"), .len = sizeof("std/sys/float.oc") - 1 }, (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 }, std_root, (&(combined_buf)), (&(loader)), (&(arena)));
    if ((!fmt_ok)) {
    fwrite("error: the float formatter std/sys/float.oc was not found; set OC_HOME to the directory that contains std/", 1, sizeof("error: the float formatter std/sys/float.oc was not found; set OC_HOME to the directory that contains std/") - 1, stdout); fputc('\n', stdout);
    buf_deinit((&(combined_buf)));
    arena_deinit((&(arena)));
    return 1;
    }
    }
    if (is_native) {
    if ((out_target == OC_TARGET_WINDOWS_X86_64)) {
    bool rt_ok = load_module_recursive((oc_string){ .ptr = (const uint8_t*)("std/sys/windows.oc"), .len = sizeof("std/sys/windows.oc") - 1 }, (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 }, std_root, (&(combined_buf)), (&(loader)), (&(arena)));
    if ((!rt_ok)) {
    fwrite("error: the native Windows runtime std/sys/windows.oc was not found; set OC_HOME to the directory that contains std/", 1, sizeof("error: the native Windows runtime std/sys/windows.oc was not found; set OC_HOME to the directory that contains std/") - 1, stdout); fputc('\n', stdout);
    buf_deinit((&(combined_buf)));
    arena_deinit((&(arena)));
    return 1;
    }
    }
    }
    oc_string combined_src = buf_to_string((&(combined_buf)));
    Parser parser = parser_init(combined_src, (&(arena)));
    ((parser).srcmap) = (&(((loader).map)));
    if ((((((parser).current)).tag) == TOK_ERROR)) {
    parser_report_lex_error((&(parser)), ((parser).current));
    }
    AstNode* prog = parse_program((&(parser)));
    if (((parser).has_error)) {
    fwrite("compilation aborted due to parse errors", 1, sizeof("compilation aborted due to parse errors") - 1, stdout); fputc('\n', stdout);
    buf_deinit((&(combined_buf)));
    arena_deinit((&(arena)));
    return 1;
    }
    Generics gen = (Generics){ .tmpl = { ((AstNode*)(0)) }, .tmpl_count = 0, .inst_name = { (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 } }, .inst_tmpl = { (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 } }, .inst_args = { (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 } }, .inst_site = { (Token){ .tag = 0, .slice = (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 }, .line = 0, .col = 0 } }, .inst_count = 0, .arena = (&(arena)), .m = (&(((loader).map))), .root = prog, .last = ((AstNode*)(0)), .ok = true };
    if ((!desugar_errors(prog, (&(arena)), (&(((loader).map)))))) {
    buf_deinit((&(combined_buf)));
    arena_deinit((&(arena)));
    return 1;
    }
    if ((!expand_generics(prog, (&(arena)), (&(((loader).map))), (&(gen))))) {
    buf_deinit((&(combined_buf)));
    arena_deinit((&(arena)));
    return 1;
    }
    bool valid = validate_program(prog, (&(((loader).map))), (&(gen)));
    if ((!valid)) {
    buf_deinit((&(combined_buf)));
    arena_deinit((&(arena)));
    return 1;
    }
    oc_string tmp_dir = (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 };
    bool need_tmp = is_temp_out;
    if ((!is_native)) {
    need_tmp = true;
    }
    if (need_tmp) {
    tmp_dir = make_private_tmp_dir((&(arena)), host_is_windows);
    if ((((tmp_dir).len) == 0)) {
    fwrite("error: failed to create a private temporary directory (tried $TMPDIR, $TEMP, /tmp and .)", 1, sizeof("error: failed to create a private temporary directory (tried $TMPDIR, $TEMP, /tmp and .)") - 1, stdout); fputc('\n', stdout);
    buf_deinit((&(combined_buf)));
    arena_deinit((&(arena)));
    return 1;
    }
    }
    if (is_temp_out) {
    if ((out_target == OC_TARGET_WINDOWS_X86_64)) {
    out_file_str = str_concat((&(arena)), tmp_dir, (oc_string){ .ptr = (const uint8_t*)("/run.exe"), .len = sizeof("/run.exe") - 1 });
    }
    else {
    out_file_str = str_concat((&(arena)), tmp_dir, (oc_string){ .ptr = (const uint8_t*)("/run"), .len = sizeof("/run") - 1 });
    }
    }
    if (is_native) {
    bool ok_native = false;
    if ((out_target == OC_TARGET_WINDOWS_X86_64)) {
    ok_native = emit_native_pe(prog, out_file_str, fp_trap, arena_poison, (&(((loader).map))));
    }
    else {
    ok_native = emit_native_elf(prog, out_file_str, fp_trap, arena_poison, (&(((loader).map))));
    }
    buf_deinit((&(combined_buf)));
    if ((!ok_native)) {
    fwrite("error: native image emission failed", 1, sizeof("error: native image emission failed") - 1, stdout); fputc('\n', stdout);
    remove_private_tmp_dir((&(arena)), tmp_dir);
    arena_deinit((&(arena)));
    return 1;
    }
    }
    else {
    CodeBuffer out = buf_init(4194304);
    ((out).fp_trap) = fp_trap;
    ((out).arena_poison) = arena_poison;
    ((out).srcmap) = (&(((loader).map)));
    codegen_program(prog, (&(out)));
    buf_deinit((&(combined_buf)));
    oc_string stage_c_path = str_concat((&(arena)), tmp_dir, (oc_string){ .ptr = (const uint8_t*)("/stage.c"), .len = sizeof("/stage.c") - 1 });
    oc_string c_code = buf_to_string((&(out)));
    if ((((emit_c_str).len) > 0)) {
    if (is_safe_path(emit_c_str, host_is_windows)) {
    write_file(emit_c_str, c_code);
    }
    }
    bool ok = write_file(stage_c_path, c_code);
    if ((!ok)) {
    fwrite("error: failed to write temporary C stage file", 1, sizeof("error: failed to write temporary C stage file") - 1, stdout); fputc('\n', stdout);
    buf_deinit((&(out)));
    remove_private_tmp_dir((&(arena)), tmp_dir);
    arena_deinit((&(arena)));
    return 1;
    }
    CodeBuffer cmd_buf = buf_init(512);
    buf_append((&(cmd_buf)), cc_cmd);
    buf_append((&(cmd_buf)), opt_flag);
    buf_append((&(cmd_buf)), (oc_string){ .ptr = (const uint8_t*)(" -ffp-contract=off -fno-math-errno"), .len = sizeof(" -ffp-contract=off -fno-math-errno") - 1 });
    if ((out_target == OC_TARGET_WINDOWS_X86_64)) {
    buf_append((&(cmd_buf)), (oc_string){ .ptr = (const uint8_t*)(" -Wl,--no-insert-timestamp"), .len = sizeof(" -Wl,--no-insert-timestamp") - 1 });
    }
    buf_append((&(cmd_buf)), (oc_string){ .ptr = (const uint8_t*)(" "), .len = sizeof(" ") - 1 });
    buf_append((&(cmd_buf)), stage_c_path);
    buf_append((&(cmd_buf)), (oc_string){ .ptr = (const uint8_t*)(" -o "), .len = sizeof(" -o ") - 1 });
    buf_append((&(cmd_buf)), out_file_str);
    buf_append_u8((&(cmd_buf)), 0);
    int32_t ret = system(((uint8_t*)(((cmd_buf).buffer))));
    buf_deinit((&(cmd_buf)));
    buf_deinit((&(out)));
    unlink_file(stage_c_path);
    if ((ret != 0)) {
    fwrite("error: C compiler failed:", 1, sizeof("error: C compiler failed:") - 1, stdout); fputc('\n', stdout);
    __oc_print(cc_cmd);
    remove_private_tmp_dir((&(arena)), tmp_dir);
    arena_deinit((&(arena)));
    return 1;
    }
    }
    if (is_run) {
    CodeBuffer run_cmd = buf_init(256);
    if (host_is_windows) {
    if ((!is_abs_path(out_file_str))) {
    buf_append_u8((&(run_cmd)), 46);
    buf_append_u8((&(run_cmd)), 92);
    }
    size_t ci = 0;
    while ((ci < ((out_file_str).len))) {
    if ((__oc_index_str(out_file_str, ci) == 47)) {
    buf_append_u8((&(run_cmd)), 92);
    }
    else {
    buf_append_u8((&(run_cmd)), __oc_index_str(out_file_str, ci));
    }
    ci = __OC_ADD(ci, 1);
    }
    }
    else {
    if ((__oc_index_str(out_file_str, 0) != 47)) {
    buf_append((&(run_cmd)), (oc_string){ .ptr = (const uint8_t*)("./"), .len = sizeof("./") - 1 });
    }
    buf_append((&(run_cmd)), out_file_str);
    }
    buf_append_u8((&(run_cmd)), 0);
    int32_t raw_ret = system(((uint8_t*)(((run_cmd).buffer))));
    buf_deinit((&(run_cmd)));
    remove_private_tmp_dir((&(arena)), tmp_dir);
    arena_deinit((&(arena)));
    int32_t exit_code = raw_ret;
    if ((!host_is_windows)) {
    if ((raw_ret < 0)) {
    exit_code = 1;
    }
    else {
    int32_t sig = __OC_MOD(raw_ret, 128);
    if ((sig != 0)) {
    exit_code = __OC_ADD(128, sig);
    }
    else {
    exit_code = __OC_MOD(__OC_DIV(raw_ret, 256), 256);
    }
    }
    }
    return exit_code;
    }
    remove_private_tmp_dir((&(arena)), tmp_dir);
    arena_deinit((&(arena)));
    return 0;
    return 0;
}

size_t fmt_put_u64(uint8_t* buf, size_t n, uint64_t v) {
    uint8_t tmp[20] = {0};
    size_t count = 0;
    uint64_t x = v;
    if ((x == 0)) {
    __OC_AT((buf), (n)) = 48;
    return __OC_ADD(n, 1);
    }
    while ((x > 0)) {
    __OC_AT((tmp), (count)) = ({ __auto_type _v = (__OC_ADD(48, __OC_MOD(x, 10))); if (_v < 0 || (uint64_t)_v > 255ULL) { fprintf(stderr, "panic: invalid unsigned cast\n"); exit(1); } (uint8_t)_v; });
    x = __OC_DIV(x, 10);
    count = __OC_ADD(count, 1);
    }
    size_t pos = n;
    while ((count > 0)) {
    count = __OC_SUB(count, 1);
    __OC_AT((buf), (pos)) = __OC_AT((tmp), (count));
    pos = __OC_ADD(pos, 1);
    }
    return pos;
}

size_t fmt_put_fixed6(uint8_t* buf, size_t n, double v) {
    uint64_t whole = ({ double _f = (double)(v); if (!(_f > -1.0 && _f < 18446744073709551616.0)) { fprintf(stderr, "panic: invalid unsigned cast\n"); exit(1); } (uint64_t)_f; });
    double frac = (v - ((double)(whole)));
    uint64_t millionths = ({ double _f = (double)(((frac * 1000000.0) + 0.5)); if (!(_f > -1.0 && _f < 18446744073709551616.0)) { fprintf(stderr, "panic: invalid unsigned cast\n"); exit(1); } (uint64_t)_f; });
    if ((millionths >= 1000000)) {
    whole = __OC_ADD(whole, 1);
    millionths = __OC_SUB(millionths, 1000000);
    }
    size_t pos = fmt_put_u64(buf, n, whole);
    __OC_AT((buf), (pos)) = 46;
    pos = __OC_ADD(pos, 1);
    uint64_t div = 100000;
    while ((div > 0)) {
    __OC_AT((buf), (pos)) = ({ __auto_type _v = (__OC_ADD(48, __OC_MOD(__OC_DIV(millionths, div), 10))); if (_v < 0 || (uint64_t)_v > 255ULL) { fprintf(stderr, "panic: invalid unsigned cast\n"); exit(1); } (uint8_t)_v; });
    pos = __OC_ADD(pos, 1);
    div = __OC_DIV(div, 10);
    }
    return pos;
}

void fmt_print_f64(double x) {
    if ((!(x <= x))) {
    sys_write(1, (((oc_string){ .ptr = (const uint8_t*)("NaN\012"), .len = sizeof("NaN\012") - 1 }).ptr), 4);
    return;
    }
    uint8_t buf[48] = {0};
    uint8_t* b = ((uint8_t*)((&(buf))));
    size_t n = 0;
    double v = x;
    if (((f64_to_bits(x) >> 63) == 1)) {
    __OC_AT((b), (0)) = 45;
    n = 1;
    v = (-v);
    }
    if ((v > 1.7976931348623157e308)) {
    __OC_AT((b), (n)) = 105;
    __OC_AT((b), (__OC_ADD(n, 1))) = 110;
    __OC_AT((b), (__OC_ADD(n, 2))) = 102;
    n = __OC_ADD(n, 3);
    }
    else {
    if ((v < 1.0e15)) {
    if ((v >= 0.0001)) {
    n = fmt_put_fixed6(b, n, v);
    }
    else {
    if ((v > 0.0)) {
    n = fmt_put_sci(b, n, v);
    }
    else {
    n = fmt_put_fixed6(b, n, 0.0);
    }
    }
    }
    else {
    n = fmt_put_sci(b, n, v);
    }
    }
    __OC_AT((b), (n)) = 10;
    sys_write(1, ((uint8_t*)(b)), __OC_ADD(n, 1));
}

size_t fmt_put_sci(uint8_t* buf, size_t n, double v) {
    double m = v;
    int32_t e = 0;
    while ((m >= 1.0e16)) {
    m = (m / 1.0e16);
    e = __OC_ADD(e, 16);
    }
    while ((m < 1.0e-15)) {
    m = (m * 1.0e16);
    e = __OC_SUB(e, 16);
    }
    while ((m >= 10.0)) {
    m = (m / 10.0);
    e = __OC_ADD(e, 1);
    }
    while ((m < 1.0)) {
    m = (m * 10.0);
    e = __OC_SUB(e, 1);
    }
    if ((m >= 9.9999995)) {
    m = 1.0;
    e = __OC_ADD(e, 1);
    }
    size_t pos = fmt_put_fixed6(buf, n, m);
    __OC_AT((buf), (pos)) = 101;
    pos = __OC_ADD(pos, 1);
    int32_t mag = e;
    if ((e < 0)) {
    __OC_AT((buf), (pos)) = 45;
    mag = __OC_SUB(0, e);
    }
    else {
    __OC_AT((buf), (pos)) = 43;
    }
    pos = __OC_ADD(pos, 1);
    if ((mag < 10)) {
    __OC_AT((buf), (pos)) = 48;
    pos = __OC_ADD(pos, 1);
    }
    return fmt_put_u64(buf, pos, ({ __auto_type _v = (mag); if (_v < 0) { fprintf(stderr, "panic: invalid unsigned cast\n"); exit(1); } (uint64_t)_v; }));
}

