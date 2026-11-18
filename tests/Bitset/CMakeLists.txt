set(TARGET BitsetTest)

add_executable(${TARGET}
	${CMAKE_CURRENT_SOURCE_DIR}/BitsetTest.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/BitsetCLinkage.c
)

# inline 関数の外部定義が無いとリンクできないことを検出するため、C ソースは常に -O0 でビルドする
set_source_files_properties(${CMAKE_CURRENT_SOURCE_DIR}/BitsetCLinkage.c PROPERTIES
	COMPILE_OPTIONS "-O0;-std=c17"
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

