#include <stdio.h>
#include "GraphX.h"
#include "core/KtTopologySort.h"
#include "core/KtBfsIter.h"
#include "util/resort.h"
#include "util/randgen.h"
#include "util/loop.h"
#include "test_util.h"


template<typename DAG>
void resort_test_(const DAG& g)
{
    KtTopologySort<DigraphDi> ts(g);
    DAG d(g);
    resort(d, ts.relabels());
    KtBfsIter<const DAG, true, true> iter(d, 0);
    while (!iter.isEnd()) { // 迭代每条有向边
        if (iter.from() > *iter) // 小编号指向大编号
            test_failed(d);
        ++iter;
    }
    printf("  > passed\n"); fflush(stdout);
}



void resort_test()
{
    printf("resort test...\n");
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

    printf("   specific dag V = %d, E = %d", g.order(), g.size());
    fflush(stdout);
    resort_test_(g);


    DigraphDi rg = randgen<DigraphDi>(300, 30000);
    erase_loop(rg);
    printf("   random dag V = %d, E = %d", rg.order(), rg.size());
    fflush(stdout);
    resort_test_(rg);
}
