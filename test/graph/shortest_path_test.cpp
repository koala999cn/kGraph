#include <stdio.h>
#include "GraphX.h"
#include "core/KtShortestPath.h"
#include "core/KtBfsIter.h"
#include "util/randgen.h"
#include "util/loop.h"
#include "util/is_connected.h"
#include "test_util.h"


// 判断图g是否含有负权值
template<typename GRAPH, typename WEIGHTOR>
bool hasNegWt(const GRAPH& g)
{
    KtBfsIter<const GRAPH, true, true> iter(g, 0);
    for (; !iter.isEnd(); ++iter)
        if (WEIGHTOR{}(iter.edge()) < 0)
            return true;

    return false;
}


// 负环检测
template<typename GRAPH>
bool hasNegLoop(const GRAPH& g)
{
    KtFsspFloyd<GRAPH> floyd(g);
    for (unsigned i = 0; i < g.order(); i++)
        if (floyd.distance(i, i) < 0)
            return true;

    return false;
}


template<typename GRAPH, typename SPTALL, typename SPT>
void equal_test(const GRAPH& g, unsigned v, const SPTALL& all, const SPT& single)
{
    for (unsigned w = 0; w < g.order(); w++) 
        if (!almostEqual(all.distance(v, w), single.distance(w))) {
            printf(" [DUMP: distance = %g vs %g", all.distance(v, w), single.distance(w));
            printf("  path = %s vs %s]", format_pathR(all.pathR(v, w)).c_str(), format_pathR(single.pathR(w)).c_str());
            test_failed(g);
        }
}


template<typename GRAPH, typename WEIGHTOR = default_wtor<GRAPH>>
void shortest_path_test_(const GRAPH& g)
{
    assert(is_connected(g));

    bool hasNegWt = ::hasNegWt<GRAPH, WEIGHTOR>(g);
    bool isDag = !has_loop(g);
    KtFsspFloyd<GRAPH, WEIGHTOR> floyd(g);

    
    printf("      floyd vs. bellman-ford");
    fflush(stdout);
    for (unsigned i = 0; i < g.order(); i++)
        equal_test(g, i, floyd, KtSsspBellmanFord<GRAPH, WEIGHTOR>(g, i));
    printf("  > passed\n"); fflush(stdout);


    if (!hasNegWt) {
        printf("      floyd vs. dijkstra");
        fflush(stdout);
        for (unsigned i = 0; i < g.order(); i++)
            equal_test(g, i, floyd, KtSsspDijkstra<GRAPH, WEIGHTOR>(g, i));
        printf("  > passed\n"); fflush(stdout);


        printf("      floyd vs. pfs");
        fflush(stdout);
        for (unsigned i = 0; i < g.order(); i++)
            equal_test(g, i, floyd, KtSsspPfs<GRAPH, WEIGHTOR>(g, i));
        printf("  > passed\n"); fflush(stdout);
    }


    printf("      floyd vs. bfs");
    fflush(stdout);
    for (unsigned i = 0; i < g.order(); i++) 
        equal_test(g, i, floyd, KtSsspBfs<GRAPH, WEIGHTOR>(g, i));
    printf("  > passed\n"); fflush(stdout);


    // dfs算法太慢，弃疗
    //printf("      floyd vs. dfs");
    //fflush(stdout);
    //for (unsigned i = 0; i < g.order(); i++)
    //    equal_test(g.order(), i, floyd, KtSsspDfs<GRAPH>(g, i));
    //printf("  > passed\n"); fflush(stdout);


    if (isDag) {
        printf("      floyd vs. ts(dag)");
        fflush(stdout);
        for (unsigned i = 0; i < g.order(); i++) 
            equal_test(g, i, floyd, KtSsspTs<GRAPH, WEIGHTOR>(g, i));
        printf("  > passed\n"); fflush(stdout);


        printf("      floyd vs. dfs(dag)");
        fflush(stdout);
        KtFsspDfs<GRAPH, WEIGHTOR> fdfs(g);
        for (unsigned v = 0; v < g.order(); v++)
            for (unsigned w = 0; w < g.order(); w++)
                if (!almostEqual(floyd.distance(v, w), fdfs.distance(v, w)))
                    test_failed(g);
        printf("  > passed\n"); fflush(stdout);
    }
}


void shortest_path_test()
{
    printf("shortest path test...\n");
    fflush(stdout);

    DigraphDd g(6);
    g.addEdge(0, 1, 0.41), g.addEdge(0, 5, 0.29); 
    g.addEdge(1, 2, 0.51), g.addEdge(1, 4, 0.32); 
    g.addEdge(2, 3, 0.50);
    g.addEdge(3, 0, 0.45), g.addEdge(3, 5, 0.38);
    g.addEdge(4, 2, 0.32), g.addEdge(4, 3, 0.36);
    g.addEdge(5, 1, 0.29), g.addEdge(5, 4, 0.20);
    printf("   specific digraph V = %d, E = %d\n", g.order(), g.size());
    fflush(stdout);
    shortest_path_test_(g);


    g.setEdge(3, 5, -g.getEdge(3, 5));
    g.setEdge(5, 1, -g.getEdge(5, 1));
    printf("   specific digraph with neg-weighted edges V = %d, E = %d\n", g.order(), g.size());
    fflush(stdout);
    shortest_path_test_(g);


    erase_loop(g);
    printf("   specific dag V = %d, E = %d\n", g.order(), g.size());
    fflush(stdout);
    shortest_path_test_(g);


    DigraphDd rg = randgen<DigraphDd>(100, 1000); 
    printf("   random digraph V = %d, E = %d\n", rg.order(), rg.size());
    fflush(stdout);
    shortest_path_test_(rg);


    // 构造无环图
    erase_loop(rg);
    printf("   random dag V = %d, E = %d\n", rg.order(), rg.size());
    fflush(stdout);
    shortest_path_test_(rg);

/*  对DAG进行测试貌似无意义，还是应该在有环图基础上构建负权值图
    TODO: 高效构建无负环的带负权值图的方法
    KtBfsIter<DigraphDd, true, true> iter(rg, 0);
    unsigned neg_wt(0);
    for (; !iter.isEnd(); ++iter)
        if (rand_p(0.1)) { // 将10%的边置为负权值
            rg.setEdge(iter.from(), *iter, -iter.value());
            ++neg_wt;
        }
    printf("   random dag with %d neg-weighted edges V = %d, E = %d\n", neg_wt, rg.order(), rg.size());
    fflush(stdout);
    shortest_path_test_(rg);*/
}
