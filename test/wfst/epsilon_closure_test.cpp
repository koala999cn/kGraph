#include "WfstX.h"
#include "KtEpsClosure.h"
#include "../graph/test_util.h"


using fst_type = StdWfstTrop<float, char>;


void epsilon_closure_test()
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


	printf("epsilon closure test...\n");
	fflush(stdout);

	printf("   specific wfst |S| = %d, |T| = %d", fst.order(), fst.size());
	fflush(stdout);

	KtEpsClosure<fst_type> closureFst(fst);
	if (closureFst->order() != 5 || closureFst->size() != 9)
		test_failed(closureFst.inside(), true);

	printf("  > passed\n"); fflush(stdout);
}