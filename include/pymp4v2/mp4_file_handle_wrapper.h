#ifndef MP4_FILE_HANDLE_WRAPPER_H
#define MP4_FILE_HANDLE_WRAPPER_H

#include "mp4v2/mp4v2.h"
#include <stdexcept>

namespace raw
{
    // Класс-обертка для MP4FileHandle с защитой от использования после закрытия
    class MP4FileHandleWrapper
    {
    public:
        MP4FileHandleWrapper();
        explicit MP4FileHandleWrapper(MP4FileHandle h);

        // Запрещаем копирование
        MP4FileHandleWrapper(const MP4FileHandleWrapper &) = delete;
        MP4FileHandleWrapper &operator=(const MP4FileHandleWrapper &) = delete;

        // Разрешаем перемещение
        MP4FileHandleWrapper(MP4FileHandleWrapper &&other) noexcept;
        MP4FileHandleWrapper &operator=(MP4FileHandleWrapper &&other) noexcept;

        // Деструктор
        ~MP4FileHandleWrapper();

        // Получение указателя для использования в C++ функциях
        MP4FileHandle get() const;

        // Проверка на валидность
        bool is_valid() const;

        // Закрытие файла и инвалидация дескриптора
        void close(uint32_t flags = 0);

    private:
        MP4FileHandle handle;
    };

} // namespace raw

#endif // MP4_FILE_HANDLE_WRAPPER_H