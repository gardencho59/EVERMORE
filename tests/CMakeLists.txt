add_executable(database_tests DatabaseTests.cpp)
add_executable(core_tests CoreTests.cpp)
add_executable(random_tests RandomTests.cpp)

foreach(test_target IN ITEMS database_tests core_tests random_tests)
    if(MSVC)
        target_compile_options(${test_target} PRIVATE /UNDEBUG)
    else()
        target_compile_options(${test_target} PRIVATE -UNDEBUG)
    endif()
endforeach()

target_link_libraries(database_tests PRIVATE evermore_core)
target_link_libraries(core_tests PRIVATE evermore_core)
target_link_libraries(random_tests PRIVATE evermore_core)

add_test(NAME DatabaseTests COMMAND database_tests)
add_test(NAME CoreTests COMMAND core_tests)
add_test(NAME RandomTests COMMAND random_tests)