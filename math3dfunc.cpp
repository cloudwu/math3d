#define LUA_LIB

#include <cmath>
#include <cassert>
#include <cstring>
#include <utility>

extern "C" {
	#include "mathid.h"
	#include "math3dfunc.h"
}

#ifndef M_PI
#define M_PI 3.1415926536
#endif

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/ext/scalar_relational.hpp>
#include <glm/ext/vector_relational.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/ext/vector_common.hpp>
#include <glm/gtx/matrix_decompose.hpp>

static const glm::vec4 XAXIS(1, 0, 0, 0);
static const glm::vec4 YAXIS(0, 1, 0, 0);
static const glm::vec4 ZAXIS(0, 0, 1, 0);
static const glm::vec4 WAXIS(0, 0, 0, 1);

static const glm::vec4 NXAXIS = -XAXIS;
static const glm::vec4 NYAXIS = -YAXIS;
static const glm::vec4 NZAXIS = -ZAXIS;

template<typename T>
inline bool
is_zero(const T& a, const T& e = T(glm::epsilon<float>())) {
	return glm::all(glm::equal(a, glm::zero<T>(), e));
}

inline bool
is_zero(const float& a, float e = glm::epsilon<float>()) {
	return glm::equal(a, glm::zero<float>(), e);
}

template<typename T>
inline bool
is_equal(const T& a, const T& b, const T& e = T(glm::epsilon<float>())) {
	return is_zero(a - b, e);
}

static inline void
check_type(struct math_context *M, math_t id, int type) {
	assert(math_type(M, id) == type);
}

static inline glm::mat4x4 &
allocmat(struct math_context *M, math_t *id) {
	*id = math_matrix(M, NULL);
	float * buf = math_init(M, *id);
	return *(glm::mat4x4 *)buf;
}

// static inline glm::mat4x4 &
// initmat(struct math_context *M, math_t id) {
// 	check_type(M, id, MATH_TYPE_MAT);
// 	float * buf = math_init(M, id);
// 	return *(glm::mat4x4 *)buf;
// }

static inline glm::quat &
allocquat(struct math_context *M, math_t *id) {
	*id = math_quat(M, NULL);
	float *buf = math_init(M, *id);
	return *(glm::quat *)buf;
}

static inline glm::vec4 &
allocvec4(struct math_context *M, math_t *id) {
	*id = math_vec4(M, NULL);
	float * buf = math_init(M, *id);
	return *(glm::vec4 *)buf;
}

static inline glm::vec4 &
initvec4(struct math_context *M, math_t id) {
	check_type(M, id, MATH_TYPE_VEC4);
	float * buf = math_init(M, id);
	return *(glm::vec4 *)buf;
}

static inline const glm::quat &
QUAT(struct math_context *M, math_t quat) {
	check_type(M, quat, MATH_TYPE_QUAT);
	const float * v = math_value(M, quat);
	return *(const glm::quat *)(v);
}

static inline const glm::mat4x4 &
MAT(struct math_context *M, math_t mat) {
	check_type(M, mat, MATH_TYPE_MAT);
	const float *v = math_value(M, mat);
	return *(const glm::mat4x4 *)(v);
}

static inline const glm::vec4 &
VEC(struct math_context *M, math_t v4) {
	check_type(M, v4, MATH_TYPE_VEC4);
	const float *v = math_value(M, v4);
	return *(const glm::vec4 *)(v);
}

static inline const glm::vec4 &
VECPTR(const float *v) {
	return *(const glm::vec4 *)(v);
}

static inline const glm::vec3 &
VEC3(struct math_context *M, math_t v3) {
	check_type(M, v3, MATH_TYPE_VEC4);
	const float *v = math_value(M, v3);
	return *(const glm::vec3 *)(v);
}

static inline const glm::vec3* V3P(const glm::vec4 &v4) { return (const glm::vec3*)(&v4.x);}
static inline const glm::vec3& V3R(const glm::vec4 &v4) { return *V3P(v4);}

struct AABB {
	const glm::vec4 &minv;
	const glm::vec4 &maxv;
};

static inline struct AABB
AABB(struct math_context *M, math_t aabb) {
	check_type(M, aabb, MATH_TYPE_VEC4);
	const float *v = math_value(M, aabb);
	return {
		*(const glm::vec4 *)(v),
		*(const glm::vec4 *)(v+4)
	};
}

struct AABB_buffer {
	glm::vec4 &minv;
	glm::vec4 &maxv;
};

static inline struct AABB_buffer
initaabb(struct math_context *M, math_t id) {
	check_type(M, id, MATH_TYPE_VEC4);
	float * buf = math_init(M, id);
	return {
		*(glm::vec4 *)buf,
		*(glm::vec4 *)(buf+4),
	};
}

math_t
math3d_matrix_from_cols(struct math_context* M, math_t c1, math_t c2, math_t c3, math_t c4){
	math_t r;
	glm::mat4x4 &m = allocmat(M, &r);
	m[0] = VEC(M, c1);
	m[1] = VEC(M, c2);
	m[2] = VEC(M, c3);
	m[3] = VEC(M, c4);
	return r;
}

math_t
math3d_quat_to_matrix(struct math_context *M, math_t quat) {
	math_t r;
	glm::mat4x4 &m = allocmat(M, &r);
	m = glm::mat4x4(QUAT(M, quat));
	return r;
}

math_t
math3d_matrix_to_quat(struct math_context *M, math_t mat) {
	math_t r;
	glm::quat &q = allocquat(M, &r);
	q = glm::quat_cast(MAT(M, mat));
	return r;
}

math_t
math3d_make_quat_from_axis(struct math_context *M, math_t axis_id, float radian) {
	math_t r;
	const float *axis = math_value(M, axis_id);
	glm::vec3 a(axis[0],axis[1],axis[2]);
	glm::quat &q = allocquat(M, &r);

	q = glm::angleAxis(radian, a);

	return r;
}

math_t
math3d_quat_between_2vectors(struct math_context *M, math_t a, math_t b) {
	math_t r;
	glm::quat &q = allocquat(M, &r);
	q = glm::quat(VEC3(M, a), VEC3(M, b));
	return r;
}

math_t
math3d_make_quat_from_euler(struct math_context *M, math_t euler) {
	glm::quat q(VEC3(M, euler));
	return math_quat(M, &q[0]);
}

static int
scale1(struct math_context *M, math_t s) {
	const float * v = math_value(M, s);
	return v[0] == 1 && v[1] == 1 && v[2] == 1;
}

static int
rot0(struct math_context *M, math_t r) {
	const float * v = math_value(M, r);
	return v[0] == 0 && v[1] == 0 && v[2] == 0 && v[3] == 1;
}

static int
trans0(struct math_context *M, math_t t) {
	const float * v = math_value(M, t);
	return v[0] == 0 && v[1] == 0 && v[2] == 0;
}

math_t
math3d_make_srt(struct math_context *M, math_t s, math_t r, math_t t) {
	math_t id;
	glm::mat4x4 &srt = allocmat(M, &id);
	int ident = 1;
	if (!math_isnull(s) && !scale1(M, s)) {
		srt = glm::mat4x4(1);
		const glm::vec3 &scale = VEC3(M, s);
		srt[0][0] = scale[0];
		srt[1][1] = scale[1];
		srt[2][2] = scale[2];
		ident = 0;
	}
	if (!math_isnull(r) && !rot0(M, r)) {
		const glm::quat &q = QUAT(M, r);
		if (!ident) {
			srt = glm::mat4x4(q) * srt;
		} else {
			srt = glm::mat4x4(q);
		}
		ident = 0;
	} else if (ident) {
		srt = glm::mat4x4(1);
	}
	if (!math_isnull(t) && !trans0(M, t)) {
		const glm::vec3 &translate = VEC3(M, t);
		srt[3][0] = translate[0];
		srt[3][1] = translate[1];
		srt[3][2] = translate[2];
		srt[3][3] = 1;
		ident = 0;
	}
	if (ident) {
		return math_identity(MATH_TYPE_MAT);
	}

	return id;
}

