#pragma once
#include "../core/KtDfsIter.h"


template<typename GRAPH>
bool has_self_loop(const GRAPH& g) 
{
    auto V = g.order();
    for(decltype(V) i = 0; i < V; i++) 
        if(g.hasEdge(i, i)) 
            return true;

    return false;
}


// 删除自环
template<typename GRAPH>
void erase_self_loop(GRAPH& g) 
{
    auto V = order();
    for(decltype(V) i = 0; i < V; i++)
        if(g.hasEdge(i, i))
            g.eraseEdge(i, i); 
}


// 一个有向图是DAG，当且仅当在使用DFS检查每条边时未遇到任何回边
template<typename GRAPH>
bool has_loop(const GRAPH& g) 
{
    KtDfsIter<const GRAPH, true, true, true> iter(g, 0);
    while(!iter.isEnd()) {
        if(iter.isBack())
            return true;
        ++iter;
    }       

    return false;
}
    

// 删除环
template<typename GRAPH>
void erase_loop(GRAPH& g) 
{
    // 删除回边. 自环也是回边
    KtDfsIter<GRAPH, true, true> iter(g, 0);
    while(!iter.isEnd()) {
        if(iter.isBack())
            iter.erase();
        else
            ++iter;
    }
}