set(idf_version "${IDF_VERSION_MAJOR}.${IDF_VERSION_MINOR}.${IDF_VERSION_PATCH}")

# Initialize an empty list to hold conditional requirements
set(conditionally_required_components "")


if (idf_version GREATER_EQUAL "5.0.0")
    list(APPEND conditionally_required_components "esp_adc")
else()
    list(APPEND conditionally_required_components "esp_adc_cal")
endif()



idf_component_register(SRCS "drv_adc.c" 
                    INCLUDE_DIRS "." 
                    REQUIRES
                        ${conditionally_required_components}
                                      )
                 

