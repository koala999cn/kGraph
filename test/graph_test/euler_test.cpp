#include <stdio.h>
#include "GraphX.h"
#include "core/KtEuler.h"
#include "util/degree.h"
#include "util/make_euler.h"
#include "test_util.h"
#include <sstream>


template<typename GRAPH>
bool is_euler_path(const GRAPH& g, const std::vector<unsigned>& path)
{
    std::vector<int> degs(g.order(), 0);
    for (unsigned v = 0; v < g.order(); v++)
        degs[v] = degree(g, v);

    for (unsigned i = 1; i < path.size(); i++) {
        if (!g.hasEdge(path[i - 1], path[i]))
            return false;
        --degs[path[i - 1]], --degs[path[i]];
    }

    for (auto d : degs)
        if (d != 0) return false;

    return true;
}


// r = 0，表示g为非欧拉图
// r = 1，表示g存在欧拉环
// r = 2，表示g存在欧拉路径
template<typename GRAPH>
void verify(const GRAPH& g, int r)
{
    unsigned v;
    KtEuler euler(g);
    if (euler.test(&v) != r)
        test_failed(g);

    if (r > 0) {
        auto path = r == 1 ? euler.getCycle() : euler.getPath(v);
        if(!is_euler_path(g, path))
            test_failed(g);
    }
    else {
        auto path = euler.getCycle();
        if (is_euler_path(g, path))
            test_failed(g);
    }

    printf("  > passed\n"); fflush(stdout);
}


void euler_test()
{
    printf("euler test...\n");
    fflush(stdout);

    // 七桥问题测试
    {
        GraphPi<> g(5);
        g.addEdge(0, 1), g.addEdge(0, 1), g.addEdge(0, 2), g.addEdge(0, 2), g.addEdge(0, 4);
        g.addEdge(1, 3), g.addEdge(1, 3), g.addEdge(1, 4);
        g.addEdge(2, 3), g.addEdge(2, 3), g.addEdge(2, 4);
        g.addEdge(3, 4);
        printf("   seven bridges' graph V = %d, E = %d", g.order(), g.size());
        fflush(stdout);
        verify(g, 0);
    }

    {
        GraphPi<> g(3);
        g.addEdge(0, 0), g.addEdge(0, 1), g.addEdge(0, 1), g.addEdge(0, 2), g.addEdge(0, 2);
        g.addEdge(1, 1), g.addEdge(1, 2), g.addEdge(1, 2);
        g.addEdge(2, 2);
        printf("   specific graph with euler cycle V = %d, E = %d", g.order(), g.size());
        fflush(stdout);
        verify(g, 1);
    }

    {
        GraphPi<> g(5);
        g.addEdge(0, 1), g.addEdge(0, 2), g.addEdge(0, 3);
        g.addEdge(1, 2);
        g.addEdge(3, 4);
        printf("   specific graph with euler path V = %d, E = %d", g.order(), g.size());
        fflush(stdout);
        verify(g, 2);
    }

    {
        auto g = make_euler<GraphPi<>>(100, 10000, true);
        printf("   random graph with euler cycle V = %d, E = %d", g.order(), g.size());
        fflush(stdout);
        verify(g, 1);
    }

    {
        auto g = make_euler<GraphPi<>>(100, 10000, false);
        printf("   random graph with euler path V = %d, E = %d", g.order(), g.size());
        fflush(stdout);
        verify(g, 2);
    }

    {   // from https://www.geeksforgeeks.org/hierholzers-algorithm-directed-graph/ 
        DigraphPi<> g(7);
        g.addEdge(0, 1), g.addEdge(0, 6);
        g.addEdge(1, 2);
        g.addEdge(2, 0), g.addEdge(2, 3);
        g.addEdge(3, 4);
        g.addEdge(4, 2), g.addEdge(4, 5);
        g.addEdge(5, 0);
        g.addEdge(6, 4);
        printf("   specific digraph with euler cycle V = %d, E = %d", g.order(), g.size());
        fflush(stdout);
        verify(g, 1);
    }
}
