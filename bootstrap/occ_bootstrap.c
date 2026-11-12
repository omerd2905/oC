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
static inline void sys_exit(int32_t code) { _exit(code); }
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
static const uint32_t NODE_BREAK = 27;
static const uint32_t NODE_CONTINUE = 28;
static const size_t NG_IMP_EXIT_PROCESS = 0;
static const size_t NG_IMP_GET_STD_HANDLE = 1;
static const size_t NG_IMP_WRITE_FILE = 2;
static const size_t NG_IMP_COUNT = 3;

typedef struct Arena Arena;
typedef struct Token Token;
typedef struct Lexer Lexer;
typedef struct AstNode AstNode;
typedef struct Parser Parser;
typedef struct VarInfo VarInfo;
typedef struct FnEffects FnEffects;
typedef struct Validator Validator;
typedef struct CodeBuffer CodeBuffer;
typedef struct NativeGen NativeGen;
typedef struct ModuleLoader ModuleLoader;

struct Arena {
    size_t offset;
    size_t capacity;
    uint8_t* buffer;
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

struct Parser {
    Lexer lex;
    Token current;
    Arena* arena;
    bool allow_struct_init;
    bool has_error;
    int32_t loop_depth;
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
    size_t count;
};

struct Validator {
    FnEffects* fx;
    VarInfo vars[512];
    size_t vars_count;
    oc_string deinit_arenas[128];
    size_t deinit_count;
    uint32_t scope_depth;
    bool has_error;
    oc_string initializing_var;
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
};

struct NativeGen {
    oc_string fn_names[256];
    size_t fn_offsets[256];
    oc_string fn_ret_types[256];
    size_t fn_count;
    size_t fixup_offsets[1024];
    oc_string fixup_names[1024];
    bool fixup_is_6byte[1024];
    size_t fixup_count;
    size_t data_reloc_code[512];
    size_t data_reloc_data[512];
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
    oc_string struct_names[64];
    int32_t struct_sizes[64];
    size_t struct_count;
    oc_string field_struct[256];
    oc_string field_names[256];
    int32_t field_offsets[256];
    oc_string field_types[256];
    bool field_packed[256];
    size_t field_count;
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
    size_t iat_fix_code[16];
    size_t iat_fix_slot[16];
    size_t iat_fix_count;
};

struct ModuleLoader {
    oc_string visited[128];
    size_t visited_count;
};

int32_t read_int(void);
uint8_t* cstr_path(oc_string path, uint8_t* out_buf);
int64_t read_file(oc_string path, uint8_t* out_buf, size_t max_len);
bool write_file(oc_string path, oc_string content);
bool chmod_exec(oc_string path);
bool unlink_file(oc_string path);
Arena arena_init(size_t capacity);
uint8_t* arena_alloc(Arena* arena, size_t size);
void arena_reset(Arena* arena);
void arena_deinit(Arena* arena);
bool is_keyword(uint32_t tag);
Lexer lexer_init(oc_string source);
bool str_eq(oc_string a, oc_string b);
bool is_space(uint8_t c);
bool is_digit(uint8_t c);
bool is_hex_digit(uint8_t c);
bool is_ident_start(uint8_t c);
bool is_ident_char(uint8_t c);
uint32_t check_keyword(oc_string s);
Token lexer_next(Lexer* lex);
AstNode* ast_alloc_node(Arena* arena, uint32_t tag, Token tok);
Parser parser_init(oc_string source, Arena* arena);
void parser_advance(Parser* p);
Token parser_expect(Parser* p, uint32_t expected_tag);
uint32_t get_precedence(uint32_t tag);
oc_string parse_type(Parser* p);
AstNode* parse_expr(Parser* p, uint32_t min_prec);
AstNode* parse_primary(Parser* p);
AstNode* parse_postfix(Parser* p);
AstNode* parse_unary(Parser* p);
AstNode* parse_expr(Parser* p, uint32_t min_prec);
AstNode* parse_stmt(Parser* p);
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
void print_err_ref_invalidated(oc_string name, oc_string bv);
void print_err_uaf_reset(oc_string name, oc_string arena);
void print_err_uaf_deinit(oc_string name, oc_string arena);
void print_err_borrow_escape(oc_string src, oc_string tgt);
void print_err_cross_arena(oc_string arena, oc_string tgt);
void print_err_ret_local(oc_string src);
void print_err_ret_arena(oc_string arena);
bool starts_with(oc_string s, oc_string prefix);
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
oc_string get_arena_arg_name(AstNode* arg);
bool fx_scan(FnEffects* fx, size_t slot, AstNode* params, AstNode* n);
oc_string get_arena_source(AstNode* expr);
oc_string get_expr_arena_borrow(Validator* v, AstNode* expr);
oc_string get_local_array_slice_source(Validator* v, AstNode* expr);
oc_string contains_stack_ref(Validator* v, AstNode* expr);
bool check_expr_use(Validator* v, AstNode* expr);
bool validate_stmt(Validator* v, AstNode* stmt);
bool validate_function(AstNode* f, FnEffects* fx);
bool validate_program(AstNode* root);
AstNode* parse_program(Parser* p);
CodeBuffer buf_init(size_t capacity);
void buf_append_u8(CodeBuffer* buf, uint8_t byte);
void buf_append(CodeBuffer* buf, oc_string text);
oc_string buf_to_string(CodeBuffer* buf);
void buf_deinit(CodeBuffer* buf);
oc_string to_c_type(oc_string type_name);
void emit_c_type(oc_string type_name, CodeBuffer* out);
bool is_type_ptr(oc_string t);
bool is_type_str(oc_string t);
bool is_expr_string(AstNode* expr, CodeBuffer* out);
bool is_expr_pointer(AstNode* expr, CodeBuffer* out);
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
void ng_fail(NativeGen* ng, oc_string msg);
void ng_add_fixup(NativeGen* ng, size_t code_off, oc_string fn_name, bool is_6byte);
void ng_add_data_reloc(NativeGen* ng, size_t code_off, size_t data_off);
void ng_add_iat_fixup(NativeGen* ng, size_t code_off, size_t slot);
void ng_emit_call(NativeGen* ng, CodeBuffer* code, oc_string fn_name);
void ng_emit_iat_call(NativeGen* ng, CodeBuffer* code, size_t slot);
uint64_t parse_u64(oc_string s);
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
oc_string ng_expr_type(NativeGen* ng, AstNode* expr);
bool ng_is_unsigned_expr(NativeGen* ng, AstNode* expr);
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
void ng_emit_range_check(NativeGen* ng, CodeBuffer* code, oc_string t);
void ng_emit_load_at_rax(CodeBuffer* code, oc_string t, int32_t width);
void ng_emit_store_at_rcx(CodeBuffer* code, int32_t width);
void ng_emit_expr(NativeGen* ng, CodeBuffer* code, CodeBuffer* data, AstNode* expr);
void ng_emit_lvalue_addr(NativeGen* ng, CodeBuffer* code, CodeBuffer* data, AstNode* expr);
void ng_emit_expr(NativeGen* ng, CodeBuffer* code, CodeBuffer* data, AstNode* expr);
void ng_emit_print_lit(NativeGen* ng, CodeBuffer* code, CodeBuffer* data, oc_string raw);
void ng_emit_stmt(NativeGen* ng, CodeBuffer* code, CodeBuffer* data, AstNode* stmt);
int32_t calc_locals_bytes_stmt(NativeGen* ng, AstNode* stmt);
void ng_emit_store_param_reg(CodeBuffer* code, size_t reg_idx, int32_t off);
void ng_define_helper(NativeGen* ng, bool* has_offset, oc_string name, size_t off);
void ng_emit_write_fd(NativeGen* ng, CodeBuffer* code);
void ng_emit_win_runtime(NativeGen* ng, CodeBuffer* code, bool* has_offset);
size_t pe_align(size_t v, size_t a);
void pe_append_zeros(CodeBuffer* buf, size_t n);
void pe_append_section_header(CodeBuffer* pe, oc_string name, size_t vsize, size_t rva, size_t raw_size, size_t raw_off, uint32_t flags);
bool ng_write_pe(NativeGen* ng, CodeBuffer* code, CodeBuffer* data, oc_string out_file_str);
bool emit_native_image(AstNode* root, oc_string out_file_str, int32_t target);
bool emit_native_elf(AstNode* root, oc_string out_file_str);
bool emit_native_pe(AstNode* root, oc_string out_file_str);
bool ng_write_elf(NativeGen* ng, CodeBuffer* code, CodeBuffer* data, oc_string out_file_str);
oc_string cstr_to_string(uint8_t* cstr);
bool is_safe_path(oc_string s, bool allow_backslash);
oc_string arena_dup_str(Arena* arena, oc_string s);
oc_string str_concat(Arena* arena, oc_string a, oc_string b);
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
void print_usage(void);
int32_t run_sys_cmd(oc_string cmd);
oc_string exe_path(Arena* arena, oc_string argv0);
int32_t handle_update(int32_t host, oc_string repo);
int32_t handle_install(int32_t host, oc_string exe, oc_string exe_dir);
int32_t handle_self_cmd(bool is_install, oc_string argv0);
int main(int argc, char** argv);

int32_t read_int(void) {
    uint8_t buf[32] = {0};
    int64_t bytes = sys_read(0, ((uint8_t*)((&(buf)))), 31);
    if ((bytes <= 0)) {
    return 0;
    }
    int32_t res = 0;
    size_t i = 0;
    while ((i < ({ __auto_type _v = (bytes); if (_v < 0) { fprintf(stderr, "panic: invalid unsigned cast\n"); exit(1); } (size_t)_v; }))) {
    uint8_t c = __OC_AT((buf), (i));
    if ((c >= 48)) {
    if ((c <= 57)) {
    res = __OC_ADD(__OC_MUL(res, 10), ({ __auto_type _v = (__OC_SUB(c, 48)); if ((_v < 0 && (int64_t)_v < -2147483648LL) || (!(_v < 0) && (uint64_t)_v > 2147483647ULL)) { fprintf(stderr, "panic: invalid signed cast\n"); exit(1); } (int32_t)_v; }));
    }
    }
    i = __OC_ADD(i, 1);
    }
    return res;
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

int64_t read_file(oc_string path, uint8_t* out_buf, size_t max_len) {
    uint8_t pbuf[4096] = {0};
    uint8_t* p = cstr_path(path, ((uint8_t*)((&(pbuf)))));
    int32_t fd = sys_open(p, OC_O_SAFE_READ, 0);
    if ((fd < 0)) {
    return (-1);
    }
    int64_t n = sys_read(fd, out_buf, max_len);
    sys_close(fd);
    return n;
}

bool write_file(oc_string path, oc_string content) {
    uint8_t pbuf[4096] = {0};
    uint8_t* p = cstr_path(path, ((uint8_t*)((&(pbuf)))));
    int32_t fd = sys_open(p, OC_O_SAFE_WRITE_TRUNC, OC_MODE_FILE);
    if ((fd < 0)) {
    return false;
    }
    int64_t n = sys_write(fd, ((uint8_t*)(((content).ptr))), ((content).len));
    sys_close(fd);
    return (n >= 0);
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
    size_t i = 0;
    while ((i < ((arena)->offset))) {
    __OC_AT((((arena)->buffer)), (i)) = 0;
    i = __OC_ADD(i, 1);
    }
    ((arena)->offset) = 0;
}

void arena_deinit(Arena* arena) {
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

bool is_space(uint8_t c) {
    if ((c == 32)) {
    return true;
    }
    if ((c == 9)) {
    return true;
    }
    if ((c == 13)) {
    return true;
    }
    return false;
}

bool is_digit(uint8_t c) {
    if ((c >= 48)) {
    if ((c <= 57)) {
    return true;
    }
    }
    return false;
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
    oc_string slice = __oc_slice_str(((lex)->source), start_idx, ((lex)->index));
    return (Token){ .tag = TOK_INT, .slice = slice, .line = tok_line, .col = tok_col };
    }
    if ((c == 34)) {
    ((lex)->index) = __OC_ADD(((lex)->index), 1);
    ((lex)->col) = __OC_ADD(((lex)->col), 1);
    while ((((lex)->index) < ((((lex)->source)).len))) {
    uint8_t sc = __oc_index_str(((lex)->source), ((lex)->index));
    if ((sc == 92)) {
    ((lex)->index) = __OC_ADD(((lex)->index), 2);
    ((lex)->col) = __OC_ADD(((lex)->col), 2);
    }
    else {
    if ((sc == 34)) {
    ((lex)->index) = __OC_ADD(((lex)->index), 1);
    ((lex)->col) = __OC_ADD(((lex)->col), 1);
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
    return (Token){ .tag = TOK_STRING, .slice = slice, .line = tok_line, .col = tok_col };
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
    printf("%s\n", "lexical error: unexpected character at line");
    __oc_print(tok_line);
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

Parser parser_init(oc_string source, Arena* arena) {
    Lexer lex = lexer_init(source);
    Token cur = lexer_next((&(lex)));
    bool err = false;
    if ((((cur).tag) == TOK_ERROR)) {
    err = true;
    }
    return (Parser){ .lex = lex, .current = cur, .arena = arena, .allow_struct_init = true, .has_error = err, .loop_depth = 0 };
}

void parser_advance(Parser* p) {
    ((p)->current) = lexer_next((&(((p)->lex))));
    if ((((((p)->current)).tag) == TOK_ERROR)) {
    ((p)->has_error) = true;
    }
}

Token parser_expect(Parser* p, uint32_t expected_tag) {
    Token tok = ((p)->current);
    if ((((tok).tag) == expected_tag)) {
    parser_advance(p);
    }
    else {
    printf("%s\n", "parse error: unexpected token");
    __oc_print(((tok).tag));
    __oc_print(((tok).slice));
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
    size_t len = __OC_SUB(((size_t)(end_ptr)), ((size_t)(start_ptr)));
    return (oc_string){ .ptr = start_ptr, .len = len };
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
    if (((p)->allow_struct_init)) {
    if ((((((p)->current)).tag) == TOK_LBRACE)) {
    parser_advance(p);
    AstNode* s_node = ast_alloc_node(((p)->arena), NODE_STRUCT_INIT, tok);
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
    return ast_alloc_node(((p)->arena), NODE_IDENT, tok);
    }
    if ((((tok).tag) == TOK_LPAREN)) {
    parser_advance(p);
    AstNode* inner = parse_expr(p, 0);
    parser_expect(p, TOK_RPAREN);
    return inner;
    }
    printf("%s\n", "parse error: unexpected token in expression");
    __oc_print(((tok).slice));
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
    return left;
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
    printf("%s\n", "parse error: 'break' outside of loop");
    ((p)->has_error) = true;
    }
    return ast_alloc_node(((p)->arena), NODE_BREAK, tok);
    }
    if ((((((p)->current)).tag) == TOK_CONTINUE)) {
    Token tok = ((p)->current);
    parser_advance(p);
    parser_expect(p, TOK_SEMICOLON);
    if ((((p)->loop_depth) <= 0)) {
    printf("%s\n", "parse error: 'continue' outside of loop");
    ((p)->has_error) = true;
    }
    return ast_alloc_node(((p)->arena), NODE_CONTINUE, tok);
    }
    if ((((((p)->current)).tag) == TOK_LBRACE)) {
    return parse_block(p);
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
    return node;
}

AstNode* parse_fn_decl(Parser* p) {
    Token fn_tok = parser_expect(p, TOK_FN);
    Token name_tok = parser_expect(p, TOK_IDENT);
    AstNode* fn_node = ast_alloc_node(((p)->arena), NODE_FN, name_tok);
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
    return fn_node;
}

void print_str_raw(oc_string s) {
    sys_write(1, ((s).ptr), ((s).len));
}

void print_err_ref_invalidated(oc_string name, oc_string bv) {
    print_str_raw((oc_string){ .ptr = (const uint8_t*)("error: reference '"), .len = sizeof("error: reference '") - 1 });
    print_str_raw(name);
    print_str_raw((oc_string){ .ptr = (const uint8_t*)("' invalidated by mutable operation on '"), .len = sizeof("' invalidated by mutable operation on '") - 1 });
    print_str_raw(bv);
    print_str_raw((oc_string){ .ptr = (const uint8_t*)("'\n"), .len = sizeof("'\n") - 1 });
}

void print_err_uaf_reset(oc_string name, oc_string arena) {
    print_str_raw((oc_string){ .ptr = (const uint8_t*)("error: use-after-free: variable '"), .len = sizeof("error: use-after-free: variable '") - 1 });
    print_str_raw(name);
    print_str_raw((oc_string){ .ptr = (const uint8_t*)("' used after arena '"), .len = sizeof("' used after arena '") - 1 });
    print_str_raw(arena);
    print_str_raw((oc_string){ .ptr = (const uint8_t*)("' was reset\n"), .len = sizeof("' was reset\n") - 1 });
}

void print_err_uaf_deinit(oc_string name, oc_string arena) {
    print_str_raw((oc_string){ .ptr = (const uint8_t*)("error: use-after-free: variable '"), .len = sizeof("error: use-after-free: variable '") - 1 });
    print_str_raw(name);
    print_str_raw((oc_string){ .ptr = (const uint8_t*)("' used after arena '"), .len = sizeof("' used after arena '") - 1 });
    print_str_raw(arena);
    print_str_raw((oc_string){ .ptr = (const uint8_t*)("' was deinitialized\n"), .len = sizeof("' was deinitialized\n") - 1 });
}

void print_err_borrow_escape(oc_string src, oc_string tgt) {
    print_str_raw((oc_string){ .ptr = (const uint8_t*)("error: borrowed argument '"), .len = sizeof("error: borrowed argument '") - 1 });
    print_str_raw(src);
    print_str_raw((oc_string){ .ptr = (const uint8_t*)("' does not live long enough (escapes into '"), .len = sizeof("' does not live long enough (escapes into '") - 1 });
    print_str_raw(tgt);
    print_str_raw((oc_string){ .ptr = (const uint8_t*)("')\n"), .len = sizeof("')\n") - 1 });
}

void print_err_cross_arena(oc_string arena, oc_string tgt) {
    print_str_raw((oc_string){ .ptr = (const uint8_t*)("error: cross-arena escape: pointer borrowed from arena '"), .len = sizeof("error: cross-arena escape: pointer borrowed from arena '") - 1 });
    print_str_raw(arena);
    print_str_raw((oc_string){ .ptr = (const uint8_t*)("' escapes into longer-lived target '"), .len = sizeof("' escapes into longer-lived target '") - 1 });
    print_str_raw(tgt);
    print_str_raw((oc_string){ .ptr = (const uint8_t*)("'\n"), .len = sizeof("'\n") - 1 });
}

void print_err_ret_local(oc_string src) {
    print_str_raw((oc_string){ .ptr = (const uint8_t*)("error: cannot return reference to local stack variable '"), .len = sizeof("error: cannot return reference to local stack variable '") - 1 });
    print_str_raw(src);
    print_str_raw((oc_string){ .ptr = (const uint8_t*)("'\n"), .len = sizeof("'\n") - 1 });
}

void print_err_ret_arena(oc_string arena) {
    print_str_raw((oc_string){ .ptr = (const uint8_t*)("error: cannot return reference to locally allocated arena '"), .len = sizeof("error: cannot return reference to locally allocated arena '") - 1 });
    print_str_raw(arena);
    print_str_raw((oc_string){ .ptr = (const uint8_t*)("'\n"), .len = sizeof("'\n") - 1 });
}

bool starts_with(oc_string s, oc_string prefix) {
    if ((((s).len) < ((prefix).len))) {
    return false;
    }
    size_t i = 0;
    while ((i < ((prefix).len))) {
    if ((__oc_index_str(s, i) != __oc_index_str(prefix, i))) {
    return false;
    }
    i = __OC_ADD(i, 1);
    }
    return true;
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
    printf("%s\n", "security error: validator deinit table capacity exceeded");
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

oc_string get_arena_arg_name(AstNode* arg) {
    oc_string u = get_underlying_address_of(arg);
    if ((((u).len) > 0)) {
    return u;
    }
    return get_ident_name(arg);
}

bool fx_scan(FnEffects* fx, size_t slot, AstNode* params, AstNode* n) {
    bool changed = false;
    AstNode* cur = n;
    while ((cur != ((AstNode*)(0)))) {
    if ((((cur)->tag) == NODE_CALL)) {
    AstNode* arg = ((cur)->left);
    size_t ai = 0;
    while ((arg != ((AstNode*)(0)))) {
    uint8_t eff = fx_param_effect(fx, ((((cur)->tok)).slice), ai);
    if ((eff > 0)) {
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
    if (fx_scan(fx, slot, params, ((cur)->left))) {
    changed = true;
    }
    if (fx_scan(fx, slot, params, ((cur)->right))) {
    changed = true;
    }
    cur = ((cur)->next);
    }
    return changed;
}

oc_string get_arena_source(AstNode* expr) {
    if ((expr == ((AstNode*)(0)))) {
    return (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 };
    }
    if ((((expr)->tag) == NODE_CALL)) {
    if (str_eq(((((expr)->tok)).slice), (oc_string){ .ptr = (const uint8_t*)("arena_alloc"), .len = sizeof("arena_alloc") - 1 })) {
    if ((((expr)->left) != ((AstNode*)(0)))) {
    oc_string u = get_underlying_address_of(((expr)->left));
    if ((((u).len) > 0)) {
    return u;
    }
    return get_ident_name(((expr)->left));
    }
    }
    return (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 };
    }
    if ((((expr)->tag) == NODE_CAST)) {
    return get_arena_source(((expr)->left));
    }
    return (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 };
}

oc_string get_expr_arena_borrow(Validator* v, AstNode* expr) {
    if ((expr == ((AstNode*)(0)))) {
    return (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 };
    }
    oc_string direct = get_arena_source(expr);
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

oc_string get_local_array_slice_source(Validator* v, AstNode* expr) {
    if ((expr == ((AstNode*)(0)))) {
    return (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 };
    }
    if ((((expr)->tag) != NODE_SLICE)) {
    return (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 };
    }
    if ((((expr)->left) == ((AstNode*)(0)))) {
    return (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 };
    }
    if ((((((expr)->left))->tag) != NODE_IDENT)) {
    return (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 };
    }
    int32_t idx = validator_find_var(v, ((((((expr)->left))->tok)).slice));
    if ((idx < 0)) {
    return (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 };
    }
    if ((!((__OC_AT((((v)->vars)), (({ __auto_type _v = (idx); if (_v < 0) { fprintf(stderr, "panic: invalid unsigned cast\n"); exit(1); } (size_t)_v; })))).is_stack_local))) {
    return (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 };
    }
    if ((!starts_with(((__OC_AT((((v)->vars)), (({ __auto_type _v = (idx); if (_v < 0) { fprintf(stderr, "panic: invalid unsigned cast\n"); exit(1); } (size_t)_v; })))).type_name), (oc_string){ .ptr = (const uint8_t*)("["), .len = sizeof("[") - 1 }))) {
    return (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 };
    }
    return ((((((expr)->left))->tok)).slice);
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
    while ((a != ((AstNode*)(0)))) {
    oc_string s = contains_stack_ref(v, a);
    if ((((s).len) > 0)) {
    return s;
    }
    a = ((a)->next);
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
    printf("%s\n", "security error: variable used in its own initializer (CWE-457)");
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
    print_str_raw((oc_string){ .ptr = (const uint8_t*)("': it has no length (slice a string or a [N]T array)\n"), .len = sizeof("': it has no length (slice a string or a [N]T array)\n") - 1 });
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
    return check_expr_use(v, ((expr)->right));
    }
    if ((((expr)->tag) == NODE_BINARY)) {
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
    }
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
    printf("%s\n", "security error: Arena cannot be copied by value (linear resource)");
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
    printf("%s\n", "security error: validator symbol table capacity exceeded");
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
    printf("%s\n", "security error: cannot assign to immutable variable");
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
    printf("%s\n", "security error: Arena cannot be copied by value (linear resource)");
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
    oc_string rhs_arena = get_expr_arena_borrow(v, ((stmt)->right));
    if ((((rhs_arena).len) > 0)) {
    if ((((tgt_opt).len) > 0)) {
    int32_t t_idx = validator_find_var(v, tgt_opt);
    if ((t_idx >= 0)) {
    int32_t a_idx = validator_find_var(v, rhs_arena);
    uint32_t arena_depth = ((v)->scope_depth);
    if ((a_idx >= 0)) {
    arena_depth = ((__OC_AT((((v)->vars)), (({ __auto_type _v = (a_idx); if (_v < 0) { fprintf(stderr, "panic: invalid unsigned cast\n"); exit(1); } (size_t)_v; })))).scope_depth);
    }
    bool cross_escape = false;
    if ((((__OC_AT((((v)->vars)), (({ __auto_type _v = (t_idx); if (_v < 0) { fprintf(stderr, "panic: invalid unsigned cast\n"); exit(1); } (size_t)_v; })))).scope_depth) < arena_depth)) {
    cross_escape = true;
    }
    else {
    if ((!((__OC_AT((((v)->vars)), (({ __auto_type _v = (t_idx); if (_v < 0) { fprintf(stderr, "panic: invalid unsigned cast\n"); exit(1); } (size_t)_v; })))).is_stack_local))) {
    cross_escape = true;
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
    printf("%s\n", "error: borrowed pointer escapes enclosing scope");
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
    printf("%s\n", "error: borrowed pointer escapes enclosing scope");
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
    if ((!validate_stmt(v, ((stmt)->right)))) {
    return false;
    }
    if ((((stmt)->right) != ((AstNode*)(0)))) {
    if ((((((stmt)->right))->next) != ((AstNode*)(0)))) {
    if ((!validate_stmt(v, ((((stmt)->right))->next)))) {
    return false;
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

bool validate_function(AstNode* f, FnEffects* fx) {
    if ((((f)->right) == ((AstNode*)(0)))) {
    return true;
    }
    Validator v = (Validator){ .fx = fx, .vars = { (VarInfo){ .name = (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 }, .type_name = (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 }, .scope_depth = 0, .is_stack_local = false, .points_to_stack_local = false, .borrowed_from_arena = (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 }, .borrowed_from_var = (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 }, .is_invalidated = false, .is_mut = false } }, .vars_count = 0, .deinit_arenas = { (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 } }, .deinit_count = 0, .scope_depth = 1, .has_error = false, .initializing_var = (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 } };
    AstNode* p = ((f)->left);
    while ((p != ((AstNode*)(0)))) {
    if ((((v).vars_count) >= 512)) {
    printf("%s\n", "security error: validator symbol table capacity exceeded");
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

bool validate_program(AstNode* root) {
    FnEffects fx = (FnEffects){ .names = { (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 } }, .effects = { 0 }, .count = 0 };
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
    if (fx_scan((&(fx)), slot, ((decl)->left), ((decl)->right))) {
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
    if ((!validate_function(decl, (&(fx))))) {
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
    printf("%s\n", "parse error: unexpected top-level token");
    __oc_print(((((p)->current)).slice));
    ((p)->has_error) = true;
    parser_advance(p);
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

CodeBuffer buf_init(size_t capacity) {
    void* raw = sys_mmap(0, capacity, 3, 34, (-1), 0);
    return (CodeBuffer){ .buffer = ((uint8_t*)(raw)), .len = 0, .capacity = capacity, .ptr_vars = { (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 } }, .ptr_vars_count = 0, .str_vars = { (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 } }, .str_vars_count = 0, .ptr_fields = { (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 } }, .ptr_fields_count = 0, .str_fields = { (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 } }, .str_fields_count = 0, .ptr_fns = { (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 } }, .ptr_fns_count = 0, .str_fns = { (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 } }, .str_fns_count = 0, .var_names = { (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 } }, .var_types = { (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 } }, .var_types_len = 0 };
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
    buf_append(out, to_c_type(base));
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("*"), .len = sizeof("*") - 1 });
    return;
    }
    buf_append(out, to_c_type(type_name));
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
    buf_append(out, ((((expr)->tok)).slice));
    return;
    }
    if ((((expr)->tag) == NODE_STRING)) {
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("(oc_string){ .ptr = (const uint8_t*)("), .len = sizeof("(oc_string){ .ptr = (const uint8_t*)(") - 1 });
    buf_append(out, ((((expr)->tok)).slice));
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("), .len = sizeof("), .len = sizeof("), .len = sizeof(") - 1 });
    buf_append(out, ((((expr)->tok)).slice));
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)(") - 1 }"), .len = sizeof(") - 1 }") - 1 });
    return;
    }
    if ((((expr)->tag) == NODE_BOOL)) {
    buf_append(out, ((((expr)->tok)).slice));
    return;
    }
    if ((((expr)->tag) == NODE_IDENT)) {
    buf_append(out, ((((expr)->tok)).slice));
    return;
    }
    if ((((expr)->tag) == NODE_UNARY)) {
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
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)(") { fprintf(stderr, \"panic: invalid unsigned cast\\n\"); exit(1); } ("), .len = sizeof(") { fprintf(stderr, \"panic: invalid unsigned cast\\n\"); exit(1); } (") - 1 });
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
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)(") { fprintf(stderr, \"panic: invalid signed cast\\n\"); exit(1); } ("), .len = sizeof(") { fprintf(stderr, \"panic: invalid signed cast\\n\"); exit(1); } (") - 1 });
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
    buf_append(out, ((((expr)->tok)).slice));
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)(")"), .len = sizeof(")") - 1 });
    return;
    }
    if ((((expr)->tag) == NODE_STRUCT_INIT)) {
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("("), .len = sizeof("(") - 1 });
    buf_append(out, to_c_type(((((expr)->tok)).slice)));
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("){ "), .len = sizeof("){ ") - 1 });
    AstNode* f = ((expr)->left);
    while ((f != ((AstNode*)(0)))) {
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("."), .len = sizeof(".") - 1 });
    buf_append(out, ((((f)->tok)).slice));
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
    buf_append(out, ((((expr)->tok)).slice));
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
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("    printf(\"%s\\n\", "), .len = sizeof("    printf(\"%s\\n\", ") - 1 });
    buf_append(out, ((((arg)->tok)).slice));
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)(");\n"), .len = sizeof(");\n") - 1 });
    return;
    }
    if ((((arg)->tag) == NODE_IDENT)) {
    if (is_u8_array_type(cg_var_type(out, ((((arg)->tok)).slice)))) {
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("    printf(\"%.*s\\n\", (int)sizeof("), .len = sizeof("    printf(\"%.*s\\n\", (int)sizeof(") - 1 });
    buf_append(out, ((((arg)->tok)).slice));
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("), (const char*)"), .len = sizeof("), (const char*)") - 1 });
    buf_append(out, ((((arg)->tok)).slice));
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)(");\n"), .len = sizeof(");\n") - 1 });
    return;
    }
    }
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
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)(");\n"), .len = sizeof(");\n") - 1 });
}

