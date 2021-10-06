#include <stdio.h>
#include <random>
#include <string>


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
