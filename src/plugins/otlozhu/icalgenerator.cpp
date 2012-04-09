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

#include "icalgenerator.h"
#include <algorithm>

namespace LeechCraft
{
namespace Otlozhu
{
	ICalGenerator& ICalGenerator::operator<< (TodoItem_ptr item)
	{
		Items_ << item;
		return *this;
	}

	ICalGenerator& ICalGenerator::operator<< (QList<TodoItem_ptr> items)
	{
		Items_ += items;
		return *this;
	}

	namespace
	{
		QList<QByteArray> Serialize (TodoItem_ptr item)
		{
			QList<QByteArray> lines;
			lines << "BEGIN:VTODO";
			const QString& dateFt = "yyyyMMddTHHmmss";
			const auto& createdStr = item->GetCreatedDate ().toString (dateFt).toLatin1 ();
			lines << ("DTSTAMP:" + createdStr + "Z");
			lines << ("UID:" + item->GetID ().toLatin1 ());
			const auto& due = item->GetDueDate ();
			if (!due.isNull ())
				lines << ("DUE:" + due.toString (dateFt).toLatin1 ());
			lines << ("SUMMARY:" + item->GetTitle ().toUtf8 ().trimmed ());
			if (!item->GetComment ().isEmpty ())
				lines << ("COMMENT:" + item->GetComment ().toUtf8 ().trimmed ());
			lines << ("PERCENT:" + QByteArray::number (item->GetPercentage ()));
			lines << "END:VTODO";
			return lines;
		}
	}

	QByteArray ICalGenerator::operator() () const
	{
		QList<QByteArray> lines;
		lines << "BEGIN:VCALENDAR"
			<< "VERSION:2.0"
			<< "PRODID:-//LeechCraft//NONSGML Otlozhu//EN";
		std::for_each (Items_.begin (), Items_.end (),
				[&lines] (TodoItem_ptr item) { lines += Serialize (item); });
		lines << "END:VCALENDAR";

		auto it = lines.begin ();
		while (it != lines.end ())
		{
			if (it->size () <= 75)
			{
				++it;
				continue;
			}

			const QByteArray line = *it;
			*it = line.left (75);
			it = lines.insert (it + 1, '\t' + line.mid (75));
		}

		QByteArray result;
		Q_FOREACH (const QByteArray& arr, lines)
			result += arr + "\r\n";
		return result;
	}
}
}
