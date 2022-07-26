#include "KuFstIO.h"
#include <fstream>
#include <memory>


bool KuFstIO::isFstHeader(std::istream& strm)
{
	stdx::istreamx isx(strm, true);
	isx.setLittleEndian();

	return isx.peek<std::int32_t>() == kFstMagicNumber;
}


bool KuFstIO::readHeader_(stdx::istreamx& strm, KpFstHeader& hdr)
{
	std::int32_t magic_number = 0;
	strm >> magic_number;
	if (magic_number != kFstMagicNumber)
		return false;

	strm.readBinString(hdr.fsttype);
	strm.readBinString(hdr.arctype);

	strm >> hdr.version
	     >> hdr.flags
	     >> hdr.properties
	     >> hdr.start
	     >> hdr.numstates
	     >> hdr.numarcs;

	return strm;
}


bool KuFstIO::readConstState_(stdx::istreamx& strm, kPrivate::KpConstState& cs)
{
	strm >> cs.weight
	 	 >> cs.pos
		 >> cs.narcs
		 >> cs.niepsilons
		 >> cs.noepsilons;

	return strm;
}


bool KuFstIO::readSymbolTable_(stdx::istreamx& strm, KgSymbolTable* table)
{
	std::int32_t magic_number = 0;
	strm >> magic_number;
	if (magic_number != kSymbolTableMagicNumber)
		return false;

	std::string name;
	strm >> name; // TODO: 

	std::int64_t available_key, size;
	strm >> available_key >> size; // ignore available_key
	if (!strm) return false;

	if (table) table->reserve(size);

	for (std::int64_t i = 0; i < size; ++i) {
		std::string symbol;
		std::int64_t key;
		strm >> symbol >> key;

		if (!strm)
			return false;

		if(table) table->addSymbol(symbol, key);
	}

	return true;
}


MmapWfst* KuFstIO::readMmap(const std::string& path)
{
	std::ifstream ifs(path, std::ios_base::binary);
	if (!ifs) return nullptr;

	stdx::istreamx strm(ifs, true);
	strm.setLittleEndian();

	/// read header

	KpFstHeader hdr;
	if (!readHeader_(strm, hdr))
		return nullptr;

	if (hdr.version < kMinFileVersion || hdr.fsttype != "const" || hdr.arctype != "standard") 
		return nullptr;

	/// read optional symbol-table

	if (hdr.flags & KpFstHeader::k_has_isymbols) {
		if (!readSymbolTable_(strm, nullptr))
			return nullptr;
	}

	if (hdr.flags & KpFstHeader::k_has_osymbols) {
		if (!readSymbolTable_(strm, nullptr))
			return nullptr;
	}

	auto off = ifs.tellg();
	ifs.close();

	/// now everything is ok, ready to read mmap
	auto mmap = std::make_unique<MmapWfst>();
	if (!mmap->map(path, hdr.numstates, hdr.numarcs, off)) // ÏÈ¶Ávertex£¬ÔÙ¶Áarc
		return nullptr;

	mmap->setInitial(hdr.start);

	// TODO: setFinal

	return mmap.release();
}