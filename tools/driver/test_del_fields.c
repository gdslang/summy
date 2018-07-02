#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <setjmp.h>
#include <stdio.h>

typedef void* obj_t;
typedef struct state* state_t;
typedef long long int int_t;
typedef char* string_t;
typedef uint64_t vec_data_t;

struct vec {
  unsigned int size;
  vec_data_t data;
};

typedef struct vec vec_t;
typedef int_t con_tag_t;

typedef struct {
  int_t addr_sz;
  int_t config;
  int_t features;
  obj_t insn;
  int_t length;
  int_t lock;
  int_t opnd_sz;
  int_t rep;
  int_t repne;
} unboxed_insndata_t;
typedef unboxed_insndata_t* insndata_t;
typedef struct {
  obj_t opnd1;
  obj_t opnd2;
} unboxed_arity2_t;
typedef unboxed_arity2_t* arity2_t;
typedef struct {
  obj_t address;
  int_t size;
} unboxed_sem_address_t;
typedef unboxed_sem_address_t* sem_address_t;
typedef struct {
  obj_t (*sem_sexpr_arb)(state_t);
  obj_t (*sem_sexpr_cmp)(state_t,int_t,obj_t);
  obj_t (*sem_sexpr_lin)(state_t,obj_t);
} unboxed_sem_sexpr_callbacks_t;
typedef unboxed_sem_sexpr_callbacks_t* sem_sexpr_callbacks_t;
typedef struct {
  obj_t (*sem_flop_)(state_t,int_t);
} unboxed_sem_flop_callbacks_t;
typedef unboxed_sem_flop_callbacks_t* sem_flop_callbacks_t;
typedef struct {
  obj_t (*arch)(state_t,string_t);
  obj_t (*shared)(state_t,int_t);
  obj_t (*virt_t)(state_t,int_t);
} unboxed_sem_id_callbacks_t;
typedef unboxed_sem_id_callbacks_t* sem_id_callbacks_t;
typedef struct {
  obj_t (*sem_address_)(state_t,int_t,obj_t);
} unboxed_sem_address_callbacks_t;
typedef unboxed_sem_address_callbacks_t* sem_address_callbacks_t;
typedef struct {
  obj_t (*sem_var_)(state_t,obj_t,int_t);
} unboxed_sem_var_callbacks_t;
typedef unboxed_sem_var_callbacks_t* sem_var_callbacks_t;
typedef struct {
  obj_t (*sem_lin_add)(state_t,obj_t,obj_t);
  obj_t (*sem_lin_imm)(state_t,int_t);
  obj_t (*sem_lin_scale)(state_t,int_t,obj_t);
  obj_t (*sem_lin_sub)(state_t,obj_t,obj_t);
  obj_t (*sem_lin_var)(state_t,obj_t);
} unboxed_sem_linear_callbacks_t;
typedef unboxed_sem_linear_callbacks_t* sem_linear_callbacks_t;
typedef struct {
  obj_t (*sem_cmpeq)(state_t,obj_t,obj_t);
  obj_t (*sem_cmples)(state_t,obj_t,obj_t);
  obj_t (*sem_cmpleu)(state_t,obj_t,obj_t);
  obj_t (*sem_cmplts)(state_t,obj_t,obj_t);
  obj_t (*sem_cmpltu)(state_t,obj_t,obj_t);
  obj_t (*sem_cmpneq)(state_t,obj_t,obj_t);
} unboxed_sem_expr_cmp_callbacks_t;
typedef unboxed_sem_expr_cmp_callbacks_t* sem_expr_cmp_callbacks_t;
typedef struct {
  obj_t (*sem_and)(state_t,obj_t,obj_t);
  obj_t (*sem_div)(state_t,obj_t,obj_t);
  obj_t (*sem_divs)(state_t,obj_t,obj_t);
  obj_t (*sem_mod)(state_t,obj_t,obj_t);
  obj_t (*sem_mods)(state_t,obj_t,obj_t);
  obj_t (*sem_mul)(state_t,obj_t,obj_t);
  obj_t (*sem_or)(state_t,obj_t,obj_t);
  obj_t (*sem_sexpr)(state_t,obj_t);
  obj_t (*sem_shl)(state_t,obj_t,obj_t);
  obj_t (*sem_shr)(state_t,obj_t,obj_t);
  obj_t (*sem_shrs)(state_t,obj_t,obj_t);
  obj_t (*sem_sx)(state_t,int_t,obj_t);
  obj_t (*sem_xor)(state_t,obj_t,obj_t);
  obj_t (*sem_zx)(state_t,int_t,obj_t);
} unboxed_sem_expr_callbacks_t;
typedef unboxed_sem_expr_callbacks_t* sem_expr_callbacks_t;
typedef struct {
  obj_t (*sem_varl_)(state_t,obj_t,int_t,int_t);
} unboxed_sem_varl_callbacks_t;
typedef unboxed_sem_varl_callbacks_t* sem_varl_callbacks_t;
typedef struct {
  obj_t (*sem_varl_list_init)(state_t);
  obj_t (*sem_varl_list_next)(state_t,obj_t,obj_t);
} unboxed_sem_varl_list_callbacks_t;
typedef unboxed_sem_varl_list_callbacks_t* sem_varl_list_callbacks_t;
typedef struct {
  obj_t (*sem_assign)(state_t,int_t,obj_t,obj_t);
  obj_t (*sem_branch)(state_t,obj_t,obj_t);
  obj_t (*sem_cbranch)(state_t,obj_t,obj_t,obj_t);
  obj_t (*sem_flop)(state_t,obj_t,obj_t,obj_t,obj_t);
  obj_t (*sem_ite)(state_t,obj_t,obj_t,obj_t);
  obj_t (*sem_load)(state_t,int_t,obj_t,obj_t);
  obj_t (*sem_prim)(state_t,string_t,obj_t,obj_t);
  obj_t (*sem_store)(state_t,int_t,obj_t,obj_t);
  obj_t (*sem_throw)(state_t,obj_t);
  obj_t (*sem_while)(state_t,obj_t,obj_t);
} unboxed_sem_stmt_callbacks_t;
typedef unboxed_sem_stmt_callbacks_t* sem_stmt_callbacks_t;
typedef struct {
  obj_t (*branch_hint_)(state_t,int_t);
} unboxed_branch_hint_callbacks_t;
typedef unboxed_branch_hint_callbacks_t* branch_hint_callbacks_t;
typedef struct {
  obj_t (*arch)(state_t,string_t);
  obj_t (*shared)(state_t,int_t);
} unboxed_sem_exception_callbacks_t;
typedef unboxed_sem_exception_callbacks_t* sem_exception_callbacks_t;
typedef struct {
  obj_t (*sem_stmt_list_init)(state_t);
  obj_t (*sem_stmt_list_next)(state_t,obj_t,obj_t);
} unboxed_sem_stmt_list_callbacks_t;
typedef unboxed_sem_stmt_list_callbacks_t* sem_stmt_list_callbacks_t;
typedef struct {
  branch_hint_callbacks_t branch_hint;
  sem_address_callbacks_t sem_address;
  sem_exception_callbacks_t sem_exception;
  sem_expr_callbacks_t sem_expr;
  sem_expr_cmp_callbacks_t sem_expr_cmp;
  sem_flop_callbacks_t sem_flop;
  sem_id_callbacks_t sem_id;
  sem_linear_callbacks_t sem_linear;
  sem_sexpr_callbacks_t sem_sexpr;
  sem_stmt_callbacks_t sem_stmt;
  sem_stmt_list_callbacks_t sem_stmt_list;
  sem_var_callbacks_t sem_var;
  sem_varl_callbacks_t sem_varl;
  sem_varl_list_callbacks_t sem_varl_list;
} unboxed_callbacks_t;
typedef unboxed_callbacks_t* callbacks_t;
typedef struct {
  obj_t insns;
  obj_t succ_a;
  obj_t succ_b;
} unboxed_translate_result_t;
typedef unboxed_translate_result_t* translate_result_t;
typedef struct {
  obj_t after;
  obj_t initial;
} unboxed_lv_super_result_t;
typedef unboxed_lv_super_result_t* lv_super_result_t;
typedef struct {
  obj_t insns;
  obj_t rreil;
} unboxed_opt_result_t;
typedef unboxed_opt_result_t* opt_result_t;
typedef struct {
  obj_t annotations;
  int_t length;
  string_t mnemonic;
  obj_t opnds;
} unboxed_asm_insn_t;
typedef unboxed_asm_insn_t* asm_insn_t;
typedef struct {
  obj_t (*annotated)(state_t,obj_t,obj_t);
  obj_t (*bounded)(state_t,obj_t,obj_t);
  obj_t (*composite)(state_t,obj_t);
  obj_t (*imm)(state_t,int_t);
  obj_t (*memory)(state_t,obj_t);
  obj_t (*opnd_register)(state_t,string_t);
  obj_t (*post_op)(state_t,obj_t,obj_t);
  obj_t (*pre_op)(state_t,obj_t,obj_t);
  obj_t (*rel)(state_t,obj_t);
  obj_t (*scale)(state_t,int_t,obj_t);
  obj_t (*sign)(state_t,obj_t,obj_t);
  obj_t (*sum)(state_t,obj_t,obj_t);
} unboxed_asm_opnd_callbacks_t;
typedef unboxed_asm_opnd_callbacks_t* asm_opnd_callbacks_t;
typedef struct {
  obj_t (*sz)(state_t,int_t);
  obj_t (*sz_o)(state_t,int_t,int_t);
} unboxed_asm_boundary_callbacks_t;
typedef unboxed_asm_boundary_callbacks_t* asm_boundary_callbacks_t;
typedef struct {
  obj_t (*asm_signed)(state_t);
  obj_t (*asm_unsigned)(state_t);
} unboxed_asm_signedness_callbacks_t;
typedef unboxed_asm_signedness_callbacks_t* asm_signedness_callbacks_t;
typedef struct {
  obj_t (*init)(state_t);
  obj_t (*opnd_list_next)(state_t,obj_t,obj_t);
} unboxed_asm_opnd_list_callbacks_t;
typedef unboxed_asm_opnd_list_callbacks_t* asm_opnd_list_callbacks_t;
typedef struct {
  obj_t (*annotation_list_next)(state_t,obj_t,obj_t);
  obj_t (*init)(state_t);
} unboxed_asm_annotation_list_callbacks_t;
typedef unboxed_asm_annotation_list_callbacks_t* asm_annotation_list_callbacks_t;
typedef struct {
  obj_t (*ann_string)(state_t,string_t);
  obj_t (*function)(state_t,string_t,obj_t);
  obj_t (*opnd)(state_t,string_t,obj_t);
} unboxed_asm_annotation_callbacks_t;
typedef unboxed_asm_annotation_callbacks_t* asm_annotation_callbacks_t;
typedef struct {
  asm_annotation_callbacks_t annotation;
  asm_annotation_list_callbacks_t annotation_list;
  asm_boundary_callbacks_t boundary;
  obj_t (*insn)(state_t,int_t,string_t,obj_t,obj_t);
  asm_opnd_callbacks_t opnd;
  asm_opnd_list_callbacks_t opnd_list;
  asm_signedness_callbacks_t signedness;
} unboxed_asm_callbacks_t;
typedef unboxed_asm_callbacks_t* asm_callbacks_t;
typedef struct {
  obj_t id;
  int_t offset;
} unboxed_sem_var_t;
typedef unboxed_sem_var_t* sem_var_t;
typedef struct {
  obj_t id;
  int_t offset;
  int_t size;
} unboxed_sem_varl_t;
typedef unboxed_sem_varl_t* sem_varl_t;

