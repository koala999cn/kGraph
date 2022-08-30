#include "WfstX.h"
#include "KtSynchronize.h"
#include "../graph/test_util.h"


using fst_type = StdWfstTrop<float, char>;


void synchronize_test()
{
	fst_type fst(3);
	fst.setInitial(0);
	fst.setFinal(2, 4);

	fst.addTrans(0, 1, '\0', 'a', 1);
	fst.addTrans(1, 1, 'a', 'b', 2);
	fst.addTrans(1, 2, 'b', '\0', 3);

	printf("synchronize test...\n");
	fflush(stdout);

	printf("   specific wfst |S| = %d, |T| = %d", fst.order(), fst.size());
	fflush(stdout);

	KtSynchronize<fst_type> sync(fst);
	if (sync->order() != 4 || sync->size() != 5)
		test_failed(sync.wfst(), true);

	printf("  > passed\n"); fflush(stdout);
}
