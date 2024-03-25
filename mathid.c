#include "mathid.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <inttypes.h>

#define DEFAULT_MAX_PAGE 256
#define PAGE_SIZE 2048
#define UNMARK_SIZE 1024
#define INVALID_MARK_COUNT 255

struct page {
	float v[PAGE_SIZE][4];
};

struct marked_count {
	uint8_t count[PAGE_SIZE];
#ifdef MATHIDSOURCE
	const char * filename[PAGE_SIZE];
	int line[PAGE_SIZE];
#endif
};

struct marked_freelist {
	struct marked_freelist *next;
	int page;
	int size;
};

struct math_ref {
	const float * ptr;
	int size;
	int type;
};

struct math_unmarked {
	int n;
	int cap;
	int64_t *index;
	int64_t tmp[UNMARK_SIZE];
};

struct pages {
	struct page * constant;
	struct page * transient;
	struct page * marked;
	struct marked_count *count;
};

struct math_context {
	struct pages *p;
	struct math_unmarked unmarked;
	struct marked_freelist *freelist;
	int maxpage;
	int frame;
	int last_frame;
	int n;
	int base;
	int top;
	int marked_page;
	int marked_n;
	int marked_slot;
	int constant_n;
	int ref_n;
	uint32_t flags;
};

static inline int
transient_used(struct math_context *M, int n) {
	return (n >= M->base) ? n - M->base : n + M->maxpage * PAGE_SIZE - M->base;
}

int
math_info(struct math_context *M, int what) {
	switch (what) {
		case MATH_INFO_MAXPAGE:
			return M->maxpage;
		case MATH_INFO_FRAME:
			return M->frame;
		case MATH_INFO_TRANSIENT:
			return transient_used(M, M->n);
		case MATH_INFO_LAST :
			return (M->base >= M->top) ? M->base - M->top : M->maxpage * PAGE_SIZE - M->top + M->base;
		case MATH_INFO_MARKED:
			return M->marked_n;
		case MATH_INFO_CONSTANT:
			return M->constant_n;
		case MATH_INFO_REF:
			return M->ref_n;
		case MATH_INFO_SLOT:
			return M->marked_slot;
		default:
			return -1;
	}
}

void
math_refcount(struct math_context *M, int delta) {
	M->ref_n += delta;
}

static inline int
check_size(int size) {
	struct math_id s;
	s.size = ~0;
	return size > 0 && (size-1) <= s.size;
}

static void
math_unmarked_init(struct math_unmarked *u) {
	u->n = 0;
	u->cap = UNMARK_SIZE;
	u->index = u->tmp;
}

static void
math_unmarked_deinit(struct math_unmarked *u) {
	if (u->index != u->tmp) {
		free(u->index);
	}
}

static size_t
math_unmarked_size(struct math_unmarked *u) {
	if (u->index != u->tmp) {
		return u->cap * sizeof(uint32_t);
	}
	return 0;
}

struct math_context *
math_new(int maxpage) {
	struct math_context * m = (struct math_context *)malloc(sizeof(*m));
	if (maxpage <= 0)
		maxpage = DEFAULT_MAX_PAGE;
	m->maxpage = maxpage;
	m->frame = 0;
	m->last_frame = 0;
	m->n = 0;
	m->marked_page = 0;
	m->freelist = NULL;
	m->p = (struct pages *)malloc(sizeof(struct pages) * maxpage);
	memset(m->p, 0, sizeof(struct pages) * maxpage);
	m->marked_n = 0;
	m->marked_slot = 0;
	m->constant_n = 0;
	m->ref_n = 0;
	m->flags = 0;
	m->base = 0;
	m->top = 0;
	math_unmarked_init(&m->unmarked);
	return m;
}

void
math_set_flag(struct math_context *M, int flag_id, int v) {
	assert(flag_id >=0 && flag_id < 32);
	if (v) {
		M->flags |= 1 << flag_id;
	} else {
		M->flags &= ~(1 << flag_id);
	}
}

int
math_get_flag(struct math_context *M, int flag_id) {
	uint32_t mask = 1 << flag_id;
	return (M->flags & mask) != 0;
}

