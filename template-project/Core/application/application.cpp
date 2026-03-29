#include <stdbool.h>
#include <stdarg.h>
#include <string.h>
#include <stdio.h>

#include <main.h>
#include "../application/adc.h"
#include "../application/coos.h"
#include "../application/erp_decoder.h"

static uint8_t m_systick = 0;

static UART_HandleTypeDef *m_pUartPrintkHandler;
static TIM_HandleTypeDef  *m_pSystickHandler;

// printk: debug messages via UART
void printk(const char *fmt, ...)
{
  char    buffer[256];
  va_list args;
  va_start(args, fmt);
  vsnprintf(buffer, sizeof(buffer), fmt, args);
  va_end(args);

  HAL_UART_Transmit(m_pUartPrintkHandler, (uint8_t *) buffer, strlen(buffer), 1);
}

[[maybe_unused]]
static void processHeartbeat()
{
  static uint32_t ms_cnt = 0;
  HAL_GPIO_TogglePin(LED_HB_GPIO_Port, LED_HB_Pin);
  //printf( "systick: %ld\n", ms_cnt );
  ms_cnt++;
}

extern "C"
{
  void applicationRun(TIM_HandleTypeDef *pTimerSystickHandler,
                      ADC_HandleTypeDef *pAdcHandler, DMA_HandleTypeDef *pAdcDmaHandler,
                      UART_HandleTypeDef *pUartPrintkHandler)
  {
    /* printk via debug uart */
    m_pUartPrintkHandler = pUartPrintkHandler;

    /* adc init */
    adcInit(pAdcHandler, pAdcDmaHandler);

    /* scheduler init */
    // 1ms systick
    coosInit();

    // coos_task_add(processHeartbeat, 0, 1);
    coosTaskAdd(adcProcess, 0, 1);

    /* start scheduler */
    m_pSystickHandler = pTimerSystickHandler;
    HAL_TIM_Base_Start_IT(pTimerSystickHandler);

    while (1)
    {
      if (m_systick == 1)
        coosUpdate(), m_systick = 0;
      coosDispatch();
    }
  }
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  if (htim == m_pSystickHandler)
    m_systick = 1;
}
