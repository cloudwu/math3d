# Math3D

A fast C/Lua library for linear algebra topics.
## Installation

```bash
git clone [repository-url]
make
```

## Basic Usage

```lua
local math3d = require "math3d"

-- Vectors
local vec = math3d.vector(1, 2, 3)      -- Create a 3D vector
local len = math3d.length(vec)          -- Get vector length
local norm = math3d.normalize(vec)       -- Normalize vector

-- Matrices and Transforms
local mat = math3d.matrix()             -- Create identity matrix
local transformed = math3d.transform(mat, vec)

```

## Features
- Vector/matrix/quaternion operations
- Transform and projection matrices
- Ray casting and intersection testing
- Frustum culling and AABB calculations
- Memory management with reference counting

## License
[License info needed]
