/* *****************************************************************************
 * File:   drv_adc.c
 * Author: XX
 *
 * Created on YYYY MM DD
 * 
 * Description: ...
 * 
 **************************************************************************** */

/* *****************************************************************************
 * Header Includes
 **************************************************************************** */
#include "drv_adc.h"

#include "sdkconfig.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "soc/soc_caps.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"
#include "esp_log.h"
#include "esp_err.h"
#include "esp_adc/adc_continuous.h"

/* *****************************************************************************
 * Configuration Definitions
 **************************************************************************** */
#define TAG "drv_adc"


/* *****************************************************************************
 * Constants and Macros Definitions
 **************************************************************************** */

#if CONFIG_DRV_ADC_AIN_0_PIN_ADC1_CH0 || CONFIG_DRV_ADC_AIN_0_PIN_ADC2_CH0
#define DRV_ADC_AIN_0_CHANNEL 0
#elif CONFIG_DRV_ADC_AIN_0_PIN_ADC1_CH1 || CONFIG_DRV_ADC_AIN_0_PIN_ADC2_CH1
#define DRV_ADC_AIN_0_CHANNEL 1
#elif CONFIG_DRV_ADC_AIN_0_PIN_ADC1_CH2 || CONFIG_DRV_ADC_AIN_0_PIN_ADC2_CH2
#define DRV_ADC_AIN_0_CHANNEL 2
#elif CONFIG_DRV_ADC_AIN_0_PIN_ADC1_CH3 || CONFIG_DRV_ADC_AIN_0_PIN_ADC2_CH3
#define DRV_ADC_AIN_0_CHANNEL 3
#elif CONFIG_DRV_ADC_AIN_0_PIN_ADC1_CH4 || CONFIG_DRV_ADC_AIN_0_PIN_ADC2_CH4
#define DRV_ADC_AIN_0_CHANNEL 4
#elif CONFIG_DRV_ADC_AIN_0_PIN_ADC1_CH5 || CONFIG_DRV_ADC_AIN_0_PIN_ADC2_CH5
#define DRV_ADC_AIN_0_CHANNEL 5
#elif CONFIG_DRV_ADC_AIN_0_PIN_ADC1_CH6 || CONFIG_DRV_ADC_AIN_0_PIN_ADC2_CH6
#define DRV_ADC_AIN_0_CHANNEL 6
#elif CONFIG_DRV_ADC_AIN_0_PIN_ADC1_CH7 || CONFIG_DRV_ADC_AIN_0_PIN_ADC2_CH7
#define DRV_ADC_AIN_0_CHANNEL 7
#elif CONFIG_DRV_ADC_AIN_0_PIN_ADC1_CH8 || CONFIG_DRV_ADC_AIN_0_PIN_ADC2_CH8
#define DRV_ADC_AIN_0_CHANNEL 8
#elif CONFIG_DRV_ADC_AIN_0_PIN_ADC1_CH9 || CONFIG_DRV_ADC_AIN_0_PIN_ADC2_CH9
#define DRV_ADC_AIN_0_CHANNEL 9
#elif defined(CONFIG_DRV_ADC_AIN_0_ADC_CHANNEL)
#define DRV_ADC_AIN_0_CHANNEL CONFIG_DRV_ADC_AIN_0_ADC_CHANNEL
#else
#define DRV_ADC_AIN_0_CHANNEL -1
#endif

#if CONFIG_DRV_ADC_AIN_0_ADC1_USE
#define DRV_ADC_AIN_0_ADC_INDEX 0       /* 0 - ADC1 */
#elif CONFIG_DRV_ADC_AIN_0_ADC2_USE
#define DRV_ADC_AIN_0_ADC_INDEX 1       /* 1 - ADC2 */
#elif defined(CONFIG_DRV_ADC_AIN_0_ADC_NUM)
#define DRV_ADC_AIN_0_ADC_INDEX CONFIG_DRV_ADC_AIN_0_ADC_NUM-1
#else
#define DRV_ADC_AIN_0_ADC_INDEX -1      /* -1 - Not Used */
#endif


#if CONFIG_DRV_ADC_AIN_1_PIN_ADC1_CH0 || CONFIG_DRV_ADC_AIN_1_PIN_ADC2_CH0
#define DRV_ADC_AIN_1_CHANNEL 0
#elif CONFIG_DRV_ADC_AIN_1_PIN_ADC1_CH1 || CONFIG_DRV_ADC_AIN_1_PIN_ADC2_CH1
#define DRV_ADC_AIN_1_CHANNEL 1
#elif CONFIG_DRV_ADC_AIN_1_PIN_ADC1_CH2 || CONFIG_DRV_ADC_AIN_1_PIN_ADC2_CH2
#define DRV_ADC_AIN_1_CHANNEL 2
#elif CONFIG_DRV_ADC_AIN_1_PIN_ADC1_CH3 || CONFIG_DRV_ADC_AIN_1_PIN_ADC2_CH3
#define DRV_ADC_AIN_1_CHANNEL 3
#elif CONFIG_DRV_ADC_AIN_1_PIN_ADC1_CH4 || CONFIG_DRV_ADC_AIN_1_PIN_ADC2_CH4
#define DRV_ADC_AIN_1_CHANNEL 4
#elif CONFIG_DRV_ADC_AIN_1_PIN_ADC1_CH5 || CONFIG_DRV_ADC_AIN_1_PIN_ADC2_CH5
#define DRV_ADC_AIN_1_CHANNEL 5
#elif CONFIG_DRV_ADC_AIN_1_PIN_ADC1_CH6 || CONFIG_DRV_ADC_AIN_1_PIN_ADC2_CH6
#define DRV_ADC_AIN_1_CHANNEL 6
#elif CONFIG_DRV_ADC_AIN_1_PIN_ADC1_CH7 || CONFIG_DRV_ADC_AIN_1_PIN_ADC2_CH7
#define DRV_ADC_AIN_1_CHANNEL 7
#elif CONFIG_DRV_ADC_AIN_1_PIN_ADC1_CH8 || CONFIG_DRV_ADC_AIN_1_PIN_ADC2_CH8
#define DRV_ADC_AIN_1_CHANNEL 8
#elif CONFIG_DRV_ADC_AIN_1_PIN_ADC1_CH9 || CONFIG_DRV_ADC_AIN_1_PIN_ADC2_CH9
#define DRV_ADC_AIN_1_CHANNEL 9
#elif defined(CONFIG_DRV_ADC_AIN_1_ADC_CHANNEL)
#define DRV_ADC_AIN_1_CHANNEL CONFIG_DRV_ADC_AIN_1_ADC_CHANNEL
#else
#define DRV_ADC_AIN_1_CHANNEL -1
#endif

