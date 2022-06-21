#include "WfstX.h"
#include "KtMinimization.h"
#include "../graph/test_util.h"


using fst_type = StdWfstTrop<float, char>;


void minimization_test()
{
	fst_type fst(6);
	fst.setInitial(0, 0.8f);
	fst.setFinal(5, 0);

	fst.addTrans(0, 1, 'a', '\0', 0);
	fst.addTrans(1, 2, 'b', '\0', 0.2f);
	fst.addTrans(1, 3, 'c', 'X', 0);
	fst.addTrans(1, 4, 'd', 'Y', 0.6f);

	fst.addTrans(2, 5, '\0', 'X', 0);
	fst.addTrans(2, 5, '\0', 'Y', 1.4f);
	fst.addTrans(3, 5, '\0', 'Z', 0);
	fst.addTrans(4, 5, '\0', 'Z', 0);

	printf("minimization test...\n");
	fflush(stdout);

	printf("   specific wfst |S| = %d, |T| = %d", fst.order(), fst.size());
	fflush(stdout);

	KtMinimization<fst_type> miniFst(fst);
	if (miniFst->order() != 5 || miniFst->size() != 7)
		test_failed(miniFst.wfst(), true);

	printf("  > passed\n"); fflush(stdout);
}