void
math_delete(struct math_context *M) {
	if (M == NULL)
		return;
	int i;
	int maxpage = M->maxpage;
	for (i=0;i<maxpage;i++) {
		if (M->p[i].constant == NULL) {
			break;
		}
		free(M->p[i].constant);
	}
	for (i=0;i<maxpage;i++) {
		free(M->p[i].transient);
	}
	for (i=0;i<maxpage;i++) {
		if (M->p[i].marked == NULL) {
			break;
		}
		free(M->p[i].marked);
	}
	for (i=0;i<maxpage;i++) {
		if (M->p[i].count == NULL) {
			break;
		}
		free(M->p[i].count);
	}
	math_unmarked_deinit(&M->unmarked);
	free(M->p);
	free(M);
}

size_t
math_memsize(struct math_context *M) {
	size_t sz = sizeof(*M);
	int i;
	int maxpage = M->maxpage;
	sz += sizeof(struct pages *) * maxpage;
	for (i=0;i<maxpage;i++) {
		if (M->p[i].constant == NULL) {
			break;
		}
		sz += sizeof(struct page);
	}
	for (i=0;i<maxpage;i++) {
		if (M->p[i].transient) {
			sz += sizeof(struct page);
		}
	}
	for (i=0;i<maxpage;i++) {
		if (M->p[i].marked == NULL) {
			break;
		}
		sz += sizeof(struct page);
	}
	for (i=0;i<maxpage;i++) {
		if (M->p[i].count == NULL) {
			break;
		}
		sz += sizeof(struct marked_count);
	}
	sz += math_unmarked_size(&M->unmarked);
	return sz;
}

static void
alloc_transient_page(struct math_context *M, int page_id) {
	int top_page = M->top / PAGE_SIZE;
	int i;
	int from_page = 0;
	if (page_id < top_page)
		from_page = page_id + 1;
	for (i = from_page; i < top_page; i++) {
		struct page * p = M->p[i].transient;
		if (p) {
			// reuse old transient page
			M->p[page_id].transient = p;
			M->p[i].transient = NULL;
			return;
		}
	}
	M->p[page_id].transient = (struct page *)malloc(sizeof(struct page));
}

static void *
allocvec(struct math_context *M, int size, int *index) {
	int n = M->n;
	int rewind = n < M->top;
	int page_id = n / PAGE_SIZE;
	int next_page_id = (n + size - 1) / PAGE_SIZE;
	int maxpage = M->maxpage;
	if (next_page_id != page_id) {
		page_id = next_page_id;
		n = page_id * PAGE_SIZE;
		assert(size <= PAGE_SIZE);
	}
	if (!rewind && page_id >= maxpage) {
		page_id = 0;
		n = 0;
		rewind = 1;
	}
	if (M->p[page_id].transient == NULL) {
		alloc_transient_page(M, page_id);
	}
	*index = n;
	M->n = n + size;
	if (rewind) {
		// transient pages overflow, too mant trabsient vectors
		assert(M->n < M->top);
	}
	return M->p[page_id].transient->v[*index % PAGE_SIZE];
}

static inline int
import(struct math_context *M, const float *v, int size) {
	int index;
	void * ptr = allocvec(M, size, &index);
	if (v) {
		memcpy(ptr, v, size * 4 * sizeof(float));
	}

	return index;
}

math_t
math_import(struct math_context *M, const float *v, int type, int size) {
	union {
		math_t id;
		struct math_id s;
	} u;
	assert(check_size(size));
	switch (type) {
	case MATH_TYPE_NULL:
		return MATH_NULL;
	case MATH_TYPE_MAT:
		u.s.index = import(M, v, 4 * size);
		break;
	case MATH_TYPE_VEC4:
	case MATH_TYPE_QUAT:
		u.s.index = import(M, v, 1 * size);
		break;
	default:
		assert(0);
	}
	u.s.frame = M->frame;
	u.s.type = type;
	u.s.size = size - 1;
	u.s.transient = 1;
	return u.id;
}

math_t
math_ref(struct math_context *M, const float *v, int type, int size) {
	union {
		math_t id;
		struct math_id s;
	} u;
	assert(check_size(size));
	int index;
	struct math_ref * r = (struct math_ref *)allocvec(M, 1, &index);
	r->ptr = v;
	r->size = size;
	assert(type == MATH_TYPE_MAT || type == MATH_TYPE_VEC4 || type == MATH_TYPE_QUAT);
	r->type = type;

	u.s.index = index;
	u.s.frame = M->frame;
	u.s.type = MATH_TYPE_REF;
	u.s.size = 0;
	u.s.transient = 1;
	return u.id;
}

