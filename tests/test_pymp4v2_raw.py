# tests/test_mp4v2.py
import pytest
import os
import tempfile
import pymp4v2.raw as raw

# Путь к тестовому MP4 файлу (замените на реальный путь)
TEST_MP4_FILE = "Vadim.mp4"

@pytest.fixture
def test_mp4_file():
    """Фикстура для проверки наличия тестового файла."""
    if not os.path.exists(TEST_MP4_FILE):
        pytest.skip(f"Test MP4 file not found: {TEST_MP4_FILE}")
    return TEST_MP4_FILE

@pytest.fixture
def temp_mp4_file():
    """Фикстура для создания временного MP4 файла."""
    with tempfile.NamedTemporaryFile(suffix='.mp4', delete=False) as tmp:
        tmp_path = tmp.name
    yield tmp_path
    # Удаляем временный файл после теста
    if os.path.exists(tmp_path):
        os.unlink(tmp_path)

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
    """Тест дампа MP4 файла."""
    # Открываем файл
    handle = raw.MP4Read(test_mp4_file)
    
    # Делаем дамп файла
    result = raw.MP4Dump(handle)
    
    # Проверяем, что дамп успешен
    assert result is True
    
    # Делаем дамп без имплицитных данных
    result = raw.MP4Dump(handle, False)
    
    # Проверяем, что дамп успешен
    assert result is True
    
    # Закрываем файл
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

# import pymp4v2

# pymp4v2.raw.MP4LogSetLevel(pymp4v2.MP4_LOG_WARNING)

# FILE_1 = "/home/dima/1/89.mp4"
# FILE_2 = "/home/dima/1/104406.mp4"

# print("=== Begin tests")

# print("===== Test MP4Read")
# h = pymp4v2.raw.MP4Read(FILE_1)
# h2 = pymp4v2.raw.MP4Read(FILE_2)
# print(f"MP4FileHandle = {h} {h2}")
# print(pymp4v2.raw.MP4GetFilename(h), pymp4v2.raw.MP4GetFilename(h2))
# pymp4v2.raw.MP4Close(h)
# pymp4v2.raw.MP4Close(h2)

# print("===== Test MP4Modify")
# h = pymp4v2.raw.MP4Modify(FILE_1)
# print(f"MP4FileHandle = {h}")
# print(pymp4v2.raw.MP4GetFilename(h))
# pymp4v2.raw.MP4Close(h)

# print("===== Test MP4Dump")
# h = pymp4v2.raw.MP4Read(FILE_1)
# print(f"MP4FileHandle = {h}")
# print(pymp4v2.raw.MP4Dump(h))
# print("========== ")
# print(pymp4v2.raw.MP4Dump(h, True))
# pymp4v2.raw.MP4Close(h)

# print("===== Test MP4Info")
# h = pymp4v2.raw.MP4Read(FILE_1)
# print(f"MP4FileHandle = {h}")
# print("========== pymp4v2.raw.MP4Info(h)")
# print(pymp4v2.raw.MP4Info(h))
# print("========== pymp4v2.raw.MP4Info(h, 1)")
# print(pymp4v2.raw.MP4Info(h, 1))
# print("========== pymp4v2.raw.MP4Info(h, pymp4v2.MP4TrackId(1)")
# print(pymp4v2.raw.MP4Info(h, pymp4v2.MP4TrackId(1)))
# pymp4v2.raw.MP4Close(h)

# print("===== Test context manager")
# with pymp4v2.raw.MP4Read(FILE_1) as file_handle:
#     print(f"File handle valid is {file_handle.is_valid()}")
#     filename = pymp4v2.raw.MP4GetFilename(file_handle)
#     print(filename)

# print(f"File handle valid is {file_handle.is_valid()}")

# print("=== End tests")
