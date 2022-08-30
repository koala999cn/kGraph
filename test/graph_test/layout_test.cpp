#include "GraphX.h"
#include "graph_test_helper.h"
#include "core/vertex_traits.h"


struct  KpVertexWithOdegree
{
    unsigned odegree;
};

template<>
struct vertex_traits<KpVertexWithOdegree>
{
    using vertex_type = KpVertexWithOdegree;

    unsigned& outdegree(KpVertexWithOdegree& v) {
        return v.odegree;
    }

    unsigned outdegree(const KpVertexWithOdegree& v) {
        return v.odegree;
    }
};

void layout_test()
{
    printf("layout test...\n"); fflush(stdout);

    printf("   GraphDf vs GraphSf...\n"); fflush(stdout);
    graph_test_helper<GraphDf, GraphSf<>>();

    printf("   GraphDf vs GraphDf with outdegree...\n"); fflush(stdout);
    graph_test_helper<GraphDf, GraphDx<float, KpVertexWithOdegree>>();

    printf("   GraphSf vs GraphFf...\n"); fflush(stdout);
    graph_test_helper<GraphSf<>, GraphFf<>>();

    printf("   GraphPf vs GraphFf...\n"); fflush(stdout);
    graph_test_helper<GraphPf<>, GraphFf<>>();

    printf("   DigraphDf vs DigraphSf...\n"); fflush(stdout);
    graph_test_helper<DigraphDf, DigraphSf<>>();

    printf("   DigraphDf vs DigraphDf with outdegree...\n"); fflush(stdout);
    graph_test_helper<DigraphDf, DigraphDx<float, KpVertexWithOdegree>>();

    printf("   DigraphSf vs DigraphFf...\n"); fflush(stdout);
    graph_test_helper<DigraphSf<>, DigraphFf<>>();

    printf("   DigraphPf vs DigraphFf...\n"); fflush(stdout);
    graph_test_helper<DigraphPf<>, DigraphFf<>>();


    printf("   GraphSf vs GraphSf<sorted>...\n"); fflush(stdout);
    graph_test_helper<GraphSf<>, GraphSf<true>>();

    printf("   GraphSf vs GraphFf<sorted>...\n"); fflush(stdout);
    graph_test_helper<GraphSf<>, GraphFf<true>>();

    printf("   GraphSf vs GraphPf<sorted>...\n"); fflush(stdout);
    graph_test_helper<GraphSf<>, GraphPf<true>>();

    printf("   DigraphSf vs DigraphSf<sorted>...\n"); fflush(stdout);
    graph_test_helper<DigraphSf<>, DigraphSf<true>>();

    printf("   DigraphSf vs DigraphFf<sorted>...\n"); fflush(stdout);
    graph_test_helper<DigraphSf<>, DigraphFf<true>>();

    printf("   DigraphSf vs DigraphPf<sorted>...\n"); fflush(stdout);
    graph_test_helper<DigraphSf<>, DigraphPf<true>>();
}
