#ifndef PYMP4V2_ERROR_H
#define PYMP4V2_ERROR_H

#include <stdexcept>
#include <string>

// All mp4v2 failures that are not predicates (e.g. MP4HaveAtom) raise this.
// It subclasses std::runtime_error so existing `except RuntimeError` still matches.
class MP4Error : public std::runtime_error
{
public:
    explicit MP4Error(const std::string &msg) : std::runtime_error(msg) {}
};

#endif // PYMP4V2_ERROR_H
