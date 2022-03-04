#include <stdio.h>
#include "../src/GraphX.h"
#include "../src/core/KtStronglyConnected.h"
#include "../src/util/randgen.h"
#include "test_util.h"


template<typename GRAPH>
void strongly_connected_test_(const GRAPH& g)
{
    KtStronglyConnectedKos<GRAPH> kos(g);
    KtStronglyConnectedTar<GRAPH> tar(g);
    KtStronglyConnectedGab<GRAPH> gab(g);
    
    printf("      kos = %d, tar = %d, gab = %d", kos.count(), tar.count(), gab.count());
    fflush(stdout);

    if (kos.count() != tar.count() || kos.count() != gab.count())
        test_failed(g);

    printf("  > passed\n"); fflush(stdout);
}


void strongly_connected_test()
{
    printf("strongly connected test...\n");
    fflush(stdout);

    DigraphDi g(13);
    g.addEdge(0, 1), g.addEdge(0, 5), g.addEdge(0, 6);
    g.addEdge(2, 0), g.addEdge(2, 3);
    g.addEdge(3, 2), g.addEdge(3, 5);
    g.addEdge(4, 2), g.addEdge(4, 3), g.addEdge(4, 11);
    g.addEdge(5, 4);
    g.addEdge(6, 4), g.addEdge(6, 9);
    g.addEdge(7, 6), g.addEdge(7, 8);
    g.addEdge(8, 7), g.addEdge(8, 9);
    g.addEdge(9, 10), g.addEdge(9, 11);
    g.addEdge(10, 12);
    g.addEdge(11, 12);
    g.addEdge(12, 9);

    printf("   specific digraph V = %d, E = %d\n", g.order(), g.size());
    fflush(stdout);
    strongly_connected_test_(g);

    DigraphDi dg = randgen<DigraphDi>(300, 1200); 
    printf("   random digraph V = %d, E = %d\n", dg.order(), dg.size());
    fflush(stdout);
    strongly_connected_test_(dg);
}

