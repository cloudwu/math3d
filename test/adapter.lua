local math3d = require "math3d"

print "===ADAPTER==="
local adapter = require "math3d.adapter" (math3d._COBJECT)
local succ, testfunc = pcall(require, "math3d.adapter.test")

if succ then
	local vector 	= adapter.vector(testfunc.vector, 1)	-- convert arguments to vector pointer from 1
	local matrix1 	= adapter.matrix(testfunc.matrix1, 1, 1)	-- convert 1 mat
	local matrix2 	= adapter.matrix(testfunc.matrix2, 1, 2)	-- convert 2 mat
	local matrix 	= adapter.matrix(testfunc.matrix2, 1)	-- convert all mat
	local var 		= adapter.variant(testfunc.vector, testfunc.matrix1, 1)
	local format 	= adapter.format(testfunc.variant, testfunc.format, 2)
	local mvq 		= adapter.getter(testfunc.getmvq, "mvq")	-- getmvq will return matrix, vector, quat
	local matrix2_v = adapter.format(testfunc.matrix2, "mm", 1)
	local retvec 	= adapter.output_vector(testfunc.retvec, 1)

    local ref1 = math3d.ref(math3d.matrix(
        1.0, 0.0, 0.0, 0.0,
        0.0, 1.0, 0.0, 0.0,
        0.0, 0.0, 1.0, 0.0,
        1.0, 2.0, 3.0, 1.0))

    local ref2 = math3d.ref(math3d.vector(1.0, 2.0, 3.0, 1.0))
	print(vector(ref2, math3d.vector{1,2,3}))
	print(matrix1(ref1))
	print(matrix2(ref1,ref1))
	print(matrix2_v(ref1,ref1))
	print(matrix(ref1,ref1))
	print(var(ref1))
	print(var(ref2))
	print(format("mv", ref1, ref2))
	local m,v, q = mvq()
	print(math3d.tostring(m), math3d.tostring(v), math3d.tostring(q))

	local v1,v2 =retvec()
	print(math3d.tostring(v1), math3d.tostring(v2))
end