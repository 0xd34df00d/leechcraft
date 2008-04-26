#ifndef FILEINFO_H
#define FILEINFO_H
#include <boost/filesystem/path.hpp>

struct FileInfo
{
	boost::filesystem::path Path_;
    quint64 Size_;
    int Priority_;
    float Progress_;
};

#endif

