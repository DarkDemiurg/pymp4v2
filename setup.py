from setuptools import setup, Extension
import pybind11
import sys
import os

extra_compile_args=['-std=c++11']
extra_link_args=[]

# Определяем флаги оптимизации в зависимости от платформы
if sys.platform == "win32":
    # Для Windows
    extra_compile_args.extend(["/O2", "/DNDEBUG", "/GL"])  # Максимальная оптимизация, без отладки, глобальная оптимизация
    extra_link_args.extend(["/LTCG", "/OPT:REF", "/OPT:ICF"])  # Link-time code generation, удаление неиспользуемого кода
else:
    # Для Linux/macOS
    extra_compile_args.extend(["-O3", "-DNDEBUG", "-flto", "-fno-fat-lto-objects"])  # Максимальная оптимизация, без отладки, LTO
    extra_link_args.extend(["-flto", "-s"])  # Link-time optimization, удаление символов


# Для GCC/Clang
if sys.platform != "win32":
    extra_compile_args.extend([
        "-ffunction-sections",
        "-fdata-sections"
    ])
    extra_link_args.extend([
        "-Wl,--gc-sections",
        "-Wl,--strip-all"
    ])

# Настройка расширения
ext_modules = [
    Extension(
        'pymp4v2',
        ['src/mp4v2_wrapper.cpp'],
        include_dirs=[
            pybind11.get_include(),
            pybind11.get_include(True),
            'include',  # Путь к заголовочным файлам MP4v2
            '/usr/local/include'  # Стандартный путь для заголовочных файлов
        ],
        libraries=['mp4v2'],
        library_dirs=[
            'lib',   # Путь к библиотеке MP4v2
            '/usr/local/lib'
        ],
        language='c++',
        extra_compile_args=extra_compile_args,
        extra_link_args=extra_link_args
    ),
]

setup(
    name='pymp4v2',
    version='0.1.0',
    author='Dmitriy Efimov',
    description='Python binding for MP4v2 library',
    ext_modules=ext_modules,
    setup_requires=['pybind11>=2.5.0'],
    zip_safe=False,
)
