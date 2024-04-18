local math3d = require "math3d"
local tu = require "test.util"
print "===TEST MATRIX DECOMPOSE==="
do
	local r2l_mat = math3d.matrix{s={-1.0, 1.0, 1.0}}
	local r2l_mat1 = math3d.matrix{s={1.0, 1.0, -1.0}}
	local r2l_mat2 = math3d.matrix{s={-1.0, -1.0, -1.0}}
	tu.print_mat(r2l_mat)
	tu.print_mat(r2l_mat1)
	tu.print_mat(r2l_mat2)
end

print "===INVERSE Z PROJECTION MATRIX==="
do
	local frustum = {
		l = -1.0, r = 1.0, t = 1.0, b = -1.0,
		n = 1.0, f = 100
	}
	local invz_proj = math3d.projmat(frustum, true)
	local proj = math3d.projmat(frustum)

	local nearpt = math3d.vector(0.0, 0.0, 1.0, 1.0)
	local farpt = math3d.vector(0.0, 0.0, 100.0, 1.0)

	local middlept = math3d.vector(0.0, 0.0, (1.0+100)*0.5, 1.0)
	local quadpt = math3d.vector(0.0, 0.0, (1.0+100)*0.25, 1.0)
	local pp0 = math3d.transform(invz_proj, nearpt, 1)
	local pp1 = math3d.transform(invz_proj, farpt, 1)
	local pp2 = math3d.transform(invz_proj, middlept, 1)
	local pp3 = math3d.transform(invz_proj, quadpt, 1)
	print "invz projection point:"
	print(math3d.tostring(pp0))
	print(math3d.tostring(pp1))
	print(math3d.tostring(pp2))
	print(math3d.tostring(pp3))

	print "projection point:"
	pp0 = math3d.transform(proj, nearpt, 	1)
	pp1 = math3d.transform(proj, farpt, 	1)
	pp2 = math3d.transform(proj, middlept, 	1)
	pp3 = math3d.transform(proj, quadpt, 1)

	print(math3d.tostring(pp0))
	print(math3d.tostring(pp1))
	print(math3d.tostring(pp2))
	print(math3d.tostring(pp3))
end

print "===PROJECTIVE MATRIX WITH INFINITE FAR PLANE TEST==="
do
	local function compare_camera(inv_z, n, f, ortho)
		local p, infp
		if ortho then
			p = math3d.projmat({l=-1,r=1,t=-1,b=1,n=n,f=f,ortho=true}, inv_z)
			infp = math3d.projmat({l=-1,r=1,t=-1,b=1,n=n,f=f,ortho=true}, true)
		else
			p = math3d.projmat({n=n, f=f, fov=60, aspect=4/3}, inv_z)
			infp = math3d.projmat({n=n, f=f, fov=60, aspect=4/3}, inv_z, true)
		end
		local updir = math3d.vector(0, 1, 0)
		local eyepos = math3d.vector(0, 0, 0)
		local direction = math3d.normalize(math3d.vector(0, 0, 1))
		local v = math3d.lookto(eyepos, direction, updir)
		local vp = math3d.mul(p, v)
		local vinfp = math3d.mul(infp, v)

		local zz_n, ww_n = math3d.index(math3d.transform(vp, math3d.vector(0, 0, n), 1), 3, 4)
		local zzinf_n, wwinf_n = math3d.index(math3d.transform(vinfp, math3d.vector(0, 0, n), 1), 3, 4)

		local zz_m, ww_m = math3d.index(math3d.transform(vp, math3d.vector(0, 0, (n+f)*0.5), 1), 3, 4)
		local zzinf_m, wwinf_m = math3d.index(math3d.transform(vinfp, math3d.vector(0, 0, (n+f)*0.5), 1), 3, 4)

		local zz_f, ww_f = math3d.index(math3d.transform(vp, math3d.vector(0, 0, f), 1), 3, 4)
		local zzinf_f, wwinf_f = math3d.index(math3d.transform(vinfp, math3d.vector(0, 0, f), 1), 3, 4)

		local ndf_n, ndf_n_inf = zz_n / ww_n, zzinf_n / wwinf_n
		local ndf_m, ndf_m_inf = zz_m / ww_m, zzinf_m / wwinf_m
		local ndf_f, ndf_f_inf = zz_f / ww_f, zzinf_f / wwinf_f
		return ndf_n, ndf_n_inf, ndf_m, ndf_m_inf, ndf_f, ndf_f_inf
	end

	local ndf_n, ndf_n_inf, ndf_m, ndf_m_inf, ndf_f, ndf_f_inf = compare_camera(false, 0.1, 10000)
	ndf_n, ndf_n_inf, ndf_m, ndf_m_inf, ndf_f, ndf_f_inf= compare_camera(true, 0.1, 20000)
	ndf_n, ndf_n_inf, ndf_m, ndf_m_inf, ndf_f, ndf_f_inf = compare_camera(false, 0.01, 1000)
	ndf_n, ndf_n_inf, ndf_m, ndf_m_inf, ndf_f, ndf_f_inf = compare_camera(true, 0.01, 2000)
	ndf_n, ndf_n_inf, ndf_m, ndf_m_inf, ndf_f, ndf_f_inf = compare_camera(false, 0.1, 10000, true)
	ndf_n, ndf_n_inf, ndf_m, ndf_m_inf, ndf_f, ndf_f_inf= compare_camera(true, 0.1, 20000, true)
	ndf_n, ndf_n_inf, ndf_m, ndf_m_inf, ndf_f, ndf_f_inf = compare_camera(false, 0.01, 1000, true)
	ndf_n, ndf_n_inf, ndf_m, ndf_m_inf, ndf_f, ndf_f_inf = compare_camera(true, 0.01, 2000, true)
end