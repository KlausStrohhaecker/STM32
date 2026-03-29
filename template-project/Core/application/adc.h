#pragma once

#include <main.h>

void adcInit(ADC_HandleTypeDef *hadc, DMA_HandleTypeDef *hdma);
void adcProcess();
void erps_process();
