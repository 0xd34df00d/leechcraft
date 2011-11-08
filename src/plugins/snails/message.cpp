/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2011  Georg Rudoy
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

#include "message.h"
#include <QtDebug>

namespace LeechCraft
{
namespace Snails
{
	Message::Message (QObject *parent)
	: QObject (parent)
	{
	}

	QByteArray Message::GetID () const
	{
		return ID_;
	}

	void Message::SetID (const QByteArray& id)
	{
		ID_ = id;
	}

	quint64 Message::GetSize () const
	{
		return Size_;
	}

	void Message::SetSize (quint64 size)
	{
		Size_ = size;
	}

	QString Message::GetFrom () const
	{
		return From_;
	}

	void Message::SetFrom (const QString& from)
	{
		From_ = from;
	}

	QString Message::GetFromEmail () const
	{
		return FromEmail_;
	}

	void Message::SetFromEmail (const QString& fe)
	{
		FromEmail_ = fe;
	}

	QDateTime Message::GetDate () const
	{
		return Date_;
	}

	void Message::SetDate (const QDateTime& date)
	{
		Date_ = date;
	}

	QStringList Message::GetRecipients () const
	{
		return Recipients_;
	}

	void Message::SetRecipients (const QStringList& recips)
	{
		Recipients_ = recips;
	}

	QString Message::GetSubject () const
	{
		return Subject_;
	}

	void Message::SetSubject (const QString& subj)
	{
		Subject_ = subj;
	}

	void Message::Dump () const
	{
		qDebug () << Q_FUNC_INFO
				<< ID_.toBase64 ()
				<< From_
				<< FromEmail_
				<< Date_
				<< Recipients_
				<< Subject_;
	}
}
}