void
math3d_decompose_matrix(struct math_context *M, math_t mat, math_t v[3]) {
	const glm::mat4x4 &m = MAT(M, mat);
	v[0] = math_vec4(M, NULL);
	v[1] = math_quat(M, NULL);
	v[2] = math_vec4(M, NULL);

	float *scale = math_init(M, v[0]);
	float *quat = math_init(M, v[1]);
	float *trans = math_init(M, v[2]);

	glm::vec3 skew;
	glm::vec4 perspective;
	glm::decompose(m, *(glm::vec3*)scale, *(glm::quat *)quat, *(glm::vec3*)trans, skew, perspective);
}

// epsilon for pow2
//#define EPSILON 0.00001f
// glm::equal(dot , 1.0f, EPSILON)

static inline int
equal_one(float f) {
	union {
		float f;
		uint32_t n;
	} u;
	u.f = f;
	return ((u.n + 0x1f) & ~0x3f) == 0x3f800000;	// float 1
}

math_t
math3d_decompose_scale(struct math_context *M, math_t mat) {
	math_t id = math_vec4(M, NULL);
	float * scale = math_init(M, id);
	const glm::mat4& m = MAT(M, mat);
	int ii;
	scale[3] = 0;
	for (ii = 0; ii < 3; ++ii) {
		const float* v = &m[ii].x;
		float dot = glm::dot(*(const glm::vec3 *)v, *(const glm::vec3 *)v);
		if (equal_one(dot)) {
			scale[ii] = 1.0f;
		} else {
			scale[ii] = sqrtf(dot);
			if (scale[ii] == 0) {
				// invalid scale, use 1 instead
				scale[0] = scale[1] = scale[2] = 1.0f;
				return id;
			}
		}
		if (glm::determinant(m) < 0){
			scale[ii] *= -1;
		}
	}
	return id;
}

math_t
math3d_decompose_rot(struct math_context *M, math_t mat) {
	math_t id;
	glm::quat &q = allocquat(M, &id);
	glm::mat4 rotMat(MAT(M, mat));
	math_t s = math3d_decompose_scale(M, mat);
	const float *scale = math_value(M, s);
	int ii;
	for (ii = 0; ii < 3; ++ii) {
		rotMat[ii] /= scale[ii];
	}
	q = glm::quat_cast(rotMat);
	return id;
}

math_t
math3d_add_vec(struct math_context *M, math_t v1, math_t v2) {
	math_t id;
	glm::vec4 &r = allocvec4(M, &id);
	r = VEC(M, v1) + VEC(M, v2);
	return id;
}

math_t
math3d_sub_vec(struct math_context *M, math_t v1, math_t v2) {
	math_t id;
	glm::vec4 &r = allocvec4(M, &id);
	r = VEC(M, v1) - VEC(M, v2);
	return id;
}

math_t
math3d_mul_vec(struct math_context *M, math_t v1, math_t v2) {
	math_t id;
	glm::vec4 &r = allocvec4(M, &id);
	r = VEC(M, v1) * VEC(M, v2);
	return id;
}

math_t
math3d_mul_quat(struct math_context *M, math_t v1, math_t v2) {
	if (math_isidentity(v1)) {
		return v2;
	}
	if (math_isidentity(v2)) {
		return v1;
	}
	math_t id;
	glm::quat &quat = allocquat(M, &id);
	quat = QUAT(M, v1) * QUAT(M, v2);
	return id;
}

math_t
math3d_mul_matrix(struct math_context *M, math_t v1, math_t v2) {
	if (math_isidentity(v1)) {
		return v2;
	}
	if (math_isidentity(v2)) {
		return v1;
	}
	math_t id;
	glm::mat4x4 &mat = allocmat(M, &id);
	mat = MAT(M, v1) * MAT(M, v2);
	return id;
}

static inline void
matrix_mul(float * output, const float *m1, const float *m2) {
	glm::mat4x4 & omat = *(glm::mat4x4 *)(output);
	const glm::mat4x4 & mat1 = *(const glm::mat4x4 *)(m1);
	const glm::mat4x4 & mat2 = *(const glm::mat4x4 *)(m2);
	omat = mat1 * mat2;
}

math_t
math3d_mul_matrix_array(struct math_context *M, math_t mat, math_t array_mat, math_t output_ref) {
	int reverse = 0;
	int sz = math_size(M, array_mat);
	if (sz == 1) {
		sz = math_size(M, mat);
		reverse = 1;
		math_t tmp = mat;
		mat = array_mat;
		array_mat = tmp;
	} else {
		int matsz = math_size(M, mat);
		if (matsz > 1) {
			// array(mat) * array(array_mat)
			if (matsz < sz)
				sz = matsz;
			if (math_isnull(output_ref)) {
				output_ref = math_import(M, NULL, MATH_TYPE_MAT, sz);
			} else {
				int output_sz = math_size(M, output_ref);
				if (output_sz < sz)
					sz = output_sz;
			}
			int i;
			const float * lm = math_value(M, mat);
			const float * rm = math_value(M, array_mat);
			float * out_buf = math_init(M, output_ref);
			for (i=0;i<sz;i++) {
				matrix_mul(out_buf + i * 16, lm + i * 16, rm + i * 16);
			}
			return output_ref;
		}
	}

	// matrix * array

	if (math_isidentity(mat)) {
		// mul identity, copy array
		if (math_isnull(output_ref)) {
			return array_mat;
		} else {
			float * result = math_init(M, output_ref);
			const float * source = math_value(M, array_mat);
			int sz_output = math_size(M, output_ref);
			if (sz_output < sz)
				sz = sz_output;
			memcpy(result, source, sz * 16 * sizeof(float));
			return output_ref;
		}
	}

	if (math_isnull(output_ref)) {
		output_ref = math_import(M, NULL, MATH_TYPE_MAT, sz);
	} else {
		int output_sz = math_size(M, output_ref);
		if (output_sz < sz)
			sz = output_sz;
	}
	int i;
	const float * m = math_value(M, mat);
	float * out_buf = math_init(M, output_ref);
	const float * in_buf = math_value(M, array_mat);
	if (reverse) {
		for (i=0;i<sz;i++) {
			matrix_mul(out_buf + i * 16, in_buf + i * 16, m);
		}
	} else {
		for (i=0;i<sz;i++) {
			matrix_mul(out_buf + i * 16, m, in_buf + i * 16);
		}
	}
	return output_ref;
}

float
math3d_length(struct math_context *M, math_t v) {
	return glm::length(VEC3(M, v));
}

math_t
math3d_floor(struct math_context *M, math_t v) {
	math_t id = math_vec4(M, NULL);
	float *vv = math_init(M, id);
	check_type(M, v, MATH_TYPE_VEC4);
	const float *value = math_value(M, v);
	vv[0] = floor(value[0]);
	vv[1] = floor(value[1]);
	vv[2] = floor(value[2]);
	vv[3] = floor(value[3]);
	return id;
}

math_t
math3d_ceil(struct math_context *M, math_t v) {
	math_t id = math_vec4(M, NULL);
	float *vv = math_init(M, id);
	check_type(M, v, MATH_TYPE_VEC4);
	const float *value = math_value(M, v);
	vv[0] = ceil(value[0]);
	vv[1] = ceil(value[1]);
	vv[2] = ceil(value[2]);
	vv[3] = ceil(value[3]);
	return id;
}

float
math3d_dot(struct math_context *M, math_t v1, math_t v2) {
	return glm::dot(VEC3(M, v1), VEC3(M, v2));
}

