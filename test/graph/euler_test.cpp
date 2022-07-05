#include <stdio.h>
#include "GraphX.h"
#include "core/KtEuler.h"
#include "test_util.h"
#include <sstream>


std::string print_path(const std::vector<unsigned>& path)
{
    std::ostringstream oss;

    for (auto v : path) 
        oss << v << " - ";

    auto s = oss.str();
    if (!s.empty())
        s.erase(s.size() - 3);

    return s;
}


void euler_test()
{
    printf("euler test...\n");
    fflush(stdout);

    // ÆßÇÅÎÊÌâ²âÊÔ
    {
        GraphPi<> g(5);
        g.addEdge(0, 1), g.addEdge(0, 1), g.addEdge(0, 2), g.addEdge(0, 2), g.addEdge(0, 4);
        g.addEdge(1, 3), g.addEdge(1, 3), g.addEdge(1, 4);
        g.addEdge(2, 3), g.addEdge(2, 3), g.addEdge(2, 4);
        g.addEdge(3, 4);
        printf("   seven bridges' graph V = %d, E = %d", g.order(), g.size());
        fflush(stdout);
        KtEuler euler(g);
        if (euler.test())
            test_failed(g);
        printf("  > passed\n"); fflush(stdout);
    }

    {
        GraphPi<> g(3);
        g.addEdge(0, 0), g.addEdge(0, 1), g.addEdge(0, 1), g.addEdge(0, 2), g.addEdge(0, 2);
        g.addEdge(1, 1), g.addEdge(1, 2), g.addEdge(1, 2);
        g.addEdge(2, 2);
        printf("   specific graph with euler cycle V = %d, E = %d", g.order(), g.size());
        fflush(stdout);
        KtEuler euler(g);
        if (euler.test() != 1)
            test_failed(g);
        auto cycle = euler.getCycle();
        printf("\n    %s  > passed\n", print_path(cycle).c_str()); fflush(stdout);
    }

    {
        GraphPi<> g(5);
        g.addEdge(0, 1), g.addEdge(0, 2), g.addEdge(0, 3);
        g.addEdge(1, 2);
        g.addEdge(3, 4);
        printf("   specific graph with euler path V = %d, E = %d", g.order(), g.size());
        fflush(stdout);
        KtEuler euler(g);
        unsigned startV;
        if (euler.test(&startV) != 2)
            test_failed(g);
        auto path = euler.getPath(startV);
        printf("\n    %s  > passed\n", print_path(path).c_str()); fflush(stdout);
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
        // 0 - 6 - 4 - 5 - 0 - 1 - 2 - 3 - 4 - 2 - 0
        printf("   specific digraph with euler cycle V = %d, E = %d", g.order(), g.size());
        fflush(stdout);
        KtEuler euler(g);
        if (euler.test() != 1)
            test_failed(g);
        auto cycle = euler.getCycle();
        printf("\n    %s  > passed\n", print_path(cycle).c_str()); fflush(stdout);
    }
}
