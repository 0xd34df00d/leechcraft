#ifndef UTIL_H
#define UTIL_H
#include <string>
#include <vector>
#include <boost/any.hpp>

class QString;
class QPixmap;
class QImage;
class QVariant;

namespace Util
{
	std::string QStringToUTF8 (const QString&);
	std::vector<char> PixmapToData (const QPixmap&,
			const char* = "PNG", int = -1);
	std::vector<char> PixmapToData (const QImage&,
			const char* = "PNG", int = -1);
	boost::any Convert (const QVariant&);
	QVariant Convert (const boost::any&);
};

#endif

