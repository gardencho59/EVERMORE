add_executable(database_tests DatabaseTests.cpp)
add_executable(core_tests CoreTests.cpp)
add_executable(random_tests RandomTests.cpp)

target_link_libraries(database_tests PRIVATE evermore_core)
target_link_libraries(core_tests PRIVATE evermore_core)
target_link_libraries(random_tests PRIVATE evermore_core)

add_test(NAME DatabaseTests COMMAND database_tests)
add_test(NAME CoreTests COMMAND core_tests)
add_test(NAME RandomTests COMMAND random_tests)