typedef struct {
  obj_t left;
  obj_t payload;
  obj_t right;
  int_t size;
} struct1_t;
typedef struct {
  int_t addrsz;
  int_t default_operand_size;
  int_t foundJump;
  int_t ins_count;
  obj_t insns;
  int_t lab;
  obj_t live;
  int_t lock;
  obj_t maybelive;
  int_t mod;
  int_t mode64;
  int_t opndsz;
  int_t ptrsz;
  int_t ptrty;
  int_t reg_slash_opcode;
  int_t rep;
  int_t repne;
  int_t rex;
  int_t rexb;
  int_t rexr;
  int_t rexw;
  int_t rexx;
  int_t rm;
  obj_t segment;
  obj_t stack;
  insndata_t (*tab)(state_t);
  int_t tmp;
  int_t vexl;
  int_t vexm;
  int_t vexv;
  int_t vexw;
} monad_t;
typedef struct {
  int_t const_;
} struct4_t;
typedef struct {
  obj_t lhs;
  obj_t rhs;
  int_t size;
} struct6_t;
typedef struct {
  obj_t hd;
  obj_t tl;
} struct7_t;
typedef struct {
  int_t const_;
  obj_t opnd;
} struct8_t;
typedef struct {
  obj_t cond;
  obj_t else_branch;
  obj_t then_branch;
} struct9_t;
typedef struct {
  obj_t cmp;
  int_t size;
} struct10_t;
typedef struct {
  obj_t opnd;
  int_t psz;
  obj_t segment;
  int_t sz;
} struct11_t;
typedef struct {
  int_t fromsize;
  obj_t opnd1;
} struct12_t;
typedef struct {
  sem_address_t address;
  obj_t lhs;
  int_t size;
} struct14_t;
typedef struct {
  int_t imm;
  obj_t opnd;
} struct15_t;
typedef struct {
  obj_t a;
  obj_t b;
} struct16_t;
typedef struct {
  int_t address;
  vec_t imm;
} struct17_t;
typedef struct {
  int_t addr_sz;
  int_t features;
  int_t lock;
  int_t opnd_sz;
  obj_t opnd1;
  obj_t opnd2;
  obj_t opnd3;
  obj_t opnd4;
  int_t rep;
  int_t repne;
} struct18_t;
typedef struct {
  obj_t id;
  int_t size;
} struct19_t;
typedef struct {
  obj_t address;
  obj_t segment;
  int_t size;
} struct20_t;
typedef struct {
  sem_address_t address;
  obj_t rhs;
  int_t size;
} struct21_t;
typedef struct {
  int_t rope_size;
  string_t rope_string;
} struct22_t;
typedef struct {
  obj_t rope_left;
  obj_t rope_right;
  int_t rope_size;
} struct23_t;
typedef struct {
  vec_t confData;
  string_t confLongName;
  obj_t confNext;
  string_t confShortName;
} struct24_t;
typedef struct {
  obj_t body;
  obj_t cond;
} struct25_t;
typedef struct {
  obj_t hint;
  sem_address_t target;
} struct26_t;
typedef struct {
  obj_t cond;
  sem_address_t target_false;
  sem_address_t target_true;
} struct27_t;
typedef struct {
  obj_t flags;
  obj_t lhs;
  obj_t op;
  obj_t rhs;
} struct28_t;
typedef struct {
  obj_t lhs;
  string_t op;
  obj_t rhs;
} struct29_t;
typedef struct {
  insndata_t insn;
  obj_t tl;
} struct45_t;
typedef struct {
  obj_t conservative;
  obj_t greedy;
} struct46_t;
typedef struct {
  obj_t live;
  obj_t maybelive;
} struct47_t;
typedef struct {
  int_t rep;
  int_t repne;
} struct50_t;
typedef struct {
  int_t addr_sz;
  int_t config;
  int_t features;
  obj_t insn;
  int_t length;
  int_t lock;
  int_t opnd_sz;
  obj_t opnd1;
  int_t rep;
  int_t repne;
} struct51_t;
typedef struct {
  obj_t opnd1;
} arity1_t;
typedef struct {
  obj_t opnd1;
  obj_t opnd2;
  obj_t opnd3;
} arity3_t;
typedef struct {
  obj_t opnd1;
  obj_t opnd2;
  obj_t opnd3;
  obj_t opnd4;
} arity4_t;
typedef struct {
  obj_t base;
  obj_t dst;
} struct55_t;
typedef struct {
  obj_t cont;
  obj_t id;
  int_t offset;
  int_t size;
} struct56_t;
typedef struct {
  obj_t cont;
  obj_t id;
  int_t inverted;
  int_t offset;
  obj_t rhs;
  int_t size;
} struct57_t;
typedef struct {
  obj_t acc;
  int_t any;
} struct59_t;
typedef struct {
  obj_t ann;
  obj_t opnd;
} struct61_t;
typedef struct {
  int_t factor;
  obj_t rhs;
} struct62_t;
typedef struct {
  obj_t lhs;
  obj_t rhs;
} struct63_t;
typedef struct {
  obj_t boundary;
  obj_t opnd;
} struct64_t;
typedef struct {
  string_t name;
  obj_t opnd;
} struct65_t;
typedef struct {
  int_t offset;
  int_t size;
} struct66_t;

