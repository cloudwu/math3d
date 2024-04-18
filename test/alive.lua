local math3d = require "math3d"

print "===ALIVE==="
do
	local v = math3d.vector(1,2,3,4)
	math3d.reset()

	print(math3d.tostring(v))

	v = math3d.live(v)

	math3d.reset()

	print(math3d.tostring(v))

	v = math3d.mark(v)

	math3d.reset()

	print(math3d.tostring(v))

	math3d.reset()

	math3d.unmark(v)

	print(math3d.tostring(v))
end