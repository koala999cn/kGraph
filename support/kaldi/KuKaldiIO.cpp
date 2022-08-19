#include "KuKaldiIO.h"
#include <fstream>
#include "../openfst/KgSymbolTable.h"
#include "../common/KuStrUtil.h"


bool KuKaldiIO::binaryTest(std::istream& strm) {
	if (strm.peek() == 0) { // 以'\0''B'开头表示binary模式
		strm.get();
		if (strm.peek() == 'B') {
			strm.get();
			return true;
		}
		strm.unget();
	}

	return false;
}


bool KuKaldiIO::writeBinFlag(std::ostream& strm)
{
	strm.put('\0');
	strm.put('B');
	return strm.good();
}


KgSymbolTable* KuKaldiIO::loadWordIdPair(const char* path)
{
	std::ifstream ifs(path);
	if (!ifs)
		return nullptr;

	auto st = new KgSymbolTable;
	std::string line;
	while (std::getline(ifs, line)) {
		auto toks = KuStrUtil::split(line, " \t");
		if (toks.size() == 2)
			st->addSymbol(toks[0], KuStrUtil::toInt(toks[1].c_str()));
	}
	return st;
}

