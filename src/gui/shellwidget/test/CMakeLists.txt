include_directories(..)

if (WIN32 AND USE_STATIC_QT)
	add_definitions(-DUSE_STATIC_QT)
endif ()

# Define shell2.txt location for bench_paint
file(TO_CMAKE_PATH ${CMAKE_CURRENT_SOURCE_DIR} TEST_SOURCE_DIR)
add_definitions(-DTEST_SOURCE_DIR="${TEST_SOURCE_DIR}")

function(add_xtest SOURCE_NAME)
	add_executable(${SOURCE_NAME} ${SOURCE_NAME}.cpp)
	target_link_libraries(${SOURCE_NAME} Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::Test qshellwidget)
	# Qt6's deferred finalization sets WIN32_EXECUTABLE=TRUE for GUI targets.
	# Defer our reset to run after Qt6's call so ctest can capture stdout on Windows.
	cmake_language(DEFER CALL set_target_properties ${SOURCE_NAME} PROPERTIES WIN32_EXECUTABLE FALSE)
	add_test(NAME ${SOURCE_NAME} COMMAND ${SOURCE_NAME} ${CTEST_EXE_ARGS})
endfunction()

add_xtest(test_cell)
add_xtest(test_shellcontents)
add_xtest(test_shellwidget)
add_xtest(bench_scroll)
add_xtest(bench_cell)
add_xtest(bench_paint)
