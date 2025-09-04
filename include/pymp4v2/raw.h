#ifndef Pymp4v2_RAW_H
#define Pymp4v2_RAW_H

#include <string>
#include <cstdint>

namespace raw
{
    uintptr_t open_file(const std::string &filename, const std::string &mode = "r");
    void close_file(uintptr_t handle);
    // int get_track_count(uintptr_t handle);
    // std::string get_track_type(uintptr_t handle, int track_id);

} // namespace raw

#endif // Pymp4v2_RAW_H
