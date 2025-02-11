cmake_minimum_required(VERSION 3.16)

project(sys_info LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)
set(ARCHIVE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core LinguistTools)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core LinguistTools)

set(TS_FILES sys_info_zh_CN.ts)

# 算法功能库头文件路径
set(ALGORITHM_FUNC_LIB_DIR ${CMAKE_CURRENT_LIST_DIR}/../JiuT_dev/algorithm_func_lib)
set(ALGORITHM_FUNC_LIB_INCLUDE_PATH ${ALGORITHM_FUNC_LIB_DIR}/include)

include_directories(
  sys_info
)
link_directories(PUBLIC ${ALGORITHM_FUNC_LIB_DIR}/lib)

add_executable(sys_info
  main.cpp
  sys_info/sys_info.cpp
  ${TS_FILES}
)
target_link_libraries(sys_info Qt${QT_VERSION_MAJOR}::Core system_info)

if(COMMAND qt_create_translation)
    qt_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
else()
    qt5_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
endif()

include(GNUInstallDirs)
install(TARGETS sys_info
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
