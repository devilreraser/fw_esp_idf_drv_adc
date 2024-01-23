set(idf_version "${IDF_VERSION_MAJOR}.${IDF_VERSION_MINOR}.${IDF_VERSION_PATCH}")

# Initialize an empty list to hold conditional requirements
set(conditionally_required_components "")


file(GLOB ESP_ADC_PATH "$ENV{IDF_PATH}/components/esp_adc")
message(STATUS "Searching for esp_adc in: $ENV{IDF_PATH}/components/esp_adc")
if(ESP_ADC_PATH)
    message(STATUS "esp_adc found")
    list(APPEND conditionally_required_components "esp_adc")
else()
    message(STATUS "esp_adc not found")
    list(APPEND conditionally_required_components "esp_adc_cal")
endif()

idf_component_register(SRCS "drv_adc.c" 
                    INCLUDE_DIRS "." 
                    REQUIRES
                        "driver"
                        ${conditionally_required_components}
                                      )
                 