struct state {
  void* userdata;      /* a pointer to arbitrary data */
  char* heap_base;    /* the beginning of the heap */
  char* heap_limit;   /* first byte beyond the heap buffer */
  char* heap;         /* current top of the heap */
  monad_t mon_state;      /* the current monadic state */
  unsigned char* ip_start;     /* beginning of code buffer */
  size_t ip_base;     /* base address of code */
  unsigned char* ip_limit;     /* first byte beyond the code buffer */
  unsigned char* ip;           /* current pointer into the buffer */
  int_t token_addr_inv;
  char* err_str;      /* a string describing the fatal error that occurred */
  jmp_buf err_tgt;    /* the position of the exception handler */
  FILE* handle;       /* the file that the puts primitve uses */
  char* const_heap_base;
  /* the following fields contain the values of constant GDSL expressions */
  obj_t bbtree_empty;
  obj_t fitree_empty;
  int_t int_max;
  vec_t endian_instr8_access8;
  obj_t varls_none;
  obj_t fmap_empty;
  obj_t optimization_config;
  obj_t registers_live_map;
  int_t config_default_opnd_sz_16;
  int_t config_mode32;
  int_t illegal_repne;
  int_t illegal_rep;
  int_t illegal_lock;
  int_t clmul;
  int_t avx;
  int_t aes;
  int_t illegal_lock_register;
  int_t xsaveopt;
  int_t ssse3;
  int_t sse3;
  int_t sse2;
  int_t sse;
  int_t rdrand;
  int_t mmx;
  int_t none;
  int_t f16c;
  int_t sse4_2;
  int_t sse4_1;
  int_t invpcid;
  int_t fsgsbase;
  obj_t substmap_initial;
  int_t isInverted;
  int_t isInverted_;
  obj_t decoder_config;
  int_t config_default;
  struct59_t first;
  obj_t asm_anns_none;
  obj_t asm_opnds_none;
  int_t gdsl_config_default_opnd_sz_16;
  int_t gdsl_config_mode32;
  int_t gdsl_config_default;
  obj_t gdsl_decoder_config;
  obj_t gdsl_optimization_config;
  int_t gdsl_int_max;
};

