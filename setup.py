import os
import sys
import subprocess

from glob import glob
from setuptools import setup
from pybind11.setup_helpers import Pybind11Extension
from pybind11 import get_include

from setuptools import setup
from setuptools.command.build_ext import build_ext

PROJECT_NAME = "pymp4v2"
PROJECT_AUTHOR = "Dmitriy Efimov <daefimov@gmail.com>"
PROJECT_VERSION = "0.1.0"
PROJECT_DESCRIPTION = "Python bindings for MP4v2 library"

extra_compile_args=['-std=c++11']
extra_link_args=[]

output_dir = 'build'

# Определяем флаги оптимизации в зависимости от платформы
if sys.platform == "win32":
    # Для Windows
    extra_compile_args.extend(["/O2", "/DNDEBUG", "/GL"])  # Максимальная оптимизация, без отладки, глобальная оптимизация
    extra_link_args.extend(["/LTCG", "/OPT:REF", "/OPT:ICF"])  # Link-time code generation, удаление неиспользуемого кода
else:
    # Для Linux/macOS
    extra_compile_args.extend(["-O3", "-DNDEBUG", "-flto", "-fno-fat-lto-objects", "-ffunction-sections", "-fdata-sections"])  # Максимальная оптимизация, без отладки, LTO
    extra_link_args.extend(["-flto", "-s", "-Wl,--gc-sections", "-Wl,--strip-all"])  # Link-time optimization, удаление символов


class MP4V2Builder(build_ext):
    def run(self):
        temp_dir = os.path.join(os.path.dirname(os.path.abspath(__file__)), output_dir)
        os.makedirs(temp_dir, exist_ok=True)
        mp4v2_dir = os.path.join(temp_dir, 'mp4v2')
        
        # Клонирование и сборка MP4v2 как СТАТИЧЕСКОЙ библиотеки
        self._run_command(['git', 'clone', '--depth', '1', '--branch', 'v2.1.3', 'https://github.com/enzo1982/mp4v2.git', mp4v2_dir])
        
        # Создание директории для сборки
        build_dir = os.path.join(mp4v2_dir, 'build')
        os.makedirs(build_dir, exist_ok=True)
        
        # Конфигурация и сборка MP4v2 как статической библиотеки
        cmake_args = [
            'cmake', '..', 
            '-DCMAKE_INSTALL_PREFIX=' + temp_dir,
            '-DCMAKE_BUILD_TYPE=Release',
            '-DBUILD_SHARED=OFF',
            '-DBUILD_UTILS=OFF',
            '-DCMAKE_POSITION_INDEPENDENT_CODE=ON',
            '-DCMAKE_CXX_FLAGS=-fPIC'
        ]
        
        self._run_command(cmake_args, cwd=build_dir)
        self._run_command(['cmake', '--build', '.', '--config', 'Release'], cwd=build_dir)
        self._run_command(['cmake', '--install', '.'], cwd=build_dir)
        
        # Сохранение путей для использования в расширении
        self.mp4v2_include_dir = os.path.join(temp_dir, 'include')
        self.mp4v2_library_dir = os.path.join(temp_dir, 'lib')

        # Генерация package_info.py из шаблона
        self._generate_package_info()

        # Продолжение стандартной сборки расширения
        super().run()
    
    def build_extension(self, ext):
        # Добавление путей MP4v2 к расширению
        ext.include_dirs.append(self.mp4v2_include_dir)
        ext.library_dirs.append(self.mp4v2_library_dir)

        # Добавление статической библиотеки mp4v2
        ext.libraries.append('mp4v2')

        # Продолжение стандартной сборки
        super().build_extension(ext)
    
    def _generate_package_info(self):
        """Генерация package_info.py из шаблона package_info.py.in"""
        template_path = os.path.join(os.getcwd(), 'package_info.py.in')
        output_path = os.path.join(os.getcwd(), output_dir, 'package_info.py')
        
        if not os.path.exists(template_path):
            print(f"Warning: Template file {template_path} not found")
            return
        
        # Чтение шаблона
        with open(template_path, 'r') as f:
            template_content = f.read()
        
        # Замена переменных в шаблоне
        processed_content = template_content.replace('@PROJECT_NAME@', PROJECT_NAME)
        processed_content = processed_content.replace('@PROJECT_VERSION@', PROJECT_VERSION)
        processed_content = processed_content.replace('@PROJECT_DESCRIPTION@', PROJECT_DESCRIPTION)
        processed_content = processed_content.replace('@PROJECT_AUTHOR@', PROJECT_AUTHOR)
        
        # Запись результата
        with open(output_path, 'w') as f:
            f.write(processed_content)
        
        print(f"Generated {output_path} from template")

    def _run_command(self, command, cwd=None):
        """Вспомогательная функция для выполнения команд"""
        try:
            subprocess.check_call(command, cwd=cwd)
        except subprocess.CalledProcessError as e:
            print(f"Command failed: {e}")
            sys.exit(1)


# Настройка расширения
ext_modules = [
    Pybind11Extension(
        'pymp4v2',
        sources=sorted(glob("src/*.cpp")),
        include_dirs=[
            get_include(),
            get_include(True),
            '/usr/local/include'  # Стандартный путь для заголовочных файлов
        ],
        libraries=['mp4v2'],
        library_dirs=[
            '/usr/local/lib'
        ],
        language='c++',
        extra_compile_args=extra_compile_args,
        extra_link_args=extra_link_args
    ),
]

setup(
    name=PROJECT_NAME,
    version=PROJECT_VERSION,
    author=PROJECT_AUTHOR,
    description=PROJECT_DESCRIPTION,
    ext_modules=ext_modules,
    cmdclass={'build_ext': MP4V2Builder},
    setup_requires=['pybind11>=2.5.0'],
    install_requires=[],
    packages=[PROJECT_NAME],
    package_data={PROJECT_NAME: ['package_info.py']},
    include_package_data=True,
    zip_safe=False,
    options={
        'build': {
            'build_lib': 'build',
        },
        'build_ext': {
            'inplace': False
        }
    }    
)