float *
get_transient(struct math_context *M, int index) {
	int page_id = index / PAGE_SIZE;
	return M->p[page_id].transient->v[index % PAGE_SIZE];
}

float *
get_reference(struct math_context *M, int index, int offset) {
	struct math_ref * r = (struct math_ref *)get_transient(M, index);
	assert(offset >= 0 && offset < r->size);
	float * base = (float *)r->ptr;
	if (r->type == MATH_TYPE_MAT) {
		return base + 16 * offset;
	} else {
		return base + 4 * offset;
	}
}

int
math_ref_size_(struct math_context *M, struct math_id id) {
	if (id.size > 0)
		return 1;
	struct math_ref * r = (struct math_ref *)get_transient(M, id.index);
	return r->size;
}

int
math_ref_type_(struct math_context *M, struct math_id id) {
	struct math_ref * r = (struct math_ref *)get_transient(M, id.index);
	return r->type;
}

static int inline
frame_alive(struct math_context *M, int f) {
	return (M->frame == f) || (M->last_frame == f);
}

int
math_valid(struct math_context *M, math_t id) {
	union {
		math_t id;
		struct math_id s;
	} u;
	u.id = id;
	if (u.s.transient) {
		return frame_alive(M, u.s.frame);
	} else {
		if (u.s.frame == 0) {
			// constant
			return u.s.index <= M->constant_n;
		} else {
			// marked
			int page_id = u.s.index / PAGE_SIZE;
			return (page_id < M->marked_page);
		}
	}
}

int
math_marked(struct math_context *M, math_t id) {
	union {
		math_t id;
		struct math_id s;
	} u;
	u.id = id;
	if (u.s.transient || u.s.frame != 1) {
		return 0;
	}
	int index = u.s.index;
	int page_id = index / PAGE_SIZE;
	index %= PAGE_SIZE;
	if (page_id >= M->marked_page)
		return 0;
	return M->p[page_id].count->count[index] > 0;
}

#ifdef MATHIDSOURCE

int
math_marked_next(struct math_context *M, struct math_marked_iter *iter) {
	int index = iter->iter;
	for (;;) {
		int page_id = index / PAGE_SIZE;
		if (page_id >= M->marked_page)
			return 0;
		int page_index = index % PAGE_SIZE;
		int count = M->p[page_id].count->count[page_index];
		if (count != INVALID_MARK_COUNT) {
			iter->iter = index+1;
			iter->count = count;
			iter->filename = M->p[page_id].count->filename[page_index];
			iter->line = M->p[page_id].count->line[page_index];
			return 1;
		} else {
			++index;
		}
	}
}

#else

int
math_marked_next(struct math_context *M, struct math_marked_iter *iter) {
	return 0;
}

#endif

static float *
get_marked(struct math_context *M, int index) {
	int page_id = index / PAGE_SIZE;
	index %= PAGE_SIZE;
	assert (page_id < M->marked_page);
	return M->p[page_id].marked->v[index];
}

static inline const float *
get_constant(struct math_context *M, int index) {
	assert(index < M->constant_n);
	int page_id = index / PAGE_SIZE;
	index %= PAGE_SIZE;
	return M->p[page_id].constant->v[index];
}

math_t
math_index(struct math_context *M, math_t id, int index) {
	union {
		math_t id;
		struct math_id s;
	} u;
	u.id = id;
	int size = math_size(M, id);
	assert(index < size);
	if (u.s.type == MATH_TYPE_REF) {
		assert(u.s.size == 0);
		u.s.size = index + 1;
		return u.id;
	} else if (!u.s.transient && u.s.frame > 0) {
		// marked
		u.s.size = 0;
		u.s.frame = 2 + index;
	} else {
		// transient or constant
		u.s.size = 0;
		if (u.s.type == MATH_TYPE_MAT)
			index *= 4;
		u.s.index += index;
	}
	return u.id;
}

static inline const float *
get_identity(int type) {
	static const float imat[16] = {
		1,0,0,0,
		0,1,0,0,
		0,0,1,0,
		0,0,0,1,
	};
	static const float ivec[4] = { 0, 0, 0, 1 };
	switch (type) {
	case MATH_TYPE_MAT:
		return imat;
	case MATH_TYPE_VEC4:
	case MATH_TYPE_QUAT:
		return ivec;
	default:
		return NULL;
	}
}

