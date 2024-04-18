local math3d = require "math3d"

print "==== array ====="
do
	local array = math3d.array_vector {
		{ 1,2,3,4 },
		math3d.vector ( 42, 0 , 0 ),
	}
	print(math3d.tostring(array))
	local arrays = math3d.serialize(array)
	array = math3d.array_vector(arrays)
	assert(math3d.array_size(array) == 2)
	print(math3d.tostring(math3d.array_index(array, 1)))

	local mat_array = math3d.array_matrix {
		{ t = { 1,2,3,4 } },
		{ s = 2 },
	}

	print(math3d.tostring(mat_array))

	local r = math3d.mul_array( math3d.matrix { t = { 4,3,2,1 } } ,mat_array)

	local tmp = math3d.array_matrix {
		{ s = 1},
		{ s = 2},
	}
	local output_ref = math3d.array_matrix_ref(math3d.value_ptr(tmp), 2)
	print("ARRAY", math3d.tostring(output_ref))
	print("ARRAY[1]", math3d.tostring(math3d.array_index(output_ref,1)))
	print("ARRAY[2]", math3d.tostring(math3d.array_index(output_ref,2)))
--	math3d.assign(outout_ref, math3d.constant "matrix")
--	print(math3d.tostring(tmp), math3d.tostring(output_ref))

	math3d.mul_array( math3d.matrix { s = 42 }, mat_array, output_ref)

	print(math3d.tostring(tmp))

	math3d.mul_array( mat_array, mat_array, output_ref)

	for i = 1, 2 do
		local m = math3d.array_index(mat_array, i)
		assert(math3d.tostring(math3d.mul(m, m)) == math3d.tostring(math3d.array_index(tmp, i)))
	end
end