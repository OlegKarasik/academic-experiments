#include <metal_stdlib>
#include <metal_integer>

using namespace metal;

kernel void calculate(
  device int* memory [[buffer(0)]],
  constant int& x    [[buffer(1)]],
  constant int& k    [[buffer(2)]],
  uint2  position    [[thread_position_in_grid]])
{
  memory[position.y * x + position.x] = min(memory[position.y * x + position.x], memory[position.y * x + k] + memory[k * x + position.x]);
}
