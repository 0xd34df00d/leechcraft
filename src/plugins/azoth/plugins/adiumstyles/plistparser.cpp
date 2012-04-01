/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2012  Georg Rudoy
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

#include "plistparser.h"
#include <QFile>
#include <QDomDocument>
#include <QtDebug>

namespace LeechCraft
{
namespace Azoth
{
namespace AdiumStyles
{
	PListParseError::PListParseError (const QString& str)
	: runtime_error (str.toUtf8 ().constData ())
	, Str_ (str)
	{
	}

	PListParseError::~PListParseError () throw ()
	{
	}

	QString PListParseError::GetStr() const
	{
		return Str_;
	}

	void PListParser::Parse (const QString& filename)
	{
		QFile file (filename);
		if (!file.open (QIODevice::ReadOnly))
			throw PListParseError (QObject::tr ("Unable to open file: %2.")
					.arg (file.errorString ()));

		QDomDocument doc;

		QString error;
		int line = 0;
		int col = 0;
		if (!doc.setContent (&file, &error, &line, &col))
			throw PListParseError (QObject::tr ("Parse error: %1 (%2:%3).")
					.arg (error)
					.arg (line)
					.arg (col));

		const auto& dict = doc.documentElement ().firstChildElement ("dict");
		auto elem = dict.firstChildElement ();
		QString currentKey;
		while (!elem.isNull ())
		{
			if (elem.tagName ().toLower () == "key")
				currentKey = elem.text ();
			else if (currentKey.isEmpty ())
				throw PListParseError (QObject::tr ("State mismatch: expected key tag."));
			else
			{
				KeyVals_ [currentKey] = elem.text ();
				currentKey.clear ();
			}

			elem = elem.nextSiblingElement ();
		}
	}

	QString PListParser::operator[] (const QString& key) const
	{
		return KeyVals_ [key];
	}
}
}
}
