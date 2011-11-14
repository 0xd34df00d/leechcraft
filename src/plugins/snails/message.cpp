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
#include <stdexcept>
#include <QtDebug>

namespace LeechCraft
{
namespace Snails
{
	Message::Message (QObject *parent)
	: QObject (parent)
	, IsRead_ (false)
	{
	}

	bool Message::IsFullyFetched () const
	{
		return !Body_.isEmpty () || !HTMLBody_.isEmpty ();
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

	QList<QPair<QString, QString>> Message::GetTo () const
	{
		return To_;
	}

	void Message::SetTo (const QList<QPair<QString, QString>>& to)
	{
		To_ = to;
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

	QString Message::GetBody () const
	{
		return Body_;
	}

	void Message::SetBody (const QString& body)
	{
		Body_ = body;
	}

	QString Message::GetHTMLBody () const
	{
		return HTMLBody_;
	}

	void Message::SetHTMLBody (const QString& body)
	{
		HTMLBody_ = body;
	}

	bool Message::IsRead () const
	{
		return IsRead_;
	}

	void Message::SetRead (bool read)
	{
		const bool shouldEmit = read != IsRead_;
		IsRead_ = read;

		if (shouldEmit)
			emit readStatusChanged (GetID (), read);
	}

	void Message::Dump () const
	{
		qDebug () << Q_FUNC_INFO
				<< ID_.toHex ()
				<< Size_
				<< From_
				<< FromEmail_
				<< To_
				<< Date_
				<< Recipients_
				<< Subject_
				<< IsRead_
				<< Body_
				<< HTMLBody_;
	}

	QByteArray Message::Serialize () const
	{
		QByteArray result;

		QDataStream str (&result, QIODevice::WriteOnly);
		str << static_cast<quint8> (1)
			<< ID_
			<< Size_
			<< From_
			<< FromEmail_
			<< To_
			<< Date_
			<< Recipients_
			<< Subject_
			<< IsRead_
			<< Body_
			<< HTMLBody_;

		return result;
	}

	void Message::Deserialize (const QByteArray& data)
	{
		QDataStream str (data);
		quint8 version = 0;
		str >> version;
		if (version != 1)
			throw std::runtime_error (qPrintable ("Failed to deserialize Message: unknown version " + QString::number (version)));

		str >> ID_
			>> Size_
			>> From_
			>> FromEmail_
			>> To_
			>> Date_
			>> Recipients_
			>> Subject_
			>> IsRead_
			>> Body_
			>> HTMLBody_;
	}
}
}
