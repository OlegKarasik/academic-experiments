#include <metal_stdlib>
#include <metal_integer>

using namespace metal;

kernel void calculate(
  device int* i_row [[buffer(0)]],
  device int* k_row [[buffer(1)]],
  device int& k [[buffer(3)]],
  uint j [[thread_position_in_grid]])
{
  i_row[j] = min(i_row[j], i_row[k] + k_row[j]);
}
