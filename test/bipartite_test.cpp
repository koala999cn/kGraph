#include <stdio.h>
#include "../src/GraphX.h"
#include "../src/core/KtBipartite.h"
#include "test_util.h"


void bipartite_test()
{
    printf("bipartite test...\n");
    fflush(stdout);

    GraphDi g(9);
    g.addEdge(0, 5);
    g.addEdge(1, 5), g.addEdge(1, 6);
    g.addEdge(2, 7), g.addEdge(2, 8);
    g.addEdge(3, 6);
    g.addEdge(4, 5), g.addEdge(4, 8);

    printf("   specific graph V = %d, E = %d", g.order(), g.size());
    fflush(stdout);

    KtBipartite<GraphDi> b(g);
    if (!b.bipartite())
        test_failed(g);

    for (size_t i = 0; i < 5; i++)
        if (b.color(i) != 1)
            test_failed(g);

    for (size_t i = 5; i < 9; i++)
        if (b.color(i) != 2)
            test_failed(g);

    printf("  > passed\n"); fflush(stdout);
}