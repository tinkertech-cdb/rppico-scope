cmake_minimum_required(VERSION 3.16)

project(rppico_scope_viewer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

qt_standard_project_setup()

qt_add_executable(rppico_scope_viewer
    src/main.cpp
    src/MainWindow.cpp
    src/ProtocolCodec.cpp
    src/DeviceSession.cpp
)

target_include_directories(rppico_scope_viewer PRIVATE
    ${CMAKE_SOURCE_DIR}/../protocol/include
)

target_link_libraries(rppico_scope_viewer PRIVATE
    Qt6::Widgets
)
