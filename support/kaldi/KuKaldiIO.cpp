#include "KuKaldiIO.h"


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