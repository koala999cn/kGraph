#pragma once
#include <string>
#include <vector>
#include <sstream>
#include <cstdint>
#include "../common/KuStrUtil.h"
#include "../common/KuLex.h"
#include "../common/istreamx.h"
#include "../graph/core/edge_traits.h"
#include "../graph/core/vertex_traits.h"
#include "../extend/wfst/KtWfst.h"
#include "../extend/wfst/KtSemiring.h"
#include "KtFlatGraphMmapImpl.h"
#include "KgSymbolTable.h"




namespace kPrivate
{
	template<typename WEIGHT_TYPE>
	struct KpStdArc
	{
		std::int32_t ilabel;
		std::int32_t olabel;
		WEIGHT_TYPE weight;
		std::int32_t nextstate;
	};

	template<typename VALUE_TYPE = float>
	using KpStdTropArc = KpStdArc<KtTropicalSemiring<VALUE_TYPE>>;

	template<typename VALUE_TYPE = float>
	using KpStdLogArc = KpStdArc<KtLogSemiring<VALUE_TYPE>>;


	struct KpConstState
	{
		float weight;              // Final weight.
		std::uint32_t pos;         // Start of state's arcs in *arcs_.
		std::uint32_t narcs;       // Number of arcs (per state).
		std::uint32_t niepsilons;  // Number of input epsilons.
		std::uint32_t noepsilons;  // Number of output epsilons.
	};
}


template<>
struct vertex_traits<kPrivate::KpConstState>
{
	using vertex_type = kPrivate::KpConstState;

	static std::uint32_t edgeindex(const vertex_type& v) { return v.pos; }
	static std::uint32_t& edgeindex(vertex_type& v) { return v.pos; }

	static std::uint32_t outdegree(const vertex_type& v) { return v.narcs; }
	static std::uint32_t& outdegree(vertex_type& v) { return v.narcs; }

	static float weight(const vertex_type& v) { return v.weight; }
	static float& weight(vertex_type& v) { return v.weight; }
};

template<>
struct vertex_traits<const kPrivate::KpConstState>
{
	using vertex_type = const kPrivate::KpConstState;

	static std::uint32_t edgeindex(const vertex_type& v) { return v.pos; }

	static std::uint32_t outdegree(const vertex_type& v) { return v.narcs; }

	static float weight(const vertex_type& v) { return v.weight; }
};


// 特化edge_traits，同时实现wfst需要的trans_traits接口
template<typename WEIGHT_TYPE>
struct edge_traits<kPrivate::KpStdArc<WEIGHT_TYPE>>
{
	using edge_type = kPrivate::KpStdArc<WEIGHT_TYPE>;
	using weight_type = decltype(std::declval<edge_type>().weight);


	static weight_type weight(const edge_type& edge) { return edge.weight; }
	static weight_type& weight(edge_type& edge) { return edge.weight; }

	static std::int32_t to(const edge_type& edge) { return edge.nextstate; }
	static std::int32_t& to(edge_type& edge) { return edge.nextstate; }

	inline static const edge_type null_edge = {};


	// 以下为trans_traits接口成员

	using trans_type = edge_type;
	using alpha_type = typename std::int32_t;
	constexpr static alpha_type eps = 0;

	static trans_type construct(const alpha_type& isym, const alpha_type& osym, const weight_type& wt) {
		return { isym, osym, wt, 0 };
	};

	static alpha_type isym(const trans_type& trans) { return trans.ilabel; }
	static alpha_type& isym(trans_type& trans) { return trans.ilabel; }

	static alpha_type osym(const trans_type& trans) { return trans.olabel; }
	static alpha_type& osym(trans_type& trans) { return trans.olabel; }
};


template<typename EDGE_TYPE, typename VERTEX_TYPE>
using DigraphMx = KtGraphX<KtGraph<KtFlatGraphMmapImpl<EDGE_TYPE, VERTEX_TYPE>, true, true, false>>;

using MmapWfst = KtWfst<kPrivate::KpStdTropArc<>, edge_traits<kPrivate::KpStdTropArc<>>,
	DigraphMx<kPrivate::KpStdTropArc<>, kPrivate::KpConstState>>;


class KuFstIO
{
public:

