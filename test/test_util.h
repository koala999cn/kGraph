#pragma once
#include <fstream>
#include <string>


// dump graph "g" to file "path"
template<typename GRAPH>
void dump(const GRAPH& g, const char* path, bool dumpValue)
{
    std::ofstream of(path);

    auto V = g.order();
    for (unsigned v = 0; v < V; v++) {
        auto iter = g.adjIter(v);
        std::string s = std::to_string(v);
        s += ", ";
        bool empty_line(true);
        for (; !iter.isEnd(); ++iter) {
            if (!g.isDigraph() && *iter < v)
                continue; // 对于无向图，只显示v <= w的边

            of << '(' << s << *iter;
            //if (dumpValue) of << ", " << iter.value();
            of << ") ";
            empty_line = false;
        }

        if (!empty_line)
            of << '\n';
    }
}


// 测试失败时调用该函数打印信息、退出程序
template<typename GRAPH>
void test_failed(const GRAPH& g, bool detail = false)
{
    printf("  > :( failed\n");
    printf("   dumping failed graph to 'graph_dump.txt'...\n");
    dump(g, "graph_dump.txt", detail);
    printf("press any key to exit.\n");
    fflush(stdout);
    getchar();
    exit(1);
}


bool almostEqual(double x1, double x2, double tol = 1e-10);


// dump the pathR to string
std::string format_pathR(const std::vector<unsigned>& pathR);