static inline int
marked_index(struct math_id s) {
	int index = s.index;
	if (s.frame > 1) {
		// indexed array
		int offset = s.frame - 2;
		if (offset && s.type == MATH_TYPE_MAT) {
			offset *= 4;
		}
		return index + offset;
	} else {
		return index;
	}
}

const float *
math_value(struct math_context *M, math_t id) {
	union {
		math_t id;
		struct math_id s;
	} u;
	u.id = id;
	if (u.s.transient) {
		assert(frame_alive(M, u.s.frame));
		if (u.s.type == MATH_TYPE_REF) {
			int index = u.s.size;
			if (index == 0)
				index = 1;
			return get_reference(M, u.s.index, index - 1);
		}
		return get_transient(M, u.s.index);
	} else {
		if (u.s.frame) {
			return get_marked(M, marked_index(u.s));
		} else {
			if (u.s.index == 0) {
				return get_identity(u.s.type);
			} else {
				return get_constant(M, u.s.index - 1);
			}
		}
	}
}

float *
math_init(struct math_context *M, math_t id) {
	union {
		math_t id;
		struct math_id s;
	} u;
	u.id = id;
	assert (u.s.transient || u.s.frame == 1);
	return (float *)math_value(M, id);
}

static struct marked_freelist *
new_marked_page(struct math_context *M) {
	int maxpage = M->maxpage;
	assert (M->marked_page < maxpage);
	int page = M->marked_page++;
	if (M->marked_page < maxpage) {
		M->p[M->marked_page].marked = NULL;
		M->p[M->marked_page].count = NULL;
	}
	assert(M->p[page].marked == NULL);
	M->p[page].marked = (struct page *)malloc(sizeof(struct page));
	assert(M->p[page].count == NULL);
	M->p[page].count = (struct marked_count *)malloc(sizeof(struct marked_count));
	memset(M->p[page].count, INVALID_MARK_COUNT, sizeof(struct marked_count));

	struct marked_freelist *node = (struct marked_freelist *)M->p[page].marked;
	node->next = NULL;
	node->page = page;
	node->size = PAGE_SIZE;
	return node;
}

static int
alloc_vecarray(struct math_context *M, int vecsize) {
	struct marked_freelist **prev = &M->freelist;
	float * mem = NULL;
	int page_id = 0;
	for (;;) {
		struct marked_freelist *node = *prev;
		if (node == NULL) {
			node = new_marked_page(M);
			*prev = node;
		}
		if (node->size < vecsize) {
			prev = &node->next;
			node = node->next;
		} else if (node->size == vecsize) {
			// use this node
			*prev = node->next;
			mem = (float *)node;
			page_id = node->page;
			break;
		} else {
			// split this node
			assert(sizeof(*node) <= sizeof(float) * 4);
			mem = (float *)node + (node->size - vecsize) * 4;
			node->size -= vecsize;
			page_id = node->page;
			break;
		}
	}
	struct page *p = M->p[page_id].marked;
	int index = (int)((mem - &p->v[0][0]) / 4);
	return index + page_id * PAGE_SIZE;
}

static void
prepare_constant_page(struct math_context *M, int page) {
	int maxpage = M->maxpage;
	assert(page < maxpage);
	if (M->p[page].constant == NULL) {
		M->p[page].constant = (struct page *)malloc(sizeof(struct page));
		if (page + 1 < maxpage) {
			M->p[page+1].constant = NULL;
		}
	}
}

static int
alloc_constant(struct math_context *M, const float *v, int n) {
	assert(n <= PAGE_SIZE);
	// search v first
	int i;
	for (i=0;i<=M->constant_n - n;i++) {
		const float * vv = get_constant(M, i);
		if (memcmp(v, vv, n * 4 * sizeof(float))== 0)
			return i;
	}

	int page_id = M->constant_n / PAGE_SIZE;
	int index = M->constant_n % PAGE_SIZE;
	prepare_constant_page(M, page_id);
	if (index + n > PAGE_SIZE) {
		page_id++;
		index = 0;
		prepare_constant_page(M, page_id);
		M->constant_n = page_id * PAGE_SIZE + n;
	} else {
		M->constant_n += n;
	}
	memcpy(M->p[page_id].constant->v[index], v, n * 4 * sizeof(float));

	return M->constant_n - n;
}

