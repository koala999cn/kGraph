#pragma once
#include "../graph/core/vertex_traits.h"
#include "../graph/core/edge_traits.h"
#include "../graph/core/KtFlatGraphBase.h"
#include "mio/mio.hpp"
#include <fcntl.h>


// 基于内存映射的KtFlatGraphBase实现
// 内存映射使用mio开源库(https://github.com/mandreyel)

namespace kPrivate
{
	template<typename EDGE_TYPE>
	using flatmg_edge_range = KtRange<const edge_has_to_t<EDGE_TYPE>*>;

	template<typename VERTEX_TYPE>
	using flatmg_vertex_range = KtRange<const vertex_has_edgeindex_t<VERTEX_TYPE>*>;

	template<typename EDGE_TYPE, typename VERTEX_TYPE>
	using flatmg_base = KtFlatGraphBase<flatmg_edge_range<EDGE_TYPE>,
		flatmg_vertex_range<VERTEX_TYPE>>;
}


template<typename EDGE_TYPE, typename VERTEX_TYPE = void>
class KtFlatGraphMmapImpl : public kPrivate::flatmg_base<EDGE_TYPE, VERTEX_TYPE>
{
public:
	using super_ = kPrivate::flatmg_base<EDGE_TYPE, VERTEX_TYPE>;
	using edge_type = EDGE_TYPE;
	using vertex_type = VERTEX_TYPE;

	KtFlatGraphMmapImpl() = default;

	~KtFlatGraphMmapImpl() { mmap_.unmap(); }


	// 从文件path的偏移foff处进行内存映射
	// 从映射空间的voff处，映射nv个顶点结构，eoff处映射ne个边结构
	bool map(const std::string& path, unsigned nv, unsigned ne, std::int64_t foff = 0, 
		std::int64_t voff = 0, std::int64_t eoff = 0) {
		std::error_code error;
		mmap_ = mio::make_mmap_source(path, static_cast<size_t>(foff), mio::map_entire_file, error);
		if (error) return false;
	
		auto vbuf = (const typename super_::vertex_type*)(mmap_.data() + voff);
		super_::vertexes_ = decltype(super_::vertexes_)(vbuf, vbuf + nv);

		if (eoff == 0) eoff = voff + nv * sizeof(super_::vertex_type);
		auto ebuf = (const typename super_::edge_type*)(mmap_.data() + eoff);
		super_::edges_ = decltype(super_::edges_)(ebuf, ebuf + ne);

		return true;
	}

private:
	mio::mmap_source mmap_;
};


template<typename EDGE_TYPE, typename VERTEX_TYPE>
struct graph_traits<KtFlatGraphMmapImpl<EDGE_TYPE, VERTEX_TYPE>>
	: public graph_traits<kPrivate::flatmg_base<EDGE_TYPE, VERTEX_TYPE>>
{
	constexpr static bool reshapable = false;
	constexpr static bool immutable = true;
};
