set(TARGET ThreadTest)

add_executable(${TARGET}
    ${CMAKE_CURRENT_SOURCE_DIR}/BackGroundTaskTest.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ThreadPoolTest.cpp
)

target_compile_features(${TARGET} PUBLIC cxx_std_20)

target_include_directories(${TARGET} PRIVATE
	${INCLUDE_DIRECTROY}
	${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(${TARGET} PRIVATE
	${LIBRARY_NAME}
	${EXTERNAL_LIBRARY}
	${TESTING_FRAMEWORK}
)

target_compile_options(${TARGET} PRIVATE
	${WARNING_OPTIONS}
	$<$<CONFIG:Debug>:${DEBUG_OPTIONS}>
	$<$<CONFIG:Release>:${NODEBUG_OPTIONS}>
	$<$<CONFIG:Coverage>:${DEBUG_OPTIONS} --coverage>
	$<$<CONFIG:Address>:${DEBUG_OPTIONS} -fsanitize=address>
)
gtest_discover_tests(${TARGET} DISCOVERY_MODE PRE_TEST)

