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
	# On Windows, QTest executables that link Qt::Widgets are GUI-subsystem applications whose
	# stdout is not captured by ctest (GitHub Actions provides no console for them). Write QTest
	# output to a file with -o so the workflow can print it after the run regardless of subsystem.
	# See: https://doc.qt.io/qt-6/qtest-overview.html
	add_test(NAME ${SOURCE_NAME} COMMAND ${SOURCE_NAME} -o "${CMAKE_BINARY_DIR}/${SOURCE_NAME}_output.txt" ${CTEST_EXE_ARGS})
endfunction()

add_xtest(test_cell)
add_xtest(test_shellcontents)
add_xtest(test_shellwidget)
add_xtest(bench_scroll)
add_xtest(bench_cell)
add_xtest(bench_paint)
