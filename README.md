# Math3D

A fast C/Lua math library for topics in linear algebra.

## Installation

```bash
# Clone the repo
git clone [repository-url]

# Build
make
```

## Quick Start

```lua
local math3d = require "math3d"

-- Create vectors
local v1 = math3d.vector(1, 0, 0)
local v2 = math3d.vector(0, 1, 0)
local dot = math3d.dot(v1, v2)     -- dot product
local cross = math3d.cross(v1, v2)  -- cross product

-- Create transformed matrix
local mat = math3d.matrix()  -- identity matrix
local pos = math3d.vector(1, 2, 3)
local transformed = math3d.transform(mat, pos)
}
```

## Main Features

- Vector/matrix/quaternion math
- Transformations (rotation, scale, etc)
- Projection matrices (perspective/ortho)
- View matrices (lookAt/lookTo)
- Bounding boxes (AABB)
- Ray casting
- Frustum culling



## Memory Usage

The library uses reference counting and a mark/unmark. Most functions return temporary values that are valid until the next frame. For persistent values, use `math3d.mark()`


## Notes

- Vectors are 4D (x,y,z,w) but most operations treat them as 3D
- Matrices are 4x4
- Use 1 as W for points, 0 for vectors/directions
- Matrix storage is column-major

## License

[License info]
