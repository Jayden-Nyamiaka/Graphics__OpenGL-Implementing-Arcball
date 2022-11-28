#ifndef COLOR_H
#define COLOR_H

#include <cstddef>
#include <string>

typedef struct color_rgb {
    float r;
    float g;
    float b;
} color_rgb_t;

color_rgb_t initColor(float red, float green, float blue);

/* Clamps rgb components down to 255 if the sum of any component > 255 */
color_rgb_t addColors(color_rgb_t color1, color_rgb_t color2);

color_rgb_t scaleColor(color_rgb_t color, float factor);

color_rgb_t negateColor(color_rgb_t color);

std::string toString(color_rgb_t color);

#endif