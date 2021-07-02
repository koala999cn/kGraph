#include <stdio.h>
#include "../GraphX.h"
#include "../core/KtMaxFlow.h"
#include "test_util.h"

template<typename GRAPH, class WEIGHTOR>
bool verify(const KtMaxFlow<GRAPH, WEIGHTOR>& mf, unsigned V, unsigned s, unsigned t)
{
	if (mf.outflow(s) != mf.inflow(t))
		return false;

	for (unsigned v = 0; v < V; v++)
		if (v != s && v != t && mf.netflow(v) != 0)
			return false;

	return true;
}


template<typename G>
void maxflow_test_(const G& g)
{
	assert(g.sources().size() == 1 && g.sinks().size() == 1);

	unsigned s = g.sources().front(), t = g.sinks().front();
	KtMaxFlowPfs<DigraphDi> pfs(g, s, t);
	printf("      pfs: outflow(s) = %d, inflow(t) = %d", pfs.outflow(s), pfs.inflow(t)); fflush(stdout);
	if (!verify(pfs, g.order(), s, t))
		test_failed(g);
	printf("  > passed\n"); fflush(stdout);


	KtMaxFlowBfs<DigraphDi> bfs(g, s, t);
	printf("      bfs: outflow(s) = %d, inflow(t) = %d", bfs.outflow(s), bfs.inflow(t)); fflush(stdout);
	if (!verify(bfs, g.order(), s, t))
		test_failed(g);
	printf("  > passed\n"); fflush(stdout);


	KtMaxFlowPre<DigraphDi> pfp(g, s, t);
	printf("      preflow: outflow(s) = %d, inflow(t) = %d", pfp.outflow(s), pfp.inflow(t)); fflush(stdout);
	if (!verify(pfp, g.order(), s, t))
		test_failed(g);
	printf("  > passed\n"); fflush(stdout);

	printf("      cross verify"); fflush(stdout);
	if (pfs.outflow(s) != bfs.outflow(s) || 
		pfs.outflow(s) != pfp.outflow(s))
		test_failed(g);
	printf("  > passed\n"); fflush(stdout);
}


void maxflow_test()
{
	printf("maxflow test...\n");
	fflush(stdout);

	DigraphDi g(6, 0);
	g.addEdge(0, 1, 2), g.addEdge(0, 2, 3);
	g.addEdge(1, 3, 3), g.addEdge(1, 4, 1);
	g.addEdge(2, 3, 1), g.addEdge(2, 4, 1);
	g.addEdge(3, 5, 2);
	g.addEdge(4, 5, 3);
	printf("   specific digraph V = %d, E = %d\n", g.order(), g.size());
	fflush(stdout);
	maxflow_test_(g);
}

