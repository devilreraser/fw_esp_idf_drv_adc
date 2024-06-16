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
#include "esp_log.h"
#include "esp_err.h"
#include "esp_idf_version.h"


/* *****************************************************************************
 * Configuration Definitions - IDF VERSION
 **************************************************************************** */
#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 0, 0)
#define ESP_ADC_VERSION_BIGGER_OR_EQUAL_TO_5    0       /* force using old adc_driver with esp-idf version >= 5.0.0 */
#endif


#ifndef ESP_ADC_VERSION_BIGGER_OR_EQUAL_TO_5
#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 0, 0)
#define ESP_ADC_VERSION_BIGGER_OR_EQUAL_TO_5    1
#else
#define ESP_ADC_VERSION_BIGGER_OR_EQUAL_TO_5    0
#endif
#endif




/* *****************************************************************************
 * Header Includes - depending on IDF VERSION
 **************************************************************************** */
#if ESP_ADC_VERSION_BIGGER_OR_EQUAL_TO_5
#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_continuous.h"
#else
#include "driver/adc.h"
#endif

#if ESP_ADC_VERSION_BIGGER_OR_EQUAL_TO_5
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"
#else
#include "esp_adc_cal.h"
#endif


/* *****************************************************************************
 * Configuration Definitions
 **************************************************************************** */
#define TAG "drv_adc"

#if ESP_ADC_VERSION_BIGGER_OR_EQUAL_TO_5
#define ADC_CONT_COEF_OVERSIZE_POOL_BUFFER  1           /* how many times to allow buffer non-processed in continuous read bunch of samples */
#define ADC_CONT_COEF_OVERSIZE_READ_BUFFER  1           /* on process data read how many frames of data to get */
#endif



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

#ifndef CONFIG_DRV_ADC_AIN_0_NAME
#define CONFIG_DRV_ADC_AIN_0_NAME "AIN0"
#endif

#ifndef CONFIG_DRV_ADC_AIN_1_NAME
#define CONFIG_DRV_ADC_AIN_1_NAME "AIN1"
#endif

#ifndef CONFIG_DRV_ADC_AIN_2_NAME
#define CONFIG_DRV_ADC_AIN_2_NAME "AIN2"
#endif

#ifndef CONFIG_DRV_ADC_AIN_3_NAME
#define CONFIG_DRV_ADC_AIN_3_NAME "AIN3"
#endif

#ifndef CONFIG_DRV_ADC_AIN_4_NAME
#define CONFIG_DRV_ADC_AIN_4_NAME "AIN4"
#endif

#ifndef CONFIG_DRV_ADC_AIN_5_NAME
#define CONFIG_DRV_ADC_AIN_5_NAME "AIN5"
#endif

#ifndef CONFIG_DRV_ADC_AIN_6_NAME
#define CONFIG_DRV_ADC_AIN_6_NAME "AIN6"
#endif

#ifndef CONFIG_DRV_ADC_AIN_7_NAME
#define CONFIG_DRV_ADC_AIN_7_NAME "AIN7"
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
static char* ain_name[CONFIG_DRV_ADC_AIN_MAX] = 
{
    CONFIG_DRV_ADC_AIN_0_NAME,
    CONFIG_DRV_ADC_AIN_1_NAME,
    CONFIG_DRV_ADC_AIN_2_NAME,
    CONFIG_DRV_ADC_AIN_3_NAME,
    CONFIG_DRV_ADC_AIN_4_NAME,
    CONFIG_DRV_ADC_AIN_5_NAME,
    CONFIG_DRV_ADC_AIN_6_NAME,
    CONFIG_DRV_ADC_AIN_7_NAME,
};


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

static int adc_raw[CONFIG_DRV_ADC_ADC_COUNT_MAX][CONFIG_DRV_ADC_CHANNEL_RANGE_MAX];
static int voltage[CONFIG_DRV_ADC_ADC_COUNT_MAX][CONFIG_DRV_ADC_CHANNEL_RANGE_MAX];

bool calibration_enabled[CONFIG_DRV_ADC_ADC_COUNT_MAX] = { false };    

