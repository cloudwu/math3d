local math3d = require "math3d"

print "===PLANE==="
do
	--[[
		    local d1dotn = math3d.dot(plane, ray.d)
			local t = 0.0
			if math.abs(d1dotn) > 1e-7 then
				local dis = math3d.index(plane, 4)
				local odotn = math3d.dot(ray.o, plane)
				t = (dis - odotn) / d1dotn
			end
			local intersetion_pt = math3d.muladd(ray.d, tt, ray.o)
	]]
	local plane_pos = math3d.vector(0, 3, 0)
	local plane_dir = math3d.vector(0, 10, 0)
	local plane = math3d.plane(plane_pos, plane_dir)

	local ray = {o = math3d.vector(0, 10, 0), d = math3d.vector(0.0, -3, 0.0)}
	local tt = math3d.plane_ray(ray.o, ray.d, plane)
	if tt == nil then
		print "plane parallel with ray"
	elseif tt < 0 then
		print "plane intersetion in ray backward"
	else
		print "plane intersetion with plane front face"
	end

	local intersetion_pt = math3d.muladd(ray.d, tt, ray.o)
	print(math3d.tostring(intersetion_pt))
end