local util = {}
local math3d = require "math3d"

local ONE_TAB<const>, TWO_TAB<const> = 1, 2

util.ONE_TAB = ONE_TAB
util.TWO_TAB = TWO_TAB

function util.create_ray(p1, p2)
	return {o=p1, d=math3d.sub(p2, p1)}
end

function util.tabs(n)
	return ('\t'):rep(n or 0)
end

function util.print_with_tab(s, tabnum)
	print(util.tabs(tabnum) .. s)
end

function util.print_ray(r, tabnum)
	util.print_with_tab(("ray.o:%s, ray.d:%s"):format(math3d.tostring(r.o), math3d.tostring(r.d)), tabnum)
end

function util.print_triangle(v0, v1, v2, tabnum)
	util.print_with_tab(("triangles:%s, %s, %s"):format(math3d.tostring(v0), math3d.tostring(v1), math3d.tostring(v2)), tabnum)
end

function util.print_points(points, tabnum)
	local t = {}
	local tab = ('\t'):rep(tabnum or 0)
	for i=1, math3d.array_size(points) do
		local p = math3d.array_index(points, i)
		t[#t+1] = tab .. math3d.tostring(p)
	end
	print(table.concat(t, "\n"))
end

function util.print_aabb(aabb, tabnum)
	local info = ("minv:%s, maxv:%s"):format(math3d.array_index(aabb, 1), math3d.array_index(aabb, 2))
	util.print_with_tab(info, tabnum)
end

function util.check_aabb(aabb, minv, maxv)
	assert(math3d.isequal(minv, math3d.array_index(aabb, 1)))
	assert(math3d.isequal(maxv, math3d.array_index(aabb, 2)))
end

function util.print_mat(mat)
	print "origin matrix:"
	print(math3d.tostring(mat))
	local p = math3d.vector(1, 2, 3, 1)

	print "transform point:(1, 2, 3, 1)"
	print(math3d.tostring(math3d.transform(mat, p, 1)))
	local s, r, t = math3d.srt(mat)

	print("srt.s:", math3d.tostring(s))
	print("srt.r:", math3d.tostring(r))
	print("srt.t:", math3d.tostring(t))

	print "transform srt.s by srt.r"
	print(math3d.tostring(math3d.transform(r, s, 0)))

	local m = math3d.matrix{s=s, r=r, t=t}
	print "combine matrix:"
	print(math3d.tostring(m))
	print "transform point:(1, 2, 3, 1) by combine matrix"
	print(math3d.tostring(math3d.transform(m, p, 1)))
end

return util