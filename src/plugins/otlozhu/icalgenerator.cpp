/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "icalgenerator.h"
#include <util/sll/prelude.h>
#include <interfaces/core/itagsmanager.h>
#include "core.h"

namespace LC
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
			lines << ("PERCENT-COMPLETE:" + QByteArray::number (item->GetPercentage ()));

			auto tm = Core::Instance ().GetProxy ()->GetTagsManager ();
			const auto& cats = Util::Map (item->GetTagIDs (), [tm] (const QString& id) { return tm->GetTag (id); });
			if (!cats.isEmpty ())
				lines << ("CATEGORIES:" + cats.join (",").toUtf8 ());

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
		lines += Util::ConcatMap (Items_, &Serialize);
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
		for (const auto& arr : lines)
			result += arr + "\r\n";
		return result;
	}
}
}
