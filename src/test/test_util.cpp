#include <stdio.h>
#include <random>
#include <string>



// 返回[x0, x1]区间的随机数
double rand(double x0, double x1)
{
    return x0 + (x1 - x0) * rand() / RAND_MAX;
}


// 以prob的概率返回true.
// 0 <= prob <= 1
bool rand_p(double prob)
{
    auto r = rand();
    return r <= RAND_MAX * prob;
}


bool almostEqual(double x1, double x2, double tol)
{
    double dif = x1 - x2;
    if (dif < 0) dif = -dif;
    return dif < tol;
}


std::string format_pathR(const std::vector<unsigned>& pathR)
{
    std::string str;
    for (auto iter = pathR.rbegin(); iter != pathR.rend(); ++iter) {
        str += std::to_string(*iter);
        str += "-";
    }
    str.pop_back(); // erase last '-'
    return str;
}