#if CONFIG_DRV_ADC_AIN_1_ADC1_USE
#define DRV_ADC_AIN_1_ADC_INDEX 0       /* 0 - ADC1 */
#elif CONFIG_DRV_ADC_AIN_1_ADC2_USE
#define DRV_ADC_AIN_1_ADC_INDEX 1       /* 1 - ADC2 */
#elif defined(CONFIG_DRV_ADC_AIN_1_ADC_NUM)
#define DRV_ADC_AIN_1_ADC_INDEX CONFIG_DRV_ADC_AIN_1_ADC_NUM-1
#else
#define DRV_ADC_AIN_1_ADC_INDEX -1      /* -1 - Not Used */
#endif



#if CONFIG_DRV_ADC_AIN_2_PIN_ADC1_CH0 || CONFIG_DRV_ADC_AIN_2_PIN_ADC2_CH0
#define DRV_ADC_AIN_2_CHANNEL 0
#elif CONFIG_DRV_ADC_AIN_2_PIN_ADC1_CH1 || CONFIG_DRV_ADC_AIN_2_PIN_ADC2_CH1
#define DRV_ADC_AIN_2_CHANNEL 1
#elif CONFIG_DRV_ADC_AIN_2_PIN_ADC1_CH2 || CONFIG_DRV_ADC_AIN_2_PIN_ADC2_CH2
#define DRV_ADC_AIN_2_CHANNEL 2
#elif CONFIG_DRV_ADC_AIN_2_PIN_ADC1_CH3 || CONFIG_DRV_ADC_AIN_2_PIN_ADC2_CH3
#define DRV_ADC_AIN_2_CHANNEL 3
#elif CONFIG_DRV_ADC_AIN_2_PIN_ADC1_CH4 || CONFIG_DRV_ADC_AIN_2_PIN_ADC2_CH4
#define DRV_ADC_AIN_2_CHANNEL 4
#elif CONFIG_DRV_ADC_AIN_2_PIN_ADC1_CH5 || CONFIG_DRV_ADC_AIN_2_PIN_ADC2_CH5
#define DRV_ADC_AIN_2_CHANNEL 5
#elif CONFIG_DRV_ADC_AIN_2_PIN_ADC1_CH6 || CONFIG_DRV_ADC_AIN_2_PIN_ADC2_CH6
#define DRV_ADC_AIN_2_CHANNEL 6
#elif CONFIG_DRV_ADC_AIN_2_PIN_ADC1_CH7 || CONFIG_DRV_ADC_AIN_2_PIN_ADC2_CH7
#define DRV_ADC_AIN_2_CHANNEL 7
#elif CONFIG_DRV_ADC_AIN_2_PIN_ADC1_CH8 || CONFIG_DRV_ADC_AIN_2_PIN_ADC2_CH8
#define DRV_ADC_AIN_2_CHANNEL 8
#elif CONFIG_DRV_ADC_AIN_2_PIN_ADC1_CH9 || CONFIG_DRV_ADC_AIN_2_PIN_ADC2_CH9
#define DRV_ADC_AIN_2_CHANNEL 9
#elif defined(CONFIG_DRV_ADC_AIN_2_ADC_CHANNEL)
#define DRV_ADC_AIN_2_CHANNEL CONFIG_DRV_ADC_AIN_2_ADC_CHANNEL
#else
#define DRV_ADC_AIN_2_CHANNEL -1
#endif

#if CONFIG_DRV_ADC_AIN_2_ADC1_USE
#define DRV_ADC_AIN_2_ADC_INDEX 0       /* 0 - ADC1 */
#elif CONFIG_DRV_ADC_AIN_2_ADC2_USE
#define DRV_ADC_AIN_2_ADC_INDEX 1       /* 1 - ADC2 */
#elif defined(CONFIG_DRV_ADC_AIN_2_ADC_NUM)
#define DRV_ADC_AIN_2_ADC_INDEX CONFIG_DRV_ADC_AIN_2_ADC_NUM-1
#else
#define DRV_ADC_AIN_2_ADC_INDEX -1      /* -1 - Not Used */
#endif



