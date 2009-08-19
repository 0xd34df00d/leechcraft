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

#include "util.h"
#include <QString>
#include <QPixmap>
#include <QImage>
#include <QVariant>
#include <QByteArray>
#include <QBuffer>
#include <QDate>
#include <WDate>
#include <WString>

std::string Util::QStringToUTF8 (const QString& var)
{
	return var.toUtf8 ().constData ();
}

std::vector<char> Util::PixmapToData (const QPixmap& pixmap,
		const char *format, int quality)
{
	QByteArray bytes;
	{
		QBuffer buffer (&bytes);
		buffer.open (QIODevice::WriteOnly);
		pixmap.save (&buffer, format, quality);
	}
	std::vector<char> result (bytes.constData (),
			bytes.constData () + bytes.size ());
	return result;
}

std::vector<char> Util::PixmapToData (const QImage& pixmap,
		const char *format, int quality)
{
	QByteArray bytes;
	{
		QBuffer buffer (&bytes);
		buffer.open (QIODevice::WriteOnly);
		pixmap.save (&buffer, format, quality);
	}
	std::vector<char> result (bytes.constData (),
			bytes.constData () + bytes.size ());
	return result;
}

boost::any Util::Convert (const QVariant& var)
{
	switch (var.type ())
	{
		case QVariant::Bool:
			return boost::any (var.toBool ());
		case QVariant::Char:
		case QVariant::String:
			return boost::any (Util::QStringToUTF8 (var.toString ()));
		case QVariant::Date:
			{
				QDate date = var.toDate ();
				Wt::WDate res (date.year (), date.month (), date.day ());
				return boost::any (res);
			}
		case QVariant::Double:
			return boost::any (var.toDouble ());
		case QVariant::Int:
			return boost::any (var.toInt ());
		case QVariant::LongLong:
			return boost::any (var.toLongLong ());
		case QVariant::UInt:
			return boost::any (var.toUInt ());
		case QVariant::ULongLong:
			return boost::any (var.toULongLong ());
		// Unconvertable
		case QVariant::BitArray:
		case QVariant::Bitmap:
		case QVariant::Brush:
		case QVariant::ByteArray:
		case QVariant::Color:
		case QVariant::Cursor:
		case QVariant::DateTime:
		case QVariant::Font:
		case QVariant::Icon:
		case QVariant::Image:
		case QVariant::Invalid:
		case QVariant::KeySequence:
		case QVariant::Line:
		case QVariant::LineF:
		case QVariant::List:
		case QVariant::Locale:
		case QVariant::Map:
		case QVariant::Matrix:
		case QVariant::Transform:
		case QVariant::Palette:
		case QVariant::Pen:
		case QVariant::Pixmap:
		case QVariant::Point:
		case QVariant::PointF:
		case QVariant::Polygon:
		case QVariant::Rect:
		case QVariant::RectF:
		case QVariant::RegExp:
		case QVariant::Region:
		case QVariant::Size:
		case QVariant::SizeF:
		case QVariant::SizePolicy:
		case QVariant::StringList:
		case QVariant::TextFormat:
		case QVariant::TextLength:
		case QVariant::Time:
		case QVariant::Url:
		case QVariant::UserType:
		case QVariant::LastType:
			return boost::any ();
	}

	return boost::any ();
}

QVariant Util::Convert (const boost::any& a)
{
	if (a.type () == typeid (bool))
		return QVariant (boost::any_cast<bool> (a));
	else if (a.type () == typeid (std::string))
		return QVariant (QString::fromStdString (boost::any_cast<std::string> (a)));
	else if (a.type () == typeid (Wt::WString))
		return QVariant (QString::fromStdString (boost::any_cast<Wt::WString> (a).toUTF8 ()));
	else if (a.type () == typeid (Wt::WDate))
	{
		Wt::WDate date = boost::any_cast<Wt::WDate> (a);
		return QVariant (QDate (date.year (), date.month (), date.day ()));
	}
	else if (a.type () == typeid (double) ||
			a.type () == typeid (float))
		return QVariant (boost::any_cast<double> (a));
	else if (a.type () == typeid (char))
		return QVariant (boost::any_cast<char> (a));
	else if (a.type () == typeid (int) ||
			a.type () == typeid (short))
		return QVariant (boost::any_cast<int> (a));
	else if (a.type () == typeid (long long) ||
			a.type () == typeid (long))
		return QVariant (boost::any_cast<long long> (a));
	else if (a.type () == typeid (unsigned int))
		return QVariant (boost::any_cast<unsigned int> (a));
	else if (a.type () == typeid (unsigned long long) ||
			a.type () == typeid (unsigned long))
		return QVariant (boost::any_cast<unsigned long long> (a));
	else
		return boost::any_cast<QVariant> (a);
}

