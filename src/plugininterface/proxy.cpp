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

#include <QCoreApplication>
#include <QtDebug>
#include "proxy.h"

using namespace LeechCraft::Util;

Proxy *Proxy::Instance_ = 0;

Proxy::Proxy ()
{
	Strings_ << "bytes" << "KB" << "MB" << "GB";
}

Proxy::~Proxy ()
{
}

/*! @brief Returns a Proxy.
 *
 * Returns an instance of Proxy which is unique through whole
 * application and it's plugins.
 *
 * @return Pointer to the Proxy instance.
 */
Proxy* Proxy::Instance ()
{
	if (!Instance_)
		Instance_ = new Proxy;
	return Instance_;
}

/*! @brief Sets strings for sizes.
 *
 * Used to simplify the translation of user interface. Main app
 * could set the translated strings here. First one is for bytes,
 * seoond - for kilobytes, third - megabytes, fourth - gigabytes.
 *
 * @param[in] str Strings for the sizes.
 * @sa MakePrettySize
 */
void Proxy::SetStrings (const QStringList& str)
{
	Strings_ = str;
}

/*! @brief Returns application name.
 *
 * @return The application name.
 * @sa GetOrganizationName
 */
QString Proxy::GetApplicationName () const
{
	return QCoreApplication::applicationName ();
}

/*! @brief Returns organization name.
 *
 * @return The organization name.
 * @sa GetApplicationName
 */
QString Proxy::GetOrganizationName () const
{
	return QCoreApplication::organizationName ();
}

/*! @brief Makes a formatted size from number.
 *
 * Converts, for example, 1048576 to 1.0 MB.
 *
 * @param[in] sourcesize Size in bytes.
 * @return Formatted string.
 *
 * @sa SetStrings
 * @sa MakeTimeFromLong
 */
QString Proxy::MakePrettySize (qint64 sourcesize) const
{
	QString dString = Strings_ [0];
	long double size = sourcesize;
	if (size >= 1024)
	{
		dString = Strings_ [1];
		size /= 1024;
	}
	if (size >= 1024)
	{
		dString = Strings_ [2];
		size /= 1024;
	}
	if (size >= 1024)
	{
		dString = Strings_ [3];
		size /= 1024;
	}

	return QString::number (size, 'f', 1) + " " + dString;
}

/*! @brief Makes a formatted time from number.
 *
 * Converts, for example 256 to 00:04:16.
 *
 * @param[in] time Time interval in seconds.
 * @return DateTime object.
 *
 * @sa MakePrettySize
 */
QString Proxy::MakeTimeFromLong (ulong time) const
{
	int d = time / 86400;
	time -= d * 86400;
	QString result;
	if (d)
		result += tr ("%n day(s), ", "", d);
	result += QTime (0, 0, 0).addSecs (time).toString ();
	return result;
}

