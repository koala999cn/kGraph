#include <stdio.h>
#include "../base/KtMatrix.h"
#include "../base/KtSparseMatrix.h"


static void test_failed()
{
	printf("  > :( failed\n");
	printf("press any key to exit.\n");
	fflush(stdout);
	getchar();
	exit(1);
}


template<typename MAT>
void randmat_(unsigned rows, unsigned cols, MAT& mat)
{
	mat.resize(rows, cols, 0.0);
	for (unsigned r = 0; r < rows; r++)
		for (unsigned c = 0; c < cols; c++)
			if(rand() < RAND_MAX / 20) // 填充5%的元素
				mat(r, c) = double(rand()) / double(RAND_MAX);
}


void matrix_test()
{
	printf("matrix test...\n");
	fflush(stdout);


	int row = rand() % 6000;
	int col = rand() % 6000;

	printf("   rand denst matrix ROW = %d, COL = %d\n", row, col);
	fflush(stdout);

	KtMatrix<double> mat;
	randmat_(row, col, mat);

	printf("      row element iter");
	fflush(stdout);

	for (unsigned r = 0; r < mat.rows(); r++) {
		auto row = mat.row(r);
		for (unsigned c = 0; !row.empty(); ++row, ++c) {
			if (c >= mat.cols() || mat(r, c) != *row)
				test_failed();
		}
	}

	printf("  > passed\n"); fflush(stdout);


	printf("      col element iter");
	fflush(stdout);

	for (unsigned c = 0; c < mat.cols(); c++) {
		auto col = mat.col(c);
		for (unsigned r = 0; !col.empty(); ++col, ++r) {
			if (r >= mat.rows() || mat(r, c) != *col)
				test_failed();
		}
	}

	printf("  > passed\n"); fflush(stdout);


	row = rand() % 1500;
	col = rand() % 1500;

	printf("   rand sparse matrix ROW = %d, COL = %d\n", row, col);
	fflush(stdout);

	KtSparseMatrix<double, false> smat;
	randmat_(row, col, smat);

	printf("      row element iter");
	fflush(stdout);

	for (unsigned r = 0; r < smat.rows(); r++) {
		auto row = smat.row(r);
		for (; !row.empty(); ++row) {
			if (smat(r, (*row).first) != (*row).second)
				test_failed();
		}
	}

	printf("  > passed\n"); fflush(stdout);


	printf("      col element iter");
	fflush(stdout);

	for (unsigned c = 0; c < smat.cols(); c++) {
		auto col = smat.col(c);
		for (; !col.empty(); ++col) {
			if (smat((*col).first, c) != (*col).second)
				test_failed();
		}
	}

	printf("  > passed\n"); fflush(stdout);


	row = rand() % 1000;
	col = rand() % 1000;

	printf("   rand multi-value sparse matrix ROW = %d, COL = %d", row, col);
	fflush(stdout);

	KtSparseMatrix<double, true> msmat;
	randmat_(row, col, msmat);

	/// 以5%的比例加入多值
	for (unsigned r = 0; r < msmat.rows(); r++)
		for (unsigned c = 0; c < msmat.cols(); c++) {
			if (msmat(r, c) != 0.0 && rand() < RAND_MAX / 20) {
				msmat.insert(r, c, double(rand()) / double(RAND_MAX));

				// 再以5%的概率加入三值
				if (rand() < RAND_MAX / 20)
					msmat.insert(r, c, double(rand()) / double(RAND_MAX));
			}
		}


	printf(", MVALS = %d\n", msmat.mvalues());
	fflush(stdout);

	printf("      row element iter");
	fflush(stdout);

	for (unsigned r = 0; r < msmat.rows(); r++) {
		auto row = msmat.row(r);
		for (; !row.empty(); ++row) {
			if (msmat(r, (*row).first, (*row).second) != (*row).second)
				test_failed();
		}
	}

	printf("  > passed\n"); fflush(stdout);


	printf("      col element iter");
	fflush(stdout);

	for (unsigned c = 0; c < msmat.cols(); c++) {
		auto col = msmat.col(c);
		for (; !col.empty(); ++col) {
			if (msmat((*col).first, c, (*col).second) != (*col).second)
				test_failed();
		}
	}

	printf("  > passed\n"); fflush(stdout);
}

