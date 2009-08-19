/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2009  Georg Rudoy
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

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

