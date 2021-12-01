#include <stdio.h>
#include "../GraphX.h"
#include "../util/randgen.h"
#include "../util/is_same.h"
#include "test_util.h"

using edges = std::vector<std::pair<std::pair<unsigned, unsigned>, double>>;


double getWeight(const edges& es, unsigned v, unsigned w)
{
	for (auto& i : es) {
		auto& pos = i.first;
		if (pos.first == v && pos.second == w)
			return i.second;
	}

	return 0;
}


template<typename GRAPH, typename ITER>
edges adj_iter_access_test_(GRAPH& g, ITER iter)
{
	edges es;

	for (; !iter.isEnd(); ++iter) {
		auto v = iter.from();
		auto w = iter.to();
		if (!g.hasEdge(v, w) ||
			g.getEdge(v, w) != iter.edge())
			test_failed(g);

		if(g.isDigraph() || v >= w)
			es.push_back({ {v, w}, iter.edge() });
	}

	return es;
}


template<typename GRAPH, typename ITER>
edges adj_iter_reedge_test_(GRAPH& g, ITER iter)
{
	edges es;

	for (; !iter.isEnd(); ++iter) {
		auto v = iter.from();
		auto w = iter.to();
		if ((g.isDigraph() || v >= w) && rand() < RAND_MAX / 4) { // 以25%的概率修改边权值
			double wt = double(rand()) / double(RAND_MAX);
			if (wt != g.nullEdge()) {
				iter.reedge(wt);
				es.push_back({ { v, w }, wt });
			}
		}
	}

	return es;
}


template<typename GRAPH, typename ITER>
std::pair<edges, unsigned> adj_iter_erase_test_(GRAPH& g, ITER iter)
{
	edges es;
	unsigned erased(0); // 连续删除的次数

	bool flag(false); // 上次是否执行删除操作
	for (; !iter.isEnd(); ) {
		auto v = iter.from();
		auto w = iter.to();
		if ((g.isDigraph() || v >= w) && rand() < RAND_MAX / 5) { // 以50%的概率删除边
			es.push_back({ { v, w }, iter.edge() });
			iter.erase();

			if (flag) ++erased;
			else flag = true;
		}
		else {
			++iter;
			flag = false;
		}
	}

	return{ es, erased };
}

template<typename GRAPH>
void adj_iter_test__(GRAPH& g, bool iterR)
{
	unsigned E(0);
	edges esAll;

	printf("         access");
	fflush(stdout);

	for (unsigned v = 0; v < g.order(); v++) {
		edges e;
		if (iterR)
			e = adj_iter_access_test_(g, g.adjIterR(v));
		else
			e = adj_iter_access_test_(g, g.adjIter(v));
		
		for (auto& i : e)
			esAll.push_back(i);
	}

	printf(" = %d", esAll.size());
	fflush(stdout);

	if(esAll.size() != g.size())
		test_failed(g);

	printf("  > passed\n"); fflush(stdout);


	printf("         reedge");
	fflush(stdout);

	edges esReedge;
	for (unsigned v = 0; v < g.order(); v++) {
		edges e;
		if (iterR)
			e = adj_iter_reedge_test_(g, g.adjIterR(v));
		else
			e = adj_iter_reedge_test_(g, g.adjIter(v));

		for (auto& i : e)
			esReedge.push_back(i);
	}
	
	printf(" = %d", esReedge.size());
	fflush(stdout);

	if (esAll.size() != g.size())
		test_failed(g);

	for (auto& i : esAll) {
		auto& pos = i.first;
		double wt = getWeight(esReedge, pos.first, pos.second);
		if (wt == 0)
			wt = i.second;

		if (g.getEdge(pos.first, pos.second) != wt) 
			test_failed(g);
	}

	printf("  > passed\n"); fflush(stdout);


	printf("         erase");
	fflush(stdout);

	edges esErased;
	unsigned continue_erased(0);

	for (unsigned v = 0; v < g.order(); v++) {
		std::pair<edges, unsigned> x;
		if (iterR) 
			x = adj_iter_erase_test_(g, g.adjIterR(v));
		
		else
			x = adj_iter_erase_test_(g, g.adjIter(v));
			

		for (auto& i : x.first)
			esErased.push_back(i);

		continue_erased += x.second;
	}

	printf(" = %d/%d", esErased.size(), continue_erased);
	fflush(stdout);

	if (esAll.size() - esErased.size() != g.size())
		test_failed(g);

	for (auto& i : esAll) {
		auto& pos = i.first;
		double wt = getWeight(esErased, pos.first, pos.second);
		if (wt != 0) { // 确认该条边已被删除
			if(g.hasEdge(pos.first, pos.second))
				test_failed(g);
		}
		else if (!g.hasEdge(pos.first, pos.second))
			test_failed(g);
	}

	printf("  > passed\n"); fflush(stdout);
}


template<typename GRAPH>
void adj_iter_test_(GRAPH& g)
{
	printf("      adj_vertex_iter\n");
	fflush(stdout);
	adj_iter_test__(g, false);

	printf("      adj_vertex_iter_r\n");
	fflush(stdout);
	adj_iter_test__(g, true);
}


void adj_iter_test()
{
	printf("adj vertex iter test...\n");
	fflush(stdout);

	unsigned V = rand() % 1000;
	GraphDd dense = randgen<GraphDd>(V, V * V / 64);
	printf("   dense graph V = %d, E = %d\n", dense.order(), dense.size());
	fflush(stdout);
	adj_iter_test_(dense);


	V = rand() % 1000;
	DigraphDd ddense = randgen<DigraphDd>(V, V * V / 64);
	printf("   dense graph V = %d, E = %d\n", ddense.order(), ddense.size());
	fflush(stdout);
	adj_iter_test_(ddense);


	V = rand() % 2000;
	GraphSd sparse = randgen<GraphSd>(V, V * V / 1000 + 50);
	printf("   sparse graph V = %d, E = %d\n", sparse.order(), sparse.size());
	fflush(stdout);
	adj_iter_test_(sparse);


	V = rand() % 2000;
	DigraphSd dsparse = randgen<DigraphSd>(V, V * V / 1000 + 50);
	printf("   sparse digraph V = %d, E = %d\n", dsparse.order(), dsparse.size());
	fflush(stdout);
	adj_iter_test_(dsparse);
}