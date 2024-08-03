#ifndef math3d_func_h
#define math3d_func_h

#include "mathid.h"

// math functions
math_t math3d_matrix_from_cols(struct math_context*, math_t c1, math_t c2, math_t c3, math_t c4);
math_t math3d_quat_to_matrix(struct math_context *, math_t quat);
math_t math3d_matrix_to_quat(struct math_context *, math_t mat);
math_t math3d_make_quat_from_axis(struct math_context *, math_t axis, float r);
math_t math3d_quat_between_2vectors(struct math_context *, math_t a, math_t b);
math_t math3d_make_quat_from_euler(struct math_context *, math_t euler);
math_t math3d_make_srt(struct math_context *, math_t s, math_t r, math_t t);
void   math3d_decompose_matrix(struct math_context *, math_t mat, math_t v[3]);
math_t math3d_decompose_scale(struct math_context *, math_t mat);
math_t math3d_decompose_rot(struct math_context *, math_t mat);
float  math3d_dot(struct math_context *, math_t v1, math_t v2);
math_t math3d_add_vec(struct math_context *, math_t v1, math_t v2);
math_t math3d_sub_vec(struct math_context *, math_t v1, math_t v2);
math_t math3d_mul_vec(struct math_context *, math_t v1, math_t v2);
math_t math3d_mul_quat(struct math_context *, math_t v1, math_t v2);
math_t math3d_mul_matrix(struct math_context *, math_t v1, math_t v2);
math_t math3d_mul_matrix_array(struct math_context *M, math_t mat, math_t array_mat, math_t output_ref);
float  math3d_length(struct math_context *, math_t v);
math_t math3d_floor(struct math_context *, math_t v);
math_t math3d_ceil(struct math_context *, math_t v);
float  math3d_dot(struct math_context *, math_t v1, math_t v2);
math_t math3d_cross(struct math_context *, math_t v1, math_t v2);
math_t math3d_normalize_vector(struct math_context *, math_t v);
math_t math3d_normalize_quat(struct math_context *, math_t quat);
math_t math3d_transpose_matrix(struct math_context *, math_t mat);
math_t math3d_inverse_quat(struct math_context *, math_t quat);
math_t math3d_inverse_matrix(struct math_context *, math_t mat);
math_t math3d_inverse_matrix_fast(struct math_context *, math_t mat);
math_t math3d_quat_transform(struct math_context *, math_t quat, math_t v);
math_t math3d_rotmat_transform(struct math_context *, math_t mat, math_t v);
math_t math3d_mulH(struct math_context *, math_t mat, math_t v);
math_t math3d_reciprocal(struct math_context *, math_t v);

math_t math3d_lookat_matrix(struct math_context *, int dir, math_t eye, math_t at, math_t up);

struct projection_flags{
    uint8_t homogeneous_depth : 1;
    uint8_t invert_z : 1;
    uint8_t infinity_far : 1;
};
math_t math3d_perspectiveLH(struct math_context *M, float fov, float aspect, float near, float far, struct projection_flags flags);
math_t math3d_frustumLH(struct math_context *M, float left, float right, float bottom, float top, float near, float far, struct projection_flags flags);
math_t math3d_orthoLH(struct math_context *M, float left, float right, float bottom, float top, float near, float far, struct projection_flags flags);

math_t math3d_base_axes(struct math_context *M, math_t forward);	// return { right , up }

math_t math3d_quat_to_viewdir(struct math_context *, math_t quat);
math_t math3d_rotmat_to_viewdir(struct math_context *, math_t mat);
math_t math3d_viewdir_to_quat(struct math_context *, math_t v);

math_t math3d_lerp(struct math_context *, math_t v0, math_t v1, float ratio);
math_t math3d_quat_lerp(struct math_context *, math_t v0, math_t v1, float ratio);
math_t math3d_quat_slerp(struct math_context *, math_t v0, math_t v1, float ratio);
math_t math3d_quat_to_euler(struct math_context *, math_t quat);
void   math3d_dir2radian(struct math_context *, math_t rad, float radians[2]);
// aabb
math_t math3d_minmax(struct math_context *, math_t transform, math_t v);   // return aabb
int    math3d_aabb_isvalid(struct math_context *, math_t aabb);
math_t math3d_aabb_merge(struct math_context *, math_t aabblhs, math_t aabbrhs);
math_t math3d_aabb_transform(struct math_context *, math_t mat, math_t aabb);
math_t math3d_aabb_center_extents(struct math_context *, math_t aabb);	// return { center , extents }
int    math3d_aabb_intersect_plane(struct math_context *, math_t aabb, math_t plane);
math_t math3d_aabb_intersection(struct math_context *, math_t aabb1, math_t aabb2);
int    math3d_aabb_test_point(struct math_context *, math_t aabb, math_t v);
math_t math3d_aabb_points(struct math_context *, math_t aabb);
math_t math3d_aabb_expand(struct math_context *, math_t aabb, math_t e);
math_t math3d_aabb_planes(struct math_context *, math_t aabb);

math_t math3d_frustum_planes(struct math_context *, math_t m, int homogeneous_depth);	// return vec4[6]
int    math3d_frustum_intersect_aabb(struct math_context *, math_t planes, math_t aabb);
math_t math3d_frustum_points(struct math_context *, math_t m, int homogeneous_depth);	// return vec4[8]
math_t math3d_frustum_points_with_nearfar(struct math_context *M, math_t m, float n, float f);
math_t math3d_box_center(struct math_context *, math_t points);
float  math3d_frustum_max_radius(struct math_context *, math_t points, math_t center);

math_t math3d_frusutm_aabb(struct math_context *, math_t points);
int    math3d_frustum_test_point(struct math_context * M, math_t planes, math_t p);
math_t math3d_frstum_aabb_intersect_points(struct math_context *M, math_t m, math_t aabb, int HOMOGENEOUS_DEPTH);

math_t math3d_box_ray(struct math_context * M, math_t o, math_t d, math_t boxpoints);

math_t math3d_ray_point(struct math_context * M, math_t o, math_t d, float t);

struct tri_pos {float v[3];};
struct triangle{
	struct tri_pos p[3];
};

struct ray_triangle_interset_result { float t; float u, v; };

int math3d_ray_triangles(struct math_context *M, math_t o, math_t d, const struct triangle* triangles, uint32_t numtriangles, struct ray_triangle_interset_result *r);

int math3d_ray_triangle_interset(struct math_context *M, math_t s0, math_t s1, math_t v0, math_t v1, math_t v2, struct ray_triangle_interset_result *r);

//plane
float  math3d_point2plane(struct math_context *, math_t pt, math_t plane);
int    math3d_plane_test_point(struct math_context * M, math_t plane, math_t p);
math_t math3d_plane(struct math_context* M, math_t n, float d);
math_t math3d_plane_from_normal_point(struct math_context* M, math_t n, math_t p);
#endif