void codegen_stmt(AstNode* stmt, CodeBuffer* out) {
    if ((stmt == ((AstNode*)(0)))) {
    return;
    }
    if ((((stmt)->left) == ((AstNode*)(0)))) {
    if (str_eq(((((stmt)->tok)).slice), (oc_string){ .ptr = (const uint8_t*)("break"), .len = sizeof("break") - 1 })) {
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("    break;\n"), .len = sizeof("    break;\n") - 1 });
    return;
    }
    if (str_eq(((((stmt)->tok)).slice), (oc_string){ .ptr = (const uint8_t*)("continue"), .len = sizeof("continue") - 1 })) {
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("    continue;\n"), .len = sizeof("    continue;\n") - 1 });
    return;
    }
    }
    if ((((stmt)->tag) == NODE_PRINT)) {
    codegen_print(stmt, out);
    return;
    }
    if ((((stmt)->tag) == NODE_LET)) {
    if ((((out)->var_types_len) < 512)) {
    __OC_AT((((out)->var_names)), (((out)->var_types_len))) = ((((stmt)->tok)).slice);
    __OC_AT((((out)->var_types)), (((out)->var_types_len))) = ((stmt)->type_name);
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
    buf_append(out, ((((stmt)->tok)).slice));
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
    if (str_eq(((((first_el)->tok)).slice), (oc_string){ .ptr = (const uint8_t*)("\"\""), .len = sizeof("\"\"") - 1 })) {
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
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("] = {0};\n"), .len = sizeof("] = {0};\n") - 1 });
    }
    else {
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("] = "), .len = sizeof("] = ") - 1 });
    codegen_expr(((stmt)->left), out);
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)(";\n"), .len = sizeof(";\n") - 1 });
    }
    return;
    }
    emit_c_type(((stmt)->type_name), out);
    }
    else {
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("__auto_type"), .len = sizeof("__auto_type") - 1 });
    }
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)(" "), .len = sizeof(" ") - 1 });
    buf_append(out, ((((stmt)->tok)).slice));
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)(" = "), .len = sizeof(" = ") - 1 });
    codegen_expr(((stmt)->left), out);
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)(";\n"), .len = sizeof(";\n") - 1 });
    return;
    }
    if ((((stmt)->tag) == NODE_RETURN)) {
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("    return"), .len = sizeof("    return") - 1 });
    if ((((stmt)->left) != ((AstNode*)(0)))) {
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)(" "), .len = sizeof(" ") - 1 });
    codegen_expr(((stmt)->left), out);
    }
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)(";\n"), .len = sizeof(";\n") - 1 });
    return;
    }
    if ((((stmt)->tag) == NODE_ASSIGN)) {
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("    "), .len = sizeof("    ") - 1 });
    codegen_expr(((stmt)->left), out);
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)(" = "), .len = sizeof(" = ") - 1 });
    codegen_expr(((stmt)->right), out);
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)(";\n"), .len = sizeof(";\n") - 1 });
    return;
    }
    size_t scope_mark = ((out)->var_types_len);
    if ((((stmt)->tag) == NODE_IF)) {
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("    if ("), .len = sizeof("    if (") - 1 });
    codegen_expr(((stmt)->left), out);
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)(") {\n"), .len = sizeof(") {\n") - 1 });
    if ((((stmt)->right) != ((AstNode*)(0)))) {
    AstNode* s = ((((stmt)->right))->left);
    while ((s != ((AstNode*)(0)))) {
    codegen_stmt(s, out);
    s = ((s)->next);
    }
    }
    ((out)->var_types_len) = scope_mark;
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("    }\n"), .len = sizeof("    }\n") - 1 });
    if ((((stmt)->right) != ((AstNode*)(0)))) {
    if ((((((stmt)->right))->next) != ((AstNode*)(0)))) {
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("    else {\n"), .len = sizeof("    else {\n") - 1 });
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
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("    }\n"), .len = sizeof("    }\n") - 1 });
    }
    }
    return;
    }
    if ((((stmt)->tag) == NODE_WHILE)) {
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("    while ("), .len = sizeof("    while (") - 1 });
    codegen_expr(((stmt)->left), out);
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)(") {\n"), .len = sizeof(") {\n") - 1 });
    if ((((stmt)->right) != ((AstNode*)(0)))) {
    AstNode* s = ((((stmt)->right))->left);
    while ((s != ((AstNode*)(0)))) {
    codegen_stmt(s, out);
    s = ((s)->next);
    }
    }
    ((out)->var_types_len) = scope_mark;
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("    }\n"), .len = sizeof("    }\n") - 1 });
    return;
    }
    if ((((stmt)->tag) == NODE_BLOCK)) {
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("    {\n"), .len = sizeof("    {\n") - 1 });
    AstNode* s = ((stmt)->left);
    while ((s != ((AstNode*)(0)))) {
    codegen_stmt(s, out);
    s = ((s)->next);
    }
    ((out)->var_types_len) = scope_mark;
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("    }\n"), .len = sizeof("    }\n") - 1 });
    return;
    }
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("    "), .len = sizeof("    ") - 1 });
    codegen_expr(stmt, out);
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)(";\n"), .len = sizeof(";\n") - 1 });
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
    return false;
}

