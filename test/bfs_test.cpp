#include <stdio.h>
#include "../src/GraphX.h"
#include "../src/core/KtBfsIter.h"
#include "../src/util/randgen.h"
#include "test_util.h"


template<typename GRAPH>
void bfs_test_(const GRAPH& g)
{
    printf("      normal mode");
    fflush(stdout);
    KtBfsIter<const GRAPH, true> bfs1(g, 0);
    std::vector<unsigned> v;
    for (; !bfs1.isEnd(); ++bfs1)
        v.push_back(*bfs1);
    if (v.size() != g.order())
        test_failed(g);
    printf("  > passed\n"); fflush(stdout);


    printf("      edge mode");
    fflush(stdout);
    KtBfsIter<const GRAPH, true, true> bfs2(g, 0);
    std::vector<std::pair<unsigned, unsigned>> e;
    for (; !bfs2.isEnd(); ++bfs2)
        e.push_back({ bfs2.from(), *bfs2 });
    if (e.size() != g.size())
        test_failed(g);
    for (auto i : e) {
        if(!g.hasEdge(i.first, i.second))
            test_failed(g);
    }

    printf("  > passed\n"); fflush(stdout);
}


void bfs_test()
{
    printf("bfs test...\n");
    fflush(stdout);

    GraphDi g(8);
    g.addEdge(0, 2), g.addEdge(0, 5), g.addEdge(0, 7);
    g.addEdge(1, 7);
    g.addEdge(2, 6);
    g.addEdge(3, 4), g.addEdge(3, 5);
    g.addEdge(4, 5), g.addEdge(4, 6), g.addEdge(4, 7);
    printf("   specific graph V = %d, E = %d\n", g.order(), g.size());
    fflush(stdout);
    bfs_test_(g);

    GraphDd ug = randgen<GraphDd>(300, 30000);
    printf("   random graph V = %d, E = %d\n", ug.order(), ug.size());
    fflush(stdout);
    bfs_test_(ug);

    DigraphDd dg = randgen<DigraphDd>(300, 30000);
    printf("   random digraph V = %d, E = %d\n", dg.order(), dg.size());
    fflush(stdout);
    bfs_test_(dg);
}

