#pragma once

#include "stdint.h"

void    coosInit(void);
int32_t coosTaskAdd(void (*task_name)(), uint32_t phase, uint32_t period);
int32_t coosTaskDelete(const uint8_t task_index);
void    coosDispatch(void);
void    coosUpdate(void);
