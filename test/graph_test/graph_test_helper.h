#include <stdio.h>
#include <vector>
#include <map>
#include "util/copy.h"
#include "util/randgen.h"
#include "test_util.h"


template<typename G1, typename G2>
bool equal_test(const G1& g1, const G2& g2)
{
    if (g1.order() != g2.order()) {
        printf("order disagree: %d vs %d ", 
            static_cast<unsigned>(g1.order()), 
            static_cast<unsigned>(g2.order()));
        return false;
    }


    if (g1.size() != g2.size()) {
        printf("size disagree: %d vs %d ", g1.size(), g2.size());
        return false;
    }


    // 各顶点的出度比较
    for (unsigned v = 0; v < g1.order(); v++) {
        if (g1.outdegree(v) != g2.outdegree(v)) {
            printf("outdegree of vertex %d disagree: %d vs %d ", v, 
                g1.outdegree(v), g2.outdegree(v));
            return false;
        }
    }

    // 各顶点的入度比较
    for (unsigned v = 0; v < g1.order(); v++) {
        if (g1.indegree(v) != g2.indegree(v)) {
            printf("indegree of vertex %d disagree: %d vs %d ", v,
                g1.indegree(v), g2.indegree(v));
            return false;
        }
    }

    // 各顶点的度比较
    for (unsigned v = 0; v < g1.order(); v++) {
        if (g1.degree(v) != g2.degree(v)) {
            printf("degree of vertex %d disagree: %d vs %d ", v,
                g1.degree(v), g2.degree(v));
            return false;
        }
    }

    // 各顶点对的edges比较
    for (unsigned v = 0; v < g1.order(); v++) {
        for (unsigned w = 0; w < g1.order(); w++) {
            if (g1.hasEdge(v, w) != g2.hasEdge(v, w)) {
                printf("hasEdge(%d, %d) disagree: %s vs %s",
                    v, w,
                    g1.hasEdge(v, w) ? "true" : "false",
                    g2.hasEdge(v, w) ? "true" : "false");
                return false;
            }

            auto r1 = g1.edges(v, w);
            auto r2 = g2.edges(v, w);
            if (r1.size() != r2.size()) {
                printf("number of edges between vertex %d and %d disagree: %d vs %d",
                    v, w, int(r1.size()), int(r2.size()));
                return false;
            }

            std::vector<typename G1::edge_type> e1;
            std::vector<typename G2::edge_type> e2;
            for (; !r1.empty(); ++r1, ++r2)
                e1.push_back(*r1), e2.push_back(*r2);

            std::sort(e1.begin(), e1.end());
            std::sort(e2.begin(), e2.end());
            if (e1 != e2) {
                printf("value of edges between vertex %d and %d disagree", v, w);
                return false;
            }

            if constexpr (g1.isMultiEdges()) {
                for (auto e : e1) {
                    if (!g1.hasEdge(v, w, e)) {
                        printf("g1.hasEdge(%d, %d) assertion failed", v, w);
                        return false;
                    }
                }
            }

            if constexpr (g2.isMultiEdges()) {
                for (auto e : e2) {
                    if (!g2.hasEdge(v, w, e)) {
                        printf("g2.hasEdge(%d, %d) assertion failed", v, w);
                        return false;
                    }
                }
            }
        }
    }

    // 各顶点值比较
    if constexpr (!std::is_void_v<typename G1::vertex_type>
        && !std::is_void_v<typename G2::vertex_type>) {
        for(unsigned v = 0; v < g1.order(); v++)
            if (g1.vertexAt(v) != g2.vertexAt(v)) {
                printf("value of vertex %d disagree", v);
                return false;
            }
    }

    return true;
}


