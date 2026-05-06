zephyr_library_amend()

if(EXISTS "${APPLICATION_SOURCE_DIR}/include")
  zephyr_library_include_directories("${APPLICATION_SOURCE_DIR}/include")
endif()
zephyr_library_include_directories(${CMAKE_CURRENT_LIST_DIR}/../../include)

zephyr_library_sources_ifdef(CONFIG_INPUT_IQS9151 iqs9151.c)
zephyr_library_sources_ifdef(CONFIG_INPUT_IQS9151 iqs9151_scroll_inertia_listener.c)
zephyr_library_sources_ifdef(CONFIG_INPUT_IQS9151 iqs9151_split_scroll_inertia.c)
