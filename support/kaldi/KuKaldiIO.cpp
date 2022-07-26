#include "KuKaldiIO.h"


bool KuKaldiIO::binaryTest(std::istream& strm) {
	if (strm.peek() == 0) { // ��'\0''B'��ͷ��ʾbinaryģʽ
		strm.get();
		if (strm.peek() == 'B') {
			strm.get();
			return true;
		}
		strm.unget();
	}

	return false;
}