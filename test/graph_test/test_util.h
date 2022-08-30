#pragma once
#include <fstream>
#include <string>
#include <vector>
#include "util/dump.h"


// dump graph "g" to file "path"
template<typename GRAPH>
void dump(const GRAPH& g, const char* path)
{
    std::ofstream of(path);
    dump(g, of);
}


// 测试失败时调用该函数打印信息、退出程序
template<typename GRAPH>
void test_failed(const GRAPH& g, bool detail = false)
{
    printf("  > :( failed\n");
    printf("   dumping failed graph to 'graph_dump.txt'...\n");
    dump(g, "graph_dump.txt");
    printf("press any key to exit.\n");
    fflush(stdout);
    getchar();
    exit(1);
}


template<typename G1, typename G2>
void test_failed(const G1& g1, const G2& g2)
{
    printf("  > :( failed\n");
    printf("   dumping failed graph to 'graph_dump1.txt & graph_dump2.txt'...\n");
    dump(g1, "graph_dump1.txt");
    dump(g2, "graph_dump2.txt");
    printf("press any key to exit.\n");
    fflush(stdout);
    getchar();
    exit(1);
}

bool almostEqual(double x1, double x2, double tol = 1e-10);


// dump the pathR to string
std::string format_pathR(const std::vector<unsigned>& pathR);
