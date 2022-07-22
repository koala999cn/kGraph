#pragma once
#include "KtEpsRemoval.h"


/* TODO: ����ͨ����
 *
 *
 */

template<typename FST>
class KtReverseEpsRemoval : public kPrivate::KtWfstHolder<FST>
{
public:

	KtReverseEpsRemoval(const FST& fst) {
		KtEpsRemoval<FST> er(fst.reverse());
		inside() = er->reverse();
	}
};
