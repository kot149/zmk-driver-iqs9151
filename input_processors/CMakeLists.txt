zephyr_library()

if(EXISTS "${APPLICATION_SOURCE_DIR}/include")
  zephyr_library_include_directories("${APPLICATION_SOURCE_DIR}/include")
endif()
zephyr_library_include_directories(${CMAKE_CURRENT_LIST_DIR}/../include)

zephyr_library_sources_ifdef(CONFIG_ZMK_INPUT_PROCESSOR_DYNAMIC_SCALER input_processor_dynamic_scaler.c)
zephyr_library_sources_ifdef(CONFIG_ZMK_INPUT_PROCESSOR_BEHAVIORS_CONSUME input_processor_behaviors_consume.c)
