cmake_minimum_required(VERSION 3.14)

# Установка имени проекта как переменной
set(PROJECT_NAME pymp4v2)
set(MODULE_NAME pymp4v2)
set(PROJECT_VERSION 0.1.0)
set(PROJECT_AUTHOR "Dmitriy Efimov <daefimov@gmail.com>")
set(PROJECT_DESCRIPTION "Python bindings for MP4v2 library")

project(${PROJECT_NAME} VERSION ${PROJECT_VERSION} LANGUAGES CXX)

# Установка типа сборки по умолчанию
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(STATUS "Установка типа сборки по умолчанию: RelWithDebInfo")
  set(CMAKE_BUILD_TYPE RelWithDebInfo CACHE STRING "Тип сборки" FORCE)
  
  # Установка возможных значений для cmake-gui
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "RelWithDebInfo" "MinSizeRel")
endif()

####################### Сборка mp4v2

set(MP4V2_URL "https://github.com/enzo1982/mp4v2.git")
set(MP4V2_TAG "v2.1.3")

# Включение модуля ExternalProject
include(ExternalProject)

# Загрузка и сборка MP4v2
ExternalProject_Add(mp4v2_external
    GIT_REPOSITORY ${MP4V2_URL}
    GIT_TAG ${MP4V2_TAG}
    PREFIX "${CMAKE_BINARY_DIR}/mp4v2"
    INSTALL_DIR "${CMAKE_BINARY_DIR}/mp4v2_install"
    CMAKE_ARGS 
        -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
        -DCMAKE_BUILD_TYPE=Release
        -DBUILD_SHARED=OFF
        -DBUILD_UTILS=OFF
        -DCMAKE_POSITION_INDEPENDENT_CODE=ON
        -DCMAKE_CXX_FLAGS=-fPIC
    BUILD_ALWAYS OFF
    UPDATE_COMMAND ""
)

# Получение путей к установленным файлам MP4v2
ExternalProject_Get_Property(mp4v2_external install_dir)
set(MP4V2_INCLUDE_DIR "${install_dir}/include")
set(MP4V2_LIBRARY "${install_dir}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}mp4v2${CMAKE_STATIC_LIBRARY_SUFFIX}")  # или .lib для Windows

message(STATUS "MP4V2_INCLUDE_DIR: ${MP4V2_INCLUDE_DIR}")
message(STATUS "MP4V2_LIBRARY: ${MP4V2_LIBRARY}")

# Проверка существования путей
if(NOT EXISTS "${MP4V2_INCLUDE_DIR}")
    message(WARNING "MP4V2 include directory does not exist, creating: ${MP4V2_INCLUDE_DIR}")
    file(MAKE_DIRECTORY "${MP4V2_INCLUDE_DIR}")
endif()

# Создание импортированной библиотеки
add_library(mp4v2 STATIC IMPORTED)
set_target_properties(mp4v2 PROPERTIES
    IMPORTED_LOCATION "${MP4V2_LIBRARY}"
    INTERFACE_INCLUDE_DIRECTORIES "${MP4V2_INCLUDE_DIR}"
)

# Добавление зависимостей
add_dependencies(mp4v2 mp4v2_external)

####################### Конец сборки mp4v2

# Поиск Python и pybind11
set(PYBIND11_FINDPYTHON ON)
find_package(pybind11 CONFIG REQUIRED)

# Создание модуля
pybind11_add_module(${MODULE_NAME} src/mp4v2_wrapper.cpp)

# Настройка включения директорий
target_include_directories(${MODULE_NAME} PRIVATE
    ${MP4V2_INCLUDE_DIR}
    ${pybind11_INCLUDE_DIRS}
)

# Настройка линковки
target_link_libraries(${MODULE_NAME} PRIVATE
    mp4v2
    ${pybind11_LIBRARIES}
    ${Python_LIBRARIES}
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
