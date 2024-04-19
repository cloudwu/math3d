local math3d	= require "math3d"
local tu		= require "test.util"
print "===PLANE==="
do
	--create plane
	local yaxis = math3d.vector(0.0, 1.0, 0.0, 0.0)
	local function plane_equal(p1, p2)
		local p1x, p1y, p1z, p1w = math3d.index(p1, 1, 2, 3, 4)
		local p2x, p2y, p2z, p2w = math3d.index(p2, 1, 2, 3, 4)

		return (p1x == p2x and p1y == p2y and p1z == p2z and p1w == p2w)
	end
	
	local p1 = math3d.plane(yaxis, math3d.vector(0.0, 3.0, 0.0, 1.0))
	local p2 = math3d.plane(yaxis, 3)
	local p3 = math3d.vector(0.0, 1.0, 0.0, -3.0)

	assert(plane_equal(p1, p2) and plane_equal(p2, p3), "Invalid math3d.plane")

	local ray = {o = math3d.vector(0, 10, 0), d = math3d.vector(0.0, -3, 0.0)}
	tu.print_ray(ray)

	local tt1, pt1 = math3d.plane_ray(ray.o, ray.d, p1, true)
	local intersetion_pt = math3d.muladd(tt1, ray.d, ray.o)
	print("plane_ray result: ", tt1, "point:", math3d.tostring(pt1), "ro+rd*t:", math3d.tostring(intersetion_pt))

	assert(tt1, "Invalid plane_ray")
	local _ = math3d.isequal(pt1, intersetion_pt) or error(("math3d.plane_ray interset point from t:%s, is different to plane_ray result: %s"):format(math3d.tostring(intersetion_pt, pt1)))
	local _ = (math3d.isequal(pt1, math3d.vector(0, 3, 0, 1)) or error(("Invalid plane_ray, interset point should be: (0, 3, 0, 1), %s:"):format(math3d.tostring(pt1))))

	local tt2, pt2 = math3d.plane_ray(ray.o, ray.d, p2, true)
	assert(tt1 == tt2)
	local _ = math3d.isequal(pt1, pt2) or error(("plane1:%s, plane2:%s, interset result are different, point1:%s, point2:%s"):format(math3d.tostring(p1), math3d.tostring(p2), math3d.tostring(pt1), math3d.tostring(pt2)))

	local testpt1 = math3d.vector(0, 4, 0, 1)
	local testresult1 = math3d.plane_test_point(p1, testpt1)
	assert(testresult1 > 0, ("Invalid plane_test_point, point: %s, should on top of plane: %s"):format(math3d.tostring(testpt1), math3d.tostring(p1)))

	local testpt2 = math3d.vector(0, 2, 0, 1)
	local testresult2 = math3d.plane_test_point(p1, testpt2)
	assert(testresult2 < 0, ("Invalid plane_test_point, point: %s, should lower to plane: %s"):format(math3d.tostring(testpt2), math3d.tostring(p1)))

	local testpt3 = math3d.vector(0, 3, 0, 1)
	local testresult3 = math3d.plane_test_point(p1, testpt3)
	assert(testresult3 == 0, ("Invalid plane_test_point, point: %s, should lay on plane: %s"):format(math3d.tostring(testpt3), math3d.tostring(p1)))

	local _ = testresult1 == math3d.plane_test_point(p2, testpt1) or error(("plane1 result is different with plane2, test point:%s, plane1:%s, plane2:%s"):format(math3d.tostring(testpt1), math3d.tostring(p1), math3d.tostring(p2)))
	local _ = testresult2 == math3d.plane_test_point(p2, testpt2) or error(("plane1 result is different with plane2, test point:%s, plane1:%s, plane2:%s"):format(math3d.tostring(testpt2), math3d.tostring(p1), math3d.tostring(p2)))
	local _ = testresult3 == math3d.plane_test_point(p2, testpt3) or error(("plane1 result is different with plane2, test point:%s, plane1:%s, plane2:%s"):format(math3d.tostring(testpt3), math3d.tostring(p1), math3d.tostring(p2)))

	local _ = math3d.point2plane(testpt1, p1) == 1 or error(("Invalid point2plane, test point:%s, plane:%s, result should be:%d, but return:%d"):format(math3d.tostring(testpt1), math3d.tostring(p1), 1, math3d.point2plane(testpt1, p1)))
	local _ = math3d.point2plane(testpt2, p1) ==-1 or error(("Invalid point2plane, test point:%s, plane:%s, result should be:%d, but return:%d"):format(math3d.tostring(testpt2), math3d.tostring(p1),-1, math3d.point2plane(testpt1, p1)))
	local _ = math3d.point2plane(testpt3, p1) == 0 or error(("Invalid point2plane, test point:%s, plane:%s, result should be:%d, but return:%d"):format(math3d.tostring(testpt3), math3d.tostring(p1), 0, math3d.point2plane(testpt1, p1)))
end


print "===AABB PLANE==="
local PLANE<const> = tu.plane_indiecs
local ONE_TAB<const>, TWO_TAB<const> = tu.ONE_TAB, tu.TWO_TAB
do
	print "\tcheck aabb planes"
	local aabb = math3d.aabb(math3d.vector(-1, -1, -1, 0), math3d.vector(1, 1, 1, 0))
	tu.print_aabb(aabb, ONE_TAB)
	local check_planes = {
		[PLANE.left] 	= math3d.plane(math3d.vector( 1, 0, 0),-1),
		[PLANE.right]	= math3d.plane(math3d.vector(-1, 0, 0),-1),
		[PLANE.bottom]	= math3d.plane(math3d.vector( 0, 1, 0),-1),
		[PLANE.top]		= math3d.plane(math3d.vector( 0,-1, 0),-1),
		[PLANE.near]	= math3d.plane(math3d.vector( 0, 0, 1),-1),
		[PLANE.far]		= math3d.plane(math3d.vector( 0, 0,-1),-1),
	}

	local aabb_planes = math3d.aabb_planes(aabb)
	print "\taabb planes:"
	tu.print_box_planes(aabb_planes)
	assert(math3d.array_size(aabb_planes) == #check_planes)
	for i=1, #check_planes do
		local p1 = check_planes[i]
		local p2 = math3d.array_index(aabb_planes, i)
		local d1 = math3d.index(p1, 4)
		local d2 = math3d.index(p2, 4)
		assert(math3d.isequal(p1, p2))
		assert(math.abs(d1 - d2) < 1e-6)
	end

	print "\ttest aabb planes by point:"
	local pt = math3d.vector(1, 1, 1)
	tu.print_with_tab("test point:" .. math3d.tostring(pt), ONE_TAB)
	local aabb2 = math3d.aabb(math3d.vector(0.5, 0.5, 0.5, 0), math3d.vector(1.5, 1.5, 1.5, 0))
	print "\ttest aabb:"
	tu.print_aabb(aabb, ONE_TAB)
	local aabb_planes2 = math3d.aabb_planes(aabb2)
	for i=1, math3d.array_size(aabb_planes2) do
		local p = math3d.array_index(aabb_planes2, 1)
		tu.print_with_tab("plane:" .. math3d.tostring(p))
		local r = math3d.plane_test_point(p, pt)
		tu.print_with_tab("test result:" .. r, TWO_TAB)
		local _ = r == 1 or error(("pt:%s, must on top of this plane:%s"):format(math3d.tostring(pt), math3d.tostring(p)))
	end
end
