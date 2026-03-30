# Optimizing STM32 A/D Conversions

## Software-Triggered Single Conversion

This is the simplest way.

A conversion is started periodically from software, say from a routine that is called every 1ms. 

A "conversion complete" DMA interrupt callback then collects the data.

```c
void ConvCpltCallback()
{
  moveRawDataToUserData();  
}


void AdcService_1ms()
{
  processUserData();
  restartConversion();
}
```

This is simple but does not exploit the features of the ADC. The ADC will be mostly idle and that time could be better spent.

## Continuous Free-Running Conversion using Asynchronous Oversampling with Sliding-Window FIR-Filtering

The conversion is only started once and the restarts itself at the fastest possible rate which is probably much faster than the required rate. The additional samples can then be used for averaging, reducing channel cross-talk etc.

The *sliding window FIR filter* should be longer than the time interval between user access, like 1.5x or so. Also, it should contain as much values as possible, to reduce the aperture error coming from the asynchronous access. This mandates that the ADC conversion rate is as fast as possible (but still applying proper setup times, see below), so set up the clock tree to maximum possible ADC clock.

Rather than actual averaging which would require rounded integer division or even floating point division, it is better to use plain integer summing and move any potential division to the data post-processing at the slower user rate.

Say we have 12 Channels and will use 16 averages (2^N values preferred for faster modulo operation).

```c
#define ADC_CHANNELS (12u)
static uint16_t adcRawBuffer[ADC_CHANNELS] = {};
static uint16_t adcAvgIdx = 0u;
#define ADC_AVG_SIZE (16u)
static volatile int adcSums[ADC_CHANNELS] = {};
static int adcBuffers[ADC_CHANNELS][ADC_AVG_SIZE] = {};

void ConvCpltCallback()
{
  for (unsigned ch = 0u; ch < ADC_CHANNELS; ch++)
  {
    adcSums[ch] -= adcBuffers[ch][adcAvgIdx];
    adcSums[ch] += (adcBuffers[ch][adcAvgIdx] = adcRawBuffer[ch]);
  }
  adcAvgIdx = (adcAvgIdx + 1u) % ADC_AVG_SIZE;
}

void AdcService_1ms()
{
  processUserData(adcSums);
}
```

One convervion is comprised of a sampling phase which charges the sampling capacitor and an aquisition phase where the actual SAR conversion takes place.

It is advisable to make the sampling phase as long as possible and the aquisition phase as fast as possible, therefore another reason for a maximum clock rate for the ADC.



#### Making use of the largest possbile conversion sequence length

The ADC can run a sequence of up to 16 conversions in a row, one conversion per ***slot***, and the slots can be arbitrarily assigned to any physcal ADC channel.

When there are less than 9 channels to convert, then one can convert the same channels several times in one go, reducing the number of callbacks. 

| # of channels | #of blocks | sequence length |
|:-------------:|:----------:|:---------------:|
| 1             | 16         | 16              |
| 2             | 8          | 16              |
| 3             | 5          | 15              |
| 4             | 4          | 16              |
| 5             | 3          | 15              |
| 6             | 2          | 12              |
| 7             | 2          | 14              |
| 8             | 2          | 16              |

For the channel distribution in the slots there are several options.



#### Slot assigment for lowest crosstalk

Since crosstalk between channels often is an issue (depending on source impedance it takes time to charge the sampling capacitor and also the stray capacitance of the channel multiplexer) is appears to best to convert the same channel multiple times in a row so the ADC stays on the same channel and the sampling capacitor has more time to fully charge. This also the solution if the sampling phase cannot be made any longer per slot to fulfil charge timing requirements.

For a 4 x 4 sequences of 4 channel the slot assignment could be:

| Slot #  | 1   | 2   | 3   | 4   | 5   | 6   | 7   | 8   |
| ------- |:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|
| Channel | 1   | 1   | 1   | 1   | 2   | 2   | 2   | 2   |

| Slot #  | 9   | 10  | 11  | 12  | 13  | 14  | 15  | 16  |
| ------- |:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|
| Channel | 3   | 3   | 3   | 3   | 4   | 4   | 4   | 4   |

If feasible, for minimized crosstalk one can even discard the first one or two and use only later samples of a channel (or apply a weighting), in the extreme case only the last sample of the block is used. The downside is that one loses the discarded samples for averaging, a trade-off depending on application and source impedance details.



#### Slot assigment for distributed crosstalk

Another option for slot assignment is to even out the crosstalk over more channels. It is the charge accumulated in the sampling capacitor of the *previous* conversion that affects the current conversion. By alternating channel orders one can replace the strong influence of always the same channel by an averaged influence of several channels:

| Slot #  | 1   | 2   | 3   | 4   | 5   | 6   | 7   | 8   |
| ------- |:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|
| Channel | 1   | 2   | 3   | 4   | 1   | 3   | 2   | 4   |

| Slot #  | 9   | 10  | 11  | 12  | 13  | 14  | 15  | 16  |
| ------- |:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|
| Channel | 2   | 3   | 1   | 4   | 3   | 2   | 1   | 4   |


