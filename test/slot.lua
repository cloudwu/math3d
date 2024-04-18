local math3d = require "math3d"

print "===INFO TEST==="

print("SLOT = ", math3d.info "slot")
print("REF = ", math3d.info "ref")
collectgarbage "collect"
print("REF = ", math3d.info "ref")
print("N = ", math3d.info "transient")
local m = math3d.marked_matrix { s = 1 , t = { 0,0,0 } }
print(math3d.tostring(m))
math3d.unmark(m)
print("N = ", math3d.info "transient")

do
	math3d.reset()
	print("LAST = ", math3d.info "last")
	for i = 1, 4096 do
		math3d.matrix {}
		math3d.vector { 0,0,0}
	end
	math3d.reset()
	print("LAST = ", math3d.info "last")

	print(math3d.tostring(math3d.vector(0,0,0)))
end

print("SLOT = ", math3d.info "slot")