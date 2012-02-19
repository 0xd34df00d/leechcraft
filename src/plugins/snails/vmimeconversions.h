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

#pragma once

#include <QString>
#include <QPair>
#include <vmime/mailbox.hpp>
#include <vmime/charsetConverter.hpp>

namespace LeechCraft
{
namespace Snails
{
	template<typename T>
	QString Stringize (const T& t, const vmime::charset& source)
	{
		vmime::string str;
		vmime::utility::outputStreamStringAdapter out (str);
		vmime::utility::charsetFilteredOutputStream fout (source,
				vmime::charset ("utf-8"), out);

		t->extract (fout);
		fout.flush ();

		return QString::fromUtf8 (str.c_str ());
	}

	template<typename T>
	QString Stringize (const T& t)
	{
		vmime::string str;
		vmime::utility::outputStreamStringAdapter out (str);

		t->extract (out);

		return QString::fromUtf8 (str.c_str ());
	}

	template<typename T>
	QString StringizeCT (const T& w)
	{
		return QString::fromUtf8 (w.getConvertedText (vmime::charsets::UTF_8).c_str ());
	}

	inline QPair<QString, QString> Mailbox2Strings (const vmime::ref<const vmime::mailbox>& mbox)
	{
		return {
					StringizeCT (mbox->getName ()),
					QString::fromUtf8 (mbox->getEmail ().c_str ())
				};
	}
}
}
