#pragma once
#include <Arduino.h>

#define PANEL_W 32
#define PANEL_H 16

void map_xy_to_physical_coord(int32_t x, int32_t y, int32_t * ret_x, int32_t * ret_y);