#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5,2,0)
/*  ADC_ATTEN_DB_12  = 3,  !<The input voltage of ADC will be attenuated extending the range of measurement by about 12 dB (3.98 x) */
static const adc_atten_t attenuation = ADC_ATTEN_DB_12;
#else
/*  ADC_ATTEN_DB_0   = 0,  !<No input attenumation, ADC can measure up to approx. 800 mV. */
/*  ADC_ATTEN_DB_2_5 = 1,  !<The input voltage of ADC will be attenuated extending the range of measurement by about 2.5 dB (1.33 x) */
/*  ADC_ATTEN_DB_6   = 2,  !<The input voltage of ADC will be attenuated extending the range of measurement by about 6 dB (2 x) */
/*  ADC_ATTEN_DB_11  = 3,  !<The input voltage of ADC will be attenuated extending the range of measurement by about 11 dB (3.55 x) */
static const adc_atten_t attenuation = ADC_ATTEN_DB_11;
#endif 


#if ESP_ADC_VERSION_BIGGER_OR_EQUAL_TO_5

adc_oneshot_unit_handle_t adc1_handle;
adc_cali_handle_t adc1_cali_handle = NULL;
#if (SOC_ADC_PERIPH_NUM >= 2)
adc_oneshot_unit_handle_t adc2_handle;
adc_cali_handle_t adc2_cali_handle = NULL;
#endif
adc_continuous_handle_t handle = NULL;
adc_digi_output_data_t continuous_read_result[CONFIG_DRV_ADC_AIN_MAX * ADC_CONT_COEF_OVERSIZE_READ_BUFFER] = {0};

static int channels_continuous_read = 0;

static char printBuffer[256];

#else   // ESP_ADC_VERSION_BIGGER_OR_EQUAL_TO_5

static esp_adc_cal_characteristics_t adc_chars[CONFIG_DRV_ADC_ADC_COUNT_MAX];



#endif  // ESP_ADC_VERSION_BIGGER_OR_EQUAL_TO_5

TaskHandle_t oneshot_task_handle = NULL;
TaskHandle_t continuous_task_handle = NULL;


static int cont_sample_notify_count = 0;
static int cont_expected_process_count = 0;
static int cont_channels_processed = 0;

static int read_channels_in_loop_min = 0xFFFF;
static int read_channels_in_loop_max = 0;
static int cont_channels_in_loop_exact_count = 0;




static uint32_t continuous_read_sample_count[CONFIG_DRV_ADC_AIN_MAX] = {0};
static uint16_t analog_input_read_data[CONFIG_DRV_ADC_AIN_MAX] = {0};

/* *****************************************************************************
 * Prototype of functions definitions
 **************************************************************************** */
#if ESP_ADC_VERSION_BIGGER_OR_EQUAL_TO_5
static bool adc_calibration_init(adc_unit_t unit, adc_atten_t atten, adc_cali_handle_t *out_handle);
static void adc_calibration_deinit(adc_cali_handle_t handle);
#endif

/* *****************************************************************************
 * Functions
 **************************************************************************** */

char* drv_adc_get_name(drv_adc_e_analog_input_t analog_input)
{
    return ain_name[analog_input];
}

uint16_t drv_adc_get_last_read_data(drv_adc_e_analog_input_t analog_input)
{
    return analog_input_read_data[analog_input];
}

int drv_adc_get_last_read_data_mV(drv_adc_e_analog_input_t analog_input)
{
    int millivolts = 0;
    if ((ain_adc[analog_input] < CONFIG_DRV_ADC_ADC_COUNT_MAX) && (ain_chn[analog_input] < CONFIG_DRV_ADC_CHANNEL_RANGE_MAX))
    {
        millivolts = voltage[ain_adc[analog_input]][ain_chn[analog_input]];
    }
    return millivolts;
}


#if ESP_ADC_VERSION_BIGGER_OR_EQUAL_TO_5

uint16_t drv_adc_get_last_read_data_millivolts(drv_adc_e_analog_input_t analog_input)
{
    int millivolts;
    adc_cali_handle_t cali_handle = (ain_adc[analog_input] == DRV_ADC_INDEX_ADC1) ? adc1_cali_handle : (ain_adc[analog_input] == DRV_ADC_INDEX_ADC2) ? adc2_cali_handle : NULL;

    adc_cali_raw_to_voltage(cali_handle, analog_input_read_data[analog_input], &millivolts);
    if ((ain_adc[analog_input] < CONFIG_DRV_ADC_ADC_COUNT_MAX) && (ain_chn[analog_input] < CONFIG_DRV_ADC_CHANNEL_RANGE_MAX))
    {
        voltage[ain_adc[analog_input]][ain_chn[analog_input]] = millivolts;
    }
    return millivolts;
}

