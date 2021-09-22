#pragma once 
#include "./core/KtGraphSparse.h"
#include "./core/KtGraphDense.h"
#include "./core/KtGraphImpl.h"
#include "./core/KtGraphVoa.h"
#include "./core/KtSubGraph.h"


/// 稠密无向图
template<typename T>
using GraphDx = KtGraphImpl<KtGraphDense<T, false>>;

using GraphDi = GraphDx<int>;
using GraphDd = GraphDx<double>;
using GraphDf = GraphDx<float>;


/// 稠密有向图
template<typename T>
using DigraphDx = KtGraphImpl<KtGraphDense<T, true>>;

using DigraphDi = DigraphDx<int>;
using DigraphDd = DigraphDx<double>;
using DigraphDf = DigraphDx<float>;


/// 稀疏无向图
template<typename T>
using GraphSx = KtGraphImpl<KtGraphSparse<T, false>>;

using GraphSi = GraphSx<int>;
using GraphSd = GraphSx<double>;
using GraphSf = GraphSx<float>;


/// 稀疏有向图
template<typename T>
using DigraphSx = KtGraphImpl<KtGraphSparse<T, true>>;

using DigraphSi = DigraphSx<int>;
using DigraphSd = DigraphSx<double>;
using DigraphSf = DigraphSx<float>;


/// 平行无向图（允许重边/平行边）
template<typename T>
using GraphPx = KtGraphImpl<KtGraphSparse<T, false, true>>;

using GraphPi = GraphPx<int>;
using GraphPd = GraphPx<double>;
using GraphPf = GraphPx<float>;


/// 平行有向图（允许重边/平行边）
template<typename T>
using DigraphPx = KtGraphImpl<KtGraphSparse<T, true, true>>;

using DigraphPi = DigraphPx<int>;
using DigraphPd = DigraphPx<double>;
using DigraphPf = DigraphPx<float>;



/// 顶点对象图

template<typename VERTEX_TYPE, typename EDGE_TYPE>
using GraphDv = KtGraphVoa<VERTEX_TYPE, GraphDx<EDGE_TYPE>>;

template<typename VERTEX_TYPE, typename EDGE_TYPE>
using DigraphDv = KtGraphVoa<VERTEX_TYPE, DigraphDx<EDGE_TYPE>>;


template<typename VERTEX_TYPE, typename EDGE_TYPE>
using GraphSv = KtGraphVoa<VERTEX_TYPE, GraphSx<EDGE_TYPE>>;

template<typename VERTEX_TYPE, typename EDGE_TYPE>
using DigraphSv = KtGraphVoa<VERTEX_TYPE, DigraphSx<EDGE_TYPE>>;


template<typename VERTEX_TYPE, typename EDGE_TYPE>
using GraphPv = KtGraphVoa<VERTEX_TYPE, GraphPx<EDGE_TYPE>>;

template<typename VERTEX_TYPE, typename EDGE_TYPE>
using DigraphPv = KtGraphVoa<VERTEX_TYPE, DigraphPx<EDGE_TYPE>>;


