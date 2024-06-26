/* *****************************************************************************
 * File:   drv_adc.h
 * Author: XX
 *
 * Created on YYYY MM DD
 * 
 * Description: ...
 * 
 **************************************************************************** */
#pragma once

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */


/* *****************************************************************************
 * Header Includes
 **************************************************************************** */
#include <stdint.h>

#include "esp_idf_version.h"
#include "esp_err.h"
#include "hal/gpio_types.h"

/* *****************************************************************************
 * Configuration Definitions
 **************************************************************************** */

/* *****************************************************************************
 * Constants and Macros Definitions
 **************************************************************************** */

/* *****************************************************************************
 * Enumeration Definitions
 **************************************************************************** */
typedef enum
{
    DRV_ADC_AIN_0,
    DRV_ADC_AIN_1,
    DRV_ADC_AIN_2,
    DRV_ADC_AIN_3,
    DRV_ADC_AIN_4,
    DRV_ADC_AIN_5,
    DRV_ADC_AIN_6,
    DRV_ADC_AIN_7,

} drv_adc_e_analog_input_t;

typedef enum
{
    DRV_ADC_INDEX_ADC1,
    DRV_ADC_INDEX_ADC2,
    DRV_ADC_INDEX_COUNT,
} drv_adc_e_adc_index_t;

/* *****************************************************************************
 * Type Definitions
 **************************************************************************** */

/* *****************************************************************************
 * Function-Like Macro
 **************************************************************************** */

/* *****************************************************************************
 * Variables External Usage
 **************************************************************************** */ 

/* *****************************************************************************
 * Function Prototypes
 **************************************************************************** */
char* drv_adc_get_name(drv_adc_e_analog_input_t analog_input);
uint16_t drv_adc_get_last_read_data(drv_adc_e_analog_input_t analog_input);
uint16_t drv_adc_get_last_read_data_millivolts(drv_adc_e_analog_input_t analog_input);
int drv_adc_sample_channel(drv_adc_e_analog_input_t analog_input);
void drv_adc_init(void);
void drv_adc_deinit(void);
void drv_adc_cont_stat_print(void);
void drv_adc_print_level_warning(void);
esp_err_t drv_adc_get_analog_input_gpio(drv_adc_e_analog_input_t analog_input, gpio_num_t *gpio_num);


#ifdef __cplusplus
}
#endif /* __cplusplus */


