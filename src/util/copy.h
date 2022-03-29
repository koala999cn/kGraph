#pragma once
#include <assert.h>
#include "../core/KtWeightor.h"


template<typename SRC_G, typename DST_G, 
    typename WEIGHTOR = KtWeightSelf<typename SRC_G::edge_type>>
DST_G copy(const SRC_G& src)
{
    DST_G dst;
    dst.reserve(src.order(), src.size());
    dst.reset(src.order()); 
    for (unsigned v = 0; v < src.order(); v++) {
        unsigned wMax = dst.isDigraph() ? unsigned(src.order()) : v + 1;
        for (unsigned w = 0; w < wMax; w++)
            for (auto r = src.edges(v, w); !r.empty(); ++r)
                dst.addEdge(v, w, WEIGHTOR{}(*r));

        if constexpr (SRC_G::hasVertex() && DST_G::hasVertex())
            dst.vertexAt(v) = src.vertexAt(v);
    }

    return dst;
}


template<typename SRC_G, typename DST_G,
    typename WEIGHTOR = KtWeightSelf<typename SRC_G::edge_type>>
void copy(const SRC_G& src, DST_G& dst)
{
    dst = copy<SRC_G, DST_G, WEIGHTOR>(src);
}