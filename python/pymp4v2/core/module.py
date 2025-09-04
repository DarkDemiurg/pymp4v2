# Этот файл будет заменен скомпилированным расширением
# Заглушка для импорта

try:
    from .. import _pymp4v2 as native
    MP4File = native.MP4File
    raw = native.raw
except ImportError:
    # Заглушка для разработки
    class MP4File:
        def __init__(self, *args, **kwargs):
            raise ImportError("Native extension not compiled")
    
    class RawModule:
        def __getattr__(self, name):
            raise ImportError("Native extension not compiled")
    
    raw = RawModule()
