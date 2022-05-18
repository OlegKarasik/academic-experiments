#include "square-shape.hpp"

#include <algorithm>
#include <fstream>
#include <iostream>
#include <random>
#include <sstream>
#include <string>

#include "graphs-generators.hpp"
#include "graphs-io.hpp"

int
main(int argc, char* argv[])
{
  size_t v = 4800;
  size_t e = size_t(((v * (v - 1)) / 2) * 0.8);

  utilz::square_shape<int>                                                                   random_adj;
  std::vector<utilz::graphs::generators::promised_path<utilz::square_shape<int>::size_type>> vec;

  utilz::procedures::square_shape_set_size<utilz::square_shape<int>> s_size;
  utilz::procedures::square_shape_set<utilz::square_shape<int>>      s_value;
  utilz::graphs::generators::random_graph(
    v, e, vec, random_adj, s_size, s_value, utilz::graphs::generators::directed_acyclic_graph_tag());

  std::mt19937_64                    distribution_engine;
  std::uniform_int_distribution<int> vertex_distribution(1, 15);

  for (auto i = 0; i < random_adj.size(); ++i)
    for (auto j = 0; j < random_adj.size(); ++j)
      if (random_adj.at(i, j) == 1)
        random_adj.at(i, j) = vertex_distribution(distribution_engine);

  utilz::procedures::square_shape_get_size<utilz::square_shape<int>> g_size;
  utilz::procedures::square_shape_get<utilz::square_shape<int>>      g_value;

  std::ofstream output("D:\\myfile.g", std::ios_base::binary);

  utilz::graphs::io::print_matrix(output, true, random_adj, g_size, g_value);
}
