import importlib
import multiprocessing
import os
import shutil
import subprocess
import sys
from glob import glob

from pybind11 import get_include
from pybind11.setup_helpers import Pybind11Extension
from setuptools import setup
from setuptools.command.build_ext import build_ext

PROJECT_NAME = "pymp4v2"
PROJECT_AUTHOR = "Dmitriy Efimov <daefimov@gmail.com>"
PROJECT_VERSION = "0.1.0"
PROJECT_DESCRIPTION = "Python bindings for MP4v2 library"

extra_compile_args = []
extra_link_args = []

output_dir = "build"

if sys.platform == "win32":
    # For Windows
    extra_compile_args.extend(
        [
            "/std:c++17",
            "/O2",
            "/DNDEBUG",
            "/GL",
            "/DMP4V2_USE_STATIC_LIB=1",
            "/DCMAKE_POSITION_INDEPENDENT_CODE=ON",
        ]
    )
    extra_link_args.extend(["/LTCG", "/OPT:REF", "/OPT:ICF"])
else:
    # For Linux/macOS
    extra_compile_args.extend(
        [
            "-std=c++17",
            "-fPIC",
            "-O3",
            "-DNDEBUG",
            "-flto",
            "-fno-fat-lto-objects",
            "-ffunction-sections",
            "-fdata-sections",
        ]
    )
    extra_link_args.extend(["-flto", "-s", "-Wl,--gc-sections", "-Wl,--strip-all"])


class MP4V2Builder(build_ext):
    def run(self):
        temp_dir = os.path.join(os.path.dirname(os.path.abspath(__file__)), output_dir)
        os.makedirs(temp_dir, exist_ok=True)
        mp4v2_dir = os.path.join(temp_dir, "mp4v2")

        # Клонирование и сборка MP4v2 как СТАТИЧЕСКОЙ библиотеки
        self._run_command(
            [
                "git",
                "clone",
                "--depth",
                "1",
                "--branch",
                "v2.1.3",
                "https://github.com/enzo1982/mp4v2.git",
                mp4v2_dir,
            ]
        )

        # Создание директории для сборки
        build_dir = os.path.join(mp4v2_dir, output_dir)
        os.makedirs(build_dir, exist_ok=True)

        cmake_args = [
            "cmake",
            "..",
            "-DCMAKE_INSTALL_PREFIX=" + temp_dir,
            "-DBUILD_SHARED=OFF",
            "-DBUILD_UTILS=OFF",
            "-DCMAKE_POSITION_INDEPENDENT_CODE=ON",
            "-DCMAKE_CXX_STANDARD=17",
            "-DCMAKE_CXX_STANDARD_REQUIRED=ON",
            "-DCMAKE_CXX_EXTENSIONS=OFF",
            "-DCMAKE_POLICY_VERSION_MINIMUM=3.5"
        ]

        if sys.platform != "win32":
            cmake_args.extend(["-DCMAKE_CXX_FLAGS=-fPIC"])
        self._run_command(cmake_args, cwd=build_dir)

        # Определяем количество потоков для сборки
        num_cores = multiprocessing.cpu_count()
        build_threads = min(num_cores, 8)

        # Добавляем флаг C++17 и для сборки MP4v2
        if sys.platform == "win32":
            self._run_command(
                ["cmake", "--build", ".", "--config", "Release", "--", "/m"],
                cwd=build_dir,
            )
        else:
            self._run_command(
                [
                    "cmake",
                    "--build",
                    ".",
                    "--config",
                    "Release",
                    "--",
                    f"-j{build_threads}",
                ],
                cwd=build_dir,
            )

        self._run_command(["cmake", "--install", "."], cwd=build_dir)

        # Сохранение путей для использования в расширении
        self.mp4v2_include_dir = os.path.join(temp_dir, "include")
        self.mp4v2_library_dir = os.path.join(temp_dir, "lib")

        # Продолжение стандартной сборки расширения
        super().run()

        # Генерация stub-файлов после сборки расширения
        self._generate_stubs()

        # Генерация package_info.py из шаблона
        self._generate_package_info()

        # Очистка временных файлов
        # self._cleanup_temporary_files()

    def build_extension(self, ext):
        # Добавление путей MP4v2 к расширению
        ext.include_dirs.append(self.mp4v2_include_dir)
        ext.library_dirs.append(self.mp4v2_library_dir)

        # Добавление статической библиотеки mp4v2
        ext.libraries.append("mp4v2")

        # Продолжение стандартной сборки
        super().build_extension(ext)

        # Копирование собранной библиотеки в целевой каталог
        import glob
        import shutil

        # Находим собранную библиотеку
        build_lib = self.build_lib or output_dir
        if sys.platform == "win32":
            lib_pattern = os.path.join(build_lib, f"*{PROJECT_NAME}*.pyd")
        else:
            lib_pattern = os.path.join(build_lib, f"*{PROJECT_NAME}*.so")

        so_files = glob.glob(lib_pattern)

        if so_files:
            target_dir = os.path.join(build_lib, PROJECT_NAME)
            os.makedirs(target_dir, exist_ok=True)

            for so_file in so_files:
                shutil.copy2(so_file, target_dir)
                print(f"Copied {so_file} to {target_dir}")

    def _cleanup_temporary_files(self):
        """Удаление всех временных файлов кроме конечной папки pymp4v2"""
        temp_dir = os.path.join(os.path.dirname(os.path.abspath(__file__)), output_dir)

        # Сохраняем пути, которые нужно оставить
        keep_paths = [
            os.path.join(temp_dir, PROJECT_NAME),
        ]

        # Удаляем все остальное в директории build
        for item in os.listdir(temp_dir):
            item_path = os.path.join(temp_dir, item)

            # Пропускаем папки, которые нужно сохранить
            if any(item_path == keep_path for keep_path in keep_paths):
                continue

            try:
                if os.path.isdir(item_path):
                    print(f"Removing temporary directory: {item_path}")
                    shutil.rmtree(item_path)
                else:
                    print(f"Removing temporary file: {item_path}")
                    os.remove(item_path)
            except Exception as e:
                print(f"Warning: Failed to remove {item_path}: {e}")

    def _generate_package_info(self):
        """Генерация package_info.py из шаблона package_info.py.in"""
        template_path = os.path.join(os.getcwd(), "package_info.py.in")
        output_path = os.path.join(
            os.getcwd(), output_dir, PROJECT_NAME, "package_info.py"
        )

        if not os.path.exists(template_path):
            print(f"Warning: Template file {template_path} not found")
            return

        # Чтение шаблона
        with open(template_path, "r") as f:
            template_content = f.read()

        # Замена переменных в шаблоне
        processed_content = template_content.replace("@PROJECT_NAME@", PROJECT_NAME)
        processed_content = processed_content.replace(
            "@PROJECT_VERSION@", PROJECT_VERSION
        )
        processed_content = processed_content.replace(
            "@PROJECT_DESCRIPTION@", PROJECT_DESCRIPTION
        )
        processed_content = processed_content.replace(
            "@PROJECT_AUTHOR@", PROJECT_AUTHOR
        )

        # Запись результата
        with open(output_path, "w") as f:
            f.write(processed_content)

        print(f"Generated {output_path} from template")

    def _generate_stubs(self):
        """Генерация stub-файлов (.pyi) для улучшения поддержки IDE"""
        try:
            # Проверяем, установлен ли pybind11-stubgen

            importlib.import_module("pybind11_stubgen")

            # Генерируем stub-файлы напрямую в целевую директорию
            stubgen_cmd = [
                sys.executable,
                "-m",
                "pybind11_stubgen",
                PROJECT_NAME,
                "--output-dir",
                output_dir,
            ]

            # Устанавливаем PYTHONPATH для поиска собранного модуля
            env = os.environ.copy()
            build_lib = output_dir
            if "PYTHONPATH" in env:
                env["PYTHONPATH"] = build_lib + os.pathsep + env["PYTHONPATH"]
            else:
                env["PYTHONPATH"] = build_lib

            self._run_command(stubgen_cmd, env=env)

        except ImportError:
            print(
                "Warning: pybind11-stubgen not installed. Stub files will not be generated."
            )
            print("Install it with: pip install pybind11-stubgen")
        except Exception as e:
            print(f"Warning: Failed to generate stub files: {e}")

    def _run_command(self, command, cwd=None, env=None):
        """Вспомогательная функция для выполнения команд"""
        try:
            subprocess.check_call(command, cwd=cwd, env=env)
        except subprocess.CalledProcessError as e:
            print(f"Command failed: {e}")
            sys.exit(1)


