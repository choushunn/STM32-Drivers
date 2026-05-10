#include "turbidity.h"

#define ADC_CH4  4

static void ADC_GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
}

static void ADC_Config(void)
{
    ADC_InitTypeDef ADC_InitStructure;
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
    RCC_ADCCLKConfig(RCC_PCLK2_Div6);
    ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
    ADC_InitStructure.ADC_ScanConvMode = DISABLE;
    ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;
    ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
    ADC_InitStructure.ADC_NbrOfChannel = 1;
    ADC_Init(ADC1, &ADC_InitStructure);
    ADC_Cmd(ADC1, ENABLE);
    ADC_ResetCalibration(ADC1);
    while (ADC_GetResetCalibrationStatus(ADC1));
    ADC_StartCalibration(ADC1);
    while (ADC_GetCalibrationStatus(ADC1));
}

void Turbidity_Init(void)
{
    ADC_GPIO_Init();
    ADC_Config();
}

static uint16_t Get_Adc(uint8_t ch)
{
    ADC_RegularChannelConfig(ADC1, ch, 1, ADC_SampleTime_239Cycles5);
    ADC_SoftwareStartConvCmd(ADC1, ENABLE);
    while (!ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC));
    return ADC_GetConversionValue(ADC1);
}

static uint16_t Get_Adc_Average(uint8_t ch, uint8_t times)
{
    uint32_t sum = 0;
    for (uint8_t i = 0; i < times; i++)
    {
        sum += Get_Adc(ch);
    }
    return sum / times;
}

float Turbidity_Read(void)
{
    uint16_t adc = Get_Adc_Average(ADC_CH4, 10);
    float turb = 100.0f - (float)adc * 100.0f / 4096.0f;
    if (turb < 0) turb = 0;
    if (turb > 100) turb = 100;
    return turb;
}