static int
is_identity(struct math_context *M, math_t v) {
	int type = math_type(M, v);
	const float *ptr = math_value(M, v);
	const float *iptr = math_value(M, math_identity(type));
	switch (type) {
	case MATH_TYPE_MAT:
		return memcmp(ptr, iptr, 16 * sizeof(float)) == 0;
	case MATH_TYPE_VEC4:
	case MATH_TYPE_QUAT:
		return memcmp(ptr, iptr, 4 * sizeof(float)) == 0;
	}
	return 0;
}

math_t
math_constant(struct math_context *M, math_t v) {
	if (math_isconstant(v))
		return v;
	int sz = math_size(M, v);
	int type = math_type(M, v);
	if (sz == 1 && is_identity(M, v)) {
		return math_identity(type);
	}
	int offset;
	const float * ptr = math_value(M, v);
	switch (type) {
	case MATH_TYPE_MAT:
		offset = alloc_constant(M, ptr, sz * 4);
		break;
	case MATH_TYPE_VEC4:
	case MATH_TYPE_QUAT:
		offset = alloc_constant(M, ptr, sz);
		break;
	default:
		assert(0);
		return MATH_NULL;
	}
	union {
		math_t id;
		struct math_id s;
	} u;
	u.s.index = offset + 1;
	u.s.size = sz - 1;
	u.s.frame = 0;
	u.s.type = type;
	u.s.transient = 0;
	return u.id;
}

static math_t
alloc_marked(struct math_context *M, const float *v, int type, int size, const char *filename, int line) {
	union {
		math_t id;
		struct math_id s;
	} u;
	
	int vecsize = size;
	if (type == MATH_TYPE_MAT)
		vecsize *= 4;

	M->marked_slot += vecsize;

	int index = alloc_vecarray(M, vecsize);

	u.s.index = index;
	u.s.size = size - 1;
	u.s.frame = 1;
	u.s.type = type;
	u.s.transient = 0;

	if (v) {
		float * ptr = get_marked(M, index);
		memcpy(ptr, v, vecsize * 4 * sizeof(float));
	}

	int page_id = index / PAGE_SIZE;
	index %= PAGE_SIZE;
	M->p[page_id].count->count[index] = 1;
#ifdef MATHIDSOURCE
	M->p[page_id].count->filename[index] = (filename != NULL) ? filename : "(null)";
	M->p[page_id].count->line[index] = line;
#endif
	return u.id;
}

static math_t
get_marked_id(struct math_context *M, math_t id, const char *filename, int line) {
	union {
		math_t id;
		struct math_id s;
	} u;
	u.id = id;
	int index = u.s.index;
	int page_id = index / PAGE_SIZE;
	index %= PAGE_SIZE;
	assert (page_id < M->marked_page);
	int count = M->p[page_id].count->count[index];
	if (count >= (INVALID_MARK_COUNT-1)) {
		assert(count != INVALID_MARK_COUNT);	// unmarked id
		const float *v = math_value(M, id);
		int size = math_size(M, id);
		return alloc_marked(M, v, u.s.type, size, filename, line);
	} else {
		// add reference count
		++M->p[page_id].count->count[index];
		return id;
	}
}

math_t
math_mark_(struct math_context *M, math_t id, const char *filename, int line) {
	union {
		math_t id;
		struct math_id s;
	} u;
	u.id = id;
	if (u.s.transient || u.s.frame > 1) {
		const float *v = math_value(M, id);
		int size = math_size(M, id);
		int type = math_type(M, id);
		M->marked_n++;
		return alloc_marked(M, v, type, size, filename, line);
	}
	if (u.s.frame == 0) {
		// constant value
		return id;
	}
	M->marked_n++;
	return get_marked_id(M, id, filename, line);
}

math_t
math_clone_(struct math_context *M, math_t id, const char *filename, int line) {
	if (math_isconstant(id))
		return id;

	const float *v = math_value(M, id);
	int size = math_size(M, id);
	int type = math_type(M, id);
	M->marked_n++;
	return alloc_marked(M, v, type, size, filename, line);
}

static inline int64_t
math_unmark_handle_(struct math_id id) {
	int size  = id.size + 1;
	if (id.type == MATH_TYPE_MAT)
		size *= 4;

	return ((int64_t)id.index << 32) | size;
}

static inline int
math_unmark_index_(int64_t handle, int *size) {
	*size = handle & 0xffffffff;

	return (int)(handle >> 32);
}

