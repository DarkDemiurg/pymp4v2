#include "pymp4v2/raw.h"
#include "mp4v2/mp4v2.h"
#include <stdexcept>

namespace raw {

    uintptr_t open_file(const std::string& filename, const std::string& mode) 
    {
        MP4FileHandle handle = nullptr;
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
        
        if (handle == nullptr)
        {
            throw std::runtime_error("Failed to open MP4 file: " + filename);
        }
        
        return reinterpret_cast<uintptr_t>(handle);
    }

    void close_file(uintptr_t handle) 
    {
        MP4Close(reinterpret_cast<MP4FileHandle>(handle));
    }

} // namespace raw
