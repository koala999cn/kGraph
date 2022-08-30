#include "WfstX.h"
#include "KtEpsRemoval.h"
#include "KtReverseEpsRemoval.h"
#include "../graph/test_util.h"


using fst_type = StdWfstTrop<float, char>;


void epsilon_removal_test()
{
	fst_type fst(5);
	fst.setInitial(0);
	fst.setFinal(4, 0.2f);

	fst.addTrans(0, 1, 'a', 'b', 0.1f);
	fst.addTrans(1, 2, '\0', '\0', 0.2f);
	fst.addTrans(1, 4, 'b', 'a', 0.2f);

	fst.addTrans(2, 0, 'b', 'a', 0.1f);
	fst.addTrans(2, 1, 'a', 'b', 0.4f);
	fst.addTrans(2, 3, '\0', '\0', 0.5f);
	fst.addTrans(2, 4, '\0', '\0', 0.25f);
	fst.addTrans(2, 4, 'a', 'b', 0.4f);

	fst.addTrans(3, 0, '\0', '\0', 0.4f);
	fst.addTrans(3, 4, '\0', '\0', 1);

	printf("epsilon removal test...\n");
	fflush(stdout);

	printf("   specific wfst |S| = %d, |T| = %d", fst.order(), fst.size());
	fflush(stdout);

	KtEpsRemoval<fst_type> er(fst);
	er->eraseDeadStates();
	er->mergeSameTrans();
	if (er->order() != 3 || er->size() != 5 || er->numFinals() != 2)
		test_failed(er.wfst(), true);

	printf("  > passed\n"); fflush(stdout);
}