static inline void
math_unmarked_insert_(struct math_unmarked *u, int64_t idx) {
	if (u->n >= u->cap) {
		int newcap = u->cap * 3 / 2;
		int64_t *newindex = (int64_t *)malloc(newcap * sizeof(int64_t));
		memcpy(newindex, u->index, u->n * sizeof(int64_t));
		if (u->index != u->tmp) {
			free(u->index);
		}
		u->index = newindex;
		u->cap = newcap;
	}
	u->index[u->n++] = idx;
}

static void
math_unmarked_insert(struct math_unmarked *u, struct math_id id) {
	math_unmarked_insert_(u, math_unmark_handle_(id));
}

int
math_unmark(struct math_context *M, math_t id) {
	union {
		math_t id;
		struct math_id s;
	} u;
	u.id = id;
	if (u.s.frame == 0)
		return 0;
	if (u.s.transient != 0 || u.s.frame != 1) {
		return -1;
	}
	int index = u.s.index;
	int page_id = index / PAGE_SIZE;
	index %= PAGE_SIZE;
	assert(page_id < M->marked_page);
	int vecsize = u.s.size + 1;
	M->marked_n--;
	if (u.s.type == MATH_TYPE_MAT) {
		vecsize *= 4;
	}
	assert(vecsize + index <= PAGE_SIZE);
	uint8_t * count = &M->p[page_id].count->count[index];
	int c = *count;
	if (c <= 1 || c == INVALID_MARK_COUNT) {
		if (c == 1) {
			// The last reference
			math_unmarked_insert(&M->unmarked, u.s);
		} else {
			return -1;
		}
	}
	*count = c - 1;
	return c;
}

math_t
math_premark(struct math_context *M, int type, int size) {
	union {
		math_t id;
		struct math_id s;
	} u;
	u.id = alloc_marked(M, NULL, type, size, "PREMARK", 0);

	int index = u.s.index;
	int page_id = index / PAGE_SIZE;
	index %= PAGE_SIZE;
	assert(page_id < M->marked_page);

	int vecsize = u.s.size + 1;
	if (u.s.type == MATH_TYPE_MAT) {
		vecsize *= 4;
	}
	assert(vecsize + index <= PAGE_SIZE);
	M->p[page_id].count->count[index] = 0;
	math_unmarked_insert(&M->unmarked, u.s);
	return u.id;
}

static int
int64_compr(const void *a, const void *b) {
	const int64_t * aa = (const int64_t *)a;
	const int64_t * bb = (const int64_t *)b;
	return (int)((*aa >> 32) - (*bb >> 32));
}

static int64_t *
block_size(int64_t *ptr, int64_t *endptr, int *r_index, int *r_size) {
	int size;
	int index = math_unmark_index_(*ptr, &size);
	*r_index = index;
	*r_size = size;
	for (;;) {
		++ptr;
		if (ptr >= endptr)
			return ptr;
		int next_size;
		int next_index = math_unmark_index_(*ptr, &next_size);
		if (index + size == next_index && (index / PAGE_SIZE == next_index / PAGE_SIZE)) {
			// continuous block and at the same page
			*r_size += next_size;
			size = next_size;
			index = next_index;
		} else {
			return ptr;
		}
	}
}

static void
merge_freelist(struct math_context *M, struct math_unmarked *unmarked, const struct math_unmarked *u, struct marked_freelist *freelist) {
	int i = 0;
	while (freelist) {
		int page_id = freelist->page;
		int index = ((float *)freelist - &M->p[page_id].marked->v[0][0]) / 4;
		index += page_id * PAGE_SIZE;
		int64_t id = (int64_t)index << 32 | freelist->size;
		while ( i < u->n && u->index[i] < id) {
			math_unmarked_insert_(unmarked, u->index[i]);
			++i;
		}
		math_unmarked_insert_(unmarked, id);
		freelist = freelist->next;
	}
	while ( i < u->n ) {
		math_unmarked_insert_(unmarked, u->index[i]);
		++i;
	}
}

static inline void
dump_unmarked(struct math_unmarked *unmarked) {
	int i;
	for (i=0;i<unmarked->n;i++) {
		uint64_t v = unmarked->index[i];
		printf("(%d/%d)", (int)(v>>32), (int)(v & 0xffffffff));
	}
	printf("\n");
}

