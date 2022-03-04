#pragma once
#include <vector>


/// graph with Vertex Object Attached

template<typename VERTEX_TYPE, typename BASE_GRAPH>
class KtGraphVoa : public BASE_GRAPH
{
public:

    using typename BASE_GRAPH::edge_type;
    using typename BASE_GRAPH::const_edge_ref;
    using typename BASE_GRAPH::edge_iter;
    using typename BASE_GRAPH::vertex_index_t;

    using vertex_type = VERTEX_TYPE;


    /// 构造函数

    KtGraphVoa() : BASE_GRAPH(), vos_() { }

    explicit KtGraphVoa(unsigned numVertex, const_edge_ref nullValue = edge_type{ 0 })
		: BASE_GRAPH(numVertex, nullValue)
		, vos_(numVertex) { }

	KtGraphVoa(const KtGraphVoa& g)
		: BASE_GRAPH(g)
		, vos_(g.vos_) { }

    KtGraphVoa(KtGraphVoa&& g)
        : BASE_GRAPH(std::forward<BASE_GRAPH>(g))
		, vos_(std::move(g.vos_)) { }


    // 重写与顶点对象有关的成员函数

    void reset(unsigned numVertex, const_edge_ref nullEdge = edge_type(0)) {
        BASE_GRAPH::reset(numVertex, nullEdge);
        vos_.resize(numVertex);
    }

    void eraseVertex(vertex_index_t v) {
        BASE_GRAPH::reset(v);
        vos_.erase(vos_.begin() + v);
    }

    // 返回新增顶点的id
    vertex_index_t addVertex() {
        auto v = BASE_GRAPH::addVertex();
        vos_.push_back(VERTEX_TYPE());
        assert(vos_.size() == order());
        return v;
    }
 
    vertex_index_t addVertex(const VERTEX_TYPE& val) {
        auto v = BASE_GRAPH::addVertex();
		vos_.push_back(val);
        assert(vos_.size() == order());
        return v;
    }

    //template<typename... ARG>
    //vertex_index_t emplaceVertex(ARG&&... arg) {
    //    auto v = BASE_GRAPH::addVertex();
    //    vos_.emplace(std::forward<ARG>(arg)...);
    //    assert(vos_.size() == v);
    //    return v;
    //}

    void reserve(unsigned V, unsigned E) {
        BASE_GRAPH::reserve(V, E);
        vos_.reserve(V);
    }

    template<typename WEIGHTOR = KtWeightSelf<edge_type>>
    KtGraphVoa copy() const {
        auto g = BASE_GRAPH::template copy<KtGraphVoa, WEIGHTOR>();
        g.vos_ = vos_;
        return g;
    }


    /// 新增与顶点相关的特有成员函数

    const VERTEX_TYPE& getVertex(vertex_index_t v) const {
        return vos_[v];
    }

    VERTEX_TYPE& getVertex(vertex_index_t v) {
        return vos_[v];
    }


private:
    std::vector<VERTEX_TYPE> vos_;
};

