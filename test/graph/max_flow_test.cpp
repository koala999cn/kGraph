#include <stdio.h>
#include "GraphX.h"
#include "core/KtMaxFlow.h"
#include "util/sources.h"
#include "util/sinks.h"
#include "util/feasible.h"
#include "util/bipartite_match.h"
#include "test_util.h"


template<typename G>
void maxflow_test_(const G& g)
{
    assert(sources(g).size() == 1 && sinks(g).size() == 1);

    unsigned s = sources(g).front(), t = sinks(g).front();
    KtMaxFlowPfs<DigraphDi> pfs(g, s, t);
    printf("      pfs: outflow(s) = %d, inflow(t) = %d", pfs.outflow(s), pfs.inflow(t)); fflush(stdout);
    if (!pfs.check(s, t))
        test_failed(g);
    printf("  > passed\n"); fflush(stdout);


    KtMaxFlowBfs<DigraphDi> bfs(g, s, t);
    printf("      bfs: outflow(s) = %d, inflow(t) = %d", bfs.outflow(s), bfs.inflow(t)); fflush(stdout);
    if (!bfs.check(s, t))
        test_failed(g);
    printf("  > passed\n"); fflush(stdout);


    KtMaxFlowPre<DigraphDi> pfp(g, s, t);
    printf("      preflow: outflow(s) = %d, inflow(t) = %d", pfp.outflow(s), pfp.inflow(t)); fflush(stdout);
    if (!pfp.check(s, t))
        test_failed(g);
    printf("  > passed\n"); fflush(stdout);

    printf("      cross verify"); fflush(stdout);
    if (pfs.outflow(s) != bfs.outflow(s) || 
        pfs.outflow(s) != pfp.outflow(s))
        test_failed(g);
    printf("  > passed\n"); fflush(stdout);
}


// TODO: 构造测试最大流算法的随机图
void maxflow_test()
{
    printf("maxflow test...\n");
    fflush(stdout);

    DigraphDi g(6);
    g.addEdge(0, 1, 2), g.addEdge(0, 2, 3);
    g.addEdge(1, 3, 3), g.addEdge(1, 4, 1);
    g.addEdge(2, 3, 1), g.addEdge(2, 4, 1);
    g.addEdge(3, 5, 2);
    g.addEdge(4, 5, 3);
    printf("   specific digraph V = %d, E = %d\n", g.order(), g.size());
    fflush(stdout);
    maxflow_test_(g);


    DigraphDx<int, int> gv(6);
    gv.addEdge(0, 1, 2), gv.addEdge(0, 2, 3);
    gv.addEdge(1, 3, 3), gv.addEdge(1, 4, 1);
    gv.addEdge(2, 3, 1), gv.addEdge(2, 4, 2);
    gv.addEdge(3, 5, 4);
    gv.addEdge(4, 5, 3);
    gv.vertexAt(0) = 3;
    gv.vertexAt(1) = 3;
    gv.vertexAt(3) = 1;
    gv.vertexAt(2) = -1;
    gv.vertexAt(4) = -1;
    gv.vertexAt(5) = -5;
    printf("    feasible test...");
    fflush(stdout);
    if(!feasible(gv))
        test_failed(gv);
    printf("  > passed\n"); fflush(stdout);


    GraphDx<bool> gm(12);
    gm.addEdge(0, 6), gm.addEdge(0, 7), gm.addEdge(0, 8);
    gm.addEdge(1, 6), gm.addEdge(1, 7), gm.addEdge(1, 11);
    gm.addEdge(2, 8), gm.addEdge(2, 9), gm.addEdge(2, 10);
    gm.addEdge(3, 6), gm.addEdge(3, 7);
    gm.addEdge(4, 9), gm.addEdge(4, 10), gm.addEdge(4, 11);
    gm.addEdge(5, 8), gm.addEdge(5, 10), gm.addEdge(5, 11);
    printf("    bipartite_match test...");
    fflush(stdout);
    auto res = bipartite_match(gm);
    if (res.size() != 6)
        test_failed(gm);
    printf("  > passed\n"); fflush(stdout);
}