	// read fst as binary
	template<typename WFST>
	static bool read(std::istream& strm, WFST& fst, KgSymbolTable* isym = nullptr, KgSymbolTable* osym = nullptr);

	// read fst as text
	template <class WFST>
	static bool readText(std::istream& strm, WFST& fst);

	// read fst as mmap
	// wfst须是flat模式，
	static MmapWfst* readMmap(const std::string& path);


	// Checks for FST magic number in stream, to indicate caller function that the
	// stream content is an FST header.
	static bool isFstHeader(std::istream& strm);


private:

	struct KpFstHeader;
	static bool readHeader_(stdx::istreamx& strm, KpFstHeader& hdr);

	static bool readConstState_(stdx::istreamx& strm, kPrivate::KpConstState& cs);

	template<typename WEIGHT_TYPE>
	static bool readStdArc_(stdx::istreamx& strm, kPrivate::KpStdArc<WEIGHT_TYPE>& sa);
	
	static bool readSymbolTable_(stdx::istreamx& strm, KgSymbolTable* table);

	template<typename WFST>
	static bool readVector_(stdx::istreamx& strm, WFST& fst, std::int64_t numStates);

	template<typename WFST>
	static bool readConst_(stdx::istreamx& strm, WFST& fst, std::int64_t numStates, std::int64_t numArcs);

private:

	// Minimum file format version supported.
	static constexpr int kMinFileVersion = 2;

	// Identifies stream data as an FST (and its endianity).
	static constexpr std::int32_t kFstMagicNumber = 2125659606;

	// Identifies stream data as a symbol table (and its endianity).
	static constexpr std::int32_t kSymbolTableMagicNumber = 2125658996;


	struct KpFstHeader 
	{
	public:
		KpFstHeader() : version(0), flags(0), properties(0), start(-1),
			numstates(0), numarcs(0) {}

		std::string fsttype;       // E.g. "vector".
		std::string arctype;       // E.g. "standard".
		std::int32_t version;      // Type version number.
		std::int32_t flags;        // File format bits.
		std::uint64_t properties;  // FST property bits.
		std::int64_t start;        // Start state.
		std::int64_t numstates;    // # of states.
		std::int64_t numarcs;      // # of arcs.

		enum { k_has_isymbols = 0x01, k_has_osymbols = 0x02 };
	};


	KuFstIO() = delete;
};


template <class WFST>
bool KuFstIO::readText(std::istream &is, WFST& fst)
{
	//std::ostringstream err;

	fst.reset(0);
	KuLex::skipWhitespace(is);

	bool ok = true;
	std::string line;
	size_t nline = 0;
	std::string separator = " \t\r\n"; // TODO: FLAGS_fst_field_separator + "\r\n";
	while (std::getline(is, line)) {
		nline++;
		auto tokens = KuStrUtil::split(line, separator, true);
		if (tokens.size() == 0) break; // Empty line is a signal to stop

		if (tokens.size() > 5) {
			//err << "Bad line in FST: " << line;
			break;
		}

		auto s = KuLex::parseInt64(tokens[0]);
		if (!s.first) {
			//err << "Bad line in FST: " << line;
			break;
		}

		typename WFST::state_index_t d = static_cast<typename WFST::state_index_t>(s.second);
		while (d >= fst.order())
			fst.addState();
		if (nline == 1) fst.setInitial(d);

		switch (tokens.size()) {

		case 1:
			fst.setFinal(d);
			break;

		case 2:
		{
			// TODO: StrToWeight
			auto w = KuLex::parseFloat(tokens[1]);
			if (!w.first) ok = false;
			else fst.setFinal(d, w.second);
			break;
		}

		case 4:
		case 5:
		{
			auto nextstate = KuLex::parseInt64(tokens[1]);
			auto ilabel = KuLex::parseInt64(tokens[2]);
			auto olabel = KuLex::parseInt64(tokens[3]);
			auto w = typename WFST::weight_type::one();

			ok = nextstate.first && ilabel.first && olabel.first;
			if (ok && tokens.size() == 5) {
				auto wt = KuLex::parseFloat(tokens[4]);
				ok = wt.first;
				w = wt.second;
			}
	
			if (ok) {
				fst.addTrans(d, nextstate.second, ilabel.second, olabel.second, w);
			}

			break;
		}
	
		case 3: // 3 columns not ok for Lattice format; it's not an acceptor.
		default:
			ok = false;
		}

		while (d >= fst.order()) fst.addState();

		if (!ok) {
			//err << "Bad line in FST: " << line;
			break;
		}
	}

	return ok;
}


