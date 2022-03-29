#pragma once 
#include "core/KtAdjGraphDenseImpl.h"
#include "core/KtAdjGraphSparseImpl.h"
#include "core/KtFlatGraphVectorImpl.h"
#include "core/KtGraph.h"


/// 稠密无向图
template<typename EDGE_TYPE, typename VERTEX_TYPE = void>
using GraphDx = KtGraph<KtAdjGraphDenseImpl<EDGE_TYPE, VERTEX_TYPE>, false, false, false>;

using GraphDi = GraphDx<int>;
using GraphDd = GraphDx<double>;
using GraphDf = GraphDx<float>;


/// 稠密有向图
template<typename EDGE_TYPE, typename VERTEX_TYPE = void>
using DigraphDx = KtGraph<KtAdjGraphDenseImpl<EDGE_TYPE, VERTEX_TYPE>, true, false, false>;

using DigraphDi = DigraphDx<int>;
using DigraphDd = DigraphDx<double>;
using DigraphDf = DigraphDx<float>;


/// 稀疏无向图
template<typename EDGE_TYPE, typename VERTEX_TYPE = void, bool alwaysSorted = false>
using GraphSx = KtGraph<KtAdjGraphSparseImpl<EDGE_TYPE, VERTEX_TYPE>, false, false, alwaysSorted>;

template<bool alwaysSorted = false>
using GraphSi = GraphSx<int, void, alwaysSorted>;

template<bool alwaysSorted = false>
using GraphSd = GraphSx<double, void, alwaysSorted>;

template<bool alwaysSorted = false>
using GraphSf = GraphSx<float, void, alwaysSorted>;


/// 稀疏有向图
template<typename EDGE_TYPE, typename VERTEX_TYPE = void, bool alwaysSorted = false>
using DigraphSx = KtGraph<KtAdjGraphSparseImpl<EDGE_TYPE, VERTEX_TYPE>, true, false, alwaysSorted>;

template<bool alwaysSorted = false>
using DigraphSi = DigraphSx<int, void, alwaysSorted>;

template<bool alwaysSorted = false>
using DigraphSd = DigraphSx<double, void, alwaysSorted>;

template<bool alwaysSorted = false>
using DigraphSf = DigraphSx<float, void, alwaysSorted>;


/// 平行无向图（允许重边/平行边）
template<typename EDGE_TYPE, typename VERTEX_TYPE = void, bool alwaysSorted = false>
using GraphPx = KtGraph<KtAdjGraphSparseImpl<EDGE_TYPE, VERTEX_TYPE>, false, true, alwaysSorted>;

template<bool alwaysSorted = false>
using GraphPi = GraphPx<int, void, alwaysSorted>;

template<bool alwaysSorted = false>
using GraphPd = GraphPx<double, void, alwaysSorted>;

template<bool alwaysSorted = false>
using GraphPf = GraphPx<float, void, alwaysSorted>;


/// 平行有向图（允许重边/平行边）
template<typename EDGE_TYPE, typename VERTEX_TYPE = void, bool alwaysSorted = false>
using DigraphPx = KtGraph<KtAdjGraphSparseImpl<EDGE_TYPE, VERTEX_TYPE>, true, true, alwaysSorted>;

template<bool alwaysSorted = false>
using DigraphPi = DigraphPx<int, void, alwaysSorted>;

template<bool alwaysSorted = false>
using DigraphPd = DigraphPx<double, void, alwaysSorted>;

template<bool alwaysSorted = false>
using DigraphPf = DigraphPx<float, void, alwaysSorted>;


/// 扁平无向图（允许重边/平行边）
template<typename EDGE_TYPE, typename VERTEX_TYPE = void, bool alwaysSorted = false>
using GraphFx = KtGraph<KtFlatGraphVectorImpl<EDGE_TYPE, VERTEX_TYPE>, false, true, alwaysSorted>;

template<bool alwaysSorted = false>
using GraphFi = GraphFx<int, void, alwaysSorted>;

template<bool alwaysSorted = false>
using GraphFd = GraphFx<double, void, alwaysSorted>;

template<bool alwaysSorted = false>
using GraphFf = GraphFx<float, void, alwaysSorted>;


/// 扁平有向图（允许重边/平行边）
template<typename EDGE_TYPE, typename VERTEX_TYPE = void, bool alwaysSorted = false>
using DigraphFx = KtGraph<KtFlatGraphVectorImpl<EDGE_TYPE, VERTEX_TYPE>, true, true, alwaysSorted>;

template<bool alwaysSorted = false>
using DigraphFi = DigraphFx<int, void, alwaysSorted>;

template<bool alwaysSorted = false>
using DigraphFd = DigraphFx<double, void, alwaysSorted>;

template<bool alwaysSorted = false>
using DigraphFf = DigraphFx<float, void, alwaysSorted>;