#if CONFIG_DRV_ADC_AIN_3_PIN_ADC1_CH0 || CONFIG_DRV_ADC_AIN_3_PIN_ADC2_CH0
#define DRV_ADC_AIN_3_CHANNEL 0
#elif CONFIG_DRV_ADC_AIN_3_PIN_ADC1_CH1 || CONFIG_DRV_ADC_AIN_3_PIN_ADC2_CH1
#define DRV_ADC_AIN_3_CHANNEL 1
#elif CONFIG_DRV_ADC_AIN_3_PIN_ADC1_CH2 || CONFIG_DRV_ADC_AIN_3_PIN_ADC2_CH2
#define DRV_ADC_AIN_3_CHANNEL 2
#elif CONFIG_DRV_ADC_AIN_3_PIN_ADC1_CH3 || CONFIG_DRV_ADC_AIN_3_PIN_ADC2_CH3
#define DRV_ADC_AIN_3_CHANNEL 3
#elif CONFIG_DRV_ADC_AIN_3_PIN_ADC1_CH4 || CONFIG_DRV_ADC_AIN_3_PIN_ADC2_CH4
#define DRV_ADC_AIN_3_CHANNEL 4
#elif CONFIG_DRV_ADC_AIN_3_PIN_ADC1_CH5 || CONFIG_DRV_ADC_AIN_3_PIN_ADC2_CH5
#define DRV_ADC_AIN_3_CHANNEL 5
#elif CONFIG_DRV_ADC_AIN_3_PIN_ADC1_CH6 || CONFIG_DRV_ADC_AIN_3_PIN_ADC2_CH6
#define DRV_ADC_AIN_3_CHANNEL 6
#elif CONFIG_DRV_ADC_AIN_3_PIN_ADC1_CH7 || CONFIG_DRV_ADC_AIN_3_PIN_ADC2_CH7
#define DRV_ADC_AIN_3_CHANNEL 7
#elif CONFIG_DRV_ADC_AIN_3_PIN_ADC1_CH8 || CONFIG_DRV_ADC_AIN_3_PIN_ADC2_CH8
#define DRV_ADC_AIN_3_CHANNEL 8
#elif CONFIG_DRV_ADC_AIN_3_PIN_ADC1_CH9 || CONFIG_DRV_ADC_AIN_3_PIN_ADC2_CH9
#define DRV_ADC_AIN_3_CHANNEL 9
#elif defined(CONFIG_DRV_ADC_AIN_3_ADC_CHANNEL)
#define DRV_ADC_AIN_3_CHANNEL CONFIG_DRV_ADC_AIN_3_ADC_CHANNEL
#else
#define DRV_ADC_AIN_3_CHANNEL -1
#endif

#if CONFIG_DRV_ADC_AIN_3_ADC1_USE
#define DRV_ADC_AIN_3_ADC_INDEX 0       /* 0 - ADC1 */
#elif CONFIG_DRV_ADC_AIN_3_ADC2_USE
#define DRV_ADC_AIN_3_ADC_INDEX 1       /* 1 - ADC2 */
#elif defined(CONFIG_DRV_ADC_AIN_3_ADC_NUM)
#define DRV_ADC_AIN_3_ADC_INDEX CONFIG_DRV_ADC_AIN_3_ADC_NUM-1
#else
#define DRV_ADC_AIN_3_ADC_INDEX -1      /* -1 - Not Used */
#endif



#if CONFIG_DRV_ADC_AIN_4_PIN_ADC1_CH0 || CONFIG_DRV_ADC_AIN_4_PIN_ADC2_CH0
#define DRV_ADC_AIN_4_CHANNEL 0
#elif CONFIG_DRV_ADC_AIN_4_PIN_ADC1_CH1 || CONFIG_DRV_ADC_AIN_4_PIN_ADC2_CH1
#define DRV_ADC_AIN_4_CHANNEL 1
#elif CONFIG_DRV_ADC_AIN_4_PIN_ADC1_CH2 || CONFIG_DRV_ADC_AIN_4_PIN_ADC2_CH2
#define DRV_ADC_AIN_4_CHANNEL 2
#elif CONFIG_DRV_ADC_AIN_4_PIN_ADC1_CH3 || CONFIG_DRV_ADC_AIN_4_PIN_ADC2_CH3
#define DRV_ADC_AIN_4_CHANNEL 3
#elif CONFIG_DRV_ADC_AIN_4_PIN_ADC1_CH4 || CONFIG_DRV_ADC_AIN_4_PIN_ADC2_CH4
#define DRV_ADC_AIN_4_CHANNEL 4
#elif CONFIG_DRV_ADC_AIN_4_PIN_ADC1_CH5 || CONFIG_DRV_ADC_AIN_4_PIN_ADC2_CH5
#define DRV_ADC_AIN_4_CHANNEL 5
#elif CONFIG_DRV_ADC_AIN_4_PIN_ADC1_CH6 || CONFIG_DRV_ADC_AIN_4_PIN_ADC2_CH6
#define DRV_ADC_AIN_4_CHANNEL 6
#elif CONFIG_DRV_ADC_AIN_4_PIN_ADC1_CH7 || CONFIG_DRV_ADC_AIN_4_PIN_ADC2_CH7
#define DRV_ADC_AIN_4_CHANNEL 7
#elif CONFIG_DRV_ADC_AIN_4_PIN_ADC1_CH8 || CONFIG_DRV_ADC_AIN_4_PIN_ADC2_CH8
#define DRV_ADC_AIN_4_CHANNEL 8
#elif CONFIG_DRV_ADC_AIN_4_PIN_ADC1_CH9 || CONFIG_DRV_ADC_AIN_4_PIN_ADC2_CH9
#define DRV_ADC_AIN_4_CHANNEL 9
#elif defined(CONFIG_DRV_ADC_AIN_4_ADC_CHANNEL)
#define DRV_ADC_AIN_4_CHANNEL CONFIG_DRV_ADC_AIN_4_ADC_CHANNEL
#else
#define DRV_ADC_AIN_4_CHANNEL -1
#endif

#if CONFIG_DRV_ADC_AIN_4_ADC1_USE
#define DRV_ADC_AIN_4_ADC_INDEX 0       /* 0 - ADC1 */
#elif CONFIG_DRV_ADC_AIN_4_ADC2_USE
#define DRV_ADC_AIN_4_ADC_INDEX 1       /* 1 - ADC2 */
#elif defined(CONFIG_DRV_ADC_AIN_4_ADC_NUM)
#define DRV_ADC_AIN_4_ADC_INDEX CONFIG_DRV_ADC_AIN_4_ADC_NUM-1
#else
#define DRV_ADC_AIN_4_ADC_INDEX -1      /* -1 - Not Used */
#endif



