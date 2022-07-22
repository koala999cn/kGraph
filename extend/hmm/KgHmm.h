#pragma once

// Hmmģ���漰��
// 3���о�����һ��״̬���ϣ����ǹ۲⼯�ϣ����ǹ۲����С�
// 3������Ҫ�أ�һ��״̬ת�Ƹ��ʾ���(״̬i->״̬j)�����ǹ۲���ʾ���״̬i->�۲�j�������ǳ�ʼ����������P{״̬i|t=0}����
// 3���������⣺һ�Ǹ��ʼ������⡣��֪�۲����к�Hmmģ�ͣ�����۲����г��ֵĸ��ʡ�����ǰ/����㷨��
//              ����ѧϰ���⡣��֪�۲����У�����Hmmģ�Ͳ�������EM�㷨��Ҳ��Baum-Welch�㷨��
//              ����Ԥ�����⣬Ҳ�ƽ������⡣��֪�۲����к�ģ��Hmm������۲����ж�Ӧ��״̬���С���Viterbi�㷨��

// ���ʾ�Ϊlog(P)
class KgHmm
{
public:
	// @numStates: ���캬��numStates��״̬��hmm. ��������ʼ����ֹ2��״̬
	KgHmm(unsigned numStates);
	~KgHmm();

	// ����hmm������״̬������������ʼ����ֹ2��״̬
	unsigned numStates() const; 

	// ��״̬s1��״̬s2�Ƿ����ת�Ƹ���
	// 0��ʾ��̬
	bool hasTrans(unsigned s1, unsigned s2) const;

	// return true if a0F != 0
	bool isTee() const;

	// ״̬s1 -> s2��ת�Ƹ���
	double transProb(unsigned s1, unsigned s2) const;

	// ��ת�Ƹ���
	// @p: ת�Ƹ��ʵ�logֵ
	void setTransProb(unsigned s1, unsigned s2, double p);

	// ɾ��״̬s1 -> s2��ת�ƻ�
	void eraseTrans(unsigned s1, unsigned s2);

	// ״̬s�ķ������
	double emissionProb(unsigned s, const double* obs) const;

private:
	void* dptr_;
};

