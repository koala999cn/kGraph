#include <stdio.h>
#include "../src/GraphX.h"
#include "../src/core/KtConnected.h"
#include "test_util.h"


void connected_component_test()
{
    printf("connected component test...\n");
    fflush(stdout);

    GraphDi g(13);
    g.addEdge(0, 1), g.addEdge(0, 9);
    g.addEdge(1, 4), g.addEdge(1, 9);
    g.addEdge(2, 7), g.addEdge(2, 10), g.addEdge(2, 12);
    g.addEdge(3, 12);
    g.addEdge(5, 12);
    g.addEdge(6, 10), g.addEdge(6, 12);
    g.addEdge(7, 10);
    g.addEdge(8, 11);

    printf("   specific graph V = %d, E = %d", g.order(), g.size());
    fflush(stdout);
    KtConnected<GraphDi> cc(g);
    if(cc.count() != 3)
        test_failed(g);
    printf("  > passed\n"); fflush(stdout);
}