#if CONFIG_DRV_ADC_AIN_5_PIN_ADC1_CH0 || CONFIG_DRV_ADC_AIN_5_PIN_ADC2_CH0
#define DRV_ADC_AIN_5_CHANNEL 0
#elif CONFIG_DRV_ADC_AIN_5_PIN_ADC1_CH1 || CONFIG_DRV_ADC_AIN_5_PIN_ADC2_CH1
#define DRV_ADC_AIN_5_CHANNEL 1
#elif CONFIG_DRV_ADC_AIN_5_PIN_ADC1_CH2 || CONFIG_DRV_ADC_AIN_5_PIN_ADC2_CH2
#define DRV_ADC_AIN_5_CHANNEL 2
#elif CONFIG_DRV_ADC_AIN_5_PIN_ADC1_CH3 || CONFIG_DRV_ADC_AIN_5_PIN_ADC2_CH3
#define DRV_ADC_AIN_5_CHANNEL 3
#elif CONFIG_DRV_ADC_AIN_5_PIN_ADC1_CH4 || CONFIG_DRV_ADC_AIN_5_PIN_ADC2_CH4
#define DRV_ADC_AIN_5_CHANNEL 4
#elif CONFIG_DRV_ADC_AIN_5_PIN_ADC1_CH5 || CONFIG_DRV_ADC_AIN_5_PIN_ADC2_CH5
#define DRV_ADC_AIN_5_CHANNEL 5
#elif CONFIG_DRV_ADC_AIN_5_PIN_ADC1_CH6 || CONFIG_DRV_ADC_AIN_5_PIN_ADC2_CH6
#define DRV_ADC_AIN_5_CHANNEL 6
#elif CONFIG_DRV_ADC_AIN_5_PIN_ADC1_CH7 || CONFIG_DRV_ADC_AIN_5_PIN_ADC2_CH7
#define DRV_ADC_AIN_5_CHANNEL 7
#elif CONFIG_DRV_ADC_AIN_5_PIN_ADC1_CH8 || CONFIG_DRV_ADC_AIN_5_PIN_ADC2_CH8
#define DRV_ADC_AIN_5_CHANNEL 8
#elif CONFIG_DRV_ADC_AIN_5_PIN_ADC1_CH9 || CONFIG_DRV_ADC_AIN_5_PIN_ADC2_CH9
#define DRV_ADC_AIN_5_CHANNEL 9
#elif defined(CONFIG_DRV_ADC_AIN_5_ADC_CHANNEL)
#define DRV_ADC_AIN_5_CHANNEL CONFIG_DRV_ADC_AIN_5_ADC_CHANNEL
#else
#define DRV_ADC_AIN_5_CHANNEL -1
#endif

#if CONFIG_DRV_ADC_AIN_5_ADC1_USE
#define DRV_ADC_AIN_5_ADC_INDEX 0       /* 0 - ADC1 */
#elif CONFIG_DRV_ADC_AIN_5_ADC2_USE
#define DRV_ADC_AIN_5_ADC_INDEX 1       /* 1 - ADC2 */
#elif defined(CONFIG_DRV_ADC_AIN_5_ADC_NUM)
#define DRV_ADC_AIN_5_ADC_INDEX CONFIG_DRV_ADC_AIN_5_ADC_NUM-1
#else
#define DRV_ADC_AIN_5_ADC_INDEX -1      /* -1 - Not Used */
#endif



#if CONFIG_DRV_ADC_AIN_6_PIN_ADC1_CH0 || CONFIG_DRV_ADC_AIN_6_PIN_ADC2_CH0
#define DRV_ADC_AIN_6_CHANNEL 0
#elif CONFIG_DRV_ADC_AIN_6_PIN_ADC1_CH1 || CONFIG_DRV_ADC_AIN_6_PIN_ADC2_CH1
#define DRV_ADC_AIN_6_CHANNEL 1
#elif CONFIG_DRV_ADC_AIN_6_PIN_ADC1_CH2 || CONFIG_DRV_ADC_AIN_6_PIN_ADC2_CH2
#define DRV_ADC_AIN_6_CHANNEL 2
#elif CONFIG_DRV_ADC_AIN_6_PIN_ADC1_CH3 || CONFIG_DRV_ADC_AIN_6_PIN_ADC2_CH3
#define DRV_ADC_AIN_6_CHANNEL 3
#elif CONFIG_DRV_ADC_AIN_6_PIN_ADC1_CH4 || CONFIG_DRV_ADC_AIN_6_PIN_ADC2_CH4
#define DRV_ADC_AIN_6_CHANNEL 4
#elif CONFIG_DRV_ADC_AIN_6_PIN_ADC1_CH5 || CONFIG_DRV_ADC_AIN_6_PIN_ADC2_CH5
#define DRV_ADC_AIN_6_CHANNEL 5
#elif CONFIG_DRV_ADC_AIN_6_PIN_ADC1_CH6 || CONFIG_DRV_ADC_AIN_6_PIN_ADC2_CH6
#define DRV_ADC_AIN_6_CHANNEL 6
#elif CONFIG_DRV_ADC_AIN_6_PIN_ADC1_CH7 || CONFIG_DRV_ADC_AIN_6_PIN_ADC2_CH7
#define DRV_ADC_AIN_6_CHANNEL 7
#elif CONFIG_DRV_ADC_AIN_6_PIN_ADC1_CH8 || CONFIG_DRV_ADC_AIN_6_PIN_ADC2_CH8
#define DRV_ADC_AIN_6_CHANNEL 8
#elif CONFIG_DRV_ADC_AIN_6_PIN_ADC1_CH9 || CONFIG_DRV_ADC_AIN_6_PIN_ADC2_CH9
#define DRV_ADC_AIN_6_CHANNEL 9
#elif defined(CONFIG_DRV_ADC_AIN_6_ADC_CHANNEL)
#define DRV_ADC_AIN_6_CHANNEL CONFIG_DRV_ADC_AIN_6_ADC_CHANNEL
#else
#define DRV_ADC_AIN_6_CHANNEL -1
#endif

