#pragma once 
#include "./core/KtGraphSparse.h"
#include "./core/KtGraphDense.h"
#include "./core/KtGraphImpl.h"

// 稀疏无向图
template<typename T>
using GraphSx = KtGraphImpl<KtGraphSparse<T, false>>;

using GraphSi = GraphSx<int>;
using GraphSd = GraphSx<double>;
using GraphSf = GraphSx<float>;


// 稀疏有向图
template<typename T>
using DigraphSx = KtGraphImpl<KtGraphSparse<T, true>>;

using DigraphSi = DigraphSx<int>;
using DigraphSd = DigraphSx<double>;
using DigraphSf = DigraphSx<float>;


// 稠密无向图
template<typename T>
using GraphDx = KtGraphImpl<KtGraphDense<T, false>>;

using GraphDi = GraphDx<int>;
using GraphDd = GraphDx<double>;
using GraphDf = GraphDx<float>;


// 稠密有向图
template<typename T>
using DigraphDx = KtGraphImpl<KtGraphDense<T, true>>;

using DigraphDi = DigraphDx<int>;
using DigraphDd = DigraphDx<double>;
using DigraphDf = DigraphDx<float>;