#else

uint16_t drv_adc_get_last_read_data_millivolts(drv_adc_e_analog_input_t analog_input)
{
    int millivolts;

    millivolts = esp_adc_cal_raw_to_voltage(adc_raw[ain_adc[analog_input]][ain_chn[analog_input]], &adc_chars[ain_adc[analog_input]]);
    if ((ain_adc[analog_input] < CONFIG_DRV_ADC_ADC_COUNT_MAX) && (ain_chn[analog_input] < CONFIG_DRV_ADC_CHANNEL_RANGE_MAX))
    {
        voltage[ain_adc[analog_input]][ain_chn[analog_input]] = millivolts;
    }
    return millivolts;
}

#endif




#if ESP_ADC_VERSION_BIGGER_OR_EQUAL_TO_5

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
    calibration_enabled[ADC_UNIT_1 - 1] = adc_calibration_init(ADC_UNIT_1, attenuation, &adc1_cali_handle);



    //-------------ADC2 Init---------------//
    adc_oneshot_unit_init_cfg_t init_config2 = {
        .unit_id = ADC_UNIT_2,
        .ulp_mode = ADC_ULP_MODE_DISABLE,
    };
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config2, &adc2_handle));

    //-------------ADC2 Calibration Init---------------//
    calibration_enabled[ADC_UNIT_2 - 1] = adc_calibration_init(ADC_UNIT_2, attenuation, &adc2_cali_handle);


    //-------------ADC Config---------------//
    adc_oneshot_chan_cfg_t config = {
        .bitwidth = ADC_BITWIDTH_DEFAULT,
        .atten = attenuation,
    };

    for (int index = 0; index < CONFIG_DRV_ADC_AIN_MAX; index++)
    {
        adc_oneshot_unit_handle_t adc_handle = (ain_adc[index] == DRV_ADC_INDEX_ADC1) ? adc1_handle : (ain_adc[index] == DRV_ADC_INDEX_ADC2) ? adc2_handle : NULL;
        if ((adc_handle != NULL) && (ain_chn[index] >= 0))
        {
            ESP_ERROR_CHECK(adc_oneshot_config_channel(adc_handle, ain_chn[index], &config));
        }
    }


}

void adc_deinit_one_shot(void)
{
    //Tear Down
    ESP_ERROR_CHECK(adc_oneshot_del_unit(adc1_handle));
    if (calibration_enabled[ADC_UNIT_1 - 1]) {
        adc_calibration_deinit(adc1_cali_handle);
    }

#if (SOC_ADC_PERIPH_NUM >= 2)
    ESP_ERROR_CHECK(adc_oneshot_del_unit(adc2_handle));
    if (calibration_enabled[ADC_UNIT_2 - 1]) {
        adc_calibration_deinit(adc2_cali_handle);
    }
#endif /* #if (SOC_ADC_PERIPH_NUM >= 2) */

}




static bool IRAM_ATTR s_conv_done_cb(adc_continuous_handle_t handle, const adc_continuous_evt_data_t *edata, void *user_data)
{
    BaseType_t mustYield = pdFALSE;
    //Notify that ADC continuous driver has done enough number of conversions
    vTaskNotifyGiveFromISR(continuous_task_handle, &mustYield);

    cont_sample_notify_count++;

    return (mustYield == pdTRUE);
}

