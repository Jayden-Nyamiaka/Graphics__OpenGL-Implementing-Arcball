#include <cstdlib>
#include <fstream>
#include <sstream>

#include "utils.h"

const double CLOSE_EPSILON = 0.00000001;

int rand_num(int min, int max) {
    return (int)( (rand() * 1.0 * (max - min + 1)) / RAND_MAX + min);
}

bool isClose(double a, double b) {
    return abs(a - b) < CLOSE_EPSILON;
}

void splitBySpace(std::string s, std::vector<std::string> &split) {
    std::stringstream stream(s);

    std::string buffer;
    while(getline(stream, buffer, ' ')) {
        split.push_back(buffer);
    }
}