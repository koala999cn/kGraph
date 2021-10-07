#pragma once
#include <type_traits>
#include <cmath>


// 返回[x0, x1]区间的随机数
inline double rand(double x0, double x1)
{
	return x0 + (x1 - x0) * rand() / RAND_MAX;
}


// 以p的概率返回true.
// 0 <= p <= 1
inline bool rand_p(double p)
{
	auto r = rand();
	return r <= RAND_MAX * p;
}


template<typename GRAPH>
GRAPH randgen(unsigned V, unsigned E)
{
	GRAPH g(V);
	double p = static_cast<double>(E) / (V * V);
	if (!g.isDigraph()) p *= 2; // 对于无向图，边的生成概率翻倍
	for (unsigned i = 0; i < V; i++) {
		unsigned jMax = g.isDigraph() ? V : i + 1;
		for (unsigned j = 0; j < jMax; j++)
			if (rand_p(p)) {
				int r = 0;
				while (r == 0) r = rand();
				using edge_type = typename GRAPH::edge_type;
				edge_type val(std::is_floating_point<edge_type>::value ? edge_type(r) / RAND_MAX : r);
				g.addEdge(i, j, val);
			}
	}

	return g;
}