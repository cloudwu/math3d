local math3d = require "math3d"
local tu = require "test.util"

print "===RAY/LINE INTERSET WITH TRIANGLE==="
do
	local v0, v1, v2 = math3d.vector(-1.0, 0.0, 0.0, 1.0), math3d.vector(0.0, 1.0, 0.0, 1.0), math3d.vector(1.0, 0.0, 0.0, 1.0)
	print "\ttriangle1:"
	tu.print_triangle(v0, v1, v2, 1)

	local v3, v4, v5 = math3d.vector(-1.0, 0.0, 2.0, 1.0), math3d.vector(0.0, 1.0, 2.0, 1.0), math3d.vector(1.0, 0.0, 2.0, 1.0)
	print "\ttriangle2:"
	tu.print_triangle(v3, v4, v5, 1)

	local v6, v7, v8 = math3d.vector(0.0, 0.0, 1.0, 1.0), math3d.vector(0.0, 1.0, 0.0, 1.0), math3d.vector(0.0, 0.0, -1.0, 1.0)
	print "\ttriangle3:"
	tu.print_triangle(v6, v7, v8, 1)

	local ray = {o=math3d.vector(0.0, 0.0, 0.0), d=math3d.vector(0.0, 0.0, 1.0)}
	print "\tinterset with ray:"
	tu.print_ray(ray, 1)

	print "\tsegment1 interset with triangle:"
	local s0, s1 = math3d.vector(0, 0, 1, 1), math3d.vector(0, 0, 3, 1)

	print("\tstart point:", math3d.tostring(s0))
	print("\tend point:", math3d.tostring(s1))

	local interset, t = math3d.triangle_ray(ray.o, ray.d, v0, v1, v2)
	if interset then
		print(("\tray triangle interset result:%f, point:%s"):format(t, math3d.tostring(math3d.muladd(ray.d, t, ray.o))))
	else
		print "\tray NOT interset with triangle"
	end

	print"\tsegment with triangle1:"
	interset, t = math3d.triangle_ray(s0, math3d.sub(s1, s0), v0, v1, v2)
	print("\tsegment interset with triangle1: ", interset, "t: ", t, "is in the segment:", 0<=t and t<=1.0)

	interset, t = math3d.triangle_ray(s0, math3d.sub(s1, s0), v3, v4, v5)
	print("\tsegment interset with triangle2: ", interset, "t: ", t, "is in the segment:", 0<=t and t<=1.0)

	interset, t = math3d.triangle_ray(s0, math3d.sub(s1, s0), v6, v7, v8)
	print("\tsegment interset with triangle3: ", interset, "t: ", t or "no intersect")

	print "\tray intersect with box:"
	local boxcorners = {math3d.vector(-1.0, -1.0, -1.0), math3d.vector(1.0, 1.0, 1.0)}
	local aabb = math3d.aabb(boxcorners[1], boxcorners[2])
	local aabbpoints = math3d.aabb_points(aabb)

	print "\tray intersect with aabb points:"
	tu.print_box_points(aabbpoints, 2)

	local ray1 = {o=math3d.vector(0.0, 0.0, 0.0, 1.0), d=math3d.vector(0.0, 0.0, -1.0)}
	local function do_ray_box_test(ray, boxpoints)
		print "\tray:"
		tu.print_ray(ray, 2)
		local resultpoints = math3d.box_ray(ray.o, ray.d, boxpoints)
		print "\tray intersect results:"
		tu.print_points(resultpoints, 2)
		return resultpoints
	end

	print "\tray box test 1:"
	local results = do_ray_box_test(ray1, aabbpoints)
	assert(math3d.array_size(results) == 1)

	print "\tray box test 2:"
	local ray2 = tu.create_ray(math3d.vector(0.0, 0.0, -1.0, 1.0), math3d.vector(-1.0, 0.0, 0.0, 1.0))
	results = do_ray_box_test(ray2, aabbpoints)
	assert(math3d.array_size(results) == 2)

	print "\tray box test 3:"
	--local ray3 = create_ray(math3d.array_index(aabbpoints, 1), math3d.array_index(aabbpoints, 5))
	local ray3 = tu.create_ray(math3d.vector(0.0, 0.0, -1.0, 1.0), math3d.vector(0.0, 0.0, 1.0, 1.0))
	results = do_ray_box_test(ray3, aabbpoints)
	assert(math3d.array_size(results) == 2)

	print "\tray box test 4:"
	local ray4 = tu.create_ray(math3d.vector(-1.0, 0.0, 0.0, 1.0), math3d.vector(1.0, 0.0, 1.0, 1.0))
	results = do_ray_box_test(ray4, aabbpoints)
	assert(math3d.array_size(results) == 2)

	local aabb2 = math3d.aabb(math3d.vector(0.0, 0.0, 0.0, 0.0), math3d.vector(2.0, 2.0, 2.0, 0.0))
	local aabbpoints2 = math3d.aabb_points(aabb2)
	print "\taabb2 points:"
	tu.print_box_points(aabbpoints2, 2)

	print "\tray box test 5:"
	local ray5 = tu.create_ray(math3d.vector(0.0, 0.0, 0.0, 1.0), math3d.vector(2.0, 2.0, 2.0, 1.0))
	results = do_ray_box_test(ray5, aabbpoints2)
	assert(math3d.array_size(results) == 2)

	print "\tray box test 6:"
	local ray6 = tu.create_ray(math3d.vector(1.0, 1.0, -1.0, 1.0), math3d.vector(2.0, 2.0, 2.0, 1.0))
	results = do_ray_box_test(ray6, aabbpoints2)
	assert(math3d.array_size(results) == 2)
end

print "===RAY INTERSET WITH MULTI TRIANGLES==="
do
    local function posstr(x, y, z)
        return ('fff'):pack(x, y, z)
    end
	local function add_tri(t, v0, v1, v2)
		t[#t+1] = v0
		t[#t+1] = v1
		t[#t+1] = v2
	end

	local function tri2buffer(t)
		return table.concat(t, "")
	end

	local triangles = {}
	add_tri(triangles, posstr(0, 0, 0), posstr(0, 1, 0), posstr(1, 0, 0))

	local tb = tri2buffer(triangles)
	local r1 = tu.create_ray(math3d.vector(0, 0, -3), math3d.vector(0, 0, 1))
	tu.print_ray(r1)

	print "\ttriangles:"
	tu.print_triangle2(triangles[1], triangles[2], triangles[3], 2)
	local t, pt = math3d.triangles_ray(r1.o, r1.d, tb, #triangles // 3, true)
	assert(t and math3d.isequal(pt, math3d.vector(0.0, 0.0, 0.0)))

	add_tri(triangles, posstr(0, 0, -1), posstr(0, 1, -1), posstr(1, 0, -1))
	print "\ttriangles:"
	tu.print_triangle2(triangles[4], triangles[5], triangles[6], 2)

	local tb1 = tri2buffer(triangles)
	local t1, pt1 = math3d.triangles_ray(r1.o, r1.d, tb1, #triangles // 3, true)

	assert(t1 ~= nil and math3d.isequal(pt1, math3d.vector(0, 0, -1)))
end