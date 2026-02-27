#include "../application/adc.h"

#include "stdbool.h"

#include "main.h"
#include "../application/erp_decoder.h"

#define ADC_CHANNELS 16

static ADC_HandleTypeDef *m_adc;
static DMA_HandleTypeDef *m_dma_adc;

static uint16_t adcRawBuffer[ADC_CHANNELS] = {};

static uint16_t adcAvgIdx = 0;
#define ADC_AVG_SIZE (8)
static int adcSums[ADC_CHANNELS]                  = {};
static int adcBuffers[ADC_CHANNELS][ADC_AVG_SIZE] = {};

void adcInit(ADC_HandleTypeDef *hadc, DMA_HandleTypeDef *hdma)
{
  m_adc     = hadc;
  m_dma_adc = hdma;

  HAL_ADC_Init(m_adc);
  HAL_ADC_Start_DMA(m_adc, (uint32_t *) adcRawBuffer, ADC_CHANNELS);
}

static inline uint16_t integerRoundedDivision_uint16(unsigned const numerator, unsigned const denominator)
{
  return (numerator + denominator / 2) / denominator;
}

void adcProcess()
{
#if 01  // ERP

  int angle = ERP_DecodeWipersToAngle(
      integerRoundedDivision_uint16(adcSums[10], 2),
      integerRoundedDivision_uint16(adcSums[11], 2));
  printk("%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d\n",
         adcSums[0], adcSums[1], adcSums[2], adcSums[3], adcSums[4], adcSums[5], adcSums[6], adcSums[7],
         adcSums[8], adcSums[9], adcSums[10], adcSums[11], adcSums[12], adcSums[13], adcSums[14], angle);

#else

  printk("%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d\n",
         adcSums[0], adcSums[1], adcSums[2], adcSums[3], adcSums[4], adcSums[5], adcSums[6], adcSums[7],
         adcSums[8], adcSums[9], adcSums[10], adcSums[11], adcSums[12], adcSums[13], adcSums[14], adcSums[15]);

#endif
}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc)
{
  if (hadc->Instance == m_adc->Instance)
  {
    // HAL_ADC_Start_DMA(m_adc, (uint32_t *) adcRawBuffer, ADC_CHANNELS);
    HAL_GPIO_TogglePin(LED_HB_GPIO_Port, LED_HB_Pin);

    for (int i = 0; i < ADC_CHANNELS; i++)
    {
      adcSums[i] -= adcBuffers[i][adcAvgIdx];
      adcSums[i] += (adcBuffers[i][adcAvgIdx] = adcRawBuffer[i]);
    }
    adcAvgIdx = (adcAvgIdx + 1) % ADC_AVG_SIZE;
  }
}
