#include <stdlib.h>
#include <cmath>
#include <algorithm>

#include "color.h"

using namespace std;

color_rgb_t initColor(float red, float green, float blue) {
    color_rgb_t color{red, green, blue};
    return color;
}

color_rgb_t addColors(color_rgb_t color1, color_rgb_t color2) {
    return initColor(min(color1.r + color2.r, 1.0F),
                     min(color1.g + color2.g, 1.0F),
                     min(color1.b + color2.b, 1.0F));
}

color_rgb_t scaleColor(color_rgb_t color, float factor) {
    return initColor(min(factor * color.r, 1.0F),
                     min(factor * color.g, 1.0F),
                     min(factor * color.b, 1.0F));
}

color_rgb_t negateColor(color_rgb_t color) {
    return initColor(1.0F - color.r, 1.0F - color.g, 1.0F - color.b);
}

std::string toString(color_rgb_t color) {
    int red = 255 * color.r;
    int green = 255 * color.g;
    int blue = 255 * color.b;
    return to_string(red) + " " + to_string(green) + " " + to_string(blue);
}