math_t
math3d_cross(struct math_context *M, math_t v1, math_t v2) {
	glm::vec3 c = glm::cross(VEC3(M, v1), VEC3(M, v2));
	math_t id = math_vec4(M, NULL);
	float *r = math_init(M, id);
	r[0] = c[0];
	r[1] = c[1];
	r[2] = c[2];
	r[3] = 0;

	return id;
}

math_t
math3d_normalize_vector(struct math_context *M, math_t id) {
	const float *v = math_value(M, id);
	glm::vec3 v3 = glm::normalize(*(const glm::vec3 *)(v));
	math_t result = math_vec4(M, NULL);
	float *r = math_init(M, result);
	r[0] = v3[0];
	r[1] = v3[1];
	r[2] = v3[2];
	r[3] = v[3];
	return result;
}

math_t
math3d_normalize_quat(struct math_context *M, math_t quat) {
	math_t id;
	glm::quat &q = allocquat(M, &id);
	q = glm::normalize(QUAT(M, quat));
	return id;
}

math_t
math3d_transpose_matrix(struct math_context *M, math_t mat) {
	math_t id;
	glm::mat4x4 &r = allocmat(M, &id);
	r = glm::transpose(MAT(M, mat));
	return id;
}

math_t
math3d_inverse_quat(struct math_context *M, math_t quat) {
	math_t id;
	glm::quat &q = allocquat(M, &id);
	q = glm::inverse(QUAT(M, quat));
	return id;
}

math_t
math3d_inverse_matrix(struct math_context *M, math_t mat) {
	math_t id;
	glm::mat4x4 &r = allocmat(M, &id);
	r = glm::inverse(MAT(M, mat));
	return id;
}

math_t
math3d_inverse_matrix_fast(struct math_context *M, math_t mat) {
	math_t id;
	const auto &m = MAT(M, mat);
	
	glm::mat4x4 &r = allocmat(M, &id);
	r = m;
	assert(	is_zero(glm::dot(V3R(m[0]), V3R(m[1])), 1e-6f) &&
			is_zero(glm::dot(V3R(m[1]), V3R(m[2])), 1e-6f) &&
			is_zero(glm::dot(V3R(m[2]), V3R(m[0])), 1e-6f));

	// DO NOT write: auto m3 = (glm::mat3*)(&m);
	// transpose 3x3
	std::swap(r[0][1], r[1][0]);
	std::swap(r[0][2], r[2][0]);
	std::swap(r[1][2], r[2][1]);

	glm::vec3& c3 = *(glm::vec3*)(&r[3]);

	// rotate t -> T = r * t ==> glm::vec3(dot(row0(r), t), dot(row1(r), t), dot(row2(r), t), here row0(r) = col0(m)
	c3 = -glm::vec3(glm::dot(V3R(m[0]), c3), glm::dot(V3R(m[1]), c3), glm::dot(V3R(m[2]), c3));
	return id;
}

math_t
math3d_quat_transform(struct math_context *M, math_t quat, math_t v) {
	math_t id;
	glm::vec4 &vv = allocvec4(M, &id);
	vv = glm::rotate(QUAT(M, quat), VEC(M, v));
	return id;
}

math_t
math3d_rotmat_transform(struct math_context *M, math_t mat, math_t v) {
	math_t id;
	glm::vec4 &vv = allocvec4(M, &id);
	vv = MAT(M, mat) * VEC(M, v);
	return id;
}

math_t
math3d_mulH(struct math_context *M, math_t mat, math_t v) {
	math_t id;
	glm::vec4 &r = allocvec4(M, &id);
	check_type(M, v, MATH_TYPE_VEC4);
	const float *vec = math_value(M, v);

	if (vec[3] != 1.f){
		glm::vec4 tmp ( vec[0], vec[1], vec[2], 1 );
		r = MAT(M, mat) * tmp;
	} else {
		r = MAT(M, mat) * (*(const glm::vec4 *)(vec));
	}

	if (r.w != 0) {
		r /= fabs(r.w);
		r.w = 1.f;
	}
	return id;
}

math_t
math3d_reciprocal(struct math_context *M, math_t v) {
	math_t id;
	glm::vec4 &vv = allocvec4(M, &id);

	check_type(M, v, MATH_TYPE_VEC4);
	const float *value = math_value(M, v);
	const glm::vec4 & vec = *(const glm::vec4 *)(value);

	vv = 1.f / vec;
	vv[3] = value[3];

	return id;
}

math_t
math3d_lookat_matrix(struct math_context *M, int direction, math_t eye, math_t at, math_t up_id) {
	math_t id;
	glm::mat4x4 &m = allocmat(M, &id);
	const float *up;
	if (math_isnull(up_id)) {
		static const float default_up[3] = {0,1,0};
		up = default_up;
	} else {
		check_type(M, up_id, MATH_TYPE_VEC4);
		up = math_value(M, up_id);
	}
	const glm::vec3 &eyev = VEC3(M, eye);
	if (direction) {
		const glm::vec3 vat = eyev + VEC3(M, at);
		m = glm::lookAtLH(eyev, vat, *(const glm::vec3 *)(up));
	} else {
		m = glm::lookAtLH(eyev, VEC3(M, at), *(const glm::vec3 *)(up));
	}
	return id;
}

static inline glm::vec2
perspective_AB(float near, float far, struct projection_flags flags){
	float A, B;
	if (flags.infinity_far){
		if (flags.homogeneous_depth){
			//(far+near) / (far-near);
			// flags.invert_z ==> near/-near ==> -1
			//					  far/far	 ==> 1
			A = flags.invert_z ? -1.f : 1.f;
		} else {
			//far / (far-near);
			//flags.invert_z ==> far/-near	==> -0	==> -glm::epsilon<float>()
			//				 ==> far/far	==> 1
			A = flags.invert_z ? -glm::epsilon<float>() : 1.f;
		}

		// -(far*near) / (far-near)
		//if invert_z ==> -far*near/-near	= far
		//			  ==> -far*near/far		=-near
		B = flags.invert_z ? far : -near;
	} else {
		const float zrange = far-near;

		A = (flags.homogeneous_depth ? (far+near) : far) / zrange;
		B =-(far*near) / zrange;
	}

	return glm::vec2(A, B);
}

static inline glm::mat4
perspectiveLH(float fovy, float aspect, float near, float far, struct projection_flags flags)
{
	if (flags.invert_z){
		std::swap(near, far);
	}

	assert(abs(aspect - glm::epsilon<float>()) > 0);
	const float tanHalfFovy = tan(fovy / 2);

	glm::mat4 r(0);
	r[0][0] = 1 / (aspect * tanHalfFovy);
	r[1][1] = 1 / (tanHalfFovy);
	r[2][3] = 1;
	const glm::vec2 AB = perspective_AB(near, far, flags);
	r[2][2] = AB[0];
	r[3][2] = AB[1];
	return r;
}


static glm::mat4
frustumLH(float left, float right, float bottom, float top, float near, float far, struct projection_flags flags)
{
	if (flags.invert_z){
		std::swap(near, far);
	}

	const float xrange = (right - left);
	const float yrange = (top - bottom);
	glm::mat4 r(0);
	r[0][0] = 2.f* near / xrange;
	r[1][1] = 2.f* near / yrange;
	r[2][0] = (right + left) / xrange;
	r[2][1] = (top + bottom) / yrange;
	r[2][3] = 1.f;

	const glm::vec2 AB = perspective_AB(near, far, flags);
	r[2][2] = AB[0];
	r[3][2] = AB[1];
	return r;
}

