#pragma once
#include <fstream>
#include "../../common/istreamx.h"
#include "../../common/KuStrUtil.h"
#include "../openfst/KuFstIO.h"
#include "../extend/lattice/KtLattice.h"
#include "../../graph/util/copy.h"
#include "KuKaldiLat.h"

// 提供kalid-lat的读取支持

class KuKaldiLat
{
public:

	template<typename LAT>
	static std::vector<std::pair<std::string, std::shared_ptr<LAT>>> load(const char* path) {

		std::vector<std::pair<std::string, std::shared_ptr<LAT>>> lats;

		std::ifstream ifs(path, std::ios_base::binary);
		stdx::istreamx strm(ifs, true);

		std::string key;
		while (strm >> key) {
			auto lat = std::make_shared<LAT>();
			strm.setBinary(KuFstIO::isFstHeader(strm));
			if (!read(strm, *lat))
				break;

			lats.emplace_back(key, lat);
		}
		
		return lats;
	}

	template<typename LAT>
	static bool read(stdx::istreamx& strm, LAT& lat) {
		if (strm.binary())
			return readBin_(strm, lat);
		else
			return readText_(strm, lat);
	}


private:

	template<typename LAT>
	static bool readBin_(stdx::istreamx& strm, LAT& lat);

	template<typename LAT>
	static bool readText_(stdx::istreamx& strm, LAT& lat);
};


namespace kPrivate
{
	template<typename LAT1, typename LAT2>
	void convertLat_(const LAT1& src, LAT2& dst) 
	{
		using trans1_t = typename LAT1::trans_type;
		using trans2_t = typename LAT2::trans_type;

		struct WTOR_ {
			trans2_t operator()(const trans1_t& trans) {
				using wt2_type = typename trans2_t::weight_type;
				wt2_type wt2; // TODO: 实现接受类型转换的构造函数，以便可以直接wt2_type wt2(wt1)
				wt2 = trans_traits<trans1_t>::weight(trans);
				return trans_traits<trans2_t>::construct(
					trans_traits<trans1_t>::isym(trans), 
					trans_traits<trans1_t>::osym(trans),
					wt2);
			}
		};

		copy<LAT1, LAT2, WTOR_>(src, dst);
	}

	// 读取kalid-lat的辅助函数(bin模式)，从strm中读取LAT2类型，转换为LAT1类型
	template<typename LAT1, typename LAT2>
	bool readBin_(stdx::istreamx& strm, const KuFstIO::KpFstHeader& hdr, LAT1& lat1)
	{
		LAT2 lat2;
		if (!KuFstIO::read(strm, hdr, lat2))
			return false;
		convertLat_(lat2, lat1);
		return true;
	}

	template<typename WEIGHT_TYPE>
	bool Str2Weight_(const std::string& str, WEIGHT_TYPE& wt)
	{
		auto toks = KuStrUtil::split(str, ",");
		if (toks.size() < 2 || toks.size() > 3)
			return false;

		auto cost0 = KuLex::parseFloat(toks[0]);
		auto cost1 = KuLex::parseFloat(toks[1]);
		if (!cost0.first || !cost1.first)
			return false;

		if constexpr (WEIGHT_TYPE::isCompact()) {
			auto& w = wt.weight();
			w.value0() = cost0.second;
			w.value1() = cost1.second;
			if (toks.size() > 2) {
				auto& s = toks[2];
				auto seq = KuStrUtil::split(s, "_");
				auto& v = wt.string();
				v.reserve(seq.size());
				for (auto& i : seq) {
					auto id = KuLex::parseInt64(i);
					if (!id.first)
						return false;
					v.push_back(id.second);
				}
			}
		}
		else {
			wt.value0() = cost0.second;
			wt.value1() = cost1.second;
		}

		return true;
	}


	template<typename LAT>
	bool readText_(stdx::istreamx& strm, LAT& lat)
	{
		using weight_type = typename LAT::weight_type;
		return KuFstIO::readText<LAT, weight_type::isCompact()>(strm, lat, Str2Weight_<weight_type>);
	}

	template<typename LAT1, typename LAT2>
	bool readTxt_(stdx::istreamx& strm, LAT1& lat1)
	{
		LAT2 lat2;
		using weight_type = typename LAT2::weight_type;
		if (!readText_(strm, lat2))
			return false;
		convertLat_(lat2, lat1);
		return true;
	}
}


template<typename LAT>
bool KuKaldiLat::readBin_(stdx::istreamx& strm, LAT& lat)
{
	assert(KuFstIO::isFstHeader(strm));

	KuFstIO::KpFstHeader hdr;
	if (!KuFstIO::readHeader(strm, hdr)
		|| hdr.fsttype != "vector")
		return false;

	if (KuStrUtil::beginWith(hdr.arctype, "lattice")) {
		int floatSize = hdr.arctype.back() - '0';
		if (floatSize == 4)
			return kPrivate::readBin_<LAT, kLatticef>(strm, hdr, lat);
		else if (floatSize == 8) 
			return kPrivate::readBin_<LAT, kLatticed>(strm, hdr, lat);
	}
	else if (KuStrUtil::beginWith(hdr.arctype, "compactlattice")) {
		int intSize = hdr.arctype.back() - '0';
		int floatSize = hdr.arctype[hdr.arctype.size() - 2] - '0';
		if (intSize == 4 && floatSize == 4) 
			return kPrivate::readBin_<LAT, kCompactLatticef32>(strm, hdr, lat);
		else if (intSize == 4 && floatSize == 8)
			return kPrivate::readBin_<LAT, kCompactLatticef64>(strm, hdr, lat);
		else if (intSize == 8 && floatSize == 4)
			return kPrivate::readBin_<LAT, kCompactLatticed32>(strm, hdr, lat);
		else if (intSize == 8 && floatSize == 8)
			return kPrivate::readBin_<LAT, kCompactLatticed64>(strm, hdr, lat);
	}

	return false;
}


template<typename LAT>
bool KuKaldiLat::readText_(stdx::istreamx& strm, LAT& lat)
{
	assert(!KuFstIO::isFstHeader(strm));
	
	auto pos = strm->tellg(); // 保存状态

	// 先尝试直接解析LAT
	if (kPrivate::readText_(strm, lat))
		return true;

	// 若失败，则尝试解析其他类型的lattice
	strm->seekg(pos); // 恢复状态
	using weight_type = typename LAT::weight_type;
	if constexpr (weight_type::isCompact()) {
		// 构造非compact类型的lattice
		using naive_weight_type = std::remove_reference_t<decltype(std::declval<weight_type>().value0())>;
		using NONCOMPACT_T = KtLattice<naive_weight_type>;
		return kPrivate::readTxt_<LAT, NONCOMPACT_T>(strm, lat);
	}
	else {
		// 构造compact类型的lattice
		using COMPACT_T = KtLattice<KtCompactLatWeight<weight_type>, int>;
		return kPrivate::readTxt_<LAT, COMPACT_T>(strm, lat);
	}
}