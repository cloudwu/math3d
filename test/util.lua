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
	local info = ("minv:%s, maxv:%s"):format(math3d.tostring(math3d.array_index(aabb, 1)), math3d.tostring(math3d.array_index(aabb, 2)))
	util.print_with_tab(info, tabnum)
end

function util.check_aabb(aabb, minv, maxv)
	assert(math3d.isequal(minv, math3d.array_index(aabb, 1)))
	assert(math3d.isequal(maxv, math3d.array_index(aabb, 2)))
end

local corner_names = {
	"lbn", "ltn", "rbn", "rtn",
	"lbf", "ltf", "rbf", "rtf",
}

util.corner_names = corner_names
util.corner_indices = {
	lbn = 1,
	ltn = 2,
	rbn = 3,
	rtn = 4,
	lbf = 5,
	ltf = 6,
	rbf = 7,
	rtf = 8,
	count = 8,
}

util.plane_indiecs = {
	left = 1,
	right = 2,
	bottom = 3,
	top = 4,
	near = 5,
	far = 6,
	count = 6,
}

util.plane_names = {
	"left",
	"right",
	"bottom",
	"top",
	"near",
	"far",
}

function util.print_box_points(points, tabnum)
	assert(math3d.array_size(points) == 8)

	local function point_name(pidx, tabnum)
		return ('\t'):rep(tabnum or 0) .. ("%s: %s"):format(corner_names[pidx], math3d.tostring(math3d.array_index(points, pidx)))
	end

	local function point_pairs(pidx1, pidx2, tabnum)
		return ("%s;\t%s"):format(point_name(pidx1, tabnum), point_name(pidx2))
	end
	local t = {
		point_pairs(1, 5, tabnum),
		point_pairs(2, 6, tabnum),
		point_pairs(3, 7, tabnum),
		point_pairs(4, 8, tabnum),
	}

	return print(table.concat(t, "\n"))
end

function util.print_box_planes(planes, tabnum)
	assert(math3d.array_size(planes) == 6)

	local function plane_name(pidx, tabnum)
		return ("%s%s:%s"):format(util.tabs(tabnum), util.plane_names[pidx], math3d.tostring(math3d.array_index(planes, pidx)))
	end
	local t = {}
	for i=1, 6 do
		t[#t+1] = plane_name(i, tabnum)
	end

	print(table.concat(t, '\n'))
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