static glm::mat4
orthoLH(float left, float right, float bottom, float top, float near, float far, struct projection_flags flags)
{
	if (flags.invert_z){
		std::swap(near, far);
	}

	glm::mat4 r(1);
	const float xrange = (right - left);
	const float yrange = (top - bottom);
	r[0][0] = 2.f / xrange;
	r[1][1] = 2.f / yrange;
	r[3][0] =-(right + left)/xrange;
	r[3][1] =-(top + bottom)/yrange;

	if (flags.infinity_far){
		//(1.f/(far-near)) | (2.f/(far-near)) ==> c/(far-near)
		//flags.invert_z ==>	c/-near ==> -0	==>-glm::epsilon<float>()
		//						c/far	==> 0
		r[2][2] = flags.invert_z ? -glm::epsilon<float>() : glm::epsilon<float>();

		if (flags.homogeneous_depth){
			//-(far+near)/(far-near);
			//flags.invert_z ==>	-near/-near ==> 1
			//						-far/far	==> -1
			r[3][2] = flags.invert_z ? 1.f : -1.f;
		} else {
			//-near/(far-near);
			//flags.invert_z ==>	-near/-near ==> 1
			//						-0/far		==> -0 ==> -glm::epsilon<float>()
			r[3][2] = flags.invert_z ? 1.f : -glm::epsilon<float>();
		}
	} else {
		const float zrange = (far - near);

		if (flags.homogeneous_depth){
			r[2][2] = 2.f /zrange;
			r[3][2] =-(far+near)/zrange;
		} else {
			r[2][2] = 1.f /zrange;
			r[3][2] =-near/zrange;
		}
	}

	return r;
}

math_t
math3d_perspectiveLH(struct math_context *M, float fov, float aspect, float near, float far, struct projection_flags flags) {
	math_t id;
	glm::mat4x4 &mat = allocmat(M, &id);
	mat = perspectiveLH(fov, aspect, near, far, flags);
	return id;
}

math_t
math3d_frustumLH(struct math_context *M, float left, float right, float bottom, float top, float near, float far, struct projection_flags flags) {
	math_t id;
	glm::mat4x4 &mat = allocmat(M, &id);
	mat = frustumLH(left, right, bottom, top, near, far, flags);
	return id;
}

math_t
math3d_orthoLH(struct math_context *M, float left, float right, float bottom, float top, float near, float far, struct projection_flags flags) {
	math_t id;
	glm::mat4x4 &mat = allocmat(M, &id);
	mat = orthoLH(left, right, bottom, top, near, far, flags);
	return id;
}

math_t
math3d_base_axes(struct math_context *M, math_t forward_id) {
	math_t result = math_import(M, NULL, MATH_TYPE_VEC4, 2);

	glm::vec4 &up = initvec4(M, math_index(M, result, 0));
	glm::vec4 &right = initvec4(M, math_index(M, result, 1));

	const glm::vec4 &forward = VEC(M, forward_id);
	const glm::vec3 &forward3 = VEC3(M, forward_id);

	if (is_equal(forward, ZAXIS)) {
		up = YAXIS;
		right = XAXIS;
	} else {
		if (is_equal(forward, YAXIS)) {
			up = NZAXIS;
			right = XAXIS;
		} else if (is_equal(forward, NYAXIS)) {
			up = ZAXIS;
			right = XAXIS;
		} else {
			right = glm::vec4(glm::normalize(glm::cross(*(const glm::vec3 *)(&YAXIS.x), forward3)), 0);
			up = glm::vec4(glm::normalize(glm::cross(forward3, *(const glm::vec3 *)(&right.x))), 0);
		}
	}
	return result;
}

math_t
math3d_quat_to_viewdir(struct math_context *M, math_t quat) {
	math_t id;
	glm::vec4 &d = allocvec4(M, &id);
	d = glm::rotate(QUAT(M, quat), glm::vec4(0, 0, 1, 0));
	return id;
}

math_t
math3d_rotmat_to_viewdir(struct math_context *M, math_t mat) {
	math_t id;
	glm::vec4 &d = allocvec4(M, &id);
	d = MAT(M, mat) * glm::vec4(0, 0, 1, 0);
	return id;
}

math_t
math3d_viewdir_to_quat(struct math_context *M, math_t v) {
	float tmp[4] = {0, 0, 1, 0};
	math_t vv = math_vec4(M, tmp);
	return math3d_quat_between_2vectors(M, vv, v);
}

static math_t
minv(struct math_context *M, math_t v0, math_t v1) {
	const float * left = math_value(M, v0);
	const float * right = math_value(M, v1);
	float tmp[4];
	int left_n = 0;
	int right_n = 0;
	int i;
	for (i=0;i<3;i++) {
		if (left[i] <= right[i]) {
			if (left[i] == right[i])
				++right_n;
			++left_n;
			tmp[i] = left[i];
		} else {
			// left[i] > right[i]
			++right_n;
			tmp[i] = right[i];
		}
	}
	if (left_n == 3) {
		return v0;
	} else if (right_n == 3) {
		return v1;
	}
	tmp[3] = 0;
	return math_vec4(M, tmp);
}

static math_t
maxv(struct math_context *M, math_t v0, math_t v1) {
	const float * left = math_value(M, v0);
	const float * right = math_value(M, v1);
	float tmp[4];
	int left_n = 0;
	int right_n = 0;
	int i;
	for (i=0;i<3;i++) {
		if (left[i] >= right[i]) {
			if (left[i] == right[i])
				++right_n;
			++left_n;
			tmp[i] = left[i];
		} else {
			// left[i] < right[i]
			++right_n;
			tmp[i] = right[i];
		}
	}
	if (left_n == 3) {
		return v0;
	} else if (right_n == 3) {
		return v1;
	}
	tmp[3] = 0;
	return math_vec4(M, tmp);
}

static inline glm::vec4
transform_pt(struct math_context *M, const glm::mat4& m, math_t p){
	glm::vec4 v = VEC(M, p);
	v.w = 1.f;//we assue p must be a point
	v = m * v;
	return v / v.w;
}

math_t
math3d_minmax(struct math_context *M, math_t transform, math_t points) {
	const size_t numpoints	= math_size(M, points);
	if (numpoints == 0)
		return MATH_NULL;

	const math_t p = math_index(M, points, 0);
	glm::vec4 minmax[2];

	if (math_isnull(transform)){
		minmax[0] = minmax[1] = VEC(M, p);
		for (int ii=1; ii<(int)numpoints; ++ii){
			const auto& pp = VEC(M, math_index(M, points, ii));
			minmax[0] = glm::min(minmax[0], pp);
			minmax[1] = glm::max(minmax[1], pp);
		}
	} else {
		const glm::mat4& m = MAT(M, transform);
		minmax[0] = minmax[1] = transform_pt(M, m, p);
		for (int ii=1; ii<(int)numpoints; ++ii){
			const glm::vec4 tpp = transform_pt(M, m, math_index(M, points, ii));
			minmax[0] = glm::min(minmax[0], tpp);
			minmax[1] = glm::max(minmax[1], tpp);
		}
	}

	return math_import(M, (const float*)minmax, MATH_TYPE_VEC4, 2);
}

math_t
math3d_aabb_merge(struct math_context *M, math_t aabblhs, math_t aabbrhs) {
	check_type(M, aabblhs, MATH_TYPE_VEC4);
	check_type(M, aabbrhs, MATH_TYPE_VEC4);

	const math_t lhsmin = math_index(M, aabblhs, 0);
	const math_t lhsmax = math_index(M, aabblhs, 1);
	
	const math_t rhsmin = math_index(M, aabbrhs, 0);
	const math_t rhsmax = math_index(M, aabbrhs, 1);

	math_t min_id = minv(M, lhsmin, rhsmin);
	math_t max_id = maxv(M, lhsmax, rhsmax);
	if (math_issame(min_id, lhsmin) && math_issame(max_id, lhsmax)) {
		return aabblhs;
	}
	if (math_issame(min_id, rhsmin) && math_issame(max_id, rhsmax)) {
		return aabbrhs;
	}
	math_t r = math_import(M, NULL, MATH_TYPE_VEC4, 2);
	float *value = math_init(M, r);
	memcpy(value, math_value(M, min_id), 4 * sizeof(float));
	memcpy(value+4, math_value(M, max_id), 4 * sizeof(float));
	return r;
}

