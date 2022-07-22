#pragma once


// ����ģ�͵Ľӿ���

class KvProbModel
{
public:

	// ��������������ά��
	virtual unsigned dim() const = 0;

	// ��������obs�����������(logֵ)
	virtual double prob(const double* obs) const = 0;

	// ����obs_seq[pos]�۲��������������
	// obs_seq��������Ϣ�ṩ�����ģ�lenΪobs_seq�ĳߴ�
	// ��obs_seq�Ĺ��Ϊ[len][dim]
	virtual double prob(const double* obs_seq[], unsigned len, unsigned pos) const {
		return prob(obs_seq[pos]); // ȱʡʵ���˻�Ϊ�������޹��������
	}
};
