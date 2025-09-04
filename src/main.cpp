#include <pybind11/pybind11.h>
#include "pymp4v2/mp4file.h"
#include "pymp4v2/raw.h"

namespace py = pybind11;

PYBIND11_MODULE(pymp4v2, m) 
{
    m.doc() = "Python binding for MP4v2 library";
    
    // Экспорт класса MP4File
    py::class_<MP4File>(m, "MP4File")
        .def(py::init<const std::string&, const std::string&>(), py::arg("filename"), py::arg("mode") = "r")
        .def("close", &MP4File::close)
        .def("get_track_count", &MP4File::get_track_count)
        .def("save", &MP4File::save)
        .def("is_open", &MP4File::is_open);
        // .def("__enter__", &MP4File::enter, py::return_value_policy::reference)
        // .def("__exit__", &MP4File::exit)
        // .def("get_track_type", &MP4File::get_track_type)
        // .def("get_all_metadata", &MP4File::get_all_metadata)
        // .def("set_metadata", &MP4File::set_metadata)
        // .def("get_metadata", &MP4File::get_metadata)
        // .def("set_metadata_item", &MP4File::set_metadata_item)
        // .def("clear_all_metadata", &MP4File::clear_all_metadata)
    
    // Создание подмодуля для raw функций
    auto raw_module = m.def_submodule("raw", "Raw MP4v2 functions");
    raw_module.def("open_file", &raw::open_file, py::arg("filename"), py::arg("mode") = "r", "Open MP4 file and return handle");
    raw_module.def("close_file", &raw::close_file, "Close MP4 file by handle");
    raw_module.def("get_track_count", &raw::get_track_count, "Get number of tracks in MP4 file");
    raw_module.def("get_track_type", &raw::get_track_type, py::arg("handle"), py::arg("track_id"), "Get type of specific track");
    // Добавьте другие функции по мере необходимости
}