typedef unsigned int field_tag_t;

#define GEN_REC_STRUCT(type)  \
struct field_ ## type {       \
  field_tag_t tag;            \
  obj_t next;                 \
  unsigned int size;          \
  type ## _t payload;         \
};                            \
                              \
typedef struct field_ ## type  field_ ## type ## _t

#define GEN_ADD_FIELD(type)                               \
static obj_t add_field_ ## type                           \
(state_t s,field_tag_t tag, type ## _t v, obj_t rec) {    \
  field_ ## type ## _t* res = (field_ ## type ## _t*)     \
    alloc(s, sizeof(field_ ## type ## _t));               \
  res->tag = tag;                                         \
  res->size = sizeof(field_ ## type ## _t);               \
  res->next = rec;                                        \
  res->payload = v;                                       \
  return res;                                             \
}

#define GEN_SELECT_FIELD(type)                            \
static type ## _t select_  ## type                        \
(state_t s,field_tag_t field, obj_t rec) {                \
  field_ ## type ## _t* v = (field_ ## type ## _t*) rec;  \
  while (v) {                                             \
    if (v->tag==field) return v->payload;                 \
    v = (field_ ## type ## _t*) v->next;                                          \
  };                                                      \
  s->err_str = "GDSL runtime: field not found in record"; \
  longjmp(s->err_tgt,1);                                  \
}                                                         \

GEN_REC_STRUCT(obj);

#define CHUNK_SIZE (4*1024)

typedef unsigned int field_tag_t;

static void alloc_heap(state_t s, char* prev_page, size_t size) {
  if (size<CHUNK_SIZE) size = CHUNK_SIZE; else size = CHUNK_SIZE*((size/CHUNK_SIZE)+1);
  s->heap_base = (char*) malloc(size);
  if (s->heap_base==NULL) {
    s->err_str = "GDSL runtime: out of memory";
    longjmp(s->err_tgt,2);
  };
  s->heap = s->heap_base+sizeof(char*);
  /* store a pointer to the previous page in the first bytes of this page */
  *((char**) s->heap_base) = prev_page;
  s->heap_limit = s->heap_base+size;
};

static void* alloc(state_t s, size_t bytes) {
  bytes = ((bytes+7)>>3)<<3;    /* align to multiple of 8 */
  if (s->heap+bytes >= s->heap_limit) alloc_heap(s, s->heap_base, bytes);
  char* res = s->heap;
  s->heap = s->heap+bytes;
  return res;
};

static obj_t del_fields(state_t s, field_tag_t tags[], int tags_size, obj_t rec) {
  field_obj_t* current = (field_obj_t*) rec;
  int idx;
  obj_t res = NULL;
  obj_t* last_next = &res;
  while (current && tags_size>0) {
    for (idx=0; idx<tags_size; idx++)
      if (current->tag == tags[idx]) break;
    if (idx<tags_size) {
      /* delete this field by doing nothing, but remove the index */
      tags[idx]=tags[--tags_size];
    } else {
      /* this field is not supposed to be deleted, copy it */
      field_obj_t* copy = (field_obj_t*) alloc(s, current->size);
      memcpy(copy,current,current->size);
      *last_next = copy;
      last_next = &copy->next;
    };
    current = (field_obj_t*) current->next;
  };
  *last_next = current;
  return res;
}

int main() {
  state_t s;
  del_fields(s, NULL, 0, NULL);
}
