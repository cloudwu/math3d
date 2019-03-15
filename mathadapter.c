#define LUA_LIB

#include <lua.h>
#include <lauxlib.h>
#include <stdint.h>
#include "math3d.h"
#include "linalg.h"

static const char * linear_type[] = {
	"MAT",
	"VEC4",
	"NUM",
	"QUAT",
	"EULER",
};

static const char *
linear_typename(int t) {
	if (t<0 || t>= LINEAR_TYPE_COUNT) {
		return "UNKNOWN";
	} else {
		return linear_type[t];
	}
}

static void
typemismatch(lua_State *L, int t1, int t2) {
	luaL_error(L, "Type mismatch, need %s, it's %s", linear_typename(t1), linear_typename(t2));
}

static void *
get_pointer(lua_State *L, struct lastack *LS, int index, int type) {
	int t;
	float *v;
	if (lua_isinteger(L, index)) {
		int64_t id = lua_tointeger(L, index);
		v = lastack_value(LS, id, &t);
	} else {
		struct refobject * ref = (struct refobject *)luaL_checkudata(L, index, LINALG_REF);
		if (ref->LS != NULL && ref->LS != LS) {
			luaL_error(L, "Math stack mismatch");
		}
		v = lastack_value(LS, ref->id, &t);
	}
	if (type != t) {
		typemismatch(L, type, t);
	}
	return v;
}

// upvalue1  mathstack
// upvalue2  cfunction
// 2 mathid or mathuserdata LINALG_REF
static int
lmatrix_adapter_1(lua_State *L) {
	struct boxstack *bp = lua_touserdata(L, lua_upvalueindex(1));
	lua_CFunction f = lua_tocfunction(L, lua_upvalueindex(2));
	void * v = get_pointer(L, bp->LS, 2, LINEAR_TYPE_MAT);
	lua_settop(L, 1);
	lua_pushlightuserdata(L, v);
	return f(L);
}

static int
lmatrix_adapter_2(lua_State *L) {
	struct boxstack *bp = lua_touserdata(L, lua_upvalueindex(1));
	lua_CFunction f = lua_tocfunction(L, lua_upvalueindex(2));
	void * v1 = get_pointer(L, bp->LS, 2, LINEAR_TYPE_MAT);
	void * v2 = get_pointer(L, bp->LS, 3, LINEAR_TYPE_MAT);
	lua_settop(L, 1);
	lua_pushlightuserdata(L, v1);
	lua_pushlightuserdata(L, v2);
	return f(L);
}

static int
lmatrix_adapter_var(lua_State *L) {
	struct boxstack *bp = lua_touserdata(L, lua_upvalueindex(1));
	lua_CFunction f = lua_tocfunction(L, lua_upvalueindex(2));
	int i;
	int top = lua_gettop(L);
	for (i=2;i<=top;i++) {
		void * v = get_pointer(L, bp->LS, i, LINEAR_TYPE_MAT);
		lua_pushlightuserdata(L, v);
		lua_replace(L, i);
	}
	return f(L);
}

// userdata mathstack
// cfunction original function
// integer n
// integer from 
static int
lbind_matrix(lua_State *L) {
	luaL_checkudata(L, 1, LINALG);
	if (!lua_iscfunction(L, 2))
		return luaL_error(L, "need a c function");
	if (lua_getupvalue(L, 2, 1) != NULL)
		luaL_error(L, "Only support light cfunction");
	int n = luaL_optinteger(L, 3, 0);
	int from = luaL_optinteger(L, 4, 2);
	if (from != 2) {
		luaL_error(L, "Only support adapter index 2 + now");
	}
	lua_CFunction f;
	switch (n) {
	case 0:
		f = lmatrix_adapter_var;
		break;
	case 1:
		f = lmatrix_adapter_1;
		break;
	case 2:
		f = lmatrix_adapter_2;
		break;
	default:
		return luaL_error(L, "Only support 1,2,0(vararg) now");
	}
	lua_settop(L, 2);
	lua_pushcclosure(L, f, 2);
	return 1;
}

LUAMOD_API int
luaopen_math3d_adapter(lua_State *L) {
	luaL_checkversion(L);
	luaL_Reg l[] = {
		{ "matrix", lbind_matrix },
		{ NULL, NULL },
	};

	luaL_newlib(L, l);

	return 1;
}
