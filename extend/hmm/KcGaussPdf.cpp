#include "KcGaussPdf.h"
#include "KtuMath.h"
#include "Eigen/Eigen"
#include <assert.h>


KcGaussPdf::KcGaussPdf(unsigned dim)
{
	mean_.resize(dim, 0);
	invCov_.resize(dim, 1);
	gconst_ = dim * log(KtuMath<double>::pi * 2);
}


// p(x) = 1/A * exp(-B/2)
// ���� A = (sqrt(2*pi))^n * sigma[0] * sigma[1] * ... * sigma[n-1]
//      B = ((x[0]-u[0])/sigma[0])^2 + ((x[1]-u[1])/sigma[1])^2 + ...
double KcGaussPdf::prob(const double* obs) const
{
	double prob = 0.0;
	auto dim = this->dim();

	if (isDiagnal()) {
		prob = gconst_;
		for (unsigned i = 0; i < dim; i++) {
			double d = obs[i] - mean_[i];
			prob += d * d * invCov_[i]; // TODO: ����cov[i] == 0�����
		}

		prob *= -0.5;
	}
	else {
		assert(invCov_.size() == dim * dim);

		std::vector<double> xmm(dim);
		KtuMath<double>::sub(obs, mean_.data(), xmm.data(), dim);

		for (unsigned j = 0; j < dim; j++)
			for (unsigned i = j + 1; i < dim; i++)
				prob += xmm[i] * xmm[j] * invCov_[i * dim + j];
		prob *= 2;
		prob += gconst_;

		for (unsigned i = 0; i < dim; i++)
			prob += xmm[i] * xmm[i] * invCov_[i * dim + i];

		prob *= -0.5;
	}

	return prob;
}


void KcGaussPdf::setMean(const double* mean)
{
	mean_.assign(mean, mean + dim());
}


bool KcGaussPdf::setCovar(const double* cov, bool bDiag)
{
	auto dim = this->dim();

	double factor = dim * log(KtuMath<double>::pi * 2);

	if (bDiag) {
		invCov_.resize(dim);
		double* inv_cov = invCov_.data();

		for (unsigned i = 0; i < dim; i++) {
			inv_cov[i] = 1.0 / cov[i];
			factor += log(cov[i]); // TODO: ����cov[i] == 0�����
		}
	}
	else {
		using MatrixRxd = Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>;

		// ��Э���������Choleski�ֽ� c = L * LT
		Eigen::Map<const MatrixRxd> c(cov, dim, dim);
		auto llt = c.selfadjointView<Eigen::Lower>().llt();
		if (llt.info() != Eigen::Success)
			return false;

		// ����Э������������c-1.
		// ת��Ϊ��� cx = I, ��L*LT*x = I
		invCov_.resize(dim, dim);
		double* inv_cov = invCov_.data();
		Eigen::Map<MatrixRxd> inv(inv_cov, dim, dim);
		inv.setIdentity();
		llt.solveInPlace(inv);

		// det(c) = det(L*LT) = det(L) * det(L)
		double LDet = llt.matrixL().determinant(); // // �����������ʽΪ�Խ���Ԫ��֮��
		factor += 2 * log(LDet); // += log(det(c))
	}

	gconst_ = factor;

	return true;
}


std::vector<double> KcGaussPdf::covar() const
{
	std::vector<double> cov;

	if (isDiagnal()) {
		cov.resize(invCov_.size());
		KtuMath<double>::recip(invCov_.data(), cov.data(), cov.size());
	}
	else {
		using MatrixRxd = Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>;

		// ��Э���������Choleski�ֽ� c = L * LT
		auto dim = this->dim();
		Eigen::Map<const MatrixRxd> c(invCov_.data(), dim, dim);
		auto llt = c.selfadjointView<Eigen::Lower>().llt();
		assert(llt.info() == Eigen::Success);

		// ����Э������������c-1.
		// ת��Ϊ��� cx = I, ��L*LT*x = I
		cov.resize(dim, dim);
		Eigen::Map<MatrixRxd> covM(cov.data(), dim, dim);
		covM.setIdentity();
		llt.solveInPlace(covM);
	}

	return cov;
}
