#include <stdio.h>
#include "../GraphX.h"
#include "../util/randgen.h"
#include "../util/is_same.h"
#include "test_util.h"


void sparse_graph_test()
{
    unsigned V = rand();
    V %= 200;
    if (V < 50) V += 50;

    printf("sparse graph test...\n");
    fflush(stdout);

    GraphDd dense = randgen<GraphDd>(V, 200);
    printf("   random add edges");
    fflush(stdout);
    GraphDd sparse = dense.copy<GraphDd>();
    if (!is_same(sparse, dense))
        test_failed(sparse);
    printf("  > passed\n"); fflush(stdout);


    printf("   random erase edges");
    fflush(stdout);
    for (unsigned i = 0; i < V; i++)
        for (unsigned j = 0; j < V; j++)
            if (sparse.hasEdge(i, j) && rand_p(0.5)) { // 以50%的概率删除边
                sparse.eraseEdge(i, j);
                dense.eraseEdge(i, j);
            }
    if (!is_same(sparse, dense))
        test_failed(sparse);
    printf("  > passed\n"); fflush(stdout);


    printf("   random set edges");
    fflush(stdout);
    for (unsigned i = 0; i < V; i++)
        for (unsigned j = 0; j < V; j++)
            if (sparse.hasEdge(i, j)) {
                double d = rand(0.0, 1.0);
                sparse.setEdge(i, j, d);
                dense.setEdge(i, j, d);
            }
    if (!is_same(sparse, dense))
        test_failed(sparse);
    printf("  > passed\n"); fflush(stdout);


    printf("   random re-add edges");
    fflush(stdout);
    for (unsigned i = 0; i < V; i++)
        for (unsigned j = 0; j < V; j++)
            if (!sparse.hasEdge(i, j) && rand_p(0.5)) { // 以50%的概率添加边
                sparse.addEdge(i, j);
                dense.addEdge(i, j);
            }
    if (!is_same(sparse, dense))
        test_failed(sparse);
    printf("  > passed\n"); fflush(stdout);

    printf("   random erase vertex");
    fflush(stdout);
    for (unsigned i = 0; i < V / 10; i++) { // 删除1/10的顶点
        unsigned v = rand() % dense.order();
        unsigned numVertex(dense.order());
        unsigned numEdge(dense.size());
        unsigned degree = dense.degree(v);
        sparse.eraseVertex(v); dense.eraseVertex(v);
        if(sparse.order() != numVertex - 1)
            test_failed(sparse);
        if (sparse.size() != numEdge - degree)
            test_failed(sparse);

        if (!is_same(sparse, dense))
            test_failed(sparse);
    }
    printf("  > passed\n"); fflush(stdout);

    printf("   add new vertex");
    fflush(stdout);
    sparse.addVertex(); dense.addVertex();
    if (!is_same(sparse, dense))
        test_failed(sparse);
    printf("  > passed\n"); fflush(stdout);
}
