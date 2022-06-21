#include <stdio.h>
#include "GraphX.h"
#include "core/KtTopologySort.h"
#include "util/randgen.h"
#include "util/loop.h"
#include "test_util.h"


template<typename DAG>
void topology_sort_test_(const DAG& g)
{
    assert(g.isDigraph() && !has_loop(g));

    printf("      normal method");
    fflush(stdout);
    KtTopologySort<DigraphDi> ts(g);
    KtDfsIter<const DigraphDi, true, true> iter(g, 0);
    while (!iter.isEnd()) { // 迭代每条有向边
        if (ts.relabel(iter.from()) >= ts.relabel(*iter)) // 小编号指向大编号
            test_failed(g);
        ++iter;
    }
    printf("  > passed\n"); fflush(stdout);


    printf("      inverse method");
    fflush(stdout);
    KtTopologySortInv<DigraphDi> tsInv(g);
    KtDfsIter<const DigraphDi, true, true> iterInv(g, 0);
    while (!iterInv.isEnd()) { // 迭代每条有向边
        if (tsInv.relabel(iterInv.from()) <= tsInv.relabel(*iterInv)) // 大编号指向小编号
            test_failed(g);
        ++iterInv;
    }
    printf("  > passed\n"); fflush(stdout);
}



void topology_sort_test()
{
    printf("topology sort test...\n");
    fflush(stdout);

    DigraphDi g(13);
    g.addEdge(0, 1), g.addEdge(0, 2), g.addEdge(0, 3), g.addEdge(0, 5), g.addEdge(0, 6);
    g.addEdge(2, 3);
    g.addEdge(3, 4), g.addEdge(3, 5);
    g.addEdge(4, 9);
    g.addEdge(6, 4), g.addEdge(6, 9);
    g.addEdge(7, 6);
    g.addEdge(8, 7);
    g.addEdge(9, 10), g.addEdge(9, 11), g.addEdge(9, 12);
    g.addEdge(11, 12);

    printf("   specific dag V = %d, E = %d\n", g.order(), g.size());
    fflush(stdout);
    topology_sort_test_(g);


    DigraphDi rg = randgen<DigraphDi>(300, 30000);
    erase_loop(rg);
    printf("   random dag V = %d, E = %d\n", rg.order(), rg.size());
    fflush(stdout);
    topology_sort_test_(rg);
}
