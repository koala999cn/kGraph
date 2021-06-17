#include <stdio.h>
#include "../GraphX.h"
#include "../core/KtMinSpanTree.h"
#include "../core/KtWeightor.h"
#include "test_util.h"


template<typename GRAPH, class WEIGHTOR>
void min_span_tree_test_(const GRAPH& g)
{
	KtMstPrim<GRAPH, WEIGHTOR> prim(g);
	KtMstKruskal<GRAPH, WEIGHTOR> kurs(g);
	KtMstBoruvka<GRAPH, WEIGHTOR> boru(g);

	printf("      prim = %f, kurs = %f, boru = %f", prim.dist(), kurs.dist(), boru.dist());
	fflush(stdout);

	if (!almostEqual(prim.dist(), kurs.dist()) || 
		!almostEqual(kurs.dist(), boru.dist()))
		test_failed(g);

	printf("  > passed\n"); fflush(stdout);
}


void min_span_tree_test()
{
	printf("minimum spanning tree test...\n");
	fflush(stdout);

	GraphDd g(8);
	g.addEdge(0, 1, 0.32), g.addEdge(0, 2, 0.29), g.addEdge(0, 5, 0.60), g.addEdge(0, 6, 0.51), g.addEdge(0, 7, 0.31);
	g.addEdge(1, 7, 0.21);
	g.addEdge(3, 4, 0.34), g.addEdge(3, 5, 0.18);
	g.addEdge(4, 5, 0.40), g.addEdge(4, 6, 0.51), g.addEdge(4, 7, 0.46);
	g.addEdge(6, 7, 0.25);
	printf("   specific graph V = %d, E = %d\n", g.order(), g.size());
	fflush(stdout);
	min_span_tree_test_<GraphDd, KtWeightorMin<KtWeightSelf<double>, KtAdder<double>>>(g);


	GraphDd dg = randGraph<GraphDd>(300, 10000); 
	printf("   random graph V = %d, E = %d\n", dg.order(), dg.size());
	fflush(stdout);
	min_span_tree_test_<GraphDd, KtWeightorMin<KtWeightSelf<double>, KtAdder<double>>>(dg);


#if 0
	KtMstPrim<GraphDd> prim(g);   
	assert(prim.ok());
	std::vector<GraphDd::edge> mst;
	for (unsigned i = 0; i < g.order() - 1; i++)
		mst.push_back(prim[i]);
	std::sort(mst.begin(), mst.end(), [](const auto& x, const auto& y) { return x.second < y.second; });
	assert(mst[0] == GraphDd::edge(7, 1));
	assert(mst[1] == GraphDd::edge(0, 2));
	assert(mst[2] == GraphDd::edge(4, 3));
	assert(mst[3] == GraphDd::edge(7, 4));
	assert(mst[4] == GraphDd::edge(3, 5));
	assert(mst[5] == GraphDd::edge(7, 6));
	assert(mst[6] == GraphDd::edge(0, 7));
#endif
}
