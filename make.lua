local lm = require "luamake"

lm.bindir = "."

lm:lua_dll "math3d"{
    sources = {
        "*.c",
        "*.cpp",
    },
    includes = {
        "./glm",
    },
    defines = {
        "_USE_MATH_DEFINES",
        "GLM_ENABLE_EXPERIMENTAL",
        "GLM_FORCE_QUAT_DATA_XYZW",
        "GLM_FORCE_INTRINSICS",
    },
    visibility = "default",
    cxx = "c++20",
}