#include <stdio.h>
#include "../GraphX.h"
#include "../core/KtMinSpanTree.h"
#include "../core/KtWeightor.h"
#include "../util/randgen.h"
#include "test_util.h"


template<typename GRAPH, class WEIGHTOR>
void min_span_tree_test_(const GRAPH& g)
{
    KtMstPrim<GRAPH, WEIGHTOR> prim(g);
    KtMstKruskal<GRAPH, WEIGHTOR> kurs(g);
    KtMstBoruvka<GRAPH, WEIGHTOR> boru(g);
    KtMstPfs<GRAPH, WEIGHTOR> pfs(g);

    printf("      prim = %f, kurs = %f, boru = %f, pfs = %f", prim.distance(), kurs.distance(), boru.distance(), pfs.distance());
    fflush(stdout);

    if (!almostEqual(prim.distance(), kurs.distance()) || 
        !almostEqual(prim.distance(), boru.distance()) ||
        !almostEqual(prim.distance(), pfs.distance() )
        )
        test_failed(g);

    printf("  > passed\n"); fflush(stdout);
}


void min_span_tree_test()
{
    printf("minimum spanning tree test...\n");
    fflush(stdout);

    GraphDd g(8);
    g.addEdge(0, 1, 0.32), g.addEdge(0, 2, 0.29), g.addEdge(0, 5, 0.60), g.addEdge(0, 6, 0.51), g.addEdge(0, 7, 0.31);
    g.addEdge(1, 7, 0.21);
    g.addEdge(3, 4, 0.34), g.addEdge(3, 5, 0.18);
    g.addEdge(4, 5, 0.40), g.addEdge(4, 6, 0.51), g.addEdge(4, 7, 0.46);
    g.addEdge(6, 7, 0.25);
    printf("   specific graph V = %d, E = %d\n", g.order(), g.size());
    fflush(stdout);
    min_span_tree_test_<GraphDd, default_min_wtor<GraphDd>>(g);


    GraphDd dg = randgen<GraphDd>(300, 10000); 
    printf("   random graph V = %d, E = %d\n", dg.order(), dg.size());
    fflush(stdout);
    min_span_tree_test_<GraphDd, default_min_wtor<GraphDd>>(dg);
}
