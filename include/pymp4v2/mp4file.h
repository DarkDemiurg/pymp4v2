#ifndef Pymp4v2_MP4FILE_H
#define Pymp4v2_MP4FILE_H

#include <pybind11/pybind11.h>
#include <string>
#include <map>
#include "mp4v2/mp4v2.h"

namespace py = pybind11;

class MP4File 
{
public:
    MP4File(const std::string& filename, const std::string& mode = "r");
    ~MP4File();
    
    void close();
    int get_track_count() const;
    void save();
    bool is_open() const;

    // MP4File* enter();
    // void exit(const py::object& exc_type, const py::object& exc_value, const py::object& traceback);    
    // std::string get_track_type(int track_id) const;

    // std::map<std::string, std::string> get_all_metadata() const;    
    // void set_metadata(const std::map<std::string, std::string>& metadata);
    // std::string get_metadata(const std::string& key) const;
    // void set_metadata_item(const std::string& key, const std::string& value);
    // void clear_all_metadata();

private:
    MP4FileHandle handle = nullptr;
};

#endif // Pymp4v2_MP4FILE_H
