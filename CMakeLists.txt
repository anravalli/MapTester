cmake_minimum_required(VERSION 3.5)

project(MapTester VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(pugixml REQUIRED)
find_package(PkgConfig REQUIRED)

pkg_check_modules(PUGIXML REQUIRED pugixml)

set(PROJECT_SOURCES
        map.cpp
        MapAssistedCollisionAheadRiskAlgo.cpp
        Geodesy.cpp
        map_tester.cpp
)

add_executable(${PROJECT_NAME} ${PROJECT_SOURCES})

target_link_libraries(${PROJECT_NAME} PUBLIC ${PUGIXML_LIBRARIES})
