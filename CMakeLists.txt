cmake_minimum_required(VERSION 3.16)

project(EVERMORE VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 게임 로직 라이브러리
add_library(evermore_core STATIC
    src/core/Random.cpp
    src/data/GameDatabase.cpp
    src/data/SampleDefinition.cpp
    src/combat/CombatUnit.cpp
    src/combat/StatCalculator.cpp
)

target_include_directories(evermore_core
    PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

# 콘솔 실행 프로그램
add_executable(EVERMORE
    app/main.cpp
)

target_link_libraries(EVERMORE
    PRIVATE evermore_core
)

# 테스트
option(BUILD_TESTS "Build tests" ON)

if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()