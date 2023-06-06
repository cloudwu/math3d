#ifndef math3d_lua_binding_h
#define math3d_lua_binding_h

#include "mathid.h"
#include <lua.h>

struct math3d_api {
	struct math_context * M;
	const void * refmeta;
	math_t (*from_lua)(lua_State *L, struct math_context *MC, int index, int type);
	math_t (*from_lua_id)(lua_State *L, struct math_context *MC, int index);
};

// binding functions

static inline math_t
math3d_from_lua(lua_State *L, struct math3d_api *M, int index, int type) {
	return M->from_lua(L, M->M, index, type);
}

static inline math_t
math3d_from_lua_id(lua_State *L, struct math3d_api *M, int index) {
	return M->from_lua_id(L, M->M, index);
}

#endif
