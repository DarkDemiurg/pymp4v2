cmake_minimum_required(VERSION 3.14)

# Установка имени проекта как переменной
set(PROJECT_NAME pymp4v2)
set(MODULE_NAME pymp4v2)
set(SOURCE_DIR src)
set(INCLUDE_DIR include)
set(LIB_DIR lib)
set(PROJECT_VERSION 1.0.0)
set(PROJECT_DESCRIPTION "Python bindings for MP4v2 library")

project(${PROJECT_NAME} VERSION ${PROJECT_VERSION} LANGUAGES CXX)

# Установка типа сборки по умолчанию
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(STATUS "Установка типа сборки по умолчанию: RelWithDebInfo")
  set(CMAKE_BUILD_TYPE RelWithDebInfo CACHE STRING "Тип сборки" FORCE)
  
  # Установка возможных значений для cmake-gui
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "RelWithDebInfo" "MinSizeRel")
endif()

# Поиск Python и pybind11
set(PYBIND11_FINDPYTHON ON)
find_package(pybind11 CONFIG REQUIRED)

# Поиск библиотеки MP4v2
find_library(MP4V2_LIBRARY mp4v2
    PATHS ${LIB_DIR} /usr/lib /usr/local/lib
)
find_path(MP4V2_INCLUDE_DIR mp4v2/mp4v2.h
    PATHS ${INCLUDE_DIR} /usr/include /usr/local/include
)

if(NOT MP4V2_LIBRARY)
    message(FATAL_ERROR "MP4v2 library not found")
endif()

if(NOT MP4V2_INCLUDE_DIR)
    message(FATAL_ERROR "MP4v2 headers not found")
endif()

# Проверка найденных компонентов
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(MP4V2
    REQUIRED_VARS MP4V2_LIBRARY MP4V2_INCLUDE_DIR
)

# Создание модуля
pybind11_add_module(${MODULE_NAME} src/mp4v2_wrapper.cpp)

# Настройка включения директорий
target_include_directories(${MODULE_NAME} PRIVATE
    ${MP4V2_INCLUDE_DIR}
    ${pybind11_INCLUDE_DIRS}
)

# Настройка линковки
target_link_libraries(${MODULE_NAME} PRIVATE
    ${MP4V2_LIBRARY}
    ${pybind11_LIBRARIES}
)

# Установка
install(TARGETS ${MODULE_NAME}
    LIBRARY DESTINATION lib/python${Python_VERSION_MAJOR}.${Python_VERSION_MINOR}/site-packages
)

# Добавление информации о пакете
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/package_info.py.in
    ${CMAKE_CURRENT_BINARY_DIR}/package_info.py
    @ONLY
)
