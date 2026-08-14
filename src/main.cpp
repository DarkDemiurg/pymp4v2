#include <pybind11/pybind11.h>
#include "pymp4v2/error.h"
#include "pymp4v2/mp4file.h"
#include "pymp4v2/raw.h"

#ifndef PYMP4V2_VERSION
#define PYMP4V2_VERSION "0.0.0"
#endif

namespace py = pybind11;

PYBIND11_MODULE(_pymp4v2, m)
{
    m.doc() = "Alpha Python bindings for a subset of the MP4v2 C API";
    m.attr("__version__") = PYMP4V2_VERSION;
    py::register_exception<MP4Error>(m, "MP4Error", PyExc_RuntimeError);

    auto m_raw = m.def_submodule("raw", "1:1 wrappers around the bound subset of the MP4v2 C API");
    m_raw.attr("MP4Error") = m.attr("MP4Error");

    m_raw.attr("MP4_INVALID_TRACK_ID") = py::cast(MP4_INVALID_TRACK_ID);
    m_raw.attr("MP4_CLOSE_DO_NOT_COMPUTE_BITRATE") = py::int_(MP4_CLOSE_DO_NOT_COMPUTE_BITRATE); // for MP4Close flags
    m_raw.attr("MP4_CREATE_64BIT_DATA") = py::int_(MP4_CREATE_64BIT_DATA);                       // for MP4Create flags
    m_raw.attr("MP4_CREATE_64BIT_TIME") = py::int_(MP4_CREATE_64BIT_TIME);                       // for MP4Create, MP4CreateEx, MP4CreateCallbacks, MP4CreateCallbacksEx flags

    py::class_<raw::MP4FileHandleWrapper>(m_raw, "MP4FileHandle")
        .def(py::init<>())
        .def("is_valid", &raw::MP4FileHandleWrapper::is_valid)
        .def("close", &raw::MP4FileHandleWrapper::close, py::arg("flags") = py::none(),
             py::call_guard<py::gil_scoped_release>(),
             "Close the handle. flags=None uses close_flags (default 0).")
        .def_property("close_flags", &raw::MP4FileHandleWrapper::close_flags,
                      &raw::MP4FileHandleWrapper::set_close_flags,
                      "Flags passed to MP4Close from close() with no args, __exit__, and the destructor.")
        .def("__enter__", [](raw::MP4FileHandleWrapper &self) -> raw::MP4FileHandleWrapper &
             { return self; })
        .def("__exit__", [](raw::MP4FileHandleWrapper &self, py::object, py::object, py::object)
             {
                 {
                     py::gil_scoped_release release;
                     self.close();
                 }
                 return false; })
        .def("__repr__", [](const raw::MP4FileHandleWrapper &self)
             {
            if (self.is_valid()) {
                return "<MP4FileHandle (valid)>";
            } else {
                return "<MP4FileHandle (closed)>";
            } });

    // C typedefs are uint32/uint64; type checkers see int. Runtime aliases are the
    // int type so they match stubs (MP4TrackId = int) and accept plain ints.
    py::object py_int = py::type::of(py::int_());
    m_raw.attr("MP4TrackId") = py_int;
    m_raw.attr("MP4SampleId") = py_int;
    m_raw.attr("MP4Timestamp") = py_int;
    m_raw.attr("MP4Duration") = py_int;
    m_raw.attr("MP4EditId") = py_int;

    m_raw.def("MP4Read", &raw::MP4Read_wrapper, py::arg("fileName"), py::return_value_policy::move,
              py::call_guard<py::gil_scoped_release>(),
              R"doc(
    Read an existing mp4 file.

    MP4Read is the first call that should be used when you want to just read an existing mp4 file. 
    It is equivalent to opening a file for reading, but in addition the mp4 file is parsed and the 
    control information is loaded into memory. 
    Note that actual track samples are not read into memory until MP4ReadSample() is called.

    Args:
        fileName (str): pathname of the file to be read. On Windows, this should be a UTF-8 encoded string. 
                        On other platforms, it should be an 8-bit encoding that is appropriate for the platform, 
                        locale, file system, etc. (prefer to use UTF-8 when possible).

    Returns:
        A handle of the file for use in subsequent calls to the library.

    Raises:
        MP4Error: On any error.
)doc");

    m_raw.def("MP4Create", &raw::MP4Create_wrapper, py::arg("fileName"), py::arg("flags") = 0, py::return_value_policy::move,
              py::call_guard<py::gil_scoped_release>(),
              R"doc(
    Create a new mp4 file.

    MP4Create is the first call that should be used when you want to create a new, empty mp4 file. 
    It is equivalent to opening a file for writing, but is also involved with the 
    creation of necessary mp4 framework structures. 
    I.e. invoking MP4Create() followed by MP4Close() will result in a file with a non-zero size.

    Args:
        fileName (str):	pathname of the file to be created. On Windows, this should be a UTF-8 encoded string. 
                        On other platforms, it should be an 8-bit encoding that is appropriate for the platform, 
                        locale, file system, etc. (prefer to use UTF-8 when possible).
        flags (int):	Default is 0. Bitmask that allows the user to set 64-bit values for data or time atoms. 
                        Valid bits may be any combination of:
                            `pymp4v2.raw.MP4_CREATE_64BIT_DATA`
                            `pymp4v2.raw.MP4_CREATE_64BIT_TIME`
    Returns:
        A handle of the newly created file for use in subsequent calls to the library.

    Raises:
        MP4Error: On any error.        
)doc");

    m_raw.def("MP4CreateEx", &raw::MP4CreateEx_wrapper, py::arg("fileName"), py::arg("flags") = 0,
              py::arg("add_ftyp") = 1, py::arg("add_iods") = 1, py::arg("majorBrand") = py::none(),
              py::arg("minorVersion") = 0, py::arg("compatibleBrands") = py::make_tuple(),
              py::return_value_policy::move, py::call_guard<py::gil_scoped_release>(),
              R"doc(
    Create a new mp4 file with extended ftyp / iods options.

    MP4CreateEx is an extended version of MP4Create(). If majorBrand is None, mp4v2
    keeps its default ftyp (major ``mp42``, compatible ``mp42`` / ``isom``). If
    majorBrand is set, compatibleBrands replaces the default list (an empty
    sequence clears it). Count is taken from the sequence length.

    Custom I/O (MP4*Callbacks, MP4ReadProvider) is not bound.

    Args:
        fileName (str): pathname of the file to be created.
        flags (int): bitmask; ``MP4_CREATE_64BIT_DATA`` / ``MP4_CREATE_64BIT_TIME``.
        add_ftyp (int): if true, an ftyp atom is created (default 1).
        add_iods (int): if true, an iods atom is created (default 1).
        majorBrand (str or None): ftyp major brand identifier, or None for defaults.
        minorVersion (int): ftyp minor version of the major brand.
        compatibleBrands (sequence of str): ftyp compatible brands.

    Returns:
        A handle of the newly created file for use in subsequent calls to the library.

    Raises:
        MP4Error: On any error.
)doc");

    m_raw.def("MP4Modify", &raw::MP4Modify_wrapper, py::arg("fileName"), py::arg("flags") = 0, py::return_value_policy::move,
              py::call_guard<py::gil_scoped_release>(),
              R"doc(
    Modify an existing mp4 file.

    MP4Modify is the first call that should be used when you want to modify an existing mp4 file. 
    It is roughly equivalent to opening a file in read/write mode.
    Since modifications to an existing mp4 file can result in a sub-optimal file layout, 
    you may want to use MP4Optimize() after you have modified and closed the mp4 file.

    Args:
        fileName (str):	pathname of the file to be modified. On Windows, this should be a UTF-8 encoded string. 
                        On other platforms, it should be an 8-bit encoding that is appropriate for the platform, 
                        locale, file system, etc. (prefer to use UTF-8 when possible).
        flags (int):	currently ignored.

    Returns:
        A handle of the file for use in subsequent calls to the library.

    Raises:
        MP4Error: On any error.        
)doc");

    m_raw.def("MP4Close", &raw::MP4Close_wrapper, py::arg("hFile"), py::arg("flags") = 0,
              py::call_guard<py::gil_scoped_release>(),
              R"doc(
    Close an mp4 file.

    MP4Close closes a previously opened mp4 file. If the file was opened writable with MP4Create() or MP4Modify(), 
    then MP4Close() will write out all pending information to disk.

    Parameters
        hFile (MP4FileHandle):  handle of file to close.
        flags (int):            Default is 0. Bitmask that allows the user to set extra options for the close commands. 
                                Valid options include:
                                    `pymp4v2.raw.MP4_CLOSE_DO_NOT_COMPUTE_BITRATE`
)doc");

    m_raw.def("MP4Dump", &raw::MP4Dump_wrapper, py::arg("hFile"), py::arg("dumpImplicits") = false,
              py::call_guard<py::gil_scoped_release>(),
              R"doc(
    Dump mp4 file contents as ASCII either to stdout or the log callback (see `MP4SetLogCallback()`)

    Dump is an invaluable debugging tool in that in can reveal all the details of the mp4 control structures. 
    However, the output will not make much sense until you familiarize yourself with the mp4 specification 
    (or the Quicktime File Format specification).

    Note that `MP4Dump()` will not print the individual values of control tables, such as the size of each sample, 
    unless the current log level is at least MP4_LOG_VERBOSE2. See MP4LogSetLevel() for how to set this.

    Args:
        hFile (MP4FileHandle):  handle of file to dump.
        dumpImplicits (bool):   prints properties which would not actually be written to the mp4 file, 
                                but still exist in mp4 control structures. 
                                I.e. they are implicit given the current values of other controlling properties.
        
    Returns:
        True on success. Raises MP4Error on failure. Output is printed to stdout (or the log
        callback), not returned as a string.
)doc");

    m_raw.def("MP4GetFilename", &raw::MP4GetFilename_wrapper, py::arg("hFile"),
              R"doc(
    Accessor for the filename associated with a file handle.

    Args:
        hFile (MP4FileHandle): a file handle.
        
    Returns:
        filename associated with hFile.
)doc");

    m_raw.def("MP4Info", &raw::MP4Info_wrapper, py::arg("hFile"), py::arg("trackId") = static_cast<MP4TrackId>(MP4_INVALID_TRACK_ID),
              py::call_guard<py::gil_scoped_release>(),
              R"doc(
    Return a textual summary of an mp4 file.

    MP4FileInfo provides a string that contains a textual summary of the contents of an mp4 file. 
    This includes the track id's, the track type, and track specific information. 
    For example, for a video track, media encoding, image size, frame rate, and bitrate are summarized.

    The C library allocates the summary string; this wrapper copies it into a Python
    str and frees the original. The returned string contains newlines and tabs.

    Args:
        hFile (MP4FileHandle):  handle of file to summarize.
        trackId (MP4TrackId):   trackId specifies track to summarize. If the value is `MP4_INVALID_TRACK_ID`,
                                the summary info is created for all tracks in the file.
        
    Returns:
        On success a string containing summary information. On failure, None.
)doc");

    m_raw.def("MP4FileInfo", &raw::MP4FileInfo_wrapper, py::arg("fileName"), py::arg("trackId") = static_cast<MP4TrackId>(MP4_INVALID_TRACK_ID),
              py::call_guard<py::gil_scoped_release>(),
              R"doc(
    Return a textual summary of an mp4 file.

    MP4FileInfo provides a string that contains a textual summary of the contents of an mp4 file. 
    This includes the track id's, the track type, and track specific information. 
    For example, for a video track, media encoding, image size, frame rate, and bitrate are summarized.

    The C library allocates the summary string; this wrapper copies it into a Python
    str and frees the original. The returned string contains newlines and tabs.

    Args:
        fileName (string):  pathname to mp4 file to summarize.
        trackId (MP4TrackId):   specifies track to summarize. If the value is MP4_INVALID_TRACK_ID, 
                                the summary info is created for all tracks in the file.
        
    Returns:
        On success a string containing summary information. On failure, None.
)doc");

    m_raw.def("MP4Optimize", &raw::MP4Optimize_wrapper, py::arg("fileName"), py::arg("newFileName") = static_cast<char *>(NULL),
              py::call_guard<py::gil_scoped_release>(),
              R"doc(
    Optimize the layout of an mp4 file.

    MP4Optimize reads an existing mp4 file and writes a new version of the file with the two important changes:

    First, the mp4 control information is moved to the beginning of the file. 
    (Frequenty it is at the end of the file due to it being constantly modified as track samples are added to an mp4 file). 
    This optimization is useful in that in allows the mp4 file to be HTTP streamed.
    Second, the track samples are interleaved so that the samples for a particular instant 
    in time are colocated within the file. 
    This eliminates disk seeks during playback of the file which results in better performance.

    There are also two important side effects of `MP4Optimize()`:

    First, any free blocks within the mp4 file are eliminated.

    Second, as a side effect of the sample interleaving process any media data chunks that are not actually 
    referenced by the mp4 control structures are deleted. 
    This is useful if you have called `MP4DeleteTrack()` which only deletes the control information for a track, 
    and not the actual media data.

    Args:
        fileName (string):  pathname of (existing) file to be optimized.
        newFileName (string):   pathname of the new optimized file. 
                                If `None` a temporary file in the same directory as the fileName will be used 
                                and fileName will be over-written upon successful completion.

    Returns:
        True on success. Raises MP4Error on failure.
)doc");

    py::enum_<MP4LogLevel>(m_raw, "MP4LogLevel")
        .value("MP4_LOG_NONE", MP4_LOG_NONE)
        .value("MP4_LOG_ERROR", MP4_LOG_ERROR)
        .value("MP4_LOG_WARNING", MP4_LOG_WARNING)
        .value("MP4_LOG_INFO", MP4_LOG_INFO)
        .value("MP4_LOG_VERBOSE1", MP4_LOG_VERBOSE1)
        .value("MP4_LOG_VERBOSE2", MP4_LOG_VERBOSE2)
        .value("MP4_LOG_VERBOSE3", MP4_LOG_VERBOSE3)
        .value("MP4_LOG_VERBOSE4", MP4_LOG_VERBOSE4)
        .export_values();

    m_raw.def("MP4LogSetLevel", &MP4LogSetLevel, py::arg("verbosity"),
              R"doc(
    Set the maximum log level.

    MP4LogSetLevel sets the maximum level of diagnostic information passed to the current log handler.

    Args:
        verbosity (MP4LogLevel): specifies the log level to set.
        
    Returns:
        None.
)doc");

    py::enum_<MP4TagArtworkType_e>(m_raw, "MP4TagArtworkType_e")
        .value("MP4_ART_UNDEFINED", MP4_ART_UNDEFINED)
        .value("MP4_ART_BMP", MP4_ART_BMP)
        .value("MP4_ART_GIF", MP4_ART_GIF)
        .value("MP4_ART_JPEG", MP4_ART_JPEG)
        .value("MP4_ART_PNG", MP4_ART_PNG)
        .export_values();

    raw::bind_extended(m_raw);
    bind_highlevel(m, m_raw);
}
