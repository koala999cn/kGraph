#include <stdio.h>
#include "GraphX.h"
#include "core/KtCutPoints.h"
#include "util/randgen.h"
#include "util/make_connect.h"
#include "util/is_connected.h"
#include "test_util.h"


template<typename GRAPH>
void cutpoints_test_(const GRAPH& g)
{
    assert(is_connected(g));
    KtCutPoints c(g);

    printf("  @%d cutpoints", unsigned(c.size()));
    fflush(stdout);

    for (unsigned v = 0; v < g.order(); v++) {
        bool cut = std::find(c.begin(), c.end(), v) != c.end();
        GRAPH gn(g);
        gn.eraseVertex(v);
        bool con = is_connected(gn);
        if(cut && con || !cut && !con)
            test_failed(gn);
    }
}


void cutpoints_test()
{
    printf("cutpoints test...\n");
    fflush(stdout);

    GraphDi g(13);
    g.addEdge(0, 1), g.addEdge(0, 5), g.addEdge(0, 6);
    g.addEdge(1, 2);
    g.addEdge(2, 6);
    g.addEdge(3, 4), g.addEdge(3, 5);
    g.addEdge(4, 5), g.addEdge(4, 9), g.addEdge(4, 11);
    g.addEdge(6, 7);
    g.addEdge(7, 8), g.addEdge(7, 10);
    g.addEdge(8, 10);
    g.addEdge(9, 11);
    g.addEdge(11, 12);

    printf("   specific graph V = %d, E = %d", g.order(), g.size());
    fflush(stdout);
    cutpoints_test_(g);
    printf("  > passed\n"); fflush(stdout);


    GraphDd rg = randgen<GraphDd>(100, 150);
    make_connect(rg);
    printf("   random graph V = %d, E = %d", rg.order(), rg.size());
    fflush(stdout);
    cutpoints_test_(rg);
    printf("  > passed\n"); fflush(stdout);

}
