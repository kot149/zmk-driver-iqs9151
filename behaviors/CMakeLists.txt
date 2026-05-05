zephyr_library()


if(EXISTS "${APPLICATION_SOURCE_DIR}/include")
  zephyr_library_include_directories("${APPLICATION_SOURCE_DIR}/include")
endif()
zephyr_library_include_directories(${CMAKE_CURRENT_LIST_DIR}/../include)

zephyr_library_sources_ifdef(CONFIG_ZMK_BEHAVIOR_TRACKPAD_TO_POS behavior_trackpad_to_pos.c)
zephyr_library_sources_ifdef(CONFIG_ZMK_BEHAVIOR_ZIP_DYNAMIC_SCALE behavior_zip_dynamic_scale.c)
zephyr_library_sources_ifdef(CONFIG_ZMK_BEHAVIOR_ZIP_DYNAMIC_SCALE_SET behavior_zip_dynamic_scale_set.c)
