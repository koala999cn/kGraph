#include <stdio.h>
#include "../src/GraphX.h"
#include "../src/core/KtTransitiveClosure.h"
#include "../src/util/randgen.h"
#include "../src/util/loop.h"
#include "test_util.h"


template<typename GRAPH, typename TC1, typename TC2>
void equal_test(const GRAPH& g, const TC1& tc1, const TC2& tc2)
{
    for (unsigned i = 0; i < g.order(); i++)
        for (unsigned j = 0; j < g.order(); j++)
            if (tc1.reachable(i, j) != tc2.reachable(i, j))
                test_failed(g);
}


template<typename GRAPH>
void transitive_closure_test_(const GRAPH& g)
{
    bool doDag = !has_loop(g);

    KtTcWarshall<GRAPH> war(g);
    KtTcDfs<GRAPH> dfs(g);
    KtTcScc<GRAPH> scc(g);
    KtTcDag<GRAPH> *dag(nullptr);
    if(doDag) dag = new KtTcDag<GRAPH>(g);

    printf("      war vs. dfs");
    fflush(stdout);
    equal_test(g, war, dfs);
    printf("  > passed\n"); fflush(stdout);

    printf("      war vs. scc");
    fflush(stdout);
    equal_test(g, war, scc);
    printf("  > passed\n"); fflush(stdout);

    if (doDag) {
        printf("      war vs. dag");
        fflush(stdout);
        equal_test(g, war, *dag);
        printf("  > passed\n"); fflush(stdout);

        delete dag;
    }
}


void transitive_closure_test()
{
    printf("transitive closure test...\n");
    fflush(stdout);

    DigraphDi g(6);
    g.addEdge(0, 2), g.addEdge(0, 5);
    g.addEdge(1, 0);
    g.addEdge(2, 1);
    g.addEdge(3, 2), g.addEdge(3, 4);
    g.addEdge(4, 5);
    g.addEdge(5, 4);
    printf("   specific digraph V = %d, E = %d\n", g.order(), g.size());
    fflush(stdout);
    transitive_closure_test_(g);


    DigraphDi dag(13);
    dag.addEdge(0, 1), dag.addEdge(0, 2), dag.addEdge(0, 3), dag.addEdge(0, 5), dag.addEdge(0, 6);
    dag.addEdge(2, 3);
    dag.addEdge(3, 4), dag.addEdge(3, 5);
    dag.addEdge(4, 9);
    dag.addEdge(6, 4), dag.addEdge(6, 9);
    dag.addEdge(7, 6);
    dag.addEdge(8, 7);
    dag.addEdge(9, 10), dag.addEdge(9, 11), dag.addEdge(9, 12);
    dag.addEdge(11, 12);
    printf("   specific dag V = %d, E = %d\n", g.order(), g.size());
    fflush(stdout);
    transitive_closure_test_(dag);


    DigraphDi dg = randgen<DigraphDi>(200, 5000); 
    printf("   random digraph V = %d, E = %d\n", dg.order(), dg.size());
    fflush(stdout);
    transitive_closure_test_(dg);


    erase_loop(dg);
    printf("   random dag V = %d, E = %d\n", dg.order(), dg.size());
    fflush(stdout);
    transitive_closure_test_(dg);
}
