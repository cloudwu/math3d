#ifndef MATH_ID_H
#define MATH_ID_H

// #define MATHIDSOURCE

#include <stdint.h>
#include <stddef.h>

struct math_context;

typedef struct { uint64_t idx; } math_t;
static const math_t MATH_NULL = { 0 };

#define MATH_TYPE_NULL 0
#define MATH_TYPE_MAT 1
#define MATH_TYPE_VEC4 2
#define MATH_TYPE_QUAT 3
#define MATH_TYPE_REF 4
#define MATH_TYPE_COUNT 5

#define MATH_INFO_MAXPAGE 0
#define MATH_INFO_FRAME 1
#define MATH_INFO_TRANSIENT 2
#define MATH_INFO_MARKED 3
#define MATH_INFO_LAST 4
#define MATH_INFO_CONSTANT 5
#define MATH_INFO_REF 6
#define MATH_INFO_SLOT 7

struct math_context * math_new(int maxpage);
void math_delete(struct math_context *);
int math_info(struct math_context *, int what);
void math_set_flag(struct math_context *, int flag_id, int v);
int math_get_flag(struct math_context *, int flag_id);
size_t math_memsize(struct math_context *);
void math_frame(struct math_context *);
int math_checkpoint(struct math_context *);
void math_recover(struct math_context *, int cp);
math_t math_import(struct math_context *, const float *v, int type, int size);
math_t math_ref(struct math_context *, const float *v, int type, int size);
math_t math_premark(struct math_context *, int type, int size);
math_t math_mark_(struct math_context *, math_t id, const char *filename, int line);
#define math_mark(M, id) math_mark_(M, id, __FILE__, __LINE__)
math_t math_clone_(struct math_context *, math_t id, const char *filename, int line);
#define math_clone(M, id) math_clone_(M, id, __FILE__, __LINE__)

struct math_marked_iter {
	int iter;
	short count;
	short line;
	const char *filename;
};

int math_marked_next(struct math_context *, struct math_marked_iter *iter);

int math_unmark(struct math_context *, math_t id);
const float * math_value(struct math_context *, math_t id);
float *math_init(struct math_context *, math_t id);
math_t math_index(struct math_context *, math_t id, int index);
int math_valid(struct math_context *, math_t id);
int math_marked(struct math_context *, math_t id);
void math_print(struct math_context *, math_t id);	// for debug only
const char * math_typename(int type);
math_t math_constant(struct math_context *, math_t);
math_t math_live(struct math_context *, math_t id);
void math_refcount(struct math_context *, int delta);

static inline int
math_issame(math_t id1, math_t id2) {
	return id1.idx == id2.idx;
}

static inline int
math_isnull(math_t id) {
	return id.idx == MATH_NULL.idx;
}

static inline math_t
math_matrix(struct math_context *ctx, const float *v) {
	return math_import(ctx, v, MATH_TYPE_MAT, 1);
}

static inline math_t
math_vec4(struct math_context *ctx, const float *v) {
	return math_import(ctx, v, MATH_TYPE_VEC4, 1);
}

static inline math_t
math_quat(struct math_context *ctx, const float *v) {
	return math_import(ctx, v, MATH_TYPE_QUAT, 1);
}

struct math_id {
	uint32_t frame		: 20;
	uint32_t size		: 12;	// array size - 1 (0 : single object), for ref type, it's index
	uint32_t index		: 28;
	uint32_t type		: 3;
	uint32_t transient	: 1;	// 0: persisent
};

static inline math_t
math_identity(int type) {
	union {
		math_t id;
		struct math_id s;
	} u;
	u.id.idx = 0;
	u.s.type = type;
	return u.id;
}

static inline int
math_isidentity(math_t id) {
	union {
		math_t id;
		struct math_id s;
	} u;
	u.id = id;
	return (!u.s.transient && u.s.frame == 0 && u.s.index == 0);
}

static inline int
math_isconstant(math_t id) {
	union {
		math_t id;
		struct math_id s;
	} u;
	u.id = id;
	return (!u.s.transient && u.s.frame == 0);
}

static inline int
math_isref(struct math_context *ctx, math_t id) {
	union {
		math_t id;
		struct math_id s;
	} u;
	u.id = id;
	return (u.s.type == MATH_TYPE_REF);
}

int math_ref_size_(struct math_context *, struct math_id id);
int math_ref_type_(struct math_context *, struct math_id id);

static inline int
math_type(struct math_context *ctx, math_t id) {
	union {
		math_t id;
		struct math_id s;
	} u;
	u.id = id;
	if (u.s.type != MATH_TYPE_REF)
		return u.s.type;
	return math_ref_type_(ctx, u.s);
}

static inline int
math_size(struct math_context *ctx, math_t id) {
	union {
		math_t id;
		struct math_id s;
	} u;
	u.id = id;
	if (u.s.type != MATH_TYPE_REF)
		return u.s.size + 1;
	else {
		return math_ref_size_(ctx, u.s);
	}
}

#endif