# Настройка расширения
ext_modules = [
    Pybind11Extension(
        PROJECT_NAME,
        sources=sorted(glob("src/*.cpp")),
        include_dirs=[
            get_include(),
            get_include(True),
            "include",
            "/usr/local/include",  # Стандартный путь для заголовочных файлов
        ],
        libraries=["mp4v2"],
        library_dirs=["/usr/local/lib"],
        language="c++",
        extra_compile_args=extra_compile_args,
        extra_link_args=extra_link_args,
    ),
]

# Установите выходной каталог для библиотеки
for ext in ext_modules:
    ext._full_name = f"{PROJECT_NAME}/{ext.name}"

setup(
    name=PROJECT_NAME,
    version=PROJECT_VERSION,
    author=PROJECT_AUTHOR,
    description=PROJECT_DESCRIPTION,
    long_description=open("README.md").read(),
    long_description_content_type="text/markdown",
    ext_modules=ext_modules,
    cmdclass={"build_ext": MP4V2Builder},
    install_requires=[],
    packages=[PROJECT_NAME],
    package_dir={PROJECT_NAME: os.path.join(output_dir, PROJECT_NAME)},
    package_data={
        PROJECT_NAME: [
            "*.so",
            "package_info.py",
            "*.pyi",
        ]
    },
    include_package_data=True,
    zip_safe=False,
    python_requires=">=3.9",
    classifiers=[
        "Private :: Do Not Upload",
        "Operating System :: Microsoft :: Windows",
        "Operating System :: POSIX",
        "Operating System :: POSIX :: Linux",
        "Development Status :: 3 - Alpha",
        "Intended Audience :: Developers",
        "Programming Language :: Python :: 3",
        "Programming Language :: Python :: 3.9",
        "Programming Language :: Python :: 3.10",
        "Programming Language :: Python :: 3.11",
        "Programming Language :: Python :: 3.12",
        "Programming Language :: Python :: 3.13",
        "Programming Language :: C++",
        "Operating System :: OS Independent",
        "Topic :: Multimedia :: Video",
        "Topic :: Software Development :: Libraries :: Python Modules",
    ],
    keywords="mp4, video, multimedia, bindings",
    options={
        "build": {
            "build_lib": output_dir,
        },
        "build_ext": {"inplace": False},
    },
)
