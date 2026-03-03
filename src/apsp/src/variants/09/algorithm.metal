#include <metal_stdlib>
#include <metal_integer>

using namespace metal;

kernel void calculate_diagonal(
  device int* memory [[buffer(0)]],
  constant int& sz   [[buffer(1)]],
  constant int& k    [[buffer(2)]],
  uint2  position    [[thread_position_in_grid]])
{
  memory[position.y * sz + position.x] = min(memory[position.y * sz + position.x], memory[position.y * sz + k] + memory[k * sz + position.x]);
}

kernel void calculate_cross_x(
  device int* memory [[buffer(0)]],
  constant int& z    [[buffer(1)]],
  constant int& x    [[buffer(2)]],
  constant int& y    [[buffer(3)]],
  constant int& sz   [[buffer(4)]],
  constant int& k    [[buffer(5)]],
  uint2  position    [[thread_position_in_grid]])
{
  memory[position.y * sz + position.x] = min(memory[position.y * sz + position.x], memory[position.y * sz + k] + memory[k * sz + position.x]);
}

kernel void calculate_cross_y(
  device int* memory [[buffer(0)]],
  constant int& z    [[buffer(1)]],
  constant int& x    [[buffer(2)]],
  constant int& y    [[buffer(3)]],
  constant int& sz   [[buffer(4)]],
  constant int& k    [[buffer(5)]],
  uint2  position    [[thread_position_in_grid]])
{
  memory[position.y * sz + position.x] = min(memory[position.y * sz + position.x], memory[position.y * sz + k] + memory[k * sz + position.x]);
}

kernel void calculate_peripheral(
  device int* memory [[buffer(0)]],
  constant int& z    [[buffer(1)]],
  constant int& x    [[buffer(2)]],
  constant int& y    [[buffer(3)]],
  constant int& sz   [[buffer(4)]],
  constant int& k    [[buffer(5)]],
  uint2  position    [[thread_position_in_grid]])
{
  memory[position.y * sz + position.x] = min(memory[position.y * sz + position.x], memory[position.y * sz + k] + memory[k * sz + position.x]);
}