math_t
math3d_lerp(struct math_context *M, math_t v0, math_t v1, float ratio) {
	math_t id;
	glm::vec4 &r = allocvec4(M, &id);
	r = glm::lerp(VEC(M, v0), VEC(M, v1), ratio);
	return id;
}

math_t
math3d_quat_lerp(struct math_context *M, math_t v0, math_t v1, float ratio) {
	math_t id;
	glm::quat &r = allocquat(M, &id);
	r = glm::lerp(QUAT(M, v0), QUAT(M, v1), ratio);
	return id;
}

math_t
math3d_quat_slerp(struct math_context *M, math_t v0, math_t v1, float ratio) {
	math_t id;
	glm::quat &r = allocquat(M, &id);
	r = glm::slerp(QUAT(M, v0), QUAT(M, v1), ratio);
	return id;
}

// x: pitch(-90, 90), y: yaw(-180, 180), z: roll(-180, 180),
static glm::vec3
limit_euler_angles(const glm::quat& q) {
	static const float MY_PI = 3.14159265358979323846264338327950288f;
	float x_ = q.x;
	float y_ = q.y;
	float z_ = q.z;
	float w_ = q.w;
	// Derivation from http://www.geometrictools.com/Documentation/EulerAngles.pdf
	// Order of rotations: Z first, then X, then Y
	float check = 2.0f * (-y_ * z_ + w_ * x_);
	if (check < -0.995f) {
		return {
			-0.5f * MY_PI,
			0.0f,
			-atan2f(2.0f * (x_ * z_ - w_ * y_), 1.0f - 2.0f * (y_ * y_ + z_ * z_))
		};
	} else if (check > 0.995f) {
		return {
			0.5f * MY_PI,
			0.0f,
			atan2f(2.0f * (x_ * z_ - w_ * y_), 1.0f - 2.0f * (y_ * y_ + z_ * z_))
		};
	} else {
		return {
			asinf(check),
			atan2f(2.0f * (x_ * z_ + w_ * y_), 1.0f - 2.0f * (x_ * x_ + y_ * y_)),
			atan2f(2.0f * (x_ * y_ + w_ * z_), 1.0f - 2.0f * (x_ * x_ + z_ * z_))
		};
	}
}

math_t
math3d_quat_to_euler(struct math_context *M, math_t q) {
	math_t id;
	glm::vec4 &r = allocvec4(M, &id);
	r[3] = 0;
	glm::vec3 &eular = *(glm::vec3 *)(&r);
	eular = limit_euler_angles(QUAT(M, q)); // glm::eulerAngles(QUAT(q)); // 
	return id;
}

void
math3d_dir2radian(struct math_context *M, math_t rad, float radians[2]) {
	const float *v = math_value(M, rad);
	const float PI = float(M_PI);
	const float HALF_PI = 0.5f * PI;
	
	if (is_equal(v[1], 1.f)){
		radians[0] = -HALF_PI;
		radians[1] = 0.f;
	} else if (is_equal(v[1], -1.f)) {
		radians[0] = HALF_PI;
		radians[1] = 0.f;
	} else if (is_equal(v[0], 1.f)) {
		radians[0] = 0.f;
		radians[1] = HALF_PI;
	} else if (is_equal(v[0], -1.f)) {
		radians[0] = 0.f;
		radians[1] = -HALF_PI;
	} else if (is_equal(v[2], 1.f)) {
		radians[0] = 0.f;
		radians[1] = 0.f;
	} else if (is_equal(v[2], -1.f)) {
		radians[0] = 0.f;
		radians[1] = PI;
	} else {
		radians[0] = is_zero(v[1]) ? 0.f : std::asin(-v[1]);
		radians[1] = is_zero(v[0]) ? 0.f : std::atan2(v[0], v[2]);
	}
}

math_t
math3d_box_center(struct math_context *M, math_t points) {
	math_t id;
	glm::vec4 &c = allocvec4(M, &id);
	c = glm::vec4(0, 0, 0, 1);
	int ii;
	for (ii = 0; ii < 8; ++ii) {
		c += VEC(M, math_index(M, points, ii));
	}

	c /= 8.f;
	c.w = 1.f;

	return id;
}

math_t
math3d_frusutm_aabb(struct math_context *M, math_t points) {
	math_t aabb = math_import(M, NULL, MATH_TYPE_VEC4, 2);

	auto t = initaabb(M, aabb);
	
	t.minv = glm::vec4(std::numeric_limits<float>::max()),
	t.maxv = glm::vec4(std::numeric_limits<float>::lowest());

	const float * points_v = math_value(M, points);

	for (int ii = 0; ii < 8; ++ii){
		const auto &p = VECPTR(points_v + ii * 4);
		t.minv = glm::min(t.minv, p);
		t.maxv = glm::max(t.maxv, p);
	}

	return aabb;
}

int
math3d_aabb_isvalid(struct math_context *M, math_t aabb) {
	auto t = AABB(M, aabb);

	return (t.minv.x <= t.maxv.x && t.minv.y <= t.maxv.y && t.minv.z <= t.maxv.z) ? 1 : 0;
}

math_t
math3d_aabb_transform(struct math_context *M, math_t trans, math_t aabb) {
	const auto& t = MAT(M, trans);

	auto a = AABB(M, aabb);

	const glm::vec4 &right	= t[0];
	const glm::vec4 &up 	= t[1];
	const glm::vec4 &forward= t[2];
	const glm::vec4 &pos 	= t[3];

	const glm::vec4 xa = right * a.minv.x;
	const glm::vec4 xb = right * a.maxv.x;

	const glm::vec4 ya = up * a.minv.y;
	const glm::vec4 yb = up * a.maxv.y;

	const glm::vec4 za = forward * a.minv.z;
	const glm::vec4 zb = forward * a.maxv.z;

	math_t r = math_import(M, NULL, MATH_TYPE_VEC4, 2);

	auto o = initaabb(M, r);

	o.minv = glm::min(xa, xb) + glm::min(ya, yb) + glm::min(za, zb) + pos;
	o.maxv = glm::max(xa, xb) + glm::max(ya, yb) + glm::max(za, zb) + pos;

	return r;
}

math_t
math3d_aabb_center_extents(struct math_context *M, math_t aabb) {
	math_t result = math_import(M, NULL, MATH_TYPE_VEC4, 2);

	auto t = AABB(M, aabb);
	auto center = initaabb(M, result);

	center.minv = (t.maxv+t.minv)*0.5f;
	center.maxv = (t.maxv-t.minv)*0.5f;

	return result;
}

// plane define:
//		nx * px + ny*py + nz*pz + d = 0 ==> dot(n, p) + d = 0 ==> d = -dot(n, p) | dot(n, p) = -d, where n is normalize vector
//	1 : in front of plane
//	0 : lay on plane
// -1 : in back of plane

// distance of point P to plane:
//	we make: Plane.xyz equal to plane's normal, Plane.w is equal to d, so the plane equation is:
//		Plane.x*px + Plane.y*py + Plane.z*pz + Plane.w = 0
//	we make D is the point P to Plane's distance:
//		D = (P.x * Plane.x + P.y * Plane.y + P.z * Plane.z + Plane.w) / length(Plane.xyz) ==> dot(P.xyz, Plane.xyz) / length(Plane.xyz)
//	if Plane's normal is 1
//		D = dot(P.xyz, Plane.xyz) + Plane.w