template<typename G1, typename G2>
void graph_test_helper()
{
    // 控制顶点数量在200-800范围内
    unsigned V = rand();
    V %= 800;
    if (V < 200) V += 200;

    G1 g1(V); G2 g2(V);

    printf("      random init graph: "); fflush(stdout);
    auto E = V * V / 10;
    if(E > 60000) E = 60000; // 不超过60000条边
    g1 = randgen<G1>(V, E);
    printf("V = %d, E = %d ", static_cast<unsigned>(g1.order()), g1.size());  fflush(stdout);
    copy(g1, g2);

    if (!equal_test(g1, g2))
        test_failed(g1, g2);
    printf("  > passed\n"); fflush(stdout);


    printf("      random erase edges: ");  fflush(stdout);
    for (unsigned i = 0; i < V; i++)
        for (unsigned j = 0; j < V; j++)
            if (g1.hasEdge(i, j) && rand_p(0.5)) { // 以50%的概率删除边
                if constexpr (g1.isMultiEdges() && g2.isMultiEdges()) {
                    decltype(auto) r = g1.edges(i, j);
                    if (r.size() > 1) { // 多边
                        // 删除特定边
                        int pos = rand() % r.size();
                        while (pos--)
                            ++r;

                        auto val = *r;
                        g1.eraseEdge(i, j, val);
                        g2.eraseEdge(i, j, val);

                        continue;
                    }
                }

                g1.eraseEdge(i, j);
                g2.eraseEdge(i, j);
            }
    if (!equal_test(g1, g2))
        test_failed(g1, g2);
    printf("  > passed\n"); fflush(stdout);


    printf("      random set edges: "); fflush(stdout);
    for (unsigned i = 0; i < V; i++)
        for (unsigned j = 0; j < V; j++)
            if (g1.hasEdge(i, j)) {
                float d = static_cast<float>(rand(0.1, 1.0));

                auto edges = g1.edges(i, j);
                auto e = *edges;
                    
                if constexpr (g1.isMultiEdges())
                    g1.setEdge(i, j, e, d);
                else
                    g1.setEdge(i, j, d);

                if constexpr (g2.isMultiEdges())
                    g2.setEdge(i, j, e, d);
                else
                    g2.setEdge(i, j, d);
            }
    if (!equal_test(g1, g2))
        test_failed(g1, g2);
    printf("  > passed\n"); fflush(stdout);


    printf("      random add edges: "); fflush(stdout);
    for (unsigned i = 0; i < V; i++)
        for (unsigned j = 0; j < V; j++)
            if (!g1.hasEdge(i, j) && rand_p(0.1)) { // 以10%的概率添加边
                int r = 0;
                while (r == 0) r = rand();
                using edge_type = typename G1::edge_type;
                edge_type val(std::is_floating_point<edge_type>::value ? edge_type(r) / RAND_MAX : r);

                g1.addEdge(i, j, val);
                g2.addEdge(i, j, val);
            }
    if (!equal_test(g1, g2))
        test_failed(g1, g2);
    printf("  > passed\n"); fflush(stdout);


    printf("      random erase vertex: "); fflush(stdout);
    for (unsigned i = 0; i < V / 10; i++) { // 删除1/10的顶点
        unsigned v = rand() % g1.order();
        unsigned numVertex(g1.order());
        unsigned numEdge(g1.size());
        unsigned degree = g1.degree(v);
        g1.eraseVertex(v); g2.eraseVertex(v);
        if (g1.order() != numVertex - 1) {
            printf("number of g1's vertex disagree, expected %d but got %d", 
                numVertex - 1, g1.order());
            test_failed(g1);
        }

        // 仅对无向图进行边数量一致性的检测，对于有向图g1.size() == numEdge - degree不成立（考虑自环）
        if (!g1.isDigraph() && g1.size() != numEdge - degree) {
            printf("number of g1's edges disagree, expected %d but got %d", 
                numEdge - degree, g1.size());
            test_failed(g1);
        }

        if (g2.order() != numVertex - 1) {
            printf("number of g2's vertex disagree, expected %d but got %d", 
                numVertex - 1, g2.order());
            test_failed(g1);
        }
        if (!g2.isDigraph() && g2.size() != numEdge - degree) {
            printf("number of g2's edges disagree, expected %d but got %d", 
                numEdge - degree, g2.size());
            test_failed(g2);
        }
    }
    if (!equal_test(g1, g2))
        test_failed(g1, g2);
    printf("  > passed\n"); fflush(stdout);

    printf("      random add vertex: from %d ", g1.order()); fflush(stdout);
    auto n = rand();
    n %= 20; n += 10;
    while (n-- != 0) {
        if constexpr (!g1.hasVertex())
            g1.addVertex();
        else
            g1.addVertex(typename G1::vertex_type{});


        if constexpr (!g2.hasVertex())
            g2.addVertex();
        else
            g2.addVertex(typename G2::vertex_type{});
    }

    printf("to %d ", g1.order()); fflush(stdout);
    if (!equal_test(g1, g2))
        test_failed(g1, g2);
    printf("  > passed\n"); fflush(stdout);


    printf("      random reset graph: "); fflush(stdout);
    n = rand();
    n %= 200;
    n += int(g1.order()) - 100;
    if (n < 0) n = 0;
    g1.reset(n); g2.reset(n);
    if (!equal_test(g1, g2))
        test_failed(g1, g2);
    printf("  > passed\n"); fflush(stdout);
}