void codegen_program(AstNode* root, CodeBuffer* out) {
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("#pragma GCC diagnostic ignored \"-Wmain\"\n"), .len = sizeof("#pragma GCC diagnostic ignored \"-Wmain\"\n") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("#pragma GCC diagnostic ignored \"-Wdiscarded-qualifiers\"\n"), .len = sizeof("#pragma GCC diagnostic ignored \"-Wdiscarded-qualifiers\"\n") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("#pragma GCC diagnostic ignored \"-Wpointer-sign\"\n"), .len = sizeof("#pragma GCC diagnostic ignored \"-Wpointer-sign\"\n") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("#pragma GCC diagnostic ignored \"-Wincompatible-pointer-types\"\n"), .len = sizeof("#pragma GCC diagnostic ignored \"-Wincompatible-pointer-types\"\n") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("#include <stdio.h>\n"), .len = sizeof("#include <stdio.h>\n") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("#include <stdint.h>\n"), .len = sizeof("#include <stdint.h>\n") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("#include <stdbool.h>\n"), .len = sizeof("#include <stdbool.h>\n") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("#include <stdlib.h>\n"), .len = sizeof("#include <stdlib.h>\n") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("#include <string.h>\n"), .len = sizeof("#include <string.h>\n") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("#include <fcntl.h>\n"), .len = sizeof("#include <fcntl.h>\n") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("#include <sys/stat.h>\n"), .len = sizeof("#include <sys/stat.h>\n") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("#ifdef _WIN32\n"), .len = sizeof("#ifdef _WIN32\n") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("#include <io.h>\n"), .len = sizeof("#include <io.h>\n") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("__declspec(dllimport) void* __stdcall VirtualAlloc(void*, size_t, unsigned long, unsigned long);\n"), .len = sizeof("__declspec(dllimport) void* __stdcall VirtualAlloc(void*, size_t, unsigned long, unsigned long);\n") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("__declspec(dllimport) int __stdcall VirtualFree(void*, size_t, unsigned long);\n"), .len = sizeof("__declspec(dllimport) int __stdcall VirtualFree(void*, size_t, unsigned long);\n") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("#define O_CLOEXEC_HOST _O_NOINHERIT\n"), .len = sizeof("#define O_CLOEXEC_HOST _O_NOINHERIT\n") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("#else\n"), .len = sizeof("#else\n") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("#include <unistd.h>\n"), .len = sizeof("#include <unistd.h>\n") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("#include <sys/mman.h>\n"), .len = sizeof("#include <sys/mman.h>\n") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("#include <sys/socket.h>\n"), .len = sizeof("#include <sys/socket.h>\n") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("#ifndef MAP_ANONYMOUS\n#define MAP_ANONYMOUS MAP_ANON\n#endif\n"), .len = sizeof("#ifndef MAP_ANONYMOUS\n#define MAP_ANONYMOUS MAP_ANON\n#endif\n") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("#define O_CLOEXEC_HOST O_CLOEXEC\n"), .len = sizeof("#define O_CLOEXEC_HOST O_CLOEXEC\n") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("#endif\n"), .len = sizeof("#endif\n") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("#ifndef O_NOFOLLOW\n#define O_NOFOLLOW 0\n#endif\n"), .len = sizeof("#ifndef O_NOFOLLOW\n#define O_NOFOLLOW 0\n#endif\n") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("#ifndef O_BINARY\n#define O_BINARY 0\n#endif\n"), .len = sizeof("#ifndef O_BINARY\n#define O_BINARY 0\n#endif\n") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("#ifdef htons\n#undef htons\n#endif\n"), .len = sizeof("#ifdef htons\n#undef htons\n#endif\n") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("#ifdef ntohs\n#undef ntohs\n#endif\n"), .len = sizeof("#ifdef ntohs\n#undef ntohs\n#endif\n") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("#ifdef htonl\n#undef htonl\n#endif\n"), .len = sizeof("#ifdef htonl\n#undef htonl\n#endif\n") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("#ifdef ntohl\n#undef ntohl\n#endif\n\n"), .len = sizeof("#ifdef ntohl\n#undef ntohl\n#endif\n\n") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("static inline int __oc_open_flags(int32_t f) { int r = O_BINARY; int acc = f & 3; if (acc == 1) r |= O_WRONLY; else if (acc == 2) r |= O_RDWR; else r |= O_RDONLY; if (f & 64) r |= O_CREAT; if (f & 512) r |= O_TRUNC; if (f & 1024) r |= O_APPEND; if (f & 131072) r |= O_NOFOLLOW; if (f & 524288) r |= O_CLOEXEC_HOST; return r; }\n"), .len = sizeof("static inline int __oc_open_flags(int32_t f) { int r = O_BINARY; int acc = f & 3; if (acc == 1) r |= O_WRONLY; else if (acc == 2) r |= O_RDWR; else r |= O_RDONLY; if (f & 64) r |= O_CREAT; if (f & 512) r |= O_TRUNC; if (f & 1024) r |= O_APPEND; if (f & 131072) r |= O_NOFOLLOW; if (f & 524288) r |= O_CLOEXEC_HOST; return r; }\n") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("#ifdef _WIN32\n"), .len = sizeof("#ifdef _WIN32\n") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("static inline int64_t sys_read(int32_t fd, char* buf, size_t count) { return (int64_t)_read(fd, buf, (unsigned)count); }\n"), .len = sizeof("static inline int64_t sys_read(int32_t fd, char* buf, size_t count) { return (int64_t)_read(fd, buf, (unsigned)count); }\n") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("static inline int64_t sys_write(int32_t fd, const char* buf, size_t count) { return (int64_t)_write(fd, buf, (unsigned)count); }\n"), .len = sizeof("static inline int64_t sys_write(int32_t fd, const char* buf, size_t count) { return (int64_t)_write(fd, buf, (unsigned)count); }\n") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("static inline int32_t sys_open(const char* path, int32_t flags, uint32_t mode) { (void)mode; return (int32_t)_open(path, __oc_open_flags(flags), _S_IREAD | _S_IWRITE); }\n"), .len = sizeof("static inline int32_t sys_open(const char* path, int32_t flags, uint32_t mode) { (void)mode; return (int32_t)_open(path, __oc_open_flags(flags), _S_IREAD | _S_IWRITE); }\n") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("static inline int32_t sys_close(int32_t fd) { return (int32_t)_close(fd); }\n"), .len = sizeof("static inline int32_t sys_close(int32_t fd) { return (int32_t)_close(fd); }\n") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("static inline void* sys_mmap(size_t addr, size_t len, int32_t prot, int32_t flags, int32_t fd, size_t off) { (void)addr; (void)prot; (void)flags; (void)fd; (void)off; void* p = VirtualAlloc(0, len, 0x3000, 0x04); return p ? p : (void*)-1; }\n"), .len = sizeof("static inline void* sys_mmap(size_t addr, size_t len, int32_t prot, int32_t flags, int32_t fd, size_t off) { (void)addr; (void)prot; (void)flags; (void)fd; (void)off; void* p = VirtualAlloc(0, len, 0x3000, 0x04); return p ? p : (void*)-1; }\n") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("static inline int32_t sys_munmap(void* addr, size_t len) { (void)len; return VirtualFree(addr, 0, 0x8000) ? 0 : -1; }\n"), .len = sizeof("static inline int32_t sys_munmap(void* addr, size_t len) { (void)len; return VirtualFree(addr, 0, 0x8000) ? 0 : -1; }\n") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("static inline int32_t sys_chmod(const char* path, uint32_t mode) { return (int32_t)_chmod(path, _S_IREAD | ((mode & 128) ? _S_IWRITE : 0)); }\n"), .len = sizeof("static inline int32_t sys_chmod(const char* path, uint32_t mode) { return (int32_t)_chmod(path, _S_IREAD | ((mode & 128) ? _S_IWRITE : 0)); }\n") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("static inline int32_t sys_unlink(const char* path) { return (int32_t)_unlink(path); }\n"), .len = sizeof("static inline int32_t sys_unlink(const char* path) { return (int32_t)_unlink(path); }\n") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("static inline int32_t sys_socket(int32_t domain, int32_t type, int32_t protocol) { (void)domain; (void)type; (void)protocol; return -1; }\n"), .len = sizeof("static inline int32_t sys_socket(int32_t domain, int32_t type, int32_t protocol) { (void)domain; (void)type; (void)protocol; return -1; }\n") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("static inline int32_t sys_connect(int32_t fd, const void* addr, uint32_t len) { (void)fd; (void)addr; (void)len; return -1; }\n"), .len = sizeof("static inline int32_t sys_connect(int32_t fd, const void* addr, uint32_t len) { (void)fd; (void)addr; (void)len; return -1; }\n") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("#else\n"), .len = sizeof("#else\n") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("static inline int64_t sys_read(int32_t fd, char* buf, size_t count) { return (int64_t)read(fd, buf, count); }\n"), .len = sizeof("static inline int64_t sys_read(int32_t fd, char* buf, size_t count) { return (int64_t)read(fd, buf, count); }\n") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("static inline int64_t sys_write(int32_t fd, const char* buf, size_t count) { return (int64_t)write(fd, buf, count); }\n"), .len = sizeof("static inline int64_t sys_write(int32_t fd, const char* buf, size_t count) { return (int64_t)write(fd, buf, count); }\n") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("static inline int32_t sys_open(const char* path, int32_t flags, uint32_t mode) { return (int32_t)open(path, __oc_open_flags(flags), (mode_t)mode); }\n"), .len = sizeof("static inline int32_t sys_open(const char* path, int32_t flags, uint32_t mode) { return (int32_t)open(path, __oc_open_flags(flags), (mode_t)mode); }\n") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("static inline int32_t sys_close(int32_t fd) { return (int32_t)close(fd); }\n"), .len = sizeof("static inline int32_t sys_close(int32_t fd) { return (int32_t)close(fd); }\n") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("static inline void* sys_mmap(size_t addr, size_t len, int32_t prot, int32_t flags, int32_t fd, size_t off) { int p = 0; if (prot & 1) p |= PROT_READ; if (prot & 2) p |= PROT_WRITE; if (prot & 4) p |= PROT_EXEC; int m = 0; if (flags & 1) m |= MAP_SHARED; if (flags & 2) m |= MAP_PRIVATE; if (flags & 32) m |= MAP_ANONYMOUS; return mmap((void*)addr, len, p, m, fd, (off_t)off); }\n"), .len = sizeof("static inline void* sys_mmap(size_t addr, size_t len, int32_t prot, int32_t flags, int32_t fd, size_t off) { int p = 0; if (prot & 1) p |= PROT_READ; if (prot & 2) p |= PROT_WRITE; if (prot & 4) p |= PROT_EXEC; int m = 0; if (flags & 1) m |= MAP_SHARED; if (flags & 2) m |= MAP_PRIVATE; if (flags & 32) m |= MAP_ANONYMOUS; return mmap((void*)addr, len, p, m, fd, (off_t)off); }\n") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("static inline int32_t sys_munmap(void* addr, size_t len) { return (int32_t)munmap(addr, len); }\n"), .len = sizeof("static inline int32_t sys_munmap(void* addr, size_t len) { return (int32_t)munmap(addr, len); }\n") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("static inline int32_t sys_chmod(const char* path, uint32_t mode) { return (int32_t)chmod(path, (mode_t)mode); }\n"), .len = sizeof("static inline int32_t sys_chmod(const char* path, uint32_t mode) { return (int32_t)chmod(path, (mode_t)mode); }\n") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("static inline int32_t sys_unlink(const char* path) { return (int32_t)unlink(path); }\n"), .len = sizeof("static inline int32_t sys_unlink(const char* path) { return (int32_t)unlink(path); }\n") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("static inline int32_t sys_socket(int32_t domain, int32_t type, int32_t protocol) { return (int32_t)socket(domain, type, protocol); }\n"), .len = sizeof("static inline int32_t sys_socket(int32_t domain, int32_t type, int32_t protocol) { return (int32_t)socket(domain, type, protocol); }\n") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("#if defined(__APPLE__) || defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__) || defined(__DragonFly__)\n"), .len = sizeof("#if defined(__APPLE__) || defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__) || defined(__DragonFly__)\n") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("static inline int32_t sys_connect(int32_t fd, const void* addr, uint32_t len) { uint8_t sa[128]; if (len > sizeof sa) return -1; memcpy(sa, addr, len); sa[1] = sa[0]; sa[0] = (uint8_t)len; return (int32_t)connect(fd, (const struct sockaddr*)sa, (socklen_t)len); }\n"), .len = sizeof("static inline int32_t sys_connect(int32_t fd, const void* addr, uint32_t len) { uint8_t sa[128]; if (len > sizeof sa) return -1; memcpy(sa, addr, len); sa[1] = sa[0]; sa[0] = (uint8_t)len; return (int32_t)connect(fd, (const struct sockaddr*)sa, (socklen_t)len); }\n") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("#else\n"), .len = sizeof("#else\n") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("static inline int32_t sys_connect(int32_t fd, const void* addr, uint32_t len) { return (int32_t)connect(fd, (const struct sockaddr*)addr, (socklen_t)len); }\n"), .len = sizeof("static inline int32_t sys_connect(int32_t fd, const void* addr, uint32_t len) { return (int32_t)connect(fd, (const struct sockaddr*)addr, (socklen_t)len); }\n") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("#endif\n"), .len = sizeof("#endif\n") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("#endif\n"), .len = sizeof("#endif\n") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("#ifdef _WIN32\n"), .len = sizeof("#ifdef _WIN32\n") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("#include <direct.h>\n"), .len = sizeof("#include <direct.h>\n") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("static inline int32_t sys_mkdtemp(char* tmpl) { return (_mktemp(tmpl) != 0 && _mkdir(tmpl) == 0) ? 0 : -1; }\n"), .len = sizeof("static inline int32_t sys_mkdtemp(char* tmpl) { return (_mktemp(tmpl) != 0 && _mkdir(tmpl) == 0) ? 0 : -1; }\n") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("static inline int32_t sys_rmdir(const char* path) { return (int32_t)_rmdir(path); }\n"), .len = sizeof("static inline int32_t sys_rmdir(const char* path) { return (int32_t)_rmdir(path); }\n") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("#else\n"), .len = sizeof("#else\n") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("static inline int32_t sys_mkdtemp(char* tmpl) { return mkdtemp(tmpl) ? 0 : -1; }\n"), .len = sizeof("static inline int32_t sys_mkdtemp(char* tmpl) { return mkdtemp(tmpl) ? 0 : -1; }\n") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("static inline int32_t sys_rmdir(const char* path) { return (int32_t)rmdir(path); }\n"), .len = sizeof("static inline int32_t sys_rmdir(const char* path) { return (int32_t)rmdir(path); }\n") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("#endif\n"), .len = sizeof("#endif\n") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("#if defined(_WIN32)\n"), .len = sizeof("#if defined(_WIN32)\n") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("__declspec(dllimport) unsigned long __stdcall GetModuleFileNameA(void*, char*, unsigned long);\n"), .len = sizeof("__declspec(dllimport) unsigned long __stdcall GetModuleFileNameA(void*, char*, unsigned long);\n") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("static inline int64_t sys_exe_path(char* buf, size_t cap) { unsigned long n = GetModuleFileNameA(0, buf, (unsigned long)cap); return (n == 0 || n >= cap) ? -1 : (int64_t)n; }\n"), .len = sizeof("static inline int64_t sys_exe_path(char* buf, size_t cap) { unsigned long n = GetModuleFileNameA(0, buf, (unsigned long)cap); return (n == 0 || n >= cap) ? -1 : (int64_t)n; }\n") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("#elif defined(__APPLE__)\n"), .len = sizeof("#elif defined(__APPLE__)\n") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("#include <limits.h>\n"), .len = sizeof("#include <limits.h>\n") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("#include <mach-o/dyld.h>\n"), .len = sizeof("#include <mach-o/dyld.h>\n") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("static inline int64_t sys_exe_path(char* buf, size_t cap) { char raw[PATH_MAX]; char real[PATH_MAX]; uint32_t sz = sizeof raw; if (_NSGetExecutablePath(raw, &sz) != 0 || !realpath(raw, real)) return -1; size_t n = strlen(real); if (n >= cap) return -1; memcpy(buf, real, n + 1); return (int64_t)n; }\n"), .len = sizeof("static inline int64_t sys_exe_path(char* buf, size_t cap) { char raw[PATH_MAX]; char real[PATH_MAX]; uint32_t sz = sizeof raw; if (_NSGetExecutablePath(raw, &sz) != 0 || !realpath(raw, real)) return -1; size_t n = strlen(real); if (n >= cap) return -1; memcpy(buf, real, n + 1); return (int64_t)n; }\n") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("#else\n"), .len = sizeof("#else\n") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("static inline int64_t sys_exe_path(char* buf, size_t cap) { ssize_t n = cap ? readlink(\"/proc/self/exe\", buf, cap - 1) : -1; if (n <= 0) return -1; buf[n] = 0; return (int64_t)n; }\n"), .len = sizeof("static inline int64_t sys_exe_path(char* buf, size_t cap) { ssize_t n = cap ? readlink(\"/proc/self/exe\", buf, cap - 1) : -1; if (n <= 0) return -1; buf[n] = 0; return (int64_t)n; }\n") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("#endif\n"), .len = sizeof("#endif\n") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("static inline void sys_exit(int32_t code) { _exit(code); }\n"), .len = sizeof("static inline void sys_exit(int32_t code) { _exit(code); }\n") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("#if defined(_WIN32)\nstatic inline int32_t sys_host_target(void) { return 2; }\n"), .len = sizeof("#if defined(_WIN32)\nstatic inline int32_t sys_host_target(void) { return 2; }\n") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("#elif defined(__APPLE__) && (defined(__aarch64__) || defined(__arm64__))\nstatic inline int32_t sys_host_target(void) { return 4; }\n"), .len = sizeof("#elif defined(__APPLE__) && (defined(__aarch64__) || defined(__arm64__))\nstatic inline int32_t sys_host_target(void) { return 4; }\n") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("#elif defined(__APPLE__) && defined(__x86_64__)\nstatic inline int32_t sys_host_target(void) { return 3; }\n"), .len = sizeof("#elif defined(__APPLE__) && defined(__x86_64__)\nstatic inline int32_t sys_host_target(void) { return 3; }\n") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("#elif defined(__linux__) && defined(__x86_64__)\nstatic inline int32_t sys_host_target(void) { return 1; }\n"), .len = sizeof("#elif defined(__linux__) && defined(__x86_64__)\nstatic inline int32_t sys_host_target(void) { return 1; }\n") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("#else\nstatic inline int32_t sys_host_target(void) { return 0; }\n#endif\n\n"), .len = sizeof("#else\nstatic inline int32_t sys_host_target(void) { return 0; }\n#endif\n\n") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("typedef struct {\n"), .len = sizeof("typedef struct {\n") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("    const uint8_t* ptr;\n"), .len = sizeof("    const uint8_t* ptr;\n") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("    size_t len;\n"), .len = sizeof("    size_t len;\n") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("} oc_string;\n\n"), .len = sizeof("} oc_string;\n\n") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("static inline void __oc_print_str(const char* s) { printf(\"%s\\n\", s); }\n"), .len = sizeof("static inline void __oc_print_str(const char* s) { printf(\"%s\\n\", s); }\n") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("static inline void __oc_print_u8_ptr(uint8_t* s) { printf(\"%s\\n\", (const char*)s); }\n"), .len = sizeof("static inline void __oc_print_u8_ptr(uint8_t* s) { printf(\"%s\\n\", (const char*)s); }\n") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("static inline void __oc_print_const_u8_ptr(const uint8_t* s) { printf(\"%s\\n\", (const char*)s); }\n"), .len = sizeof("static inline void __oc_print_const_u8_ptr(const uint8_t* s) { printf(\"%s\\n\", (const char*)s); }\n") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("static inline void __oc_print_oc_string(oc_string s) { printf(\"%.*s\\n\", (int)s.len, (const char*)s.ptr); }\n"), .len = sizeof("static inline void __oc_print_oc_string(oc_string s) { printf(\"%.*s\\n\", (int)s.len, (const char*)s.ptr); }\n") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("static inline void __oc_print_bool(bool b) { printf(\"%s\\n\", b ? \"true\" : \"false\"); }\n"), .len = sizeof("static inline void __oc_print_bool(bool b) { printf(\"%s\\n\", b ? \"true\" : \"false\"); }\n") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("static inline void __oc_print_u64(uint64_t u) { printf(\"%llu\\n\", (unsigned long long)u); }\n"), .len = sizeof("static inline void __oc_print_u64(uint64_t u) { printf(\"%llu\\n\", (unsigned long long)u); }\n") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("static inline void __oc_print_i64(int64_t i) { printf(\"%lld\\n\", (long long)i); }\n\n"), .len = sizeof("static inline void __oc_print_i64(int64_t i) { printf(\"%lld\\n\", (long long)i); }\n\n") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("#define __oc_print(x) _Generic((x), \\\n"), .len = sizeof("#define __oc_print(x) _Generic((x), \\\n") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("    char*: __oc_print_str, \\\n"), .len = sizeof("    char*: __oc_print_str, \\\n") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("    const char*: __oc_print_str, \\\n"), .len = sizeof("    const char*: __oc_print_str, \\\n") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("    uint8_t*: __oc_print_u8_ptr, \\\n"), .len = sizeof("    uint8_t*: __oc_print_u8_ptr, \\\n") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("    const uint8_t*: __oc_print_const_u8_ptr, \\\n"), .len = sizeof("    const uint8_t*: __oc_print_const_u8_ptr, \\\n") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("    oc_string: __oc_print_oc_string, \\\n"), .len = sizeof("    oc_string: __oc_print_oc_string, \\\n") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("    bool: __oc_print_bool, \\\n"), .len = sizeof("    bool: __oc_print_bool, \\\n") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("    signed char: __oc_print_i64, \\\n"), .len = sizeof("    signed char: __oc_print_i64, \\\n") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("    short: __oc_print_i64, \\\n"), .len = sizeof("    short: __oc_print_i64, \\\n") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("    int: __oc_print_i64, \\\n"), .len = sizeof("    int: __oc_print_i64, \\\n") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("    long: __oc_print_i64, \\\n"), .len = sizeof("    long: __oc_print_i64, \\\n") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("    long long: __oc_print_i64, \\\n"), .len = sizeof("    long long: __oc_print_i64, \\\n") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("    default: __oc_print_u64 \\\n"), .len = sizeof("    default: __oc_print_u64 \\\n") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)(")(x)\n\n"), .len = sizeof(")(x)\n\n") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("static inline oc_string __oc_slice_str(oc_string s, size_t start, size_t end) {\n"), .len = sizeof("static inline oc_string __oc_slice_str(oc_string s, size_t start, size_t end) {\n") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("    if (start > end || end > s.len) { fprintf(stderr, \"panic: slice index out of bounds\\n\"); exit(1); }\n"), .len = sizeof("    if (start > end || end > s.len) { fprintf(stderr, \"panic: slice index out of bounds\\n\"); exit(1); }\n") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("    return (oc_string){ .ptr = s.ptr + start, .len = end - start };\n"), .len = sizeof("    return (oc_string){ .ptr = s.ptr + start, .len = end - start };\n") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("}\n\n"), .len = sizeof("}\n\n") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("#define __OC_ARR(a) ((oc_string){ .ptr = (const uint8_t*)(a), .len = __builtin_choose_expr(__builtin_types_compatible_p(__typeof__(a), __typeof__(&(a)[0])), (void)0, sizeof(a)) })\n\n"), .len = sizeof("#define __OC_ARR(a) ((oc_string){ .ptr = (const uint8_t*)(a), .len = __builtin_choose_expr(__builtin_types_compatible_p(__typeof__(a), __typeof__(&(a)[0])), (void)0, sizeof(a)) })\n\n") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("static inline uint8_t __oc_index_str(oc_string s, size_t idx) {\n"), .len = sizeof("static inline uint8_t __oc_index_str(oc_string s, size_t idx) {\n") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("    if (idx >= s.len) { fprintf(stderr, \"panic: slice index out of bounds\\n\"); exit(1); }\n"), .len = sizeof("    if (idx >= s.len) { fprintf(stderr, \"panic: slice index out of bounds\\n\"); exit(1); }\n") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("    return s.ptr[idx];\n"), .len = sizeof("    return s.ptr[idx];\n") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("}\n\n"), .len = sizeof("}\n\n") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("#define __OC_AT(a, i) (*__builtin_choose_expr(__builtin_types_compatible_p(__typeof__(a), __typeof__(&(a)[0])), &(a)[i], ({ size_t _i = (size_t)(i); if (_i >= (sizeof(a) + 0) / sizeof((a)[0])) { fprintf(stderr, \"panic: slice index out of bounds\\n\"); exit(1); } &(a)[_i]; })))\n\n"), .len = sizeof("#define __OC_AT(a, i) (*__builtin_choose_expr(__builtin_types_compatible_p(__typeof__(a), __typeof__(&(a)[0])), &(a)[i], ({ size_t _i = (size_t)(i); if (_i >= (sizeof(a) + 0) / sizeof((a)[0])) { fprintf(stderr, \"panic: slice index out of bounds\\n\"); exit(1); } &(a)[_i]; })))\n\n") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("static inline int64_t __oc_div_check(int64_t a, int64_t b) {\n"), .len = sizeof("static inline int64_t __oc_div_check(int64_t a, int64_t b) {\n") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("    if (b == 0) { fprintf(stderr, \"panic: division by zero\\n\"); exit(1); }\n"), .len = sizeof("    if (b == 0) { fprintf(stderr, \"panic: division by zero\\n\"); exit(1); }\n") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("    return a / b;\n"), .len = sizeof("    return a / b;\n") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("}\n\n"), .len = sizeof("}\n\n") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("static inline int64_t __oc_mod_check(int64_t a, int64_t b) {\n"), .len = sizeof("static inline int64_t __oc_mod_check(int64_t a, int64_t b) {\n") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("    if (b == 0) { fprintf(stderr, \"panic: division by zero\\n\"); exit(1); }\n"), .len = sizeof("    if (b == 0) { fprintf(stderr, \"panic: division by zero\\n\"); exit(1); }\n") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("    return a % b;\n"), .len = sizeof("    return a % b;\n") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("}\n\n"), .len = sizeof("}\n\n") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("#define __oc_notnull(p) ({ __auto_type _p = (p); if (!_p) { fprintf(stderr, \"panic: null pointer dereference\\n\"); exit(1); } _p; })\n"), .len = sizeof("#define __oc_notnull(p) ({ __auto_type _p = (p); if (!_p) { fprintf(stderr, \"panic: null pointer dereference\\n\"); exit(1); } _p; })\n") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("#define __OC_DIV(a, b) ({ __auto_type _a = (a); __auto_type _b = (b); if (_b == 0) { fprintf(stderr, \"panic: division by zero\\n\"); exit(1); } _a / _b; })\n"), .len = sizeof("#define __OC_DIV(a, b) ({ __auto_type _a = (a); __auto_type _b = (b); if (_b == 0) { fprintf(stderr, \"panic: division by zero\\n\"); exit(1); } _a / _b; })\n") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("#define __OC_MOD(a, b) ({ __auto_type _a = (a); __auto_type _b = (b); if (_b == 0) { fprintf(stderr, \"panic: division by zero\\n\"); exit(1); } _a % _b; })\n"), .len = sizeof("#define __OC_MOD(a, b) ({ __auto_type _a = (a); __auto_type _b = (b); if (_b == 0) { fprintf(stderr, \"panic: division by zero\\n\"); exit(1); } _a % _b; })\n") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("#define __OC_ADD(a, b) ({ __auto_type _a = (a); __auto_type _b = (b); __typeof__(__builtin_choose_expr(sizeof(_a) < 4, _a, __builtin_choose_expr(sizeof(_b) < 4, _b, _a + _b))) _r; if (__builtin_add_overflow(_a, _b, &_r)) { fprintf(stderr, \"panic: integer overflow\\n\"); exit(1); } _r; })\n"), .len = sizeof("#define __OC_ADD(a, b) ({ __auto_type _a = (a); __auto_type _b = (b); __typeof__(__builtin_choose_expr(sizeof(_a) < 4, _a, __builtin_choose_expr(sizeof(_b) < 4, _b, _a + _b))) _r; if (__builtin_add_overflow(_a, _b, &_r)) { fprintf(stderr, \"panic: integer overflow\\n\"); exit(1); } _r; })\n") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("#define __OC_SUB(a, b) ({ __auto_type _a = (a); __auto_type _b = (b); __typeof__(__builtin_choose_expr(sizeof(_a) < 4, _a, __builtin_choose_expr(sizeof(_b) < 4, _b, _a - _b))) _r; if (__builtin_sub_overflow(_a, _b, &_r)) { fprintf(stderr, \"panic: integer overflow\\n\"); exit(1); } _r; })\n"), .len = sizeof("#define __OC_SUB(a, b) ({ __auto_type _a = (a); __auto_type _b = (b); __typeof__(__builtin_choose_expr(sizeof(_a) < 4, _a, __builtin_choose_expr(sizeof(_b) < 4, _b, _a - _b))) _r; if (__builtin_sub_overflow(_a, _b, &_r)) { fprintf(stderr, \"panic: integer overflow\\n\"); exit(1); } _r; })\n") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("#define __OC_MUL(a, b) ({ __auto_type _a = (a); __auto_type _b = (b); __typeof__(__builtin_choose_expr(sizeof(_a) < 4, _a, __builtin_choose_expr(sizeof(_b) < 4, _b, _a * _b))) _r; if (__builtin_mul_overflow(_a, _b, &_r)) { fprintf(stderr, \"panic: integer overflow\\n\"); exit(1); } _r; })\n"), .len = sizeof("#define __OC_MUL(a, b) ({ __auto_type _a = (a); __auto_type _b = (b); __typeof__(__builtin_choose_expr(sizeof(_a) < 4, _a, __builtin_choose_expr(sizeof(_b) < 4, _b, _a * _b))) _r; if (__builtin_mul_overflow(_a, _b, &_r)) { fprintf(stderr, \"panic: integer overflow\\n\"); exit(1); } _r; })\n") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("#define __OC_WRAP(op, a, b) ({ __auto_type _a = (a); __auto_type _b = (b); __typeof__(__builtin_choose_expr(sizeof(_a) < 4, _a, __builtin_choose_expr(sizeof(_b) < 4, _b, _a + _b))) _r; (void)__builtin_##op##_overflow(_a, _b, &_r); _r; })\n\n"), .len = sizeof("#define __OC_WRAP(op, a, b) ({ __auto_type _a = (a); __auto_type _b = (b); __typeof__(__builtin_choose_expr(sizeof(_a) < 4, _a, __builtin_choose_expr(sizeof(_b) < 4, _b, _a + _b))) _r; (void)__builtin_##op##_overflow(_a, _b, &_r); _r; })\n\n") - 1 });
    __OC_AT((((out)->ptr_fields)), (0)) = (oc_string){ .ptr = (const uint8_t*)("ptr"), .len = sizeof("ptr") - 1 };
    ((out)->ptr_fields_count) = 1;
    AstNode* scan_decl = ((root)->left);
    while ((scan_decl != ((AstNode*)(0)))) {
    if ((((scan_decl)->tag) == NODE_STRUCT_DECL)) {
    AstNode* sf = ((scan_decl)->left);
    while ((sf != ((AstNode*)(0)))) {
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
    if ((((scan_decl)->tag) == NODE_FN)) {
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
    buf_append(out, ((((g_decl)->tok)).slice));
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)(" = "), .len = sizeof(" = ") - 1 });
    codegen_expr(((g_decl)->left), out);
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)(";\n"), .len = sizeof(";\n") - 1 });
    }
    g_decl = ((g_decl)->next);
    }
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("\n"), .len = sizeof("\n") - 1 });
    AstNode* s_decl = ((root)->left);
    while ((s_decl != ((AstNode*)(0)))) {
    if ((((s_decl)->tag) == NODE_STRUCT_DECL)) {
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("typedef struct "), .len = sizeof("typedef struct ") - 1 });
    buf_append(out, ((((s_decl)->tok)).slice));
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)(" "), .len = sizeof(" ") - 1 });
    buf_append(out, ((((s_decl)->tok)).slice));
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)(";\n"), .len = sizeof(";\n") - 1 });
    }
    s_decl = ((s_decl)->next);
    }
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("\n"), .len = sizeof("\n") - 1 });
    s_decl = ((root)->left);
    while ((s_decl != ((AstNode*)(0)))) {
    if ((((s_decl)->tag) == NODE_STRUCT_DECL)) {
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("struct "), .len = sizeof("struct ") - 1 });
    if (str_eq(((s_decl)->type_name), (oc_string){ .ptr = (const uint8_t*)("packed"), .len = sizeof("packed") - 1 })) {
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("__attribute__((packed)) "), .len = sizeof("__attribute__((packed)) ") - 1 });
    }
    buf_append(out, ((((s_decl)->tok)).slice));
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)(" {\n"), .len = sizeof(" {\n") - 1 });
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
    buf_append(out, ((((f)->tok)).slice));
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("["), .len = sizeof("[") - 1 });
    buf_append(out, sz_str);
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("];\n"), .len = sizeof("];\n") - 1 });
    }
    else {
    emit_c_type(((f)->type_name), out);
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)(" "), .len = sizeof(" ") - 1 });
    buf_append(out, ((((f)->tok)).slice));
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)(";\n"), .len = sizeof(";\n") - 1 });
    }
    f = ((f)->next);
    }
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("};\n\n"), .len = sizeof("};\n\n") - 1 });
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
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("int main(void);\n"), .len = sizeof("int main(void);\n") - 1 });
    }
    else {
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("int main(int argc, char** argv);\n"), .len = sizeof("int main(int argc, char** argv);\n") - 1 });
    }
    }
    else {
    emit_c_type(((decl)->type_name), out);
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)(" "), .len = sizeof(" ") - 1 });
    buf_append(out, fn_name);
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("("), .len = sizeof("(") - 1 });
    AstNode* param = ((decl)->left);
    if ((param == ((AstNode*)(0)))) {
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("void"), .len = sizeof("void") - 1 });
    }
    else {
    while ((param != ((AstNode*)(0)))) {
    emit_c_type(((param)->type_name), out);
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)(" "), .len = sizeof(" ") - 1 });
    buf_append(out, ((((param)->tok)).slice));
    if ((((param)->next) != ((AstNode*)(0)))) {
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)(", "), .len = sizeof(", ") - 1 });
    }
    param = ((param)->next);
    }
    }
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)(");\n"), .len = sizeof(");\n") - 1 });
    }
    }
    decl = ((decl)->next);
    }
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("\n"), .len = sizeof("\n") - 1 });
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
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("int main(void) {\n"), .len = sizeof("int main(void) {\n") - 1 });
    }
    else {
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("int main(int argc, char** argv) {\n"), .len = sizeof("int main(int argc, char** argv) {\n") - 1 });
    }
    AstNode* body_block = ((decl)->right);
    if ((body_block != ((AstNode*)(0)))) {
    AstNode* stmt = ((body_block)->left);
    while ((stmt != ((AstNode*)(0)))) {
    codegen_stmt(stmt, out);
    stmt = ((stmt)->next);
    }
    }
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("    return 0;\n"), .len = sizeof("    return 0;\n") - 1 });
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("}\n\n"), .len = sizeof("}\n\n") - 1 });
    }
    else {
    emit_c_type(((decl)->type_name), out);
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)(" "), .len = sizeof(" ") - 1 });
    buf_append(out, fn_name);
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("("), .len = sizeof("(") - 1 });
    AstNode* param = ((decl)->left);
    if ((param == ((AstNode*)(0)))) {
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("void"), .len = sizeof("void") - 1 });
    }
    else {
    while ((param != ((AstNode*)(0)))) {
    emit_c_type(((param)->type_name), out);
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)(" "), .len = sizeof(" ") - 1 });
    buf_append(out, ((((param)->tok)).slice));
    if ((((param)->next) != ((AstNode*)(0)))) {
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)(", "), .len = sizeof(", ") - 1 });
    }
    param = ((param)->next);
    }
    }
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)(") {\n"), .len = sizeof(") {\n") - 1 });
    AstNode* body_block = ((decl)->right);
    AstNode* stmt = ((body_block)->left);
    while ((stmt != ((AstNode*)(0)))) {
    codegen_stmt(stmt, out);
    stmt = ((stmt)->next);
    }
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("}\n\n"), .len = sizeof("}\n\n") - 1 });
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
    int64_t neg = (-({ __auto_type _v = (v); if (!(_v < 0) && (uint64_t)_v > 9223372036854775807ULL) { fprintf(stderr, "panic: invalid signed cast\n"); exit(1); } (int64_t)_v; }));
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
    int64_t neg = (-({ __auto_type _v = (v); if (!(_v < 0) && (uint64_t)_v > 9223372036854775807ULL) { fprintf(stderr, "panic: invalid signed cast\n"); exit(1); } (int64_t)_v; }));
    uv = __OC_SUB(4294967296, ({ __auto_type _v = (neg); if (_v < 0) { fprintf(stderr, "panic: invalid unsigned cast\n"); exit(1); } (uint64_t)_v; }));
    }
    else {
    uv = ({ __auto_type _v = (v); if (_v < 0) { fprintf(stderr, "panic: invalid unsigned cast\n"); exit(1); } (uint64_t)_v; });
    }
    elf_patch_u32(buf, offset, ({ __auto_type _v = (uv); if (_v < 0 || (uint64_t)_v > 4294967295ULL) { fprintf(stderr, "panic: invalid unsigned cast\n"); exit(1); } (uint32_t)_v; }));
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
    if ((((ng)->data_reloc_count) < 512)) {
    __OC_AT((((ng)->data_reloc_code)), (((ng)->data_reloc_count))) = code_off;
    __OC_AT((((ng)->data_reloc_data)), (((ng)->data_reloc_count))) = data_off;
    ((ng)->data_reloc_count) = __OC_ADD(((ng)->data_reloc_count), 1);
    }
    else {
    ng_fail(ng, (oc_string){ .ptr = (const uint8_t*)("native codegen error: data relocation table full (512)"), .len = sizeof("native codegen error: data relocation table full (512)") - 1 });
    }
}