static void continuous_adc_init(adc_continuous_handle_t *out_handle)
{
    adc_continuous_handle_t handle = NULL;

    adc_continuous_handle_cfg_t adc_config = {
        .max_store_buf_size = SOC_ADC_DIGI_RESULT_BYTES * ADC_CONT_COEF_OVERSIZE_POOL_BUFFER * CONFIG_DRV_ADC_AIN_MAX,
        .conv_frame_size = SOC_ADC_DIGI_RESULT_BYTES * CONFIG_DRV_ADC_AIN_MAX,
    };

    adc_continuous_config_t dig_cfg = {
        .sample_freq_hz = SOC_ADC_SAMPLE_FREQ_THRES_LOW,    /* 20 kHz */
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
            adc_pattern[i].atten = attenuation;
            adc_pattern[i].channel = ch;
            adc_pattern[i].unit = unit;
            adc_pattern[i].bit_width = SOC_ADC_DIGI_MAX_BITWIDTH;

            ESP_LOGI(TAG, "adc_pattern[%d].atten is :%x", i, adc_pattern[i].atten);
            ESP_LOGI(TAG, "adc_pattern[%d].channel is :%x", i, adc_pattern[i].channel);
            ESP_LOGI(TAG, "adc_pattern[%d].unit is :%x", i, adc_pattern[i].unit);

        }
        else
        {
            ESP_LOGE(TAG, "adc_pattern[%d] unit:%x channel:%x Invalid - fix CONFIG_DRV_ADC_AIN_MAX - limited to %d", i, ain_adc[i] + 1, ain_chn[i], i);
            dig_cfg.pattern_num = i;
            channels_continuous_read = i;
            break;
        }

    }
    dig_cfg.adc_pattern = adc_pattern;
    adc_config.max_store_buf_size = SOC_ADC_DIGI_RESULT_BYTES * ADC_CONT_COEF_OVERSIZE_POOL_BUFFER * channels_continuous_read;
    adc_config.conv_frame_size = SOC_ADC_DIGI_RESULT_BYTES * channels_continuous_read;
    ESP_ERROR_CHECK(adc_continuous_new_handle(&adc_config, &handle));
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
    //int ainPrint[CONFIG_DRV_ADC_AIN_MAX];
    //int channelPrint[CONFIG_DRV_ADC_AIN_MAX];
    //uint16_t dataPrint[CONFIG_DRV_ADC_AIN_MAX];
    

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

        int read_channels_in_loop = 0;

        while (1) 
        {
            uint32_t ret_num = 0;
            esp_err_t ret;
            //ret = adc_continuous_read(handle, (uint8_t*)continuous_read_result, channels_continuous_read * SOC_ADC_DIGI_RESULT_BYTES, &ret_num, 0);
            ret = adc_continuous_read(handle, (uint8_t*)continuous_read_result, sizeof(continuous_read_result), &ret_num, 0);
            if (ret == ESP_OK) 
            {


                int channels_read = ret_num / SOC_ADC_DIGI_RESULT_BYTES;
                ESP_LOGV("TASK", "ret is %x, ret_num is %"PRIu32" bytes (%d Channels)", ret, ret_num, channels_read);
                //printBuffer[0] = 0; //clear buffer

                read_channels_in_loop += channels_read;

                cont_channels_processed += channels_read;

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
                        adc_raw[ain_adc[ain_index]][ain_chn[ain_index]] = p->type1.data;
                    }
                    else
                    {
                        continuous_read_sample_count[ain_index]++;
                        analog_input_read_data[ain_index] = p->type1.data;
                    }


                    //ainPrint[i] = ain_index;
                    //channelPrint[i] = p->type1.channel;
                    //dataPrint[i] = p->type1.data;

                    //sprintf(&printBuffer[strlen(printBuffer)], "AIN_%d[ch_%d]=%4d ", ainPrint[i], channelPrint[i], dataPrint[i]);//to do check buffer ovf
                    //sprintf(&printBuffer[strlen(printBuffer)], "A%d=%4d ", ainPrint[i], dataPrint[i]);//to do check buffer ovf
                    //ESP_LOGV(TAG, "AIN: %d, Unit: %d, Channel: %d, Value: %x", ain_index, 1, p->type1.channel, p->type1.data);

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
                //ESP_LOGD(TAG, "%s", printBuffer);
                /**
                 * Because printing is slow, so every time you call `ulTaskNotifyTake`, it will immediately return.
                 * To avoid a task watchdog timeout, add a delay here. When you replace the way you process the data,
                 * usually you don't need this delay (as this task will block for a while).
                 */
                //vTaskDelay(1);
                if (channels_read == channels_continuous_read)
                {
                    cont_expected_process_count++;
                }
                if (channels_read >= channels_continuous_read)
                {
                    break;
                }
            } 
            else if (ret == ESP_ERR_TIMEOUT) 
            {
                //We try to read `CONTINUOUS_READ_LEN` until API returns timeout, which means there's no available data
                break;
            }
            else
            {
                ESP_LOGE(TAG, "Read ADC continuous error %d (%s)", ret, esp_err_to_name(ret));
            }
        }


        if (read_channels_in_loop_min > read_channels_in_loop)
        {
            read_channels_in_loop_min = read_channels_in_loop;
        }
        if (read_channels_in_loop_max < read_channels_in_loop)
        {
            read_channels_in_loop_max = read_channels_in_loop;
        }
        if (read_channels_in_loop == channels_continuous_read)
        {
            cont_channels_in_loop_exact_count++;
        }
    }
}

