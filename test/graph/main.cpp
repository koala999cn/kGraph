#include <stdio.h>
#include <assert.h>


extern void layout_test();
extern void adj_iter_test();
extern void bfs_test();
extern void dfs_test();
extern void connected_component_test();
extern void bipartite_test();
extern void bridge_test();
extern void cutpoints_test();
extern void transitive_closure_test();
extern void topology_sort_test();
extern void strongly_connected_test();
extern void min_span_tree_test();
extern void shortest_path_test();
extern void maxflow_test();
extern void resort_test();
extern void euler_test();


int main(int argc, char const *argv[])
{
    euler_test(); printf("\n");
    layout_test(); printf("\n");
    adj_iter_test(); printf("\n");
    bfs_test(); printf("\n");
    dfs_test(); printf("\n");
    connected_component_test(); printf("\n");
    bipartite_test(); printf("\n");
    bridge_test(); printf("\n");
    cutpoints_test(); printf("\n");
    transitive_closure_test(); printf("\n");
    topology_sort_test(); printf("\n");
    strongly_connected_test(); printf("\n");
    min_span_tree_test(); printf("\n");
    shortest_path_test(); printf("\n");
    maxflow_test(); printf("\n");
    resort_test(); printf("\n");
    
    printf(" :) All passed! press any key to exit.\n");
    getchar();

    return 0;
}
