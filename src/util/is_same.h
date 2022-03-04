#pragma once


// 判断两个图g1、g2是否相同
template<typename G1, typename G2>
bool is_same(const G1& g1, const G2& g2)
{
    if (g1.order() != g2.order() || g1.size() != g2.size())
        return false;

    for (unsigned v = 0; v < g1.order(); v++)
        for (unsigned w = 0; w < g1.order(); w++)
            if (g1.getEdge(v, w) != g2.getEdge(v, w))
                return false;

    return true;
}
