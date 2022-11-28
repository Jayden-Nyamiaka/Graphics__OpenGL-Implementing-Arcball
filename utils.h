#ifndef UTILS_H
#define UTILS_H

#include <string>
#include <vector>

int rand_num(int min, int max);

bool isClose(double a, double b);

void splitBySpace(std::string s, std::vector<std::string> &split);

#endif