#if CONFIG_DRV_ADC_AIN_6_ADC1_USE
#define DRV_ADC_AIN_6_ADC_INDEX 0       /* 0 - ADC1 */
#elif CONFIG_DRV_ADC_AIN_6_ADC2_USE
#define DRV_ADC_AIN_6_ADC_INDEX 1       /* 1 - ADC2 */
#elif defined(CONFIG_DRV_ADC_AIN_6_ADC_NUM)
#define DRV_ADC_AIN_6_ADC_INDEX CONFIG_DRV_ADC_AIN_6_ADC_NUM-1
#else
#define DRV_ADC_AIN_6_ADC_INDEX -1      /* -1 - Not Used */
#endif


#if CONFIG_DRV_ADC_AIN_7_PIN_ADC1_CH0 || CONFIG_DRV_ADC_AIN_7_PIN_ADC2_CH0
#define DRV_ADC_AIN_7_CHANNEL 0
#elif CONFIG_DRV_ADC_AIN_7_PIN_ADC1_CH1 || CONFIG_DRV_ADC_AIN_7_PIN_ADC2_CH1
#define DRV_ADC_AIN_7_CHANNEL 1
#elif CONFIG_DRV_ADC_AIN_7_PIN_ADC1_CH2 || CONFIG_DRV_ADC_AIN_7_PIN_ADC2_CH2
#define DRV_ADC_AIN_7_CHANNEL 2
#elif CONFIG_DRV_ADC_AIN_7_PIN_ADC1_CH3 || CONFIG_DRV_ADC_AIN_7_PIN_ADC2_CH3
#define DRV_ADC_AIN_7_CHANNEL 3
#elif CONFIG_DRV_ADC_AIN_7_PIN_ADC1_CH4 || CONFIG_DRV_ADC_AIN_7_PIN_ADC2_CH4
#define DRV_ADC_AIN_7_CHANNEL 4
#elif CONFIG_DRV_ADC_AIN_7_PIN_ADC1_CH5 || CONFIG_DRV_ADC_AIN_7_PIN_ADC2_CH5
#define DRV_ADC_AIN_7_CHANNEL 5
#elif CONFIG_DRV_ADC_AIN_7_PIN_ADC1_CH6 || CONFIG_DRV_ADC_AIN_7_PIN_ADC2_CH6
#define DRV_ADC_AIN_7_CHANNEL 6
#elif CONFIG_DRV_ADC_AIN_7_PIN_ADC1_CH7 || CONFIG_DRV_ADC_AIN_7_PIN_ADC2_CH7
#define DRV_ADC_AIN_7_CHANNEL 7
#elif CONFIG_DRV_ADC_AIN_7_PIN_ADC1_CH8 || CONFIG_DRV_ADC_AIN_7_PIN_ADC2_CH8
#define DRV_ADC_AIN_7_CHANNEL 8
#elif CONFIG_DRV_ADC_AIN_7_PIN_ADC1_CH9 || CONFIG_DRV_ADC_AIN_7_PIN_ADC2_CH9
#define DRV_ADC_AIN_7_CHANNEL 9
#elif defined(CONFIG_DRV_ADC_AIN_7_ADC_CHANNEL)
#define DRV_ADC_AIN_7_CHANNEL CONFIG_DRV_ADC_AIN_7_ADC_CHANNEL
#else
#define DRV_ADC_AIN_7_CHANNEL -1
#endif

#if CONFIG_DRV_ADC_AIN_7_ADC1_USE
#define DRV_ADC_AIN_7_ADC_INDEX 0       /* 0 - ADC1 */
#elif CONFIG_DRV_ADC_AIN_7_ADC2_USE
#define DRV_ADC_AIN_7_ADC_INDEX 1       /* 1 - ADC2 */
#elif defined(CONFIG_DRV_ADC_AIN_7_ADC_NUM)
#define DRV_ADC_AIN_7_ADC_INDEX CONFIG_DRV_ADC_AIN_7_ADC_NUM-1
#else
#define DRV_ADC_AIN_7_ADC_INDEX -1      /* -1 - Not Used */
#endif



#if CONFIG_IDF_TARGET_ESP32
#define ADC_CONV_MODE       ADC_CONV_SINGLE_UNIT_1  //ESP32 only supports ADC1 DMA mode
#define ADC_OUTPUT_TYPE     ADC_DIGI_OUTPUT_FORMAT_TYPE1
#elif CONFIG_IDF_TARGET_ESP32S2
#define ADC_CONV_MODE       ADC_CONV_BOTH_UNIT
#define ADC_OUTPUT_TYPE     ADC_DIGI_OUTPUT_FORMAT_TYPE2
#elif CONFIG_IDF_TARGET_ESP32C3 || CONFIG_IDF_TARGET_ESP32H2 || CONFIG_IDF_TARGET_ESP32C2
#define ADC_CONV_MODE       ADC_CONV_ALTER_UNIT     //ESP32C3 only supports alter mode
#define ADC_OUTPUT_TYPE     ADC_DIGI_OUTPUT_FORMAT_TYPE2
#elif CONFIG_IDF_TARGET_ESP32S3
#define ADC_CONV_MODE       ADC_CONV_BOTH_UNIT
#define ADC_OUTPUT_TYPE     ADC_DIGI_OUTPUT_FORMAT_TYPE2
#endif


/* *****************************************************************************
 * Enumeration Definitions
 **************************************************************************** */

/* *****************************************************************************
 * Type Definitions
 **************************************************************************** */

/* *****************************************************************************
 * Function-Like Macros
 **************************************************************************** */

/* *****************************************************************************
 * Variables Definitions
 **************************************************************************** */

