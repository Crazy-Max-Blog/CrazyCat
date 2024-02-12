#pragma once
#include <Arduino.h>

namespace gh {

// расстояние между двумя точками
uint16_t dist(int16_t x0, int16_t y0, int16_t x1, int16_t y1);

// точка лежит внутри прямоугольника (координаты угла и размеры)
bool inRect(int16_t x, int16_t y, int16_t rx, int16_t ry, uint16_t w, uint16_t h);

// точка лежит внутри окружности (координаты центра и радиус)
bool inCircle(int16_t x, int16_t y, int16_t cx, int16_t cy, uint16_t r);

}