void adc_deinit_continuous(void)
{
    ESP_ERROR_CHECK(adc_continuous_stop(handle));
    ESP_ERROR_CHECK(adc_continuous_deinit(handle));
}

#else   //ESP_ADC_VERSION_BIGGER_OR_EQUAL_TO_5




static esp_adc_cal_value_t check_efuse(void)
{
    esp_adc_cal_value_t best_cal_scheme = ESP_ADC_CAL_VAL_EFUSE_VREF;
#if CONFIG_IDF_TARGET_ESP32
    //Check if TP with curve is burned into eFuse
    if (esp_adc_cal_check_efuse(ESP_ADC_CAL_VAL_EFUSE_TP_FIT) == ESP_OK) 
    {
        printf("eFuse Two Point with curve: Supported\n");
        best_cal_scheme = ESP_ADC_CAL_VAL_EFUSE_TP_FIT;
    } 
    else 
    {
        printf("eFuse Two Point with curve: NOT supported\n");
    }
    //Check if TP is burned into eFuse
    if (esp_adc_cal_check_efuse(ESP_ADC_CAL_VAL_EFUSE_TP) == ESP_OK)
    {
        printf("eFuse Two Point: Supported\n");
        if (best_cal_scheme < ESP_ADC_CAL_VAL_EFUSE_TP) 
        {
            best_cal_scheme = ESP_ADC_CAL_VAL_EFUSE_TP;
        }   
    } 
    else 
    {
        printf("eFuse Two Point: NOT supported\n");
    }
    //Check Vref is burned into eFuse
    if (esp_adc_cal_check_efuse(ESP_ADC_CAL_VAL_EFUSE_VREF) == ESP_OK) 
    {
        printf("eFuse Vref: Supported\n");
        if (best_cal_scheme < ESP_ADC_CAL_VAL_EFUSE_VREF) 
        {
            best_cal_scheme = ESP_ADC_CAL_VAL_EFUSE_VREF;
        }   
    }
    else 
    {
        printf("eFuse Vref: NOT supported\n");
    }
    //Check default Vref is burned into eFuse
    if (esp_adc_cal_check_efuse(ESP_ADC_CAL_VAL_DEFAULT_VREF) == ESP_OK) 
    {
        if (best_cal_scheme == ESP_ADC_CAL_VAL_EFUSE_VREF) 
        {
            best_cal_scheme = ESP_ADC_CAL_VAL_DEFAULT_VREF;
        }   
        printf("eFuse default Vref: Supported\n");
    } 
    else 
    {
        printf("eFuse default Vref: NOT supported\n");
    }
#elif CONFIG_IDF_TARGET_ESP32S2
    if (esp_adc_cal_check_efuse(ESP_ADC_CAL_VAL_EFUSE_TP) == ESP_OK) 
    {
        printf("eFuse Two Point: Supported\n");
        if (best_cal_scheme < ESP_ADC_CAL_VAL_EFUSE_TP) 
        {
            best_cal_scheme = ESP_ADC_CAL_VAL_EFUSE_TP;
        }   
    } 
    else 
    {
        printf("Cannot retrieve eFuse Two Point calibration values. Default calibration values will be used.\n");
    }
#else
#error "This example is configured for ESP32/ESP32S2."
#endif
    return best_cal_scheme;
}

static void print_char_val_type(esp_adc_cal_value_t val_type)
{
    if (val_type == ESP_ADC_CAL_VAL_EFUSE_TP) {
        printf("Characterized using Two Point Value\n");
    } else if (val_type == ESP_ADC_CAL_VAL_EFUSE_VREF) {
        printf("Characterized using eFuse Vref\n");
    } else if (val_type == ESP_ADC_CAL_VAL_EFUSE_TP_FIT) {
        printf("Characterized using eFuse Vref\n");
    } else {
        printf("Characterized using Default Vref\n");
    }
}