static int ain_adc[CONFIG_DRV_ADC_AIN_MAX] = 
{
    DRV_ADC_AIN_0_ADC_INDEX,
    DRV_ADC_AIN_1_ADC_INDEX,
    DRV_ADC_AIN_2_ADC_INDEX,
    DRV_ADC_AIN_3_ADC_INDEX,
    DRV_ADC_AIN_4_ADC_INDEX,
    DRV_ADC_AIN_5_ADC_INDEX,
    DRV_ADC_AIN_6_ADC_INDEX,
    DRV_ADC_AIN_7_ADC_INDEX,
};

static int ain_chn[CONFIG_DRV_ADC_AIN_MAX] = 
{
    DRV_ADC_AIN_0_CHANNEL,
    DRV_ADC_AIN_1_CHANNEL,
    DRV_ADC_AIN_2_CHANNEL,
    DRV_ADC_AIN_3_CHANNEL,
    DRV_ADC_AIN_4_CHANNEL,
    DRV_ADC_AIN_5_CHANNEL,
    DRV_ADC_AIN_6_CHANNEL,
    DRV_ADC_AIN_7_CHANNEL,
};


bool do_calibration1 = false;
adc_oneshot_unit_handle_t adc1_handle;
adc_cali_handle_t adc1_cali_handle = NULL;
#if (SOC_ADC_PERIPH_NUM >= 2)
bool do_calibration2 = false;
adc_oneshot_unit_handle_t adc2_handle;
adc_cali_handle_t adc2_cali_handle = NULL;
#endif

adc_continuous_handle_t handle = NULL;

static int adc_raw[CONFIG_DRV_ADC_ADC_COUNT_MAX][CONFIG_DRV_ADC_CHANNEL_RANGE_MAX];
static int voltage[CONFIG_DRV_ADC_ADC_COUNT_MAX][CONFIG_DRV_ADC_CHANNEL_RANGE_MAX];


TaskHandle_t oneshot_task_handle = NULL;
TaskHandle_t continuous_task_handle = NULL;


adc_digi_output_data_t continuous_read_result[CONFIG_DRV_ADC_ADC_COUNT_MAX] = {0};
int channels_continuous_read = 0;


/* *****************************************************************************
 * Prototype of functions definitions
 **************************************************************************** */
static bool adc_calibration_init(adc_unit_t unit, adc_atten_t atten, adc_cali_handle_t *out_handle);
static void adc_calibration_deinit(adc_cali_handle_t handle);

/* *****************************************************************************
 * Functions
 **************************************************************************** */

static bool adc_calibration_init(adc_unit_t unit, adc_atten_t atten, adc_cali_handle_t *out_handle)
{
    adc_cali_handle_t handle = NULL;
    esp_err_t ret = ESP_FAIL;
    bool calibrated = false;

#if ADC_CALI_SCHEME_CURVE_FITTING_SUPPORTED
    if (!calibrated) {
        ESP_LOGI(TAG, "calibration scheme version is %s", "Curve Fitting");
        adc_cali_curve_fitting_config_t cali_config = {
            .unit_id = unit,
            .atten = atten,
            .bitwidth = ADC_BITWIDTH_DEFAULT,
        };
        ret = adc_cali_create_scheme_curve_fitting(&cali_config, &handle);
        if (ret == ESP_OK) {
            calibrated = true;
        }
    }
#endif

#if ADC_CALI_SCHEME_LINE_FITTING_SUPPORTED
    if (!calibrated) {
        ESP_LOGI(TAG, "calibration scheme version is %s", "Line Fitting");
        adc_cali_line_fitting_config_t cali_config = {
            .unit_id = unit,
            .atten = atten,
            .bitwidth = ADC_BITWIDTH_DEFAULT,
        };
        ret = adc_cali_create_scheme_line_fitting(&cali_config, &handle);
        if (ret == ESP_OK) {
            calibrated = true;
        }
    }
#endif

    *out_handle = handle;
    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "Calibration Success");
    } else if (ret == ESP_ERR_NOT_SUPPORTED || !calibrated) {
        ESP_LOGW(TAG, "eFuse not burnt, skip software calibration");
    } else {
        ESP_LOGE(TAG, "Invalid arg or no memory");
    }

    return calibrated;
}

static void adc_calibration_deinit(adc_cali_handle_t handle)
{
#if ADC_CALI_SCHEME_CURVE_FITTING_SUPPORTED
    ESP_LOGI(TAG, "deregister %s calibration scheme", "Curve Fitting");
    ESP_ERROR_CHECK(adc_cali_delete_scheme_curve_fitting(handle));

#elif ADC_CALI_SCHEME_LINE_FITTING_SUPPORTED
    ESP_LOGI(TAG, "deregister %s calibration scheme", "Line Fitting");
    ESP_ERROR_CHECK(adc_cali_delete_scheme_line_fitting(handle));
#endif
}

void adc_init_one_shot(void)
{
    //-------------ADC1 Init---------------//
    adc_oneshot_unit_init_cfg_t init_config1 = {
        .unit_id = ADC_UNIT_1,
    };
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config1, &adc1_handle));

    //-------------ADC1 Calibration Init---------------//
    do_calibration1 = adc_calibration_init(ADC_UNIT_1, ADC_ATTEN_DB_11, &adc1_cali_handle);



    //-------------ADC2 Init---------------//
    adc_oneshot_unit_init_cfg_t init_config2 = {
        .unit_id = ADC_UNIT_2,
        .ulp_mode = ADC_ULP_MODE_DISABLE,
    };
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config2, &adc2_handle));

    //-------------ADC2 Calibration Init---------------//
    do_calibration2 = adc_calibration_init(ADC_UNIT_2, ADC_ATTEN_DB_11, &adc2_cali_handle);


    //-------------ADC Config---------------//
    adc_oneshot_chan_cfg_t config = {
        .bitwidth = ADC_BITWIDTH_DEFAULT,
        .atten = ADC_ATTEN_DB_11,
    };

    for (int index = 0; index < CONFIG_DRV_ADC_AIN_MAX; index++)
    {
        adc_oneshot_unit_handle_t adc_handle = (ain_adc[index] == 0) ? adc1_handle : (ain_adc[index] == 1) ? adc2_handle : NULL;
        if ((adc_handle != NULL) && (ain_chn[index] >= 0))
        {
            ESP_ERROR_CHECK(adc_oneshot_config_channel(adc_handle, ain_chn[index], &config));
        }
    }


}