static inline glm::vec4
create_plane(const glm::vec3& n, float d){
	return glm::vec4(n, -d);
}

static inline glm::vec4
create_plane(const glm::vec3& n, const glm::vec3& p){
	return create_plane(n, glm::dot(n, p));
}

math_t
math3d_plane(struct math_context* M, math_t n, float d){
	math_t planeid;
	auto &plane = allocvec4(M, &planeid);
	plane = create_plane(VEC3(M, n), d);
	return planeid;
}

math_t
math3d_plane_from_normal_point(struct math_context* M, math_t n, math_t p) {
	math_t planeid;
	auto &plane = allocvec4(M, &planeid);
	plane = create_plane(VEC3(M, n), VEC3(M, p));
	return planeid;
}

//we assume plane's normal is normalized
static inline float
point2plane(const glm::vec3 &pt, const glm::vec4 &plane){
	return glm::dot(pt, V3R(plane)) + plane.w;
}

float
math3d_point2plane(struct math_context *M, math_t pt, math_t plane) {
	return point2plane(VEC3(M, pt), VEC(M, plane));
}

static inline int
plane_test_point(const glm::vec4 &plane, const glm::vec3 &p){
	const float d = point2plane(p, plane);
	// outside frustum
	if (std::fabs(d) < 1e-6f){
		return 0;
	}

	if (d < 0){
		return -1;
	}

	return 1;
}

int
math3d_plane_test_point(struct math_context * M, math_t plane, math_t p){
	return plane_test_point(VEC(M, plane), VEC3(M, p));
}

static int
plane_aabb_intersect(const glm::vec4& plane, const struct AABB &aabb) {
	const glm::vec4 &min = aabb.minv;
	const glm::vec4 &max = aabb.maxv;
	float minD, maxD;
	if (plane.x > 0.0f) {
		minD = plane.x * min.x;
		maxD = plane.x * max.x;
	}
	else {
		minD = plane.x * max.x;
		maxD = plane.x * min.x;
	}

	if (plane.y > 0.0f) {
		minD += plane.y * min.y;
		maxD += plane.y * max.y;
	}
	else {
		minD += plane.y * max.y;
		maxD += plane.y * min.y;
	}

	if (plane.z > 0.0f) {
		minD += plane.z * min.z;
		maxD += plane.z * max.z;
	}
	else {
		minD += plane.z * max.z;
		maxD += plane.z * min.z;
	}

	// in front of the plane
	if (minD > -plane.w) {
		return 1;
	}

	// in back of the plane
	if (maxD < -plane.w) {
		return -1;
	}

	// straddle of the plane
	return 0;
}

int
math3d_aabb_intersect_plane(struct math_context *M, math_t aabb, math_t plane) {
	return plane_aabb_intersect(VEC(M, plane), AABB(M, aabb));
}

math_t
math3d_aabb_intersection(struct math_context *M, math_t aabb1, math_t aabb2) {
	check_type(M, aabb1, MATH_TYPE_VEC4);
	check_type(M, aabb2, MATH_TYPE_VEC4);
	math_t aabb[2] = {
		maxv(M, math_index(M, aabb1, 0),  math_index(M, aabb2, 0)),
		minv(M, math_index(M, aabb1, 1),  math_index(M, aabb2, 1)),
	};
	const float * v[2] = {
		math_value(M, aabb[0]),
		math_value(M, aabb[1]),
	};
	if (v[0] + 4 == v[1]) {
		// It's already a vector array
		if (v[0] == math_value(M, aabb1))
			return aabb1;
		if (v[0] == math_value(M, aabb2))
			return aabb2;
	}
	math_t r = math_import(M, NULL, MATH_TYPE_VEC4, 2);
	float * vv = math_init(M, r);
	memcpy(vv, v[0], 4 * sizeof(float));
	memcpy(vv+4, v[1], 4 * sizeof(float));
	return r;
}

int
math3d_aabb_test_point(struct math_context *M, math_t aabb, math_t v) {
	check_type(M, aabb, MATH_TYPE_VEC4);
	const float * aabb_value = math_value(M, aabb);
	const float * p = math_value(M, v);
	const float * minv = &aabb_value[0];
	const float * maxv = &aabb_value[4];

	int where = 1;
	int ii;
	for (ii=0;ii<3;++ii){
		if (minv[ii] > p[ii] || maxv[ii] < p[ii])
			return -1;
		if (minv[ii] == p[ii] || maxv[ii] == p[ii])
			where = 0;
	}

	return where;
}

enum BoxPoint {
	BP_lbn = 0,
	BP_ltn,
	BP_rbn,
	BP_rtn,

	BP_lbf,
	BP_ltf,
	BP_rbf,
	BP_rtf,

	BP_count,
};

math_t
math3d_aabb_points(struct math_context *M, math_t aabb) {
	auto t = AABB(M, aabb);

	math_t r = math_import(M, NULL, MATH_TYPE_VEC4, BP_count);

	glm::vec4* points = (glm::vec4*)math_value(M, r);

	points[BP_lbn] = t.minv;
	points[BP_ltn] = glm::vec4(t.minv.x, t.maxv.y, t.minv.z, 0);
	points[BP_rbn] = glm::vec4(t.maxv.x, t.minv.y, t.minv.z, 0);
	points[BP_rtn] = glm::vec4(t.maxv.x, t.maxv.y, t.minv.z, 0);

	points[BP_lbf] = glm::vec4(t.minv.x, t.minv.y, t.maxv.z, 0);
	points[BP_ltf] = glm::vec4(t.minv.x, t.maxv.y, t.maxv.z, 0);
	points[BP_rbf] = glm::vec4(t.maxv.x, t.minv.y, t.maxv.z, 0);
	points[BP_rtf] = t.maxv;

	return r;
}

math_t
math3d_aabb_expand(struct math_context *M, math_t aabb, math_t e) {
	math_t r = math_import(M, NULL, MATH_TYPE_VEC4, 2);
	auto o = initaabb(M, r);
	auto t = AABB(M, aabb);

	const glm::vec4 &v = VEC(M, e);

	o.minv = t.minv - v;
	o.maxv = t.maxv + v;

	return r;
}

// vplane [left, right, bottom, top, near, far]
enum PlaneName {
	PN_left = 0,
	PN_right,
	PN_bottom,
	PN_top,
	PN_near,
	PN_far,
	PN_count,
};

math_t
math3d_aabb_planes(struct math_context *M, math_t aabb){
	math_t planes = math_import(M, NULL, MATH_TYPE_VEC4, PN_count);
	glm::vec4* pp = (glm::vec4*)(math_value(M, planes));

	const glm::vec4* minv = (const glm::vec4*)math_value(M, aabb);
	const glm::vec4* maxv = minv + 1;

	pp[PN_left]		= create_plane(glm::vec3( 1.f, 0.f, 0.f), V3R(*minv));
	pp[PN_bottom]	= create_plane(glm::vec3( 0.f, 1.f, 0.f), V3R(*minv));
	pp[PN_near]		= create_plane(glm::vec3( 0.f, 0.f, 1.f), V3R(*minv));

	pp[PN_right]	= create_plane(glm::vec3(-1.f, 0.f, 0.f), V3R(*maxv));
	pp[PN_top] 		= create_plane(glm::vec3( 0.f,-1.f, 0.f), V3R(*maxv));
	pp[PN_far] 		= create_plane(glm::vec3( 0.f, 0.f,-1.f), V3R(*maxv));

	return planes;
}

