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