void adc_task_one_shot(void* param)
{

    while (1) 
    {
        bool sample_passed = false;

        for (int index = 0; index < CONFIG_DRV_ADC_AIN_MAX; index++)
        {

            adc_oneshot_unit_handle_t adc_handle = (ain_adc[index] == 0) ? adc1_handle : (ain_adc[index] == 1) ? adc2_handle : NULL;
            if ((adc_handle != NULL) && (ain_chn[index] >= 0))
            {
                //ESP_LOGI(TAG, "ADC%d Channel[%d] Requested", ain_adc[index] + 1, ain_chn[index]);
                //vTaskDelay(pdMS_TO_TICKS(1000));

                esp_err_t err = adc_oneshot_read(adc_handle, ain_chn[index], &adc_raw[ain_adc[index]][ain_chn[index]]);
                if (err != ESP_OK)
                {
                    ESP_LOGI(TAG, "ADC%d Channel[%d] Fail to read err %X (%s)", ain_adc[index] + 1, ain_chn[index], err, esp_err_to_name(err));
                }
                ESP_LOGI(TAG, "ADC%d Channel[%d] Raw Data: %d", ain_adc[index] + 1, ain_chn[index], adc_raw[ain_adc[index]][ain_chn[index]]);
                if (do_calibration1) 
                {
                    ESP_ERROR_CHECK(adc_cali_raw_to_voltage(adc1_cali_handle, adc_raw[ain_adc[index]][ain_chn[index]], &voltage[ain_adc[index]][ain_chn[index]]));
                    ESP_LOGI(TAG, "ADC%d Channel[%d] Cali Voltage: %d mV", ain_adc[index] + 1, ain_chn[index], voltage[ain_adc[index]][ain_chn[index]]);
                }
                sample_passed = true;
                vTaskDelay(pdMS_TO_TICKS(100));
            }
            
        }
        if (sample_passed == false)
        {
            vTaskDelay(pdMS_TO_TICKS(900));
        }
        else
        {
            vTaskDelay(pdMS_TO_TICKS(1000));
        }
        
    }
    
}

void adc_deinit_one_shot(void)
{
    //Tear Down
    ESP_ERROR_CHECK(adc_oneshot_del_unit(adc1_handle));
    if (do_calibration1) {
        adc_calibration_deinit(adc1_cali_handle);
    }

#if (SOC_ADC_PERIPH_NUM >= 2)
    ESP_ERROR_CHECK(adc_oneshot_del_unit(adc2_handle));
    if (do_calibration2) {
        adc_calibration_deinit(adc2_cali_handle);
    }
#endif /* #if (SOC_ADC_PERIPH_NUM >= 2) */

}




static bool IRAM_ATTR s_conv_done_cb(adc_continuous_handle_t handle, const adc_continuous_evt_data_t *edata, void *user_data)
{
    BaseType_t mustYield = pdFALSE;
    //Notify that ADC continuous driver has done enough number of conversions
    vTaskNotifyGiveFromISR(continuous_task_handle, &mustYield);

    return (mustYield == pdTRUE);
}

static void continuous_adc_init(adc_continuous_handle_t *out_handle)
{
    adc_continuous_handle_t handle = NULL;

    adc_continuous_handle_cfg_t adc_config = {
        .max_store_buf_size = 1024,
        .conv_frame_size = sizeof(continuous_read_result),
    };
    ESP_ERROR_CHECK(adc_continuous_new_handle(&adc_config, &handle));

    adc_continuous_config_t dig_cfg = {
        .sample_freq_hz = 20 * 1000,
        .conv_mode = ADC_CONV_MODE,
        .format = ADC_OUTPUT_TYPE,
    };

    adc_digi_pattern_config_t adc_pattern[SOC_ADC_PATT_LEN_MAX] = {0};
    dig_cfg.pattern_num = CONFIG_DRV_ADC_AIN_MAX;
    channels_continuous_read = CONFIG_DRV_ADC_AIN_MAX;
    for (int i = 0; i < CONFIG_DRV_ADC_AIN_MAX; i++) 
    {
        uint8_t unit = ain_adc[i];
        uint8_t ch = ain_chn[i];

        if ((ain_adc[i] >= 0) && (ain_chn[i] >= 0))
        {
            adc_pattern[i].atten = ADC_ATTEN_DB_0;
            adc_pattern[i].channel = ch;
            adc_pattern[i].unit = unit;
            adc_pattern[i].bit_width = SOC_ADC_DIGI_MAX_BITWIDTH;

            ESP_LOGI(TAG, "adc_pattern[%d].atten is :%x", i, adc_pattern[i].atten);
            ESP_LOGI(TAG, "adc_pattern[%d].channel is :%x", i, adc_pattern[i].channel);
            ESP_LOGI(TAG, "adc_pattern[%d].unit is :%x", i, adc_pattern[i].unit);

        }
        else
        {
            ESP_LOGE(TAG, "adc_pattern[%d] unit:%x channel:%x Invalid - fix CONFIG_DRV_ADC_AIN_MAX - limited to %d", i, ain_adc[i], ain_chn[i], i);
            dig_cfg.pattern_num = i;
            channels_continuous_read = i;
            break;
        }

    }
    dig_cfg.adc_pattern = adc_pattern;
    ESP_ERROR_CHECK(adc_continuous_config(handle, &dig_cfg));

    *out_handle = handle;
}