template<typename WFST>
bool KuFstIO::read(std::istream& strm_, WFST& fst, KgSymbolTable* isym, KgSymbolTable* osym)
{
	stdx::istreamx strm(strm_, true);
	strm.setLittleEndian();

	/// read header

	KpFstHeader hdr;
	if (!readHeader_(strm, hdr))
		return false;

	if (hdr.version < kMinFileVersion)
		return false;

	if (hdr.fsttype != "vector" && hdr.fsttype != "const") // TODO: 只支持读取vector和const类型fst
		return false;

	if (hdr.arctype != "standard")  // TODO: 只支持读取标准型arc
		return false;

	/// read optional symbol-table

	if (hdr.flags & KpFstHeader::k_has_isymbols) {
		if (!readSymbolTable_(strm, isym))
			return false;
	}

	if (hdr.flags & KpFstHeader::k_has_osymbols) {
		if (!readSymbolTable_(strm, osym))
			return false;
	}


	/// read states & transitions

	fst.reset(hdr.numstates);
	fst.setInitial(hdr.start);

	if (hdr.fsttype == "vector")
		return readVector_(strm, fst, hdr.numstates);
	else if (hdr.fsttype == "const")
		return readConst_(strm, fst, hdr.numstates, hdr.numarcs);

	return false;
}


template<typename WFST>
bool KuFstIO::readVector_(stdx::istreamx& strm, WFST& fst, std::int64_t numStates)
{
	using traits_type = typename WFST::traits_type;

	for (std::int64_t s = 0; s < numStates; s++) {
		float finalWt;
		strm >> finalWt; // final weight of current state
		if (!strm) return false;
		if (WFST::weight_type::zero() != WFST::weight_type(finalWt))
			fst.setFinal(s, finalWt); // TODO: 是这个意思吗

		std::int64_t narcs;
		strm >> narcs;
		if (!strm) return false;
		fst.reserveEdges(s, narcs);

		for (std::int64_t i = 0; i < narcs; i++) {
			kPrivate::KpStdArc<typename WFST::weight_type> arc;
			if (!readStdArc_(strm, arc))
				return false;

			fst.addTrans(s, arc.nextstate, arc.ilabel, arc.olabel, 
				WFST::weight_type(arc.weight.value()));
		}
	}

	return true;
}


template<typename WFST>
bool KuFstIO::readConst_(stdx::istreamx& strm, WFST& fst, std::int64_t numStates, std::int64_t numArcs)
{
	auto stateInitPos = strm->tellg();
	auto arcInitPos = stateInitPos;
	arcInitPos  += numStates * sizeof(kPrivate::KpConstState);

	for (std::int64_t s = 0; s < numStates; s++) {
		strm->seekg(stateInitPos + std::streamoff(s * sizeof(kPrivate::KpConstState)));
		kPrivate::KpConstState cs;
		if (!readConstState_(strm, cs))
			return false;

		if(WFST::weight_type::zero() != WFST::weight_type(cs.weight))
			fst.setFinal(s, cs.weight); // TODO: 

		strm->seekg(arcInitPos + std::streamoff(cs.pos * sizeof(kPrivate::KpStdLogArc<float>)));
		for (std::uint32_t i = 0; i < cs.narcs; i++) {
			kPrivate::KpStdLogArc<float> arc;
			if (!readStdArc_(strm, arc))
				return false;

			fst.addTrans(s, arc.nextstate, arc.ilabel, arc.olabel, 
				WFST::weight_type(arc.weight.value()));
		}
	}

	return true;
}


template<typename WEIGHT_TYPE>
bool KuFstIO::readStdArc_(stdx::istreamx& strm, kPrivate::KpStdArc<WEIGHT_TYPE>& arc)
{
	strm >> arc.ilabel
		>> arc.olabel
		>> arc.weight
		>> arc.nextstate;

	return strm;
}
