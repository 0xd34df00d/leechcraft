/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2013  Georg Rudoy
 *
 * Boost Software License - Version 1.0 - August 17th, 2003
 *
 * Permission is hereby granted, free of charge, to any person or organization
 * obtaining a copy of the software and accompanying documentation covered by
 * this license (the "Software") to use, reproduce, display, distribute,
 * execute, and transmit the Software, and to prepare derivative works of the
 * Software, and to permit third-parties to whom the Software is furnished to
 * do so, all subject to the following:
 *
 * The copyright notices in the Software and this entire statement, including
 * the above license grant, this restriction and the following disclaimer,
 * must be included in all copies of the Software, in whole or in part, and
 * all derivative works of the Software, unless such copies or derivative
 * works are solely in the form of machine-executable object code generated by
 * a source language processor.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
 * SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
 * FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
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
