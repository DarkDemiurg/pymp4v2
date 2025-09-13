# tests/test_mp4v2.py
from contextlib import contextmanager
import io
import os
import sys
import tempfile

import pytest

import pymp4v2.raw as raw

# Путь к тестовому MP4 файлу (замените на реальный путь)
TEST_MP4_FILE = "sample.mp4"


@pytest.fixture
def test_mp4_file():
    """Фикстура для проверки наличия тестового файла."""
    if not os.path.exists(TEST_MP4_FILE):
        pytest.skip(f"Test MP4 file not found: {TEST_MP4_FILE}")
    return TEST_MP4_FILE


@pytest.fixture
def temp_mp4_file():
    """Фикстура для создания временного MP4 файла."""
    with tempfile.NamedTemporaryFile(suffix=".mp4", delete=False) as tmp:
        tmp_path = tmp.name
    yield tmp_path
    # Удаляем временный файл после теста
    if os.path.exists(tmp_path):
        os.unlink(tmp_path)

@contextmanager
def suppress_output():
    """Менеджер контекста для подавления вывода на консоль."""
    original_stdout = sys.stdout
    original_stderr = sys.stderr
    
    sys.stdout = io.StringIO()
    sys.stderr = io.StringIO()
    
    try:
        yield
    finally:
        sys.stdout = original_stdout
        sys.stderr = original_stderr

def test_mp4read_and_mp4close(test_mp4_file):
    """Тест открытия и закрытия MP4 файла."""
    # Открываем файл
    handle = raw.MP4Read(test_mp4_file)

    # Проверяем, что дескриптор валиден
    assert handle.is_valid()

    # Получаем имя файла
    filename = raw.MP4GetFilename(handle)
    assert filename == test_mp4_file

    # Закрываем файл
    raw.MP4Close(handle)

    # Проверяем, что дескриптор стал невалидным
    assert not handle.is_valid()


def test_mp4create_and_mp4close(temp_mp4_file):
    """Тест создания и закрытия MP4 файла."""
    # Создаем файл
    handle = raw.MP4Create(temp_mp4_file)

    # Проверяем, что дескриптор валиден
    assert handle.is_valid()

    # Получаем имя файла
    filename = raw.MP4GetFilename(handle)
    assert filename == temp_mp4_file

    # Закрываем файл
    raw.MP4Close(handle)

    # Проверяем, что дескриптор стал невалидным
    assert not handle.is_valid()

    # Проверяем, что файл был создан
    assert os.path.exists(temp_mp4_file)


def test_mp4modify_and_mp4close(test_mp4_file):
    """Тест открытия для модификации и закрытия MP4 файла."""
    # Открываем файл для модификации
    handle = raw.MP4Modify(test_mp4_file)

    # Проверяем, что дескриптор валиден
    assert handle.is_valid()

    # Получаем имя файла
    filename = raw.MP4GetFilename(handle)
    assert filename == test_mp4_file

    # Закрываем файл
    raw.MP4Close(handle)

    # Проверяем, что дескриптор стал невалидным
    assert not handle.is_valid()


def test_mp4info(test_mp4_file):
    """Тест получения информации о MP4 файле."""
    # Открываем файл
    handle = raw.MP4Read(test_mp4_file)

    # Получаем информацию о файле
    info = raw.MP4Info(handle)

    # Проверяем, что информация не пустая
    assert info is not None
    assert isinstance(info, str)
    assert len(info) > 0

    # Закрываем файл
    raw.MP4Close(handle)


def test_mp4info_with_track_id(test_mp4_file):
    """Тест получения информации о конкретном треке."""
    # Открываем файл
    handle = raw.MP4Read(test_mp4_file)

    # Пробуем получить информацию о несуществующем треке
    info = raw.MP4Info(handle, 9999)

    # Информация должна быть None для несуществующего трека
    assert info is None

    # Закрываем файл
    raw.MP4Close(handle)


def test_mp4dump(test_mp4_file):
    """Тест дампа MP4 файла с подавлением вывода."""
    with suppress_output():
        handle = raw.MP4Read(test_mp4_file)
        
        # Получение дампа в виде строки
        dump_output = raw.MP4Dump(handle)
        
        # Проверяем, что вывод не пустой
        assert dump_output != ""
        
        # Или с параметром dumpImplicits
        dump_output_with_implicits = raw.MP4Dump(handle, True)
        assert dump_output_with_implicits != ""
        
        raw.MP4Close(handle)


def test_invalid_file_operations():
    """Тест операций с невалидным дескриптором."""
    # Создаем невалидный дескриптор
    handle = raw.MP4FileHandle()

    # Проверяем, что дескриптор невалиден
    assert not handle.is_valid()

    # Пытаемся получить имя файла - должно вызвать исключение
    with pytest.raises(RuntimeError):
        raw.MP4GetFilename(handle)

    # Пытаемся закрыть файл - не должно вызывать исключение
    raw.MP4Close(handle)


def test_context_manager(test_mp4_file):
    """Тест использования контекстного менеджера."""
    with raw.MP4Read(test_mp4_file) as handle:
        # Проверяем, что дескриптор валиден внутри контекста
        assert handle.is_valid()

        # Получаем имя файла
        filename = raw.MP4GetFilename(handle)
        assert filename == test_mp4_file

    # Проверяем, что дескриптор автоматически закрылся после выхода из контекста
    assert not handle.is_valid()


def test_mp4create_with_flags(temp_mp4_file):
    """Тест создания MP4 файла с флагами."""
    # Создаем файл с флагами
    handle = raw.MP4Create(temp_mp4_file, 0)  # 0 - стандартные флаги

    # Проверяем, что дескриптор валиден
    assert handle.is_valid()

    # Закрываем файл
    raw.MP4Close(handle)

    # Проверяем, что файл был создан
    assert os.path.exists(temp_mp4_file)


def test_mp4modify_with_flags(test_mp4_file):
    """Тест модификации MP4 файла с флагами."""
    # Открываем файл для модификации с флагами
    handle = raw.MP4Modify(test_mp4_file, 0)  # 0 - стандартные флаги

    # Проверяем, что дескриптор валиден
    assert handle.is_valid()

    # Закрываем файл
    raw.MP4Close(handle)


def test_double_close(test_mp4_file):
    """Тест двойного закрытия файла."""
    # Открываем файл
    handle = raw.MP4Read(test_mp4_file)

    # Первое закрытие
    raw.MP4Close(handle)
    assert not handle.is_valid()

    # Второе закрытие (не должно вызывать ошибок)
    raw.MP4Close(handle)
    assert not handle.is_valid()


if __name__ == "__main__":
    pytest.main([__file__, "-v"])
