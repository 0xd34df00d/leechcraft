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

#include "todoitem.h"
#include <QUuid>
#include <QDataStream>
#include <QtDebug>

namespace LeechCraft
{
namespace Otlozhu
{
	TodoItem::TodoItem ()
	: ID_ (QUuid::createUuid ().toString ())
	, Created_ (QDateTime::currentDateTime ())
	, Percentage_ (0)
	{
	}

	TodoItem_ptr TodoItem::Clone () const
	{
		TodoItem_ptr clone (new TodoItem);
		clone->Title_ = Title_;
		clone->Comment_ = Comment_;
		clone->TagIDs_ = TagIDs_;
		clone->Created_ = Created_;
		clone->Due_ = Due_;
		clone->Percentage_ = Percentage_;
		clone->Deps_ = Deps_;
		return clone;
	}

	TodoItem_ptr TodoItem::Deserialize (const QByteArray& data)
	{
		QDataStream str (data);
		quint8 version = 0;
		str >> version;
		if (version != 1)
		{
			qWarning () << Q_FUNC_INFO
					<< "unknown version"
					<< version;
			return TodoItem_ptr ();
		}

		TodoItem_ptr item (new TodoItem);
		str >> item->ID_
			>> item->Title_
			>> item->Comment_
			>> item->TagIDs_
			>> item->Created_
			>> item->Due_
			>> item->Percentage_
			>> item->Deps_;
		return item;
	}

	QByteArray TodoItem::Serialize () const
	{
		QByteArray res;
		QDataStream out (&res, QIODevice::WriteOnly);

		out << static_cast<quint8> (1)
			<< ID_
			<< Title_
			<< Comment_
			<< TagIDs_
			<< Created_
			<< Due_
			<< Percentage_
			<< Deps_;

		return res;
	}

	QString TodoItem::GetID () const
	{
		return ID_;
	}

	QString TodoItem::GetTitle () const
	{
		return Title_;
	}

	void TodoItem::SetTitle (const QString& title)
	{
		Title_ = title;
	}

	QString TodoItem::GetComment () const
	{
		return Comment_;
	}

	void TodoItem::SetComment (const QString& comment)
	{
		Comment_ = comment;
	}

	QStringList TodoItem::GetTagIDs () const
	{
		return TagIDs_;
	}

	void TodoItem::SetTagIDs (const QStringList& tagIds)
	{
		TagIDs_ = tagIds;
	}

	QDateTime TodoItem::GetCreatedDate () const
	{
		return Created_;
	}

	void TodoItem::SetCreatedDate (const QDateTime& created)
	{
		Created_ = created;
	}

	QDateTime TodoItem::GetDueDate () const
	{
		return Due_;
	}

	void TodoItem::SetDueDate (const QDateTime& due)
	{
		Due_ = due;
	}

	int TodoItem::GetPercentage () const
	{
		return Percentage_;
	}

	void TodoItem::SetPercentage (int p)
	{
		Percentage_ = p;
	}

	QStringList TodoItem::GetDeps () const
	{
		return Deps_;
	}

	void TodoItem::SetDeps (const QStringList& deps)
	{
		Deps_ = deps;
	}

	void TodoItem::AddDep (const QString& dep)
	{
		Deps_ << dep;
	}
}
}