math_t
math3d_frustum_planes(struct math_context *M, math_t m, int homogeneous_depth) {
	math_t result = math_import(M, NULL, MATH_TYPE_VEC4, 6);

	const auto &mat = MAT(M, m);
	const auto& c0 = mat[0], &c1 = mat[1], & c2 = mat[2], & c3 = mat[3];

	auto& leftplane = initvec4(M, math_index(M, result, PN_left));
	leftplane[0] = c0[0] + c0[3];
	leftplane[1] = c1[0] + c1[3];
	leftplane[2] = c2[0] + c2[3];
	leftplane[3] = c3[0] + c3[3];

	auto& rightplane = initvec4(M, math_index(M, result, PN_right));
	rightplane[0] = c0[3] - c0[0];
	rightplane[1] = c1[3] - c1[0];
	rightplane[2] = c2[3] - c2[0];
	rightplane[3] = c3[3] - c3[0];

	auto& bottomplane = initvec4(M, math_index(M, result, PN_bottom));
	bottomplane[0] = c0[3] + c0[1];
	bottomplane[1] = c1[3] + c1[1];
	bottomplane[2] = c2[3] + c2[1];
	bottomplane[3] = c3[3] + c3[1];

	auto& topplane = initvec4(M, math_index(M, result, PN_top));
	topplane[0] = c0[3] - c0[1];
	topplane[1] = c1[3] - c1[1];
	topplane[2] = c2[3] - c2[1];
	topplane[3] = c3[3] - c3[1];

	auto& nearplane = initvec4(M, math_index(M, result, PN_near));
	if (homogeneous_depth) {
		nearplane[0] = c0[3] + c0[2];
		nearplane[1] = c1[3] + c1[2];
		nearplane[2] = c2[3] + c2[2];
		nearplane[3] = c3[3] + c3[2];
	} else {
		nearplane[0] = c0[2];
		nearplane[1] = c1[2];
		nearplane[2] = c2[2];
		nearplane[3] = c3[2];
	}

	auto& farplane = initvec4(M, math_index(M, result, PN_far));
	farplane[0] = c0[3] - c0[2];
	farplane[1] = c1[3] - c1[2];
	farplane[2] = c2[3] - c2[2];
	farplane[3] = c3[3] - c3[2];

	// normalize
	int ii;
	for (ii = 0; ii < 6; ++ii){
		math_t v = math_index(M, result, ii);
		auto& p = initvec4(M, v);
		auto len = glm::length(VEC3(M, v));
		if (glm::abs(len) >= glm::epsilon<float>())
			p /= len;
	}

	return result;
}

int
math3d_frustum_intersect_aabb(struct math_context *M, math_t planes, math_t aabb) {
	check_type(M, aabb, MATH_TYPE_VEC4);

	auto a = AABB(M, aabb);

	const float * planes_v = math_value(M, planes);
	int where = 1;
	for (int ii = 0; ii < 6; ++ii){
		const auto &p = VECPTR(planes_v + ii * 4);
		const int w = plane_aabb_intersect(p, a);
		if (w < 0)
			return -1;

		if (w == 0)
			where = 0;
	}

	// where = 1, aabb inside frustum
	// where = 0, aabb intersect with one of frustum planes
	return where;
}

struct frustum_corners {
	glm::vec4 c[BP_count];
	static frustum_corners corners(float n, float f) {
		return frustum_corners {
			.c = {
				glm::vec4(-1.f,-1.f, n, 1.f),
				glm::vec4(-1.f, 1.f, n, 1.f),
				glm::vec4( 1.f,-1.f, n, 1.f),
				glm::vec4( 1.f, 1.f, n, 1.f),

				glm::vec4(-1.f,-1.f, f, 1.f),
				glm::vec4(-1.f, 1.f, f, 1.f),
				glm::vec4( 1.f,-1.f, f, 1.f),
				glm::vec4( 1.f, 1.f, f, 1.f),
			}
		};
	}
};

static const frustum_corners ndc_points_ZO = frustum_corners::corners(0.f, 1.f);
static const frustum_corners ndc_points_NO = frustum_corners::corners(-1.f, 1.f);

static math_t
math3d_frustum_points_(struct math_context *M, math_t m, const frustum_corners& fc) {
	math_t result = math_import(M, NULL, MATH_TYPE_VEC4, BP_count);
	auto invmat = glm::inverse(MAT(M, m));
	for (int ii = 0; ii < BP_count; ++ii){
		auto &p = initvec4(M, math_index(M, result, ii));
		p = invmat * fc.c[ii];
		p /= p.w;
	}
	return result;
}

math_t
math3d_frustum_points_with_nearfar(struct math_context *M, math_t m, float n, float f) {
	return math3d_frustum_points_(M, m, frustum_corners::corners(n, f));
}

math_t
math3d_frustum_points(struct math_context *M, math_t m, int homogeneous_depth) {
	return math3d_frustum_points_(M, m, homogeneous_depth ? ndc_points_NO : ndc_points_ZO);
}

// from: https://fileadmin.cs.lth.se/cs/Personal/Tomas_Akenine-Moller/raytri/
static inline
bool intersect_triangle3(const glm::vec3& orig, const glm::vec3& dir,
			const glm::vec3 &v0, const glm::vec3 &v1, const glm::vec3 &v2, 
			ray_triangle_interset_result &r)
{
	constexpr float EPSILON = 1e-6f;

	/* find vectors for two edges sharing vert0 */
	const auto edge1 = v1 - v0;
	const auto edge2 = v2 - v0;

	/* begin calculating determinant - also used to calculate U parameter */
	const auto pvec = glm::cross(dir, edge2);

	/* if determinant is near zero, ray lies in plane of triangle */
	const float det = glm::dot(edge1, pvec);
	const float inv_det = 1.f / det;

	/* calculate distance from vert0 to ray origin */
	const auto tvec = orig - v0;
	
	const auto qvec = glm::cross(tvec, edge1);

	if (det > EPSILON)
	{
		r.u = glm::dot(tvec, pvec);
		if (r.u < 0.f || r.u > det)
			return false;

		/* calculate V parameter and test bounds */
		r.v = glm::dot(dir, qvec);
		if (r.v < 0.0 || (r.u+r.v) > det)
			return false;
	}
	else if (det < -EPSILON)
	{
		/* calculate U parameter and test bounds */
		r.u = glm::dot(tvec, pvec);
		if (r.u > 0.0 || r.u < det)
			return false;

		/* calculate V parameter and test bounds */
		r.v = glm::dot(dir, qvec);
		if (r.v > 0.0 || (r.u+r.v) < det)
			return false;
	}
	else
		return false; /* ray is parallell to the plane of the triangle */

	r.t = glm::dot(edge2, qvec) * inv_det;
	r.u *= inv_det;
	r.v *= inv_det;

	return true;
}

int
math3d_ray_triangle_interset(struct math_context *M, math_t o, math_t d, math_t v0, math_t v1, math_t v2, struct ray_triangle_interset_result *r){
	return intersect_triangle3(VEC3(M, o), VEC3(M, d), VEC3(M, v0), VEC3(M, v1), VEC3(M, v2), *r);
}

math_t
math3d_ray_point(struct math_context * M, math_t o, math_t d, float t){
	glm::vec4 p = VEC(M, o) + VEC(M, d) * t;
	p.w = 1.f;
	return math_import(M, &p.x, MATH_TYPE_VEC4, 1);
}

static_assert(sizeof(struct triangle) == 36, "Invalid triangle size");

