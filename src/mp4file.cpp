#include "pymp4v2/mp4file.h"
#include <stdexcept>

MP4File::MP4File(const std::string& filename, const std::string& mode) 
{
    if (mode == "r") 
    {
        handle = MP4Read(filename.c_str());
    } 
    else if (mode == "w" || mode == "a") 
    {
        handle = MP4Modify(filename.c_str(), 0);
    } 
    else 
    {
        throw std::runtime_error("Unsupported mode: " + mode);
    }
    
    if (handle != nullptr)
    {
        throw std::runtime_error("Failed to open MP4 file: " + filename);
    }
}

MP4File::~MP4File() 
{
    close();
}

void MP4File::close() 
{
    if (handle) 
    {
        MP4Close(handle);
        handle = nullptr;
    }
}

// Метод для получения количества треков
int MP4File::get_track_count() const 
{
    if (handle != nullptr) 
    {
        return MP4GetNumberOfTracks(handle);
    }
    
    return -1;
}

// Сохранить изменения в файле
void MP4File::save() 
{
    if (handle == nullptr) throw std::runtime_error("MP4 file is closed");
    // Закрываем и открываем заново для сохранения изменений
    MP4Close(handle);
    // В реальной реализации здесь должна быть более сложная логика
    handle = nullptr;
}

bool MP4File::is_open() const
{
    return handle != nullptr;
}