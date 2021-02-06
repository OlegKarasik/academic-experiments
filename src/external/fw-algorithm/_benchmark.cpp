#include "benchmark/benchmark.h"

#include "algorithm.hpp"

#include "../../utilz/graphs-cio.hpp"
#include "../../utilz/graphs-fio.hpp"

#include "../../utilz/square-shape-io.hpp"
#include "../../utilz/square-shape.hpp"

#include <filesystem>

#ifdef _WIN32
#pragma comment(lib, "Shlwapi.lib")
#endif

using namespace utilz;
using namespace graphs::io;

template<typename T>
class Fixture : public benchmark::Fixture
{
//   // Constant value which indicates that there is no path between two vertexes.
//   // Please note: this value can be used ONLY when input paths are >= 0.
//   //
//   static constexpr T
//   no_path_value()
//   {
//     return ((std::numeric_limits<T>::max)() / 2) - 1;
//   };

//   using shape_memory       = square_shape_memory<T, no_path_value()>;
//   using shape_output_proxy = square_shape_graph_io_proxy<T>;
//   using shape_output       = square_shape_graph_out<T, shape_memory, shape_output_proxy>;

// private:
//   shape_memory m_m;

// public:
//   square_shape<T> shape;

//   Fixture()
//   {
//     // Read graph used for benchmarking
//     //
//     shape_output output(m_m);

//     fscan_graph<T, shape_output>("D:\\GitHub\\academic-experiments\\data\\_benchmarks\\direct-acyclic-graphs\\4800-4799.out.g", output);

//     this->shape = (square_shape<T>)this->m_m;
//   }
//   ~Fixture()
//   {
//   }
};

BENCHMARK_TEMPLATE_DEFINE_F(Fixture, ExecuteInt, int)
(benchmark::State& state)
{
  // for (auto _ : state)
  //   impl(this->shape);
}

BENCHMARK_REGISTER_F(Fixture, ExecuteInt)->UseRealTime()->Iterations(5);
