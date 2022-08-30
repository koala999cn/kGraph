#include "KuFstIO.h"
#include <fstream>
#include <memory>


bool KuFstIO::isFstHeader(std::istream& strm)
{
	stdx::istreamx isx(strm, true);
	isx.setLittleEndian();

	return isx.peek<std::int32_t>() == kFstMagicNumber;
}


bool KuFstIO::readHeader(stdx::istreamx& strm, KpFstHeader& hdr)
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