static void
free_unmarked(struct math_context *M) {
	int n = M->unmarked.n;
	if (n == 0)
		return;
	qsort(M->unmarked.index, n, sizeof(int64_t), int64_compr);

	// remove alive and dup index
	int i;
	int p = 0;
	int sz;
	int last = math_unmark_index_(M->unmarked.index[0], &sz);
	int page_id = last / PAGE_SIZE;
	uint8_t *count = &M->p[page_id].count->count[last % PAGE_SIZE];
	if (*count == 0) {
		*count = INVALID_MARK_COUNT;
		++p;
		M->marked_slot -= sz;
	}
	for (i=1;i<n;i++) {
		int current = math_unmark_index_(M->unmarked.index[i], &sz);
		if (current != last) {
			M->marked_slot -= sz;
			last = current;
			page_id = current / PAGE_SIZE;
			uint8_t *count = &M->p[page_id].count->count[current % PAGE_SIZE];
			if (*count == 0) {
				*count = INVALID_MARK_COUNT;
				M->unmarked.index[p++] = M->unmarked.index[i];
			}
		}
	}

	M->unmarked.n = p;

	if (p == 0)
		return;

	struct math_unmarked tmp;
	struct math_unmarked *unmarked;
	math_unmarked_init(&tmp);
	if (M->freelist) {
		unmarked = &tmp;
		merge_freelist(M, unmarked, &M->unmarked, M->freelist);
	} else {
		unmarked = &M->unmarked;
	}

//	dump_unmarked(unmarked);

	int64_t *ptr = unmarked->index;
	int64_t *endptr = unmarked->index + unmarked->n;

	struct marked_freelist **list_next = &M->freelist;

	while (ptr < endptr) {
		int index;
		int sz;
		ptr = block_size(ptr, endptr, &index, &sz);
		struct marked_freelist * node = (struct marked_freelist *)get_marked(M, index);
		*list_next = node;
		list_next = &node->next;
		node->size = sz;
		node->page = index / PAGE_SIZE;
	}
	*list_next = NULL;
	math_unmarked_deinit(&tmp);

	M->unmarked.n = 0;
}

static inline int
check_freelist(struct math_context *M) {
	struct marked_freelist *list = M->freelist;
	while (list) {
		int pageid = list->page;
		if (pageid < 0 || pageid >= M->marked_page) {
			return 0;
		}
		struct page * p = M->p[pageid].marked;
		if (p == NULL)
			return 0;
		uintptr_t begin = (uintptr_t)p;
		uintptr_t end = (uintptr_t)(p+1);
		uintptr_t address = (uintptr_t)list;
		if (address < begin || address >= end)
			return 0;
		list = list->next;
	}
	return 1;
}

void
math_frame(struct math_context *M) {
	union {
		math_t id;
		struct math_id s;
	} u;
	memset(&u, 0xff, sizeof(u));
	M->last_frame = M->frame;
	++M->frame;
	if (M->frame > u.s.frame) {
		M->frame = 0;
	}
	int i;
	for (i=(M->n / PAGE_SIZE) + 1; i < M->maxpage; i ++) {
		if (M->p[i].transient == NULL)
			break;
		free(M->p[i].transient);
		M->p[i].transient = NULL;
	}
	free_unmarked(M);
	M->top = M->base;
	M->base = M->n;
//	assert(check_freelist(M));
}

int
math_checkpoint(struct math_context *M) {
	return M->n;
}

void
math_recover(struct math_context *M, int cp) {
	assert(transient_used(M, M->n) >= transient_used(M, cp));
	M->n = cp;
}

math_t
math_live(struct math_context *ctx, math_t id) {
	union {
		math_t id;
		struct math_id s;
	} u;
	u.id = id;
	if (u.s.transient) {
		int f = u.s.frame;
		if (ctx->frame == f)
			return id;
	} else if (u.s.frame == 0) {
		// constant
		return id;
	}
	return math_import(ctx, math_value(ctx, id), math_type(ctx, id), math_size(ctx, id));
}

const char *
math_typename(int t) {
	static const char * type_names[] = {
		"null",
		"mat",
		"v4",
		"quat",
	};
	if (t < 0 || t >= sizeof(type_names)/sizeof(type_names[0]))
		return "unknown";
	return type_names[t];
}

#include <stdio.h>