#if !CONFIG_IDF_TARGET_ESP32
static bool check_valid_data(const adc_digi_output_data_t *data)
{
    const unsigned int unit = data->type2.unit;
    if (unit > 2) return false;
    if (data->type2.channel >= SOC_ADC_CHANNEL_NUM(unit)) return false;

    return true;
}
#endif





void adc_init_continuous(void)
{
    continuous_adc_init(&handle);

    adc_continuous_evt_cbs_t cbs = 
    {
        .on_conv_done = s_conv_done_cb,
    };
    ESP_ERROR_CHECK(adc_continuous_register_event_callbacks(handle, &cbs, NULL));
    ESP_ERROR_CHECK(adc_continuous_start(handle));
}

void adc_task_continuous(void* param)
{
    memset(continuous_read_result, 0xFF, sizeof(continuous_read_result));   //set invalid data

    //continuous_task_handle = xTaskGetCurrentTaskHandle(); //not needed - made on Create

    while (1) 
    {
        /**
         * This is to show you the way to use the ADC continuous mode driver event callback.
         * This `ulTaskNotifyTake` will block when the data processing in the task is fast.
         * However in this example, the data processing (print) is slow, so you barely block here.
         *
         * Without using this event callback (to notify this task), you can still just call
         * `adc_continuous_read()` here in a loop, with/without a certain block timeout.
         */
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

        while (1) 
        {
            uint32_t ret_num = 0;
            esp_err_t ret;
            ret = adc_continuous_read(handle, (uint8_t*)continuous_read_result, channels_continuous_read * SOC_ADC_DIGI_RESULT_BYTES, &ret_num, 0);
            if (ret == ESP_OK) 
            {
                int channels_read = ret_num / SOC_ADC_DIGI_RESULT_BYTES;
                ESP_LOGI("TASK", "ret is %x, ret_num is %"PRIu32" bytes (%d Channels)", ret, ret_num, channels_read);
                for (int i = 0; i < channels_read; i++) 
                {
                    adc_digi_output_data_t *p = &continuous_read_result[i];

                    int ch;
                    int ch_ain;
                    int ain_index;
                    int unit_ain;


                    #if CONFIG_IDF_TARGET_ESP32

                    ch = p->type1.channel;
                    for (ain_index = 0; ain_index < channels_read; ain_index++)
                    {
                        if (ain_chn[ain_index] == ch)
                        {
                            break;
                        }
                    }


                    if (ain_index >= channels_read)
                    {
                        ESP_LOGE(TAG, "Read Channel[%d] Not found in AIN Channels", ch);
                        unit_ain = ain_adc[ain_index];
                        ch_ain = ain_chn[ain_index];
                        adc_raw[unit_ain][ch_ain] = p->type1.data;
                    }



                    ESP_LOGI(TAG, "AIN: %d, Unit: %d, Channel: %d, Value: %x", ain_index, 1, p->type1.channel, p->type1.data);
                    #else
                    if (ADC_CONV_MODE == ADC_CONV_BOTH_UNIT || ADC_CONV_MODE == ADC_CONV_ALTER_UNIT) 
                    {
                        if (check_valid_data(p)) 
                        {
                            ESP_LOGI(TAG, "Unit: %d,_Channel: %d, Value: %x", p->type2.unit+1, p->type2.channel, p->type2.data);
                        } 
                        else 
                        {
                            ESP_LOGI(TAG, "Invalid data [%d_%d_%x]", p->type2.unit+1, p->type2.channel, p->type2.data);
                        }
                    }
                    #if CONFIG_IDF_TARGET_ESP32S2
                    else if (ADC_CONV_MODE == ADC_CONV_SINGLE_UNIT_2) 
                    {
                        ESP_LOGI(TAG, "Unit: %d, Channel: %d, Value: %x", 2, p->type1.channel, p->type1.data);
                    } 
                    else if (ADC_CONV_MODE == ADC_CONV_SINGLE_UNIT_1) 
                    {
                        ESP_LOGI(TAG, "Unit: %d, Channel: %d, Value: %x", 1, p->type1.channel, p->type1.data);
                    }
                    #endif  /* #if CONFIG_IDF_TARGET_ESP32S2 */

                    #endif  /* #if CONFIG_IDF_TARGET_ESP32 */

                    
                }
                /**
                 * Because printing is slow, so every time you call `ulTaskNotifyTake`, it will immediately return.
                 * To avoid a task watchdog timeout, add a delay here. When you replace the way you process the data,
                 * usually you don't need this delay (as this task will block for a while).
                 */
                vTaskDelay(100);
            } else if (ret == ESP_ERR_TIMEOUT) {
                //We try to read `CONTINUOUS_READ_LEN` until API returns timeout, which means there's no available data
                break;
            }
        }
    }
}

void adc_deinit_continuous(void)
{
    ESP_ERROR_CHECK(adc_continuous_stop(handle));
    ESP_ERROR_CHECK(adc_continuous_deinit(handle));
}


void drv_adc_init(void)
{
    #if CONFIG_DRV_ADC_CONTINUOUS
    adc_init_continuous();
    #else
    adc_init_one_shot();
    #endif


    #if CONFIG_DRV_ADC_CONTINUOUS
    xTaskCreate(adc_task_continuous, "adc_continuous", 4096, NULL, configMAX_PRIORITIES - 20, &continuous_task_handle);
    #else
    xTaskCreate(adc_task_one_shot, "adc_oneshot", 4096, NULL, configMAX_PRIORITIES - 20, &oneshot_task_handle);
    #endif

}

void drv_adc_deinit(void)
{
    #if CONFIG_DRV_ADC_CONTINUOUS
    adc_deinit_continuous();
    #else
    adc_deinit_one_shot();
    #endif 
}
