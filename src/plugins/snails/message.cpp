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

#include "message.h"
#include <stdexcept>
#include <QtDebug>

namespace LeechCraft
{
namespace Snails
{
	uint qHash (Message::Address a)
	{
		return static_cast<uint> (a);
	}

	QDataStream& operator<< (QDataStream& out, Message::Address a)
	{
		out << static_cast<quint16> (a);
		return out;
	}

	QDataStream& operator>> (QDataStream& in, Message::Address& a)
	{
		quint16 t = 0;
		in >> t;
		a = static_cast<Message::Address> (t);
		return in;
	}

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

	QList<QStringList> Message::GetFolders () const
	{
		return Folders_;
	}

	void Message::AddFolder (const QStringList& folder)
	{
		Folders_ << folder;
	}

	void Message::SetFolders (const QList<QStringList>& folders)
	{
		Folders_ = folders;
	}

	quint64 Message::GetSize () const
	{
		return Size_;
	}

	void Message::SetSize (quint64 size)
	{
		Size_ = size;
	}

	Message::Address_t Message::GetAddress (Message::Address a) const
	{
		return GetAddresses (a).value (0);
	}

	Message::Addresses_t Message::GetAddresses (Message::Address a) const
	{
		return Addresses_ [a];
	}

	void Message::AddAddress (Message::Address a, const Message::Address_t& pair)
	{
		Addresses_ [a] << pair;
	}

	void Message::SetAddress (Message::Address a, const Message::Address_t& pair)
	{
		SetAddresses (a, { pair });
	}

	void Message::SetAddresses (Message::Address a, const Message::Addresses_t& list)
	{
		Addresses_ [a] = list;
	}

	QDateTime Message::GetDate () const
	{
		return Date_;
	}

	void Message::SetDate (const QDateTime& date)
	{
		Date_ = date;
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

	QList<AttDescr> Message::GetAttachments () const
	{
		return Attachments_;
	}

	void Message::AddAttachment (const AttDescr& att)
	{
		Attachments_ << att;
	}

	void Message::SetAttachmentList (const QList<AttDescr>& list)
	{
		Attachments_ = list;
	}

	void Message::Dump () const
	{
		qDebug () << Q_FUNC_INFO
				<< ID_.toHex ()
				<< Folders_
				<< Size_
				<< Date_
				<< Recipients_
				<< Subject_
				<< IsRead_
				<< Body_
				<< HTMLBody_;
		Q_FOREACH (const auto key, Addresses_.keys ())
			qDebug () << static_cast<int> (key)
					<< Addresses_ [key];
		qDebug () << Attachments_.size () << "attachments";
		Q_FOREACH (const auto& att, Attachments_)
			att.Dump ();
	}

	QByteArray Message::Serialize () const
	{
		QByteArray result;

		QDataStream str (&result, QIODevice::WriteOnly);
		str << static_cast<quint8> (1)
			<< ID_
			<< Folders_
			<< Size_
			<< Date_
			<< Recipients_
			<< Subject_
			<< IsRead_
			<< Body_
			<< HTMLBody_
			<< Addresses_
			<< Attachments_;

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
			>> Folders_
			>> Size_
			>> Date_
			>> Recipients_
			>> Subject_
			>> IsRead_
			>> Body_
			>> HTMLBody_
			>> Addresses_
			>> Attachments_;
	}

	QString GetNiceMail (const Message::Address_t& pair)
	{
		const QString& fromName = pair.first;
		return fromName.isEmpty () ?
				pair.second :
				fromName + " <" + pair.second + ">";
	}
}
}

uint qHash (const LeechCraft::Snails::Message_ptr msg)
{
	return qHash (msg->GetID ());
}
