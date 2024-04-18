local math3d = require "math3d"
local tu = require "test.util"
local ONE_TAB<const>, TWO_TAB<const> = tu.ONE_TAB, tu.TWO_TAB

print "===AABB&FRUSTUM==="
local box_line_indices<const> = {
	0, 4, 1, 5,
	2, 6, 3, 7,

	0, 2, 1, 3,
	4, 6, 5, 7,

	0, 1, 2, 3,
	4, 5, 6, 7,
}

local CORNER<const> = tu.corner_indices
local PLANE<const> = tu.plane_indiecs
do
	local aabb = math3d.minmax{math3d.vector(0, 4, 2), math3d.vector(2, 2, 4)}
	assert(math3d.array_size(aabb) == 2)
	local check_minv, check_maxv = math3d.vector(0, 2, 2), math3d.vector(2, 4, 4)
	tu.check_aabb(aabb, check_minv, check_maxv)
	tu.print_aabb(aabb, ONE_TAB)

	local transformmat = math3d.matrix(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 1, 1, 1, 1)
	tu.print_with_tab("transform:" .. math3d.tostring(transformmat), ONE_TAB)

	aabb = math3d.aabb_transform(transformmat, aabb)
	local translte_delta = math3d.vector(1, 1, 1, 1)
	tu.check_aabb(aabb, math3d.add(translte_delta, check_minv), math3d.add(translte_delta, check_maxv))
	print "\ttransformed aabb:"
	tu.print_aabb(aabb, ONE_TAB)

	print "\t===TEST FRUSTUM POINTS==="
	do
		local fov60<const> 				= 60
		local fov60radian<const> 		= math.rad(fov60)
		local half_fov60radian<const> 	= fov60radian * 0.5
	
		local near<const>, far<const> 	= 1, 3
		local aspect<const> 			= 1 -- width/height

		local hfov60<const>				= aspect * fov60 --width/height = hfov/fov ==> aspect * fov = hfov
		local hfov60radian<const>		= math.rad(hfov60)
		local half_hfov60radian<const>	= hfov60radian * 0.5
		local projmat = math3d.projmat{aspect=aspect, fov=fov60, n=near, f=far}

		local test_plane_normals<const> = {
			[PLANE.left] 	= math3d.transform(math3d.quaternion{axis=math3d.vector(0.0, 1.0, 0.0), r=-half_hfov60radian}, math3d.vector( 1.0, 0.0, 0.0, 0.0), 0),
			[PLANE.right] 	= math3d.transform(math3d.quaternion{axis=math3d.vector(0.0, 1.0, 0.0), r= half_hfov60radian}, math3d.vector(-1.0, 0.0, 0.0, 0.0), 0),
			[PLANE.bottom]	= math3d.transform(math3d.quaternion{axis=math3d.vector(1.0, 0.0, 0.0), r= half_fov60radian},  math3d.vector( 0.0, 1.0, 0.0, 0.0), 0),
			[PLANE.top]		= math3d.transform(math3d.quaternion{axis=math3d.vector(1.0, 0.0, 0.0), r=-half_fov60radian},  math3d.vector( 0.0,-1.0, 0.0, 0.0), 0),
			[PLANE.near]	= math3d.vector(0.0, 0.0, 1.0, 0.0),
			[PLANE.far]		= math3d.vector(0.0, 0.0,-1.0, 0.0),
		}

		--check planes
		local frustum_planes = math3d.frustum_planes(projmat)
		print(tu.tabs(2) .. "frustum planes:")
		tu.print_box_planes(frustum_planes, TWO_TAB)

		assert(#test_plane_normals == math3d.array_size(frustum_planes))
		for i=1, #test_plane_normals do
			local p1 = math3d.normalize(test_plane_normals[i])
			local p2 = math3d.normalize(math3d.array_index(frustum_planes, i))
			assert(math3d.isequal(p1, p2))
		end

		--check points
		--near half width
		local tanv = math.tan(half_hfov60radian)
		local nhw<const> = near*tanv
		local nhh<const> = nhw/aspect

		--far half width
		local fhw<const> = far*tanv
		local fhh<const> = fhw/aspect
		local testpoints<const> = {
			[CORNER.lbn] = math3d.vector(-nhw, -nhh, near),
			[CORNER.ltn] = math3d.vector(-nhw,  nhh, near),
			[CORNER.rbn] = math3d.vector( nhw, -nhh, near),
			[CORNER.rtn] = math3d.vector( nhw,  nhh, near),

			[CORNER.lbf] = math3d.vector(-fhw, -fhh, far),
			[CORNER.ltf] = math3d.vector(-fhw,  fhh, far),
			[CORNER.rbf] = math3d.vector( fhw, -fhh, far),
			[CORNER.rtf] = math3d.vector( fhw,  fhh, far),
		}

		local frustum_points = math3d.frustum_points(projmat)
		assert(math3d.array_size(frustum_points) == #testpoints)
		print(tu.tabs(TWO_TAB) .. "frustum points:")
		tu.print_box_points(frustum_points, TWO_TAB)
		for i=1, #testpoints do
			assert(math3d.isequal(math3d.array_index(frustum_points, i), testpoints[i]))
		end
	end

	print "\t===FRUSTUM AABB INTERSET==="
	do
		local projmat = math3d.projmat{ortho=true, l=-1, r=1, b=-1, t=1, n=0, f=2}
		local aabb = math3d.aabb(math3d.vector(0.0, 0.0, 0.0, 0.0), math3d.vector(1.0, 1.0, 1.0, 0.0))
		tu.print_aabb(aabb, TWO_TAB)

		local frustum_points = math3d.frustum_points(projmat)
		tu.print_box_points(frustum_points, TWO_TAB)

		local frustum_planes = math3d.frustum_planes(projmat)
		tu.print_box_planes(frustum_planes, TWO_TAB)

		assert(math3d.frustum_intersect_aabb(frustum_planes, aabb) == 0)

		local aabb2 = math3d.aabb(math3d.vector(-3, 0.0, 0.0, 0.0), math3d.vector(-2, 1.0, 1.0, 0.0))
		assert(math3d.frustum_intersect_aabb(frustum_planes, aabb2) < 0)

		local aabb3 = math3d.aabb(math3d.vector(0.0, 0.0, 0.5, 0.0), math3d.vector(0.5, 0.5, 1.0, 0.0))
		assert(math3d.frustum_intersect_aabb(frustum_planes, aabb3) > 0)
	end

	print "\t===TEST BOX CENTER AND RADIUS==="
	do
		local aabb 		= math3d.aabb(math3d.vector(0.0, 0.0, 0.0, 0.0), math3d.vector(1.0, 1.0, 1.0, 0.0))
		tu.print_aabb(aabb, ONE_TAB)
		local points 	= math3d.aabb_points(aabb)

		local center 	= math3d.points_center(points)
		local aabb2		= math3d.minmax(points)
		local center2, extents = math3d.aabb_center_extents(aabb2)
		assert(math3d.isequal(center, center2))
		assert(math3d.isequal(center, math3d.vector(0.5, 0.5, 0.5)))
		assert(math.abs(math3d.length(extents) * 2 - math.sqrt(3)) < 1e-6)
	end

	print "\t===frustum test points==="
	do
		local p = math3d.vector(0.0, 0.0, 0.0, 1.0)
		local p1 = math3d.vector(0.0, 0.0, 10.0, 1.0)

		local f = math3d.frustum_planes(math3d.projmat{ortho=true, l=-1, r=1, b=-1, t=1, n=-1, f=1})
		assert(math3d.frustum_test_point(f, p) > 0)
		assert(math3d.frustum_test_point(f, p1) < 0)
		assert(math3d.frustum_test_point(f, math3d.vector(-1.0, 0.0, 0.0, 1.0)) == 0)

		local f1 = math3d.frustum_planes(math3d.projmat{l=-1, r=1, b=-1, t=1, n=1, f=20})
		assert(math3d.frustum_test_point(f1, p) < 0)
		assert(math3d.frustum_test_point(f1, p1) > 0)

		print "\t passed!"
	end

	print "\t===AABB&minmax===="

	local points = {
		math3d.vector(1, 0, -1, -10),
		math3d.vector(1, 2, -1, 1),
		math3d.vector(1, 4, -5, 1),
		math3d.vector(-2, 0, -1, 1),
	}

	local aabb = math3d.minmax(points)
	local aabb2 = math3d.aabb()
	aabb2 = math3d.aabb_append(aabb2, points)

	print("minmax-aabb:", math3d.tostring(aabb))
	print("aabb-append:", math3d.tostring(aabb2))

	--aabb test merge
	local aabb = math3d.aabb()
	assert(not math3d.aabb_isvalid(aabb))

	local aabb2 = math3d.aabb(math3d.vector(-1.0, 0.0, 0.0), math3d.vector(0.0, 2.0, 3.0))

	local mergeaabb = math3d.aabb_merge(aabb, aabb2)
	assert(math3d.aabb_isvalid(mergeaabb))

	assert(math3d.isequal(math3d.vector(-1.0, 0.0, 0.0), math3d.array_index(mergeaabb, 1)))
	assert(math3d.isequal(math3d.vector(0.0, 2.0, 3.0), math3d.array_index(mergeaabb, 2)))


	aabb = math3d.aabb(math3d.vector(-2.0, -3.0, -5.0), math3d.vector(1.0, 2.0, 3.0))
	aabb2 = math3d.aabb(math3d.vector(-20.0, 31.0, 5.0), math3d.vector(1.0, 32.0, 36.0))

	mergeaabb = math3d.aabb_merge(aabb, aabb2)

	assert(math3d.isequal(math3d.vector(-20.0, -3.0, -5.0), math3d.array_index(mergeaabb, 1)))
	assert(math3d.isequal(math3d.vector(1.0, 32.0, 36.0), math3d.array_index(mergeaabb, 2)))


	local aabb = math3d.aabb(math3d.vector(-1, 1, -1), math3d.vector(1, 2, 1))
	local insidept, outsidept, layonpt = math3d.vector(0, 1.5, 0), math3d.vector(-2, 0, 0), math3d.vector(1, 2, 0)
	local isinside	= assert(math3d.aabb_test_point(aabb, insidept) > 0)
	local isoutside	= assert(math3d.aabb_test_point(aabb, outsidept) < 0)
	local islayon	= assert(math3d.aabb_test_point(aabb, layonpt) == 0)

	print "aabb test point, aabb:"
	print("\tinside point:",	math3d.tostring(insidept), ", result:", isinside)
	print("\toutside point:",	math3d.tostring(insidept), ", result:", isoutside)
	print("\tlayon point:",		math3d.tostring(insidept), ", result:", islayon)

	--create an aabb and ortho frustum, and make them overlap each other
	local aabb = math3d.aabb(math3d.vector(-1, -1, -1), math3d.vector(1, 1, 1))
	print "\t1. test aabb points:"
	tu.print_box_points(math3d.aabb_points(aabb), TWO_TAB)

	local projmat = math3d.projmat{ortho=true, l=-1, r=1, t=1, b=-1, n=0, f=1}
	print "\t1. test frustum point:"
	tu.print_box_points(math3d.frustum_points(projmat), TWO_TAB)

	local intersectpoints = math3d.frustum_aabb_intersect_points(projmat, aabb)
	print "\t1. intersect results:"
	tu.print_points(intersectpoints, TWO_TAB)

	local aabb2 = math3d.aabb(math3d.vector(-2, -2, -2), math3d.vector(2, 2, 2))
	print "\t2. test aabb points:"
	local aabbpoints2 = math3d.aabb_points(aabb2)
	tu.print_box_points(aabbpoints2, TWO_TAB)
	local projmat2 = math3d.projmat{l=-1, r=1, t=1, b=-1, n = 1, f=10}
	print "\t2. test frustum point:"
	local f2points = math3d.frustum_points(projmat2)
	tu.print_box_points(f2points, TWO_TAB)

	intersectpoints = math3d.frustum_aabb_intersect_points(projmat2, aabb2)
	print "\t2. intersect results:"
	tu.print_points(intersectpoints, TWO_TAB)
end