static bool adc_calibration_init(void)
{
    esp_adc_cal_value_t cali_scheme = ESP_ADC_CAL_VAL_EFUSE_TP_FIT;

    esp_err_t ret;
    bool cali_enable = false;

    //Check if Two Point or Vref are burned into eFuse for info
    esp_adc_cal_value_t best_cal_scheme = check_efuse();
    if (best_cal_scheme < cali_scheme)
    {
        cali_scheme = best_cal_scheme;
    }

    ret = esp_adc_cal_check_efuse(cali_scheme);
    if (ret == ESP_ERR_NOT_SUPPORTED) {
        ESP_LOGW(TAG, "Calibration scheme not supported, skip software calibration");
    } else if (ret == ESP_ERR_INVALID_VERSION) {
        ESP_LOGW(TAG, "eFuse not burnt, skip software calibration");
    } else if (ret == ESP_OK) {
        cali_enable = true;
    } else {
        ESP_LOGE(TAG, "Invalid arg");
    }

    return cali_enable;
}


#endif  //ESP_ADC_VERSION_BIGGER_OR_EQUAL_TO_5

int drv_adc_sample_channel(drv_adc_e_analog_input_t analog_input)
{
    int raw_data = 0;

    if (ain_adc[analog_input] == 0)
    {
        adc_raw[ain_adc[analog_input]][ain_chn[analog_input]] = adc1_get_raw(ain_chn[analog_input]);
        raw_data = adc_raw[ain_adc[analog_input]][ain_chn[analog_input]];
    }
    else
    {
        esp_err_t err = adc2_get_raw(ain_chn[analog_input], ADC_WIDTH_BIT_DEFAULT, &adc_raw[ain_adc[analog_input]][ain_chn[analog_input]]);
        raw_data = adc_raw[ain_adc[analog_input]][ain_chn[analog_input]];
        if (err != ESP_OK)
        {
            ESP_LOGE(TAG, "adc2_get_raw failure %s", esp_err_to_name(err));
        }
        else
        {

        }
    }
    return raw_data;
}


