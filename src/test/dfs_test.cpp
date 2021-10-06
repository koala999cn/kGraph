#include <stdio.h>
#include "../GraphX.h"
#include "../core/KtDfsIter.h"
#include "../util/randgen.h"
#include "test_util.h"


template<typename GRAPH>
void dfs_test_(const GRAPH& g)
{
    printf("      normal mode");
    fflush(stdout);
    KtDfsIter<const GRAPH, true> dfs1(g, 0);
    std::vector<unsigned> v;
    for (; !dfs1.isEnd(); ++dfs1) 
        v.push_back(*dfs1);
    if (v.size() != g.order())
        test_failed(g);
    printf("  > passed\n"); fflush(stdout);


    printf("      edge mode");
    fflush(stdout);
    KtDfsIter<const GRAPH, true, true> dfs2(g, 0);
    std::vector<std::pair<unsigned, unsigned>> e;
    for (; !dfs2.isEnd(); ++dfs2)
        e.push_back({ dfs2.from(), *dfs2 });
    if (e.size() != g.size())
        test_failed(g);
    for (auto i : e) {
        if (!g.hasEdge(i.first, i.second))
            test_failed(g);
    }
    printf("  > passed\n"); fflush(stdout);


    printf("      normal popping mode");
    fflush(stdout);
    KtDfsIter<const GRAPH, true, false, true> dfs3(g, 0);
    v.clear();
    for (; !dfs3.isEnd(); ++dfs3) 
        v.push_back(*dfs3);
    if (v.size() != 2 * g.order())
        test_failed(g);
    printf("  > passed\n"); fflush(stdout);
}


void dfs_test()
{
    printf("dfs test...\n");
    fflush(stdout);

    GraphDi sg(8);
    sg.addEdge(0, 2), sg.addEdge(0, 5), sg.addEdge(0, 7);
    sg.addEdge(1, 7);
    sg.addEdge(2, 6);
    sg.addEdge(3, 4), sg.addEdge(3, 5);
    sg.addEdge(4, 5), sg.addEdge(4, 6), sg.addEdge(4, 7);
    printf("   specific graph V = %d, E = %d\n", sg.order(), sg.size());
    fflush(stdout);
    dfs_test_(sg);

    DigraphSi sdg(13);
    sdg.addEdge(10, 12); sdg.addEdge(2, 3);
    sdg.addEdge(7, 6); sdg.addEdge(8, 7); sdg.addEdge(8, 9); sdg.addEdge(0, 6);
    sdg.addEdge(6, 4); sdg.addEdge(7, 8); sdg.addEdge(9, 10); sdg.addEdge(0, 1);
    sdg.addEdge(6, 9); sdg.addEdge(3, 2); sdg.addEdge(9, 11); sdg.addEdge(3, 5);
    sdg.addEdge(0, 5); sdg.addEdge(4, 2); sdg.addEdge(12, 9); sdg.addEdge(2, 0);
    sdg.addEdge(5, 4); sdg.addEdge(4, 11); sdg.addEdge(11, 12); sdg.addEdge(4, 3);
    printf("   specific digraph V = %d, E = %d\n", sdg.order(), sdg.size());
    fflush(stdout);
    dfs_test_(sdg);

    GraphDd rg = randgen<GraphDd>(300, 30000);
    printf("   random graph V = %d, E = %d\n", rg.order(), rg.size());
    fflush(stdout);
    dfs_test_(rg);

    DigraphDd rdg = randgen<DigraphDd>(300, 30000); 
    printf("   random digraph V = %d, E = %d\n", rdg.order(), rdg.size());
    fflush(stdout);
    dfs_test_(rdg);
}