void ng_add_iat_fixup(NativeGen* ng, size_t code_off, size_t slot) {
    if ((((ng)->iat_fix_count) < 16)) {
    __OC_AT((((ng)->iat_fix_code)), (((ng)->iat_fix_count))) = code_off;
    __OC_AT((((ng)->iat_fix_slot)), (((ng)->iat_fix_count))) = slot;
    ((ng)->iat_fix_count) = __OC_ADD(((ng)->iat_fix_count), 1);
    }
    else {
    ng_fail(ng, (oc_string){ .ptr = (const uint8_t*)("native codegen error: import fixup table full (16)"), .len = sizeof("native codegen error: import fixup table full (16)") - 1 });
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

uint64_t parse_u64(oc_string s) {
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
    return ({ __auto_type _v = (parse_u64(__oc_slice_str(t, 1, close_idx))); if (_v < 0) { fprintf(stderr, "panic: invalid unsigned cast\n"); exit(1); } (size_t)_v; });
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
    if ((((expr)->tag) == NODE_IDENT)) {
    return ng_find_var_type(ng, ((((expr)->tok)).slice));
    }
    if ((((expr)->tag) == NODE_CALL)) {
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
    oc_string t = ng_expr_type(ng, expr);
    if (is_unsigned_type_name(t)) {
    return true;
    }
    if ((((expr)->tag) == NODE_BINARY)) {
    if (ng_is_unsigned_expr(ng, ((expr)->left))) {
    return true;
    }
    if (ng_is_unsigned_expr(ng, ((expr)->right))) {
    return true;
    }
    }
    return false;
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
    uint8_t c = __oc_index_str(raw, k);
    if ((c == 92)) {
    if ((__OC_ADD(k, 1) < ((raw).len))) {
    uint8_t nc = __oc_index_str(raw, __OC_ADD(k, 1));
    if ((nc == 110)) {
    buf_append_u8(data, 10);
    emitted = __OC_ADD(emitted, 1);
    k = __OC_ADD(k, 2);
    }
    else {
    if ((nc == 116)) {
    buf_append_u8(data, 9);
    emitted = __OC_ADD(emitted, 1);
    k = __OC_ADD(k, 2);
    }
    else {
    if ((nc == 114)) {
    buf_append_u8(data, 13);
    emitted = __OC_ADD(emitted, 1);
    k = __OC_ADD(k, 2);
    }
    else {
    if ((nc == 48)) {
    buf_append_u8(data, 0);
    emitted = __OC_ADD(emitted, 1);
    k = __OC_ADD(k, 2);
    }
    else {
    if ((nc == 92)) {
    buf_append_u8(data, 92);
    emitted = __OC_ADD(emitted, 1);
    k = __OC_ADD(k, 2);
    }
    else {
    if ((nc == 34)) {
    buf_append_u8(data, 34);
    emitted = __OC_ADD(emitted, 1);
    k = __OC_ADD(k, 2);
    }
    else {
    buf_append_u8(data, c);
    emitted = __OC_ADD(emitted, 1);
    k = __OC_ADD(k, 1);
    }
    }
    }
    }
    }
    }
    }
    else {
    buf_append_u8(data, c);
    emitted = __OC_ADD(emitted, 1);
    k = __OC_ADD(k, 1);
    }
    }
    else {
    buf_append_u8(data, c);
    emitted = __OC_ADD(emitted, 1);
    k = __OC_ADD(k, 1);
    }
    }
    return emitted;
}

void ng_emit_range_check(NativeGen* ng, CodeBuffer* code, oc_string t) {
    if ((((t).len) == 0)) {
    return;
    }
    if (str_eq(t, (oc_string){ .ptr = (const uint8_t*)("u8"), .len = sizeof("u8") - 1 })) {
    buf_append_u8(code, 72);
    buf_append_u8(code, 61);
    elf_append_u32(code, 255);
    ng_add_fixup(ng, ((code)->len), (oc_string){ .ptr = (const uint8_t*)("__trap_overflow"), .len = sizeof("__trap_overflow") - 1 }, true);
    buf_append_u8(code, 15);
    buf_append_u8(code, 135);
    elf_append_u32(code, 0);
    return;
    }
    if (str_eq(t, (oc_string){ .ptr = (const uint8_t*)("u16"), .len = sizeof("u16") - 1 })) {
    buf_append_u8(code, 72);
    buf_append_u8(code, 61);
    elf_append_u32(code, 65535);
    ng_add_fixup(ng, ((code)->len), (oc_string){ .ptr = (const uint8_t*)("__trap_overflow"), .len = sizeof("__trap_overflow") - 1 }, true);
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
    ng_add_fixup(ng, ((code)->len), (oc_string){ .ptr = (const uint8_t*)("__trap_overflow"), .len = sizeof("__trap_overflow") - 1 }, true);
    buf_append_u8(code, 15);
    buf_append_u8(code, 135);
    elf_append_u32(code, 0);
    return;
    }
    if (str_eq(t, (oc_string){ .ptr = (const uint8_t*)("i8"), .len = sizeof("i8") - 1 })) {
    buf_append_u8(code, 72);
    buf_append_u8(code, 61);
    elf_append_i32(code, (-128));
    ng_add_fixup(ng, ((code)->len), (oc_string){ .ptr = (const uint8_t*)("__trap_overflow"), .len = sizeof("__trap_overflow") - 1 }, true);
    buf_append_u8(code, 15);
    buf_append_u8(code, 140);
    elf_append_u32(code, 0);
    buf_append_u8(code, 72);
    buf_append_u8(code, 61);
    elf_append_u32(code, 127);
    ng_add_fixup(ng, ((code)->len), (oc_string){ .ptr = (const uint8_t*)("__trap_overflow"), .len = sizeof("__trap_overflow") - 1 }, true);
    buf_append_u8(code, 15);
    buf_append_u8(code, 143);
    elf_append_u32(code, 0);
    return;
    }
    if (str_eq(t, (oc_string){ .ptr = (const uint8_t*)("i16"), .len = sizeof("i16") - 1 })) {
    buf_append_u8(code, 72);
    buf_append_u8(code, 61);
    elf_append_i32(code, (-32768));
    ng_add_fixup(ng, ((code)->len), (oc_string){ .ptr = (const uint8_t*)("__trap_overflow"), .len = sizeof("__trap_overflow") - 1 }, true);
    buf_append_u8(code, 15);
    buf_append_u8(code, 140);
    elf_append_u32(code, 0);
    buf_append_u8(code, 72);
    buf_append_u8(code, 61);
    elf_append_u32(code, 32767);
    ng_add_fixup(ng, ((code)->len), (oc_string){ .ptr = (const uint8_t*)("__trap_overflow"), .len = sizeof("__trap_overflow") - 1 }, true);
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
    ng_add_fixup(ng, ((code)->len), (oc_string){ .ptr = (const uint8_t*)("__trap_overflow"), .len = sizeof("__trap_overflow") - 1 }, true);
    buf_append_u8(code, 15);
    buf_append_u8(code, 140);
    elf_append_u32(code, 0);
    buf_append_u8(code, 72);
    buf_append_u8(code, 185);
    elf_append_u64(code, 2147483647);
    buf_append_u8(code, 72);
    buf_append_u8(code, 57);
    buf_append_u8(code, 200);
    ng_add_fixup(ng, ((code)->len), (oc_string){ .ptr = (const uint8_t*)("__trap_overflow"), .len = sizeof("__trap_overflow") - 1 }, true);
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
    uint64_t val = parse_u64(((((expr)->tok)).slice));
    buf_append_u8(code, 72);
    buf_append_u8(code, 184);
    elf_append_u64(code, val);
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
    buf_append_u8(code, 72);
    buf_append_u8(code, 131);
    buf_append_u8(code, 236);
    buf_append_u8(code, 32);
    AstNode* f = ((expr)->left);
    while ((f != ((AstNode*)(0)))) {
    int32_t f_off = ng_find_field_offset(ng, ((((expr)->tok)).slice), ((((f)->tok)).slice));
    int32_t f_w = ng_field_width(ng, ((((expr)->tok)).slice), ((((f)->tok)).slice));
    ng_emit_expr(ng, code, data, ((f)->left));
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
    oc_string bt = ng_strip_ptr_type(ng_expr_type(ng, ((expr)->left)));
    oc_string ft = ng_find_field_type(ng, bt, ((((expr)->tok)).slice));
    ng_emit_lvalue_addr(ng, code, data, expr);
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
    ng_emit_load_at_rax(code, ft, ng_field_width(ng, bt, ((((expr)->tok)).slice)));
    return;
    }
    if ((((expr)->tag) == NODE_CAST)) {
    ng_emit_expr(ng, code, data, ((expr)->left));
    oc_string dst = ((expr)->type_name);
    oc_string src = ng_expr_type(ng, ((expr)->left));
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
    if (str_eq(dst, (oc_string){ .ptr = (const uint8_t*)("u8"), .len = sizeof("u8") - 1 })) {
    ng_emit_range_check(ng, code, dst);
    return;
    }
    if (str_eq(dst, (oc_string){ .ptr = (const uint8_t*)("u16"), .len = sizeof("u16") - 1 })) {
    ng_emit_range_check(ng, code, dst);
    return;
    }
    if (str_eq(dst, (oc_string){ .ptr = (const uint8_t*)("u32"), .len = sizeof("u32") - 1 })) {
    ng_emit_range_check(ng, code, dst);
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
    ng_add_fixup(ng, ((code)->len), (oc_string){ .ptr = (const uint8_t*)("__trap_overflow"), .len = sizeof("__trap_overflow") - 1 }, true);
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
    ng_add_fixup(ng, ((code)->len), (oc_string){ .ptr = (const uint8_t*)("__trap_overflow"), .len = sizeof("__trap_overflow") - 1 }, true);
    buf_append_u8(code, 15);
    buf_append_u8(code, 135);
    elf_append_u32(code, 0);
    }
    else {
    ng_emit_range_check(ng, code, dst);
    }
    return;
    }
    if (str_eq(dst, (oc_string){ .ptr = (const uint8_t*)("i64"), .len = sizeof("i64") - 1 })) {
    if (src_uns) {
    buf_append_u8(code, 72);
    buf_append_u8(code, 133);
    buf_append_u8(code, 192);
    ng_add_fixup(ng, ((code)->len), (oc_string){ .ptr = (const uint8_t*)("__trap_overflow"), .len = sizeof("__trap_overflow") - 1 }, true);
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
    uint64_t mag = parse_u64(((((((expr)->left))->tok)).slice));
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
    buf_append_u8(code, 72);
    buf_append_u8(code, 247);
    buf_append_u8(code, 216);
    ng_add_fixup(ng, ((code)->len), (oc_string){ .ptr = (const uint8_t*)("__trap_overflow"), .len = sizeof("__trap_overflow") - 1 }, true);
    buf_append_u8(code, 15);
    buf_append_u8(code, 128);
    elf_append_u32(code, 0);
    return;
    }
    ng_emit_expr(ng, code, data, ((expr)->left));
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
    bool is_uns = false;
    if (ng_is_unsigned_expr(ng, ((expr)->left))) {
    is_uns = true;
    }
    if (ng_is_unsigned_expr(ng, ((expr)->right))) {
    is_uns = true;
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
    ng_emit_range_check(ng, code, res_type);
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
    ng_emit_range_check(ng, code, res_type);
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
    ng_emit_range_check(ng, code, res_type);
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
    oc_string w_t = ng_expr_type(ng, expr);
    if (str_eq(w_t, (oc_string){ .ptr = (const uint8_t*)("u8"), .len = sizeof("u8") - 1 })) {
    buf_append_u8(code, 15);
    buf_append_u8(code, 182);
    buf_append_u8(code, 192);
    }
    else {
    if (str_eq(w_t, (oc_string){ .ptr = (const uint8_t*)("i8"), .len = sizeof("i8") - 1 })) {
    buf_append_u8(code, 72);
    buf_append_u8(code, 15);
    buf_append_u8(code, 190);
    buf_append_u8(code, 192);
    }
    else {
    if (str_eq(w_t, (oc_string){ .ptr = (const uint8_t*)("u16"), .len = sizeof("u16") - 1 })) {
    buf_append_u8(code, 15);
    buf_append_u8(code, 183);
    buf_append_u8(code, 192);
    }
    else {
    if (str_eq(w_t, (oc_string){ .ptr = (const uint8_t*)("i16"), .len = sizeof("i16") - 1 })) {
    buf_append_u8(code, 72);
    buf_append_u8(code, 15);
    buf_append_u8(code, 191);
    buf_append_u8(code, 192);
    }
    else {
    if (str_eq(w_t, (oc_string){ .ptr = (const uint8_t*)("u32"), .len = sizeof("u32") - 1 })) {
    buf_append_u8(code, 137);
    buf_append_u8(code, 192);
    }
    else {
    if (str_eq(w_t, (oc_string){ .ptr = (const uint8_t*)("i32"), .len = sizeof("i32") - 1 })) {
    buf_append_u8(code, 72);
    buf_append_u8(code, 99);
    buf_append_u8(code, 192);
    }
    }
    }
    }
    }
    }
    return;
    }
    size_t reg_count = 0;
    AstNode* a = ((expr)->left);
    while ((a != ((AstNode*)(0)))) {
    ng_emit_expr(ng, code, data, a);
    if (ng_is_expr_slice(ng, a)) {
    buf_append_u8(code, 80);
    buf_append_u8(code, 82);
    reg_count = __OC_ADD(reg_count, 2);
    }
    else {
    buf_append_u8(code, 80);
    reg_count = __OC_ADD(reg_count, 1);
    }
    a = ((a)->next);
    }
    size_t idx = reg_count;
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
    if ((!((ng)->has_error))) {
    printf("%s\n", "native codegen error: syscall intrinsic unavailable on target windows-x86_64 (use --backend c):");
    __oc_print(fn_name);
    }
    ((ng)->has_error) = true;
    return;
    }
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
    ng_add_fixup(ng, ((code)->len), ((((expr)->tok)).slice), false);
    buf_append_u8(code, 232);
    elf_append_u32(code, 0);
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
    if (str_eq(((((first_el)->tok)).slice), (oc_string){ .ptr = (const uint8_t*)("\"\""), .len = sizeof("\"\"") - 1 })) {
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
    ng_emit_expr(ng, code, data, el);
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
    if ((((stmt)->left) != ((AstNode*)(0)))) {
    if ((((((stmt)->left))->tag) == NODE_STRUCT_INIT)) {
    AstNode* f = ((((stmt)->left))->left);
    while ((f != ((AstNode*)(0)))) {
    int32_t f_off = ng_find_field_offset(ng, t_name, ((((f)->tok)).slice));
    oc_string f_type = ng_find_field_type(ng, t_name, ((((f)->tok)).slice));
    ng_emit_expr(ng, code, data, ((f)->left));
    if (str_eq(f_type, (oc_string){ .ptr = (const uint8_t*)("string"), .len = sizeof("string") - 1 })) {
    ng_emit_disp_store_rax(code, __OC_ADD(base_off, f_off));
    ng_emit_disp_store_rdx(code, __OC_ADD(__OC_ADD(base_off, f_off), 8));
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
    ng_emit_expr(ng, code, data, ((stmt)->left));
    ng_emit_range_check(ng, code, t_name);
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
    ng_emit_expr(ng, code, data, ((stmt)->right));
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
    ng_emit_range_check(ng, code, vt);
    ng_emit_disp_store_rax(code, off);
    if (ng_is_var_slice(ng, ((((((stmt)->left))->tok)).slice))) {
    ng_emit_disp_store_rdx(code, __OC_SUB(off, 8));
    }
    return;
    }
    oc_string lhs_t = ng_expr_type(ng, ((stmt)->left));
    ng_emit_expr(ng, code, data, ((stmt)->right));
    ng_emit_range_check(ng, code, lhs_t);
    buf_append_u8(code, 80);
    if (str_eq(lhs_t, (oc_string){ .ptr = (const uint8_t*)("string"), .len = sizeof("string") - 1 })) {
    buf_append_u8(code, 82);
    }
    ng_emit_lvalue_addr(ng, code, data, ((stmt)->left));
    buf_append_u8(code, 72);
    buf_append_u8(code, 137);
    buf_append_u8(code, 193);
    if (str_eq(lhs_t, (oc_string){ .ptr = (const uint8_t*)("string"), .len = sizeof("string") - 1 })) {
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
    ng_emit_expr(ng, code, data, ((stmt)->left));
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

void ng_define_helper(NativeGen* ng, bool* has_offset, oc_string name, size_t off) {
    if ((((ng)->fn_count) < 256)) {
    __OC_AT((((ng)->fn_names)), (((ng)->fn_count))) = name;
    __OC_AT((has_offset), (((ng)->fn_count))) = true;
    __OC_AT((((ng)->fn_offsets)), (((ng)->fn_count))) = off;
    ((ng)->fn_count) = __OC_ADD(((ng)->fn_count), 1);
    }
    else {
    ng_fail(ng, (oc_string){ .ptr = (const uint8_t*)("native codegen error: function table full (256)"), .len = sizeof("native codegen error: function table full (256)") - 1 });
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
    oc_string imp_names[3] = {0};
    __OC_AT((imp_names), (NG_IMP_EXIT_PROCESS)) = (oc_string){ .ptr = (const uint8_t*)("ExitProcess"), .len = sizeof("ExitProcess") - 1 };
    __OC_AT((imp_names), (NG_IMP_GET_STD_HANDLE)) = (oc_string){ .ptr = (const uint8_t*)("GetStdHandle"), .len = sizeof("GetStdHandle") - 1 };
    __OC_AT((imp_names), (NG_IMP_WRITE_FILE)) = (oc_string){ .ptr = (const uint8_t*)("WriteFile"), .len = sizeof("WriteFile") - 1 };
    oc_string dll_name = (oc_string){ .ptr = (const uint8_t*)("KERNEL32.dll"), .len = sizeof("KERNEL32.dll") - 1 };
    size_t thunk_bytes = __OC_MUL(__OC_ADD(NG_IMP_COUNT, 1), 8);
    size_t iat_off = 0;
    size_t ilt_off = __OC_ADD(iat_off, thunk_bytes);
    size_t idt_off = __OC_ADD(ilt_off, thunk_bytes);
    size_t idt_bytes = __OC_MUL(2, 20);
    size_t hint_offs[3] = {0};
    size_t cur = __OC_ADD(idt_off, idt_bytes);
    size_t k = 0;
    while ((k < NG_IMP_COUNT)) {
    __OC_AT((hint_offs), (k)) = cur;
    cur = pe_align(__OC_ADD(__OC_ADD(__OC_ADD(cur, 2), ((__OC_AT((imp_names), (k))).len)), 1), 2);
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
    printf("%s\n", "native codegen error: unexpected data relocation form for PE32+");
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
    buf_append(pe, __OC_AT((imp_names), (k)));
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
    printf("%s\n", "native codegen error: PE32+ image size mismatch");
    buf_deinit(pe);
    return false;
    }
    oc_string pe_str = buf_to_string(pe);
    bool ok = write_file(out_file_str, pe_str);
    buf_deinit(pe);
    return ok;
}

bool emit_native_image(AstNode* root, oc_string out_file_str, int32_t target) {
    CodeBuffer code_buf = buf_init(262144);
    CodeBuffer data_buf = buf_init(65536);
    CodeBuffer* code = (&(code_buf));
    CodeBuffer* data = (&(data_buf));
    NativeGen ng_state = (NativeGen){ .fn_names = { (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 } }, .fn_offsets = { 0 }, .fn_ret_types = { (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 } }, .fn_count = 0, .fixup_offsets = { 0 }, .fixup_names = { (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 } }, .fixup_is_6byte = { false }, .fixup_count = 0, .data_reloc_code = { 0 }, .data_reloc_data = { 0 }, .data_reloc_count = 0, .var_names = { (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 } }, .var_offsets = { 0 }, .var_is_slice = { false }, .var_types = { (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 } }, .var_active = { false }, .var_scope = { 0 }, .var_arr_len = { 0 }, .var_count = 0, .curr_off = 0, .scope_depth = 0, .struct_names = { (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 } }, .struct_sizes = { 0 }, .struct_count = 0, .field_struct = { (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 } }, .field_names = { (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 } }, .field_offsets = { 0 }, .field_types = { (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 } }, .field_packed = { false }, .field_count = 0, .const_names = { (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 } }, .const_vals = { 0 }, .const_count = 0, .loop_start = 0, .loop_depth = 0, .break_patches = { 0 }, .break_depths = { 0 }, .break_count = 0, .target = target, .has_error = false, .iat_fix_code = { 0 }, .iat_fix_slot = { 0 }, .iat_fix_count = 0 };
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
    if ((((ng)->field_count) < 256)) {
    __OC_AT((((ng)->field_struct)), (((ng)->field_count))) = s_name;
    __OC_AT((((ng)->field_names)), (((ng)->field_count))) = ((((sf)->tok)).slice);
    __OC_AT((((ng)->field_offsets)), (((ng)->field_count))) = f_off;
    __OC_AT((((ng)->field_types)), (((ng)->field_count))) = ft;
    __OC_AT((((ng)->field_packed)), (((ng)->field_count))) = is_packed;
    ((ng)->field_count) = __OC_ADD(((ng)->field_count), 1);
    }
    f_off = __OC_ADD(f_off, f_sz);
    sf = ((sf)->next);
    }
    if ((((ng)->struct_count) < 64)) {
    __OC_AT((((ng)->struct_names)), (((ng)->struct_count))) = s_name;
    __OC_AT((((ng)->struct_sizes)), (((ng)->struct_count))) = f_off;
    ((ng)->struct_count) = __OC_ADD(((ng)->struct_count), 1);
    }
    }
    else {
    if ((((scan)->tag) == NODE_LET)) {
    if ((((scan)->left) != ((AstNode*)(0)))) {
    if ((((ng)->const_count) < 128)) {
    __OC_AT((((ng)->const_names)), (((ng)->const_count))) = ((((scan)->tok)).slice);
    __OC_AT((((ng)->const_vals)), (((ng)->const_count))) = parse_u64(((((((scan)->left))->tok)).slice));
    ((ng)->const_count) = __OC_ADD(((ng)->const_count), 1);
    }
    }
    }
    else {
    if ((((scan)->tag) == NODE_FN)) {
    if ((((ng)->fn_count) < 256)) {
    __OC_AT((((ng)->fn_names)), (((ng)->fn_count))) = ((((scan)->tok)).slice);
    __OC_AT((((ng)->fn_offsets)), (((ng)->fn_count))) = 0;
    __OC_AT((((ng)->fn_ret_types)), (((ng)->fn_count))) = ((scan)->type_name);
    ((ng)->fn_count) = __OC_ADD(((ng)->fn_count), 1);
    }
    }
    }
    }
    scan = ((scan)->next);
    }
    bool fn_has_offset[256] = {0};
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
    if ((((ng)->fn_count) < 256)) {
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
    p = ((decl)->left);
    size_t reg_idx = 0;
    size_t p_var_idx = 0;
    while ((p != ((AstNode*)(0)))) {
    int32_t off = __OC_AT((((ng)->var_offsets)), (p_var_idx));
    ng_emit_store_param_reg(code, reg_idx, off);
    reg_idx = __OC_ADD(reg_idx, 1);
    if (__OC_AT((((ng)->var_is_slice)), (p_var_idx))) {
    ng_emit_store_param_reg(code, reg_idx, __OC_SUB(off, 8));
    reg_idx = __OC_ADD(reg_idx, 1);
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
    uint32_t used_sz = ({ __auto_type _v = (__OC_MUL(__OC_DIV(__OC_ADD((-((ng)->curr_off)), 15), 16), 16)); if (_v < 0 || (uint64_t)_v > 4294967295ULL) { fprintf(stderr, "panic: invalid unsigned cast\n"); exit(1); } (uint32_t)_v; });
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
    int32_t neg = (-rel8);
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
    oc_string traps[4] = {0};
    __OC_AT((traps), (0)) = (oc_string){ .ptr = (const uint8_t*)("__trap_null_deref"), .len = sizeof("__trap_null_deref") - 1 };
    __OC_AT((traps), (1)) = (oc_string){ .ptr = (const uint8_t*)("__trap_slice_bounds"), .len = sizeof("__trap_slice_bounds") - 1 };
    __OC_AT((traps), (2)) = (oc_string){ .ptr = (const uint8_t*)("__trap_overflow"), .len = sizeof("__trap_overflow") - 1 };
    __OC_AT((traps), (3)) = (oc_string){ .ptr = (const uint8_t*)("__trap_div_zero"), .len = sizeof("__trap_div_zero") - 1 };
    oc_string trap_msgs[4] = {0};
    __OC_AT((trap_msgs), (0)) = (oc_string){ .ptr = (const uint8_t*)("panic: null pointer dereference\n"), .len = sizeof("panic: null pointer dereference\n") - 1 };
    __OC_AT((trap_msgs), (1)) = (oc_string){ .ptr = (const uint8_t*)("panic: slice index out of bounds\n"), .len = sizeof("panic: slice index out of bounds\n") - 1 };
    __OC_AT((trap_msgs), (2)) = (oc_string){ .ptr = (const uint8_t*)("panic: integer overflow\n"), .len = sizeof("panic: integer overflow\n") - 1 };
    __OC_AT((trap_msgs), (3)) = (oc_string){ .ptr = (const uint8_t*)("panic: division by zero\n"), .len = sizeof("panic: division by zero\n") - 1 };
    size_t ti = 0;
    while ((ti < 4)) {
    ng_define_helper(ng, has_offset, __OC_AT((traps), (ti)), ((code)->len));
    oc_string tmsg = __OC_AT((trap_msgs), (ti));
    size_t t_data_off = ((data)->len);
    size_t tk = 0;
    while ((tk < ((tmsg).len))) {
    buf_append_u8(data, __oc_index_str(tmsg, tk));
    tk = __OC_ADD(tk, 1);
    }
    if ((!is_win)) {
    buf_append_u8(code, 72);
    buf_append_u8(code, 199);
    buf_append_u8(code, 192);
    elf_append_u32(code, 1);
    }
    buf_append_u8(code, 72);
    buf_append_u8(code, 199);
    buf_append_u8(code, 199);
    elf_append_u32(code, 2);
    buf_append_u8(code, 72);
    buf_append_u8(code, 190);
    size_t t_patch = ((code)->len);
    elf_append_u64(code, 0);
    ng_add_data_reloc(ng, t_patch, t_data_off);
    buf_append_u8(code, 72);
    buf_append_u8(code, 199);
    buf_append_u8(code, 194);
    elf_append_u32(code, ({ __auto_type _v = (((tmsg).len)); if (_v < 0 || (uint64_t)_v > 4294967295ULL) { fprintf(stderr, "panic: invalid unsigned cast\n"); exit(1); } (uint32_t)_v; }));
    if (is_win) {
    ng_emit_call(ng, code, (oc_string){ .ptr = (const uint8_t*)("__win_write"), .len = sizeof("__win_write") - 1 });
    buf_append_u8(code, 72);
    buf_append_u8(code, 199);
    buf_append_u8(code, 199);
    elf_append_u32(code, 1);
    ng_emit_call(ng, code, (oc_string){ .ptr = (const uint8_t*)("__win_exit"), .len = sizeof("__win_exit") - 1 });
    }
    else {
    buf_append_u8(code, 15);
    buf_append_u8(code, 5);
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
    printf("%s\n", "native codegen error: unresolved symbol:");
    __oc_print(f_name);
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

bool emit_native_elf(AstNode* root, oc_string out_file_str) {
    return emit_native_image(root, out_file_str, OC_TARGET_LINUX_X86_64);
}

bool emit_native_pe(AstNode* root, oc_string out_file_str) {
    return emit_native_image(root, out_file_str, OC_TARGET_WINDOWS_X86_64);
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

oc_string cstr_to_string(uint8_t* cstr) {
    size_t len = 0;
    while ((__OC_AT((cstr), (len)) != 0)) {
    len = __OC_ADD(len, 1);
    }
    return (oc_string){ .ptr = cstr, .len = len };
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

oc_string str_concat(Arena* arena, oc_string a, oc_string b) {
    size_t total = __OC_ADD(((a).len), ((b).len));
    if ((total == 0)) {
    return (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 };
    }
    uint8_t* buf = arena_alloc(arena, total);
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
    if ((((base_dir).len) > 0)) {
    if ((((path).len) > 0)) {
    if ((!is_abs_path(path))) {
    oc_string cand1 = str_concat(arena, base_dir, path);
    bytes = read_file(cand1, file_buf, 262143);
    if ((bytes >= 0)) {
    resolved_path = cand1;
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
    printf("%s\n", "error: failed to read file: ");
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
    buf_append(out, src);
    buf_append(out, (oc_string){ .ptr = (const uint8_t*)("\n"), .len = sizeof("\n") - 1 });
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

void print_usage(void) {
    printf("%s\n", "Usage: occ [run|build] <input.oc> [-o <output>] [--backend c|native] [--target <target>] [-O1|-O2|-O3] [--emit-c <file.c>]");
    printf("%s\n", "       occ install | update | version | help   (also: --version, -v, --help, -h)");
    printf("%s\n", "install: make 'occ' runnable by name (symlink in ~/.local/bin; user PATH on Windows). update: git pull + rebuild this install.");
    printf("%s\n", "Environment: OC_HOME = directory containing std/ (default: next to the occ binary's bin/ directory)");
    printf("%s\n", "Targets (default: host): linux-x86_64, windows-x86_64 (alias: windows), macos-x86_64, macos-aarch64");
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
    printf("%s\n", "Updating oC from git...");
    buf_append((&(cmd)), (oc_string){ .ptr = (const uint8_t*)("git "), .len = sizeof("git ") - 1 });
    if ((((repo).len) > 0)) {
    buf_append((&(cmd)), (oc_string){ .ptr = (const uint8_t*)("-C "), .len = sizeof("-C ") - 1 });
    buf_append((&(cmd)), repo);
    buf_append((&(cmd)), (oc_string){ .ptr = (const uint8_t*)(" "), .len = sizeof(" ") - 1 });
    }
    buf_append((&(cmd)), (oc_string){ .ptr = (const uint8_t*)("pull"), .len = sizeof("pull") - 1 });
    int32_t pull_ret = run_sys_cmd(buf_to_string((&(cmd))));
    if ((pull_ret != 0)) {
    printf("%s\n", "error: 'git pull' failed. Make sure git is installed and occ lives in <oC checkout>/bin (checkout used:)");
    __oc_print(repo);
    buf_deinit((&(cmd)));
    return 1;
    }
    printf("%s\n", "Rebuilding oC compiler...");
    ((cmd).len) = 0;
    if ((host == OC_TARGET_WINDOWS_X86_64)) {
    buf_append((&(cmd)), (oc_string){ .ptr = (const uint8_t*)("if exist "), .len = sizeof("if exist ") - 1 });
    buf_append((&(cmd)), repo);
    buf_append((&(cmd)), (oc_string){ .ptr = (const uint8_t*)("bin\\occ.exe move /y "), .len = sizeof("bin\\occ.exe move /y ") - 1 });
    buf_append((&(cmd)), repo);
    buf_append((&(cmd)), (oc_string){ .ptr = (const uint8_t*)("bin\\occ.exe "), .len = sizeof("bin\\occ.exe ") - 1 });
    buf_append((&(cmd)), repo);
    buf_append((&(cmd)), (oc_string){ .ptr = (const uint8_t*)("bin\\occ.exe.old >nul 2>&1 & gcc -O2 "), .len = sizeof("bin\\occ.exe.old >nul 2>&1 & gcc -O2 ") - 1 });
    buf_append((&(cmd)), repo);
    buf_append((&(cmd)), (oc_string){ .ptr = (const uint8_t*)("bootstrap\\occ_bootstrap.c -o "), .len = sizeof("bootstrap\\occ_bootstrap.c -o ") - 1 });
    buf_append((&(cmd)), repo);
    buf_append((&(cmd)), (oc_string){ .ptr = (const uint8_t*)("bin\\occ.exe"), .len = sizeof("bin\\occ.exe") - 1 });
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
    printf("%s\n", "error: failed to rebuild compiler from bootstrap/occ_bootstrap.c");
    return 1;
    }
    printf("%s\n", "Successfully updated oC!");
    return 0;
}

int32_t handle_install(int32_t host, oc_string exe, oc_string exe_dir) {
    bool host_is_windows = (host == OC_TARGET_WINDOWS_X86_64);
    if ((!is_safe_path(exe, host_is_windows))) {
    printf("%s\n", "error: cannot install: the path of this occ binary has characters that are unsafe on a command line:");
    __oc_print(exe);
    return 1;
    }
    CodeBuffer cmd = buf_init(8192);
    if (host_is_windows) {
    oc_string bin_dir = __oc_slice_str(exe_dir, 0, __OC_SUB(((exe_dir).len), 1));
    buf_append((&(cmd)), (oc_string){ .ptr = (const uint8_t*)("powershell -NoProfile -Command \"$d='"), .len = sizeof("powershell -NoProfile -Command \"$d='") - 1 });
    buf_append((&(cmd)), bin_dir);
    buf_append((&(cmd)), (oc_string){ .ptr = (const uint8_t*)("'; $p=[Environment]::GetEnvironmentVariable('Path','User'); if (-not $p) { $p='' }; if (($p -split ';') -notcontains $d) { [Environment]::SetEnvironmentVariable('Path', ($p.TrimEnd(';') + ';' + $d).TrimStart(';'), 'User') }\""), .len = sizeof("'; $p=[Environment]::GetEnvironmentVariable('Path','User'); if (-not $p) { $p='' }; if (($p -split ';') -notcontains $d) { [Environment]::SetEnvironmentVariable('Path', ($p.TrimEnd(';') + ';' + $d).TrimStart(';'), 'User') }\"") - 1 });
    }
    else {
    buf_append((&(cmd)), (oc_string){ .ptr = (const uint8_t*)("mkdir -p \"$HOME/.local/bin\" && ln -sf "), .len = sizeof("mkdir -p \"$HOME/.local/bin\" && ln -sf ") - 1 });
    buf_append((&(cmd)), exe);
    buf_append((&(cmd)), (oc_string){ .ptr = (const uint8_t*)(" \"$HOME/.local/bin/occ\""), .len = sizeof(" \"$HOME/.local/bin/occ\"") - 1 });
    }
    int32_t ret = run_sys_cmd(buf_to_string((&(cmd))));
    buf_deinit((&(cmd)));
    if ((ret != 0)) {
    printf("%s\n", "error: install failed");
    return 1;
    }
    if (host_is_windows) {
    printf("%s\n", "Added the oC bin directory to your user PATH. Open a new terminal, then run: occ --version");
    }
    else {
    printf("%s\n", "Linked ~/.local/bin/occ. If 'occ' is not found, add ~/.local/bin to your PATH.");
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
    printf("%s\n", "error: cannot install: could not determine the absolute path of this occ binary");
    }
    }
    else {
    oc_string up = (oc_string){ .ptr = (const uint8_t*)("../"), .len = sizeof("../") - 1 };
    if (host_is_windows) {
    up = (oc_string){ .ptr = (const uint8_t*)("..\\"), .len = sizeof("..\\") - 1 };
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
    oc_string target_name = (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 };
    bool target_explicit = false;
    int32_t i = 1;
    if ((i < argc)) {
    oc_string first = cstr_to_string(__OC_AT((argv), (i)));
    if (str_eq(first, (oc_string){ .ptr = (const uint8_t*)("version"), .len = sizeof("version") - 1 })) {
    printf("%s\n", "oC 1.0.1");
    return 0;
    }
    else {
    if (str_eq(first, (oc_string){ .ptr = (const uint8_t*)("--version"), .len = sizeof("--version") - 1 })) {
    printf("%s\n", "oC 1.0.1");
    return 0;
    }
    else {
    if (str_eq(first, (oc_string){ .ptr = (const uint8_t*)("-v"), .len = sizeof("-v") - 1 })) {
    printf("%s\n", "oC 1.0.1");
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
    printf("%s\n", "error: --target requires a value");
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
    printf("%s\n", "error: unknown option:");
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
    printf("%s\n", "error: unknown --target (expected linux-x86_64, windows-x86_64, macos-x86_64 or macos-aarch64):");
    __oc_print(target_name);
    return 1;
    }
    }
    int32_t out_target = target;
    if (is_native) {
    if ((target != OC_TARGET_WINDOWS_X86_64)) {
    if ((target != OC_TARGET_LINUX_X86_64)) {
    if (target_explicit) {
    printf("%s\n", "error: --backend native emits linux-x86_64 (ELF64) or windows-x86_64 (PE32+); use --backend c for target:");
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
    printf("%s\n", "error: no C cross-compiler convention for this host and target (macOS targets need a macOS host):");
    __oc_print(target_name);
    return 1;
    }
    }
    if (is_run) {
    if (target_explicit) {
    if ((out_target != host)) {
    printf("%s\n", "error: cannot run a binary for another platform on this host; use 'occ build' for target:");
    __oc_print(target_name);
    return 1;
    }
    }
    }
    if ((!is_safe_path(in_file_str, host_is_windows))) {
    printf("%s\n", "error: unsafe characters in file path");
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
    printf("%s\n", "error: unsafe characters in file path");
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
    ModuleLoader loader = (ModuleLoader){ .visited = { (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 } }, .visited_count = 0 };
    CodeBuffer combined_buf = buf_init(4194304);
    bool load_ok = load_module_recursive(in_file_str, (oc_string){ .ptr = (const uint8_t*)(""), .len = sizeof("") - 1 }, std_root, (&(combined_buf)), (&(loader)), (&(arena)));
    if ((!load_ok)) {
    buf_deinit((&(combined_buf)));
    arena_deinit((&(arena)));
    return 1;
    }
    oc_string combined_src = buf_to_string((&(combined_buf)));
    Parser parser = parser_init(combined_src, (&(arena)));
    AstNode* prog = parse_program((&(parser)));
    if (((parser).has_error)) {
    printf("%s\n", "compilation aborted due to parse errors");
    buf_deinit((&(combined_buf)));
    arena_deinit((&(arena)));
    return 1;
    }
    bool valid = validate_program(prog);
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
    printf("%s\n", "error: failed to create a private temporary directory (tried $TMPDIR, $TEMP, /tmp and .)");
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
    ok_native = emit_native_pe(prog, out_file_str);
    }
    else {
    ok_native = emit_native_elf(prog, out_file_str);
    }
    buf_deinit((&(combined_buf)));
    if ((!ok_native)) {
    printf("%s\n", "error: native image emission failed");
    remove_private_tmp_dir((&(arena)), tmp_dir);
    arena_deinit((&(arena)));
    return 1;
    }
    }
    else {
    CodeBuffer out = buf_init(4194304);
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
    printf("%s\n", "error: failed to write temporary C stage file");
    buf_deinit((&(out)));
    remove_private_tmp_dir((&(arena)), tmp_dir);
    arena_deinit((&(arena)));
    return 1;
    }
    CodeBuffer cmd_buf = buf_init(512);
    buf_append((&(cmd_buf)), cc_cmd);
    buf_append((&(cmd_buf)), opt_flag);
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
    printf("%s\n", "error: C compiler failed:");
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