void adc_task_one_shot(void* param)
{

    while (1) 
    {
        bool sample_passed = false;

        for (int index = 0; index < CONFIG_DRV_ADC_AIN_MAX; index++)
        {
            #if ESP_ADC_VERSION_BIGGER_OR_EQUAL_TO_5

            adc_oneshot_unit_handle_t adc_handle = (ain_adc[index] == DRV_ADC_INDEX_ADC1) ? adc1_handle : (ain_adc[index] == DRV_ADC_INDEX_ADC2) ? adc2_handle : NULL;
            if ((adc_handle != NULL) && (ain_chn[index] >= 0))
            {
                //ESP_LOGI(TAG, "ADC%d Channel[%d] Requested", ain_adc[index] + 1, ain_chn[index]);
                //vTaskDelay(pdMS_TO_TICKS(1000));

                esp_err_t err = adc_oneshot_read(adc_handle, ain_chn[index], &adc_raw[ain_adc[index]][ain_chn[index]]);
                if (err != ESP_OK)
                {
                    ESP_LOGE(TAG, "ADC%d Channel[%d] Fail to read err %X (%s)", ain_adc[index] + 1, ain_chn[index], err, esp_err_to_name(err));
                }
                else
                {
                    analog_input_read_data[index] = adc_raw[ain_adc[index]][ain_chn[index]];
                }
                    
                ESP_LOGD(TAG, "ADC%d Channel[%d] Raw Data: %d", ain_adc[index] + 1, ain_chn[index], adc_raw[ain_adc[index]][ain_chn[index]]);
                if (calibration_enabled[ain_adc[index]]) 
                {
                    ESP_ERROR_CHECK(adc_cali_raw_to_voltage(adc1_cali_handle, adc_raw[ain_adc[index]][ain_chn[index]], &voltage[ain_adc[index]][ain_chn[index]]));
                    ESP_LOGD(TAG, "ADC%d Channel[%d] Cali Voltage: %d mV", ain_adc[index] + 1, ain_chn[index], voltage[ain_adc[index]][ain_chn[index]]);
                }
                sample_passed = true;
                vTaskDelay(pdMS_TO_TICKS(100));
            }

            #else

            if((ain_adc[index] < SOC_ADC_PERIPH_NUM) && (ain_chn[index] >= 0))
            {
                esp_err_t err = ESP_OK;
                if (ain_adc[index] == 0)
                {
                    adc_raw[ain_adc[index]][ain_chn[index]] = adc1_get_raw(ain_chn[index]);
                }
                else
                {
                    err = adc2_get_raw(ain_chn[index], ADC_WIDTH_BIT_DEFAULT, &adc_raw[ain_adc[index]][ain_chn[index]]);
                }
                if (err != ESP_OK)
                {
                    ESP_LOGE(TAG, "ADC%d Channel[%d] Fail to read err %X (%s)", ain_adc[index] + 1, ain_chn[index], err, esp_err_to_name(err));
                }
                else
                {
                    analog_input_read_data[index] = adc_raw[ain_adc[index]][ain_chn[index]];
                }

                //Convert adc_reading to voltage in mV
                if (calibration_enabled[ain_adc[index]]) 
                {
                    voltage[ain_adc[index]][ain_chn[index]] = esp_adc_cal_raw_to_voltage(adc_raw[ain_adc[index]][ain_chn[index]], &adc_chars[ain_adc[index]]);
                    ESP_LOGI(TAG, "ADC%d Channel[%d] Cali Voltage: %d mV", ain_adc[index] + 1, ain_chn[index], voltage[ain_adc[index]][ain_chn[index]]);
                }
                else
                {
                    ESP_LOGI(TAG, "ADC%d Channel[%d] Raw Data: %d", ain_adc[index] + 1, ain_chn[index], adc_raw[ain_adc[index]][ain_chn[index]]);
                }

                sample_passed = true;
                vTaskDelay(pdMS_TO_TICKS(100));
            }

            #endif
            
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



void drv_adc_init(void)
{
    #if ESP_ADC_VERSION_BIGGER_OR_EQUAL_TO_5
    #if CONFIG_DRV_ADC_SAMPLING_EXTERNAL == 0
    #if CONFIG_DRV_ADC_CONTINUOUS
    xTaskCreate(adc_task_continuous, "adc_continuous", 4096, NULL, configMAX_PRIORITIES - 20, &continuous_task_handle);
    #else
    xTaskCreate(adc_task_one_shot, "adc_oneshot", 4096, NULL, configMAX_PRIORITIES - 20, &oneshot_task_handle);
    #endif
    #endif

    #if CONFIG_DRV_ADC_CONTINUOUS
    adc_init_continuous();  /* continuous_task_handle must be initialized here (adc_task_continuous must be created) otherwise task notify callback might fail */
    #else
    adc_init_one_shot();
    #endif

    int io_num;
    esp_err_t ret;
    for (int index = 0; index < CONFIG_DRV_ADC_AIN_MAX; index++)
    {
        if ((ain_adc[index] >= 0) && (ain_chn[index] >= 0))
        {
            ret = adc_oneshot_channel_to_io(ain_adc[index] + ADC_UNIT_1, ain_chn[index], &io_num);
            if (ret == ESP_OK)
            {
                ESP_LOGI(TAG, "Setup AIN[%d] ADC[%d] CH[%d] IO[%d]", index, ain_adc[index], ain_chn[index], io_num);
            }
        }
    }
    #else

    static const adc_bits_width_t width = ADC_WIDTH_BIT_12;

    static const uint32_t default_vref = 1100;


    bool cali_enable = adc_calibration_init();
    bool calibrate_adc1 = false;
    bool calibrate_adc2 = false;



    for (int index = 0; index < CONFIG_DRV_ADC_AIN_MAX; index++)
    {
        if((ain_adc[index] < SOC_ADC_PERIPH_NUM) && (ain_chn[index] >= 0))
        {
            adc_unit_t unit = (adc_unit_t)(ain_adc[index] + ADC_UNIT_1);

            if (unit == ADC_UNIT_1)
            {
                adc1_config_width(width);
                adc1_channel_t channel = (adc1_channel_t)ain_chn[index];
                adc1_config_channel_atten(channel, attenuation);
                calibrate_adc1 = true;
            }
            else if (unit == ADC_UNIT_2)
            {
                adc2_channel_t channel = (adc2_channel_t)ain_chn[index];
                adc2_config_channel_atten(channel, attenuation);
                calibrate_adc2 = true;
            }
        }
    }

    if (cali_enable) 
    {
        if (calibrate_adc1)
        {
            int index_adc = DRV_ADC_INDEX_ADC1;
            esp_adc_cal_value_t val_type = esp_adc_cal_characterize(index_adc + ADC_UNIT_1, attenuation, width, default_vref, &adc_chars[index_adc]);
            print_char_val_type(val_type);
            calibration_enabled[index_adc] = true;
        }
        if (calibrate_adc2)
        {
            int index_adc = DRV_ADC_INDEX_ADC2;
            esp_adc_cal_value_t val_type = esp_adc_cal_characterize(index_adc + ADC_UNIT_2, attenuation, width, default_vref, &adc_chars[index_adc]);
            print_char_val_type(val_type);
            calibration_enabled[index_adc] = true;
        }
    }
    #if CONFIG_DRV_ADC_SAMPLING_EXTERNAL == 0
    #if CONFIG_DRV_ADC_CONTINUOUS
    xTaskCreate(adc_task_continuous, "adc_continuous", 4096, NULL, configMAX_PRIORITIES - 20, &continuous_task_handle);
    #else
    xTaskCreate(adc_task_one_shot, "adc_oneshot", 4096, NULL, configMAX_PRIORITIES - 20, &oneshot_task_handle);
    #endif
    #endif

    #endif
}

void drv_adc_deinit(void)
{
    #if ESP_ADC_VERSION_BIGGER_OR_EQUAL_TO_5
    #if CONFIG_DRV_ADC_CONTINUOUS
    adc_deinit_continuous();
    #else
    adc_deinit_one_shot();
    #endif 
    #endif
}

void drv_adc_cont_stat_print(void)
{
    uint32_t sample_count_diff[CONFIG_DRV_ADC_AIN_MAX];
    uint32_t sample_count[CONFIG_DRV_ADC_AIN_MAX];
    static uint32_t prev_sample_count[CONFIG_DRV_ADC_AIN_MAX] = {0};

    static int64_t time_prev = 0;
    int64_t time_now = esp_timer_get_time();
    int64_t time_diff = time_now - time_prev;
    time_prev = time_now;
    char print_stat_buffer[256];
    int samples_per_second[CONFIG_DRV_ADC_AIN_MAX];
    

    memcpy(sample_count, continuous_read_sample_count, sizeof(sample_count));
    print_stat_buffer[0] = 0;
    for (int index = 0; index < CONFIG_DRV_ADC_AIN_MAX; index++)
    {
        if (sample_count[index] > 0)
        {
            sample_count_diff[index] = sample_count[index] - prev_sample_count[index];
            prev_sample_count[index] = sample_count[index];
            samples_per_second[index] = ((uint64_t)sample_count_diff[index] * 1000000 + time_diff / 2 ) / time_diff;
            sprintf(&print_stat_buffer[strlen(print_stat_buffer)], "AIN_%d=%4d %5d/%4lldms(%dsps)%6d ", index, (int)analog_input_read_data[index], (int)sample_count_diff[index], (time_diff+500)/1000, samples_per_second[index], (int)sample_count[index]);    //to do buffer ovf check
        }

        
    }
    
    sprintf(&print_stat_buffer[strlen(print_stat_buffer)], "[%5d/%5d|%6d] ", cont_sample_notify_count, cont_expected_process_count, cont_channels_processed);
    sprintf(&print_stat_buffer[strlen(print_stat_buffer)], "[%d/%d|%6d]", read_channels_in_loop_min, read_channels_in_loop_max, cont_channels_in_loop_exact_count);

    ESP_LOGI(TAG, "%s", print_stat_buffer);
}

void drv_adc_print_level_warning(void)
{
    esp_log_level_set(TAG, ESP_LOG_WARN);
}

esp_err_t drv_adc_get_analog_input_gpio(drv_adc_e_analog_input_t analog_input, gpio_num_t *gpio_num)
{
    esp_err_t ret = ESP_FAIL;
    if ((analog_input >= 0) && (analog_input < CONFIG_DRV_ADC_AIN_MAX))
    {
        if (ain_adc[analog_input] == DRV_ADC_INDEX_ADC1)
        {
            ret = adc1_pad_get_io_num(ain_chn[analog_input], gpio_num);
        }
        else if (ain_adc[analog_input] == DRV_ADC_INDEX_ADC2)
        {
            ret = adc2_pad_get_io_num(ain_chn[analog_input], gpio_num);
        }
    }
    return ret;
}