void
math_print(struct math_context *M, math_t id) {
	if (!math_valid(M, id)) {
		printf("[INVALID (%" PRIx64 "]\n", id.idx);
		return;
	}
	const float * v = math_value(M, id);
	int type = math_type(M, id);
	int size = math_size(M, id);
	int n = 0;
	union {
		math_t id;
		struct math_id s;
	} u;
	u.id = id;
	switch (type) {
	case MATH_TYPE_NULL:
		printf("[NULL]\n");
		return;
	case MATH_TYPE_MAT:
		printf("[MAT (%" PRIx64 , id.idx);
		n = 16;
		break;
	case MATH_TYPE_VEC4:
		printf("[VEC4 (%" PRIx64 , id.idx);
		n = 4;
		break;
	case MATH_TYPE_QUAT:
		printf("[QUAT (%" PRIx64 , id.idx);
		n = 4;
		break;
	default:
		printf("[INVALID (%" PRIx64 "]\n", id.idx);
		return;
	}
	if (u.s.transient) {
		printf(") :");
	} else {
		if (u.s.frame > 1) {
			int offset = u.s.frame - 2;
			printf("<%d/?>) :", offset);
		} else {
			int index = u.s.index;
			int page = index / PAGE_SIZE;
			index %= PAGE_SIZE;
			int c = M->p[page].count->count[index];
			printf("/%d) :", c);
		}
	}
	if (size == 1 || u.s.type == MATH_TYPE_REF) {
		int i;
		if ( u.s.type == MATH_TYPE_REF ) {
			printf(" <%d/%d>", u.s.size, size);
		}
		for (i=0;i<n;i++) {
			printf(" %g", v[i]);
		}
	} else {
		int i,j;
		for (i=0;i<size;i++) {
			printf(" <%d/%d>", i, size);
			for (j=0;j<n;j++) {
				printf(" %g", *v);
				++v;
			}
		}
	}
	printf("]\n");
}

#ifdef TEST_MATHID

int
main() {
	struct math_context *M = math_new(0);
	float v[4] = { 1,2,3,4 };
	float array[3][4] = {
		{ 1, 0, 0, 0 },
		{ 2, 0, 0, 0 },
		{ 3, 0, 0, 0 },
	};
	float stack[2][16] = {
		{ 1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16},
		{ 15,14,13,12,11,10,9,8,7,6,5,4,3,2,1},
	};
	math_t p[8];
	p[0] = math_vec4(M, v);
	p[1] = math_import(M, NULL, MATH_TYPE_QUAT, 3);
	float *buf = math_init(M, p[1]);
	memcpy(buf, &array[0][0], sizeof(array));
	p[2] = math_index(M, p[1], 2);
	p[3] = math_ref(M, &stack[0][0], MATH_TYPE_MAT, 2);
	p[4] = math_index(M, p[3], 1);
	p[5] = math_mark(M, p[0]);

	math_unmark(M, p[5]);
	math_mark(M, p[5]);	// relive
	p[6] = math_mark(M, p[3]);
	p[7] = MATH_NULL;

	int i;
	for (i=0;i<8;i++) {
		printf("%d:", i);
		math_print(M, p[i]);
	}

	for (i=0;i<8;i++) {
		printf("%d : %s %s\n", i, math_valid(M, p[i]) ? "valid" : "invalid", math_marked(M, p[i]) ? "marked" : "");
	}

	math_frame(M);

	p[7] = math_index(M, p[6], 1);
	p[7] = math_mark(M, p[7]);

	math_unmark(M, p[5]);

	for (i=0;i<8;i++) {
		printf("%d : %s %s ", i, math_valid(M, p[i]) ? "valid" : "invalid", math_marked(M, p[i]) ? "marked" : "");
		math_print(M, p[i]);
	}

	math_unmark(M, p[6]);

	math_frame(M);


	printf("mem : %d\n", (int)math_memsize(M));
	printf("NULL : ");
	math_print(M, math_identity(MATH_TYPE_NULL));
	printf("IMAT : ");
	math_print(M, math_identity(MATH_TYPE_MAT));
	printf("IVEC : ");
	math_print(M, math_identity(MATH_TYPE_VEC4));

	math_t id = math_premark(M, MATH_TYPE_VEC4, 1);
	float * t = math_init(M, id);
	t[0] = 42;
	t[1] = 0;
	t[2] = 0;
	t[3] = 0;

	math_mark(M, id);

	math_frame(M);

	math_print(M, id);

	math_delete(M);
	return 0;
}

#endif