int
math3d_ray_triangles(struct math_context *M, math_t o, math_t d, const struct triangle* triangles, uint32_t numtriangles, struct ray_triangle_interset_result *r){
	const glm::vec4& ro = VEC(M, o);
	const glm::vec4& rd = VEC(M, d);

	struct ray_triangle_interset_result rr;
	rr.t = FLT_MAX;
	bool found = false;
	for (uint32_t ii=0; ii<numtriangles; ++ii){
		const struct triangle& tri = triangles[ii];
		struct ray_triangle_interset_result rrr;
		const glm::vec3 * tri0 = (const glm::vec3 *)tri.p[0].v;
		const glm::vec3 * tri1 = (const glm::vec3 *)tri.p[1].v;
		const glm::vec3 * tri2 = (const glm::vec3 *)tri.p[2].v;
		if (intersect_triangle3(ro, rd, *tri0, *tri1, *tri2, rrr) && rrr.t >= 0.f){
			if (rrr.t < rr.t){
				rr = rrr;
				found = true;
			}
		}
	}

	if (found){
		*r = rr;
		return 1;
	}
	return 0;
}

// face normal point to box center
static constexpr uint8_t FACE_INDICES[PN_count * 4] = {
	BP_lbn, BP_ltn, BP_lbf, BP_ltf, //left
	BP_rbf, BP_rtf, BP_rbn, BP_rtn, //right

	BP_lbn, BP_lbf, BP_rbn, BP_rbf, //bottom
	BP_ltf, BP_ltn, BP_rtf, BP_rtn, //top

	BP_rbn, BP_rtn, BP_lbn, BP_ltn, //near
	BP_lbf, BP_ltf, BP_rbf, BP_rtf, //far
};

static constexpr uint8_t TRI_INDICES_IN_FACE[6] = {0, 1, 2, 1, 3, 2};

static glm::vec4& GETPT(struct math_context * M, math_t points, int idx){ return *(glm::vec4*)(math_value(M, math_index(M, points, idx))); }

static constexpr uint8_t MAX_INTERSECT_POINTS = 2;

//one line intersect with a box, the max points is 6: intersect 2 corners of the box will generate 6 intersect points
constexpr uint8_t MAX_POTENTIAL_INTERSECT_POINTS = 6;
static inline bool check_add_t(float t, float *samet, uint8_t &numt) {
	if (0.f <= t && t <= 1.f){
		for (uint8_t ii=0; ii<numt; ++ii){
			if (is_equal(samet[ii], t))
				return false;
		}
		assert(numt < MAX_POTENTIAL_INTERSECT_POINTS);
		samet[numt++] = t;
		return true;
	}
	return false;
}

// return intersect points number from [0, 2]
static inline uint8_t
ray_interset_box(struct math_context * M, const glm::vec4 &o, const glm::vec4& d, math_t boxpoints, glm::vec4 *resultpoints){
	uint8_t numpoint = 0, numt = 0;
	float samet[MAX_POTENTIAL_INTERSECT_POINTS] = {0};

	assert(BP_count == math_size(M, boxpoints));

	for (uint8_t iface=0; iface<PN_count; ++iface){
		const uint8_t ifaceidx = iface*4;
		for (uint8_t it=0; it<6; it += 3){
			const uint8_t v0idx = FACE_INDICES[ifaceidx+TRI_INDICES_IN_FACE[it+0]];
			const uint8_t v1idx = FACE_INDICES[ifaceidx+TRI_INDICES_IN_FACE[it+1]];
			const uint8_t v2idx = FACE_INDICES[ifaceidx+TRI_INDICES_IN_FACE[it+2]];

			const auto& v0 = GETPT(M, boxpoints, v0idx);
			const auto& v1 = GETPT(M, boxpoints, v1idx);
			const auto& v2 = GETPT(M, boxpoints, v2idx);

			ray_triangle_interset_result r;
			if (intersect_triangle3(V3R(o), V3R(d), V3R(v0), V3R(v1), V3R(v2), r) && check_add_t(r.t, samet, numt)){
				assert(0 <= numpoint && numpoint < MAX_INTERSECT_POINTS);
				resultpoints[numpoint++] = o + d * r.t;
				break;
			}
		}
	}
	return numpoint;
}

math_t
math3d_box_ray(struct math_context * M, math_t o, math_t d, math_t boxpoints){
	glm::vec4 points[MAX_INTERSECT_POINTS];

	const uint8_t n = ray_interset_box(M, VEC(M, o), VEC(M, d), boxpoints, points);
	return n > 0 ? math_import(M, (const float*)points, MATH_TYPE_VEC4, n) : MATH_NULL;
}

int
math3d_frustum_test_point(struct math_context * M, math_t planes, math_t p){
	const auto& v3p = VEC3(M, p);
	int where = 1;
	for (int ii=0; ii<PN_count; ++ii){
		const auto& plane = VEC(M, math_index(M, planes, ii));
		const int w = plane_test_point(plane, v3p);
		if (w < 0){
			return -1;
		}

		if (w == 0){
			where = 0;
		}
	}

	return where;
}

static inline uint8_t
find_points_in_aabb(struct math_context* M, math_t testpoints, math_t aabb, glm::vec4 *points){
	uint8_t n = 0;
	for (uint8_t ii=0; ii<BP_count;++ii){
		math_t p = math_index(M, testpoints, ii);
		if (math3d_aabb_test_point(M, aabb, p) >= 0){
			points[n++] = VEC(M, p);
		}
	}
	return n;
}

static inline uint8_t
find_points_in_frustum(struct math_context *M, math_t testpoints, math_t frustumplanes, glm::vec4 *points){
	uint8_t n = 0;
	for (uint8_t ii=0; ii<BP_count;++ii){
		math_t p = math_index(M, testpoints, ii);
		if (math3d_frustum_test_point(M, frustumplanes, p) >= 0){
			points[n++] = VEC(M, p);
		}
	}
	return n;
}

math_t
math3d_frstum_aabb_intersect_points(struct math_context * M, math_t m, math_t aabb, int HOMOGENEOUS_DEPTH){
	constexpr int MAXPOINT = 64;
	glm::vec4 points[MAXPOINT];

	const math_t frustumpoints	= math3d_frustum_points(M, m, HOMOGENEOUS_DEPTH);

	uint8_t numpoint = find_points_in_aabb(M, frustumpoints, aabb, points);
	if (8 == numpoint){
		return math_import(M, (const float*)(&points), MATH_TYPE_VEC4, numpoint);
	}

	const math_t aabbpoints		= math3d_aabb_points(M, aabb);
	const math_t frustumplanes	= math3d_frustum_planes(M, m, HOMOGENEOUS_DEPTH);
	const uint8_t ptinfrustum = find_points_in_frustum(M, aabbpoints, frustumplanes, points+numpoint);
	numpoint += ptinfrustum;
	if (8 == ptinfrustum){
		return math_import(M, (const float*)(&points), MATH_TYPE_VEC4, numpoint);
	}

	constexpr uint8_t MAXLINE_POINTS = 12*2;
	constexpr uint8_t lineindices[MAXLINE_POINTS] = {
		0, 4, 1, 5,
		2, 6, 3, 7,

		0, 2, 1, 3,
		4, 6, 5, 7,

		0, 1, 2, 3,
		4, 5, 6, 7,
	};

	// from near to far
	for (uint8_t il=0; il < MAXLINE_POINTS; il+=2){
		// generate line from aabbpoints and frustumpoints
		const uint8_t s0 = lineindices[il], s1 = lineindices[il+1];

		const auto& o0 = GETPT(M, aabbpoints, 	s0);
		const auto& d0 = GETPT(M, aabbpoints, 	s1) - o0;
		numpoint += ray_interset_box(M, o0, d0, frustumpoints,points+numpoint);
		assert(numpoint <= MAXPOINT);

		const auto& o1 = GETPT(M, frustumpoints, 	s0);
		const auto& d1 = GETPT(M, frustumpoints, 	s1) - o1;
		numpoint += ray_interset_box(M, o1, d1, aabbpoints,	points+numpoint);
		assert(numpoint <= MAXPOINT);
	}
	return (numpoint > 0) ? math_import(M, (const float*)(&points), MATH_TYPE_VEC4, numpoint) : MATH_NULL;
}