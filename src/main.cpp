#include <pybind11/pybind11.h>
#include "pymp4v2/mp4file.h"
#include "pymp4v2/raw.h"

namespace py = pybind11;

// Создание привязки для enum MP4LogLevel
void bind_mp4_log_level(py::module &m)
{
    py::enum_<MP4LogLevel>(m, "MP4LogLevel")
        .value("MP4_LOG_NONE", MP4_LOG_NONE)
        .value("MP4_LOG_ERROR", MP4_LOG_ERROR)
        .value("MP4_LOG_WARNING", MP4_LOG_WARNING)
        .value("MP4_LOG_INFO", MP4_LOG_INFO)
        .value("MP4_LOG_VERBOSE1", MP4_LOG_VERBOSE1)
        .value("MP4_LOG_VERBOSE2", MP4_LOG_VERBOSE2)
        .value("MP4_LOG_VERBOSE3", MP4_LOG_VERBOSE3)
        .value("MP4_LOG_VERBOSE4", MP4_LOG_VERBOSE4)
        .export_values();
}

PYBIND11_MODULE(pymp4v2, m)
{
    m.doc() = "Python binding for MP4v2 library";

    // Регистрируем класс MP4FileHandleWrapper
    py::class_<raw::MP4FileHandleWrapper>(m, "MP4FileHandle")
        .def(py::init<>())
        .def("is_valid", &raw::MP4FileHandleWrapper::is_valid)
        .def("close", &raw::MP4FileHandleWrapper::close, py::arg("flags") = 0)
        .def("__enter__", [](raw::MP4FileHandleWrapper &self) -> raw::MP4FileHandleWrapper &
             { return self; })
        .def("__exit__", [](raw::MP4FileHandleWrapper &self, py::object exc_type, py::object exc_val, py::object exc_tb)
             {
                 self.close(0);
                 return false; // Пропускаем исключения дальше
             })
        .def("__repr__", [](const raw::MP4FileHandleWrapper &self)
             {
            if (self.is_valid()) {
                return "<MP4FileHandle (valid)>";
            } else {
                return "<MP4FileHandle (closed)>";
            } });

    // Экспорт класса MP4File
    py::class_<MP4File>(m, "MP4File")
        .def(py::init<const std::string &, const std::string &>(), py::arg("filename"), py::arg("mode") = "r")
        .def("close", &MP4File::close)
        .def("get_track_count", &MP4File::get_track_count)
        .def("save", &MP4File::save)
        .def("is_open", &MP4File::is_open)
        .def("get_info", &MP4File::get_info);
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
    // raw_module.def("get_track_count", &raw::get_track_count, "Get number of tracks in MP4 file");
    // raw_module.def("get_track_type", &raw::get_track_type, py::arg("handle"), py::arg("track_id"), "Get type of specific track");
    // Добавьте другие функции по мере необходимости

    raw_module.def("MP4Read", &raw::MP4Read_wrapper, py::arg("fileName"), py::return_value_policy::move, "Read an existing mp4 file.");
    raw_module.def("MP4Modify", &raw::MP4Modify_wrapper, py::arg("fileName"), py::arg("flags") = 0, py::return_value_policy::move, "Modify an existing mp4 file.");
    raw_module.def("MP4Close", &raw::MP4Close_wrapper, py::arg("hFile"), py::arg("flags") = 0, "Close an mp4 file.");
    raw_module.def("MP4GetFilename", &raw::MP4GetFilename_wrapper, py::arg("hFile"), "Accessor for the filename associated with a file handle.");

    // Привязка enum MP4LogLevel
    bind_mp4_log_level(m);

    raw_module.def("MP4LogSetLevel", &MP4LogSetLevel, py::arg("verbosity"), "Set the maximum log level.");
}
