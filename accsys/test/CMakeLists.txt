macro(accsys_add_test TESTNAME FILES)
    add_executable(${TESTNAME} ${FILES})
    target_link_libraries(${TESTNAME} PRIVATE GTest::gtest_main)

    set_target_properties(${TESTNAME}
            PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
            COMMAND ${TESTNAME}
            )

    # Add the test under CTest.
    add_test(${TESTNAME} ${CMAKE_CURRENT_BINARY_DIR}/${TESTNAME} --gtest_color=yes
            --gtest_output=xml:${CMAKE_CURRENT_BINARY_DIR}/${TESTNAME}.xml)

endmacro()


add_subdirectory(ir)