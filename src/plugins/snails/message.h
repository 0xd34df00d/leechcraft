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

#include <memory>
#include <QObject>
#include <QDateTime>
#include <QStringList>
#include <QMetaType>
#include <QPair>
#include <QHash>
#include <QSet>
#include "attdescr.h"

namespace LeechCraft
{
namespace Snails
{
	class Message : public QObject
	{
		Q_OBJECT

		QByteArray ID_;
		QList<QStringList> Folders_;
		quint64 Size_;
		QDateTime Date_;
		QStringList Recipients_;
		QString Subject_;

		QString Body_;
		QString HTMLBody_;

		bool IsRead_;

		QList<AttDescr> Attachments_;
	public:
		enum class Address
		{
			To,
			Cc,
			Bcc,
			From,
			ReplyTo
		};

		typedef QPair<QString, QString> Address_t;
		typedef QList<Address_t> Addresses_t;
	private:
		QHash<Address, Addresses_t> Addresses_;
	public:
		Message (QObject* = 0);

		bool IsFullyFetched () const;

		QByteArray GetID () const;
		void SetID (const QByteArray&);

		QList<QStringList> GetFolders () const;
		void AddFolder (const QStringList&);
		void SetFolders (const QList<QStringList>&);

		quint64 GetSize () const;
		void SetSize (quint64);

		Address_t GetAddress (Address) const;
		Addresses_t GetAddresses (Address) const;
		void AddAddress (Address, const Address_t&);
		void SetAddress (Address, const Address_t&);
		void SetAddresses (Address, const Addresses_t&);

		QDateTime GetDate () const;
		void SetDate (const QDateTime&);

		QString GetSubject () const;
		void SetSubject (const QString&);

		QString GetBody () const;
		void SetBody (const QString&);

		QString GetHTMLBody () const;
		void SetHTMLBody (const QString&);

		bool IsRead () const;
		void SetRead (bool);

		QList<AttDescr> GetAttachments () const;
		void AddAttachment (const AttDescr&);
		void SetAttachmentList (const QList<AttDescr>&);

		void Dump () const;

		QByteArray Serialize () const;
		void Deserialize (const QByteArray&);
	signals:
		void readStatusChanged (const QByteArray&, bool);
	};

	typedef std::shared_ptr<Message> Message_ptr;
	typedef QSet<Message_ptr> MessageSet;

	QString GetNiceMail (const Message::Address_t&);
}
}

uint qHash (const LeechCraft::Snails::Message_ptr);

Q_DECLARE_METATYPE (LeechCraft::Snails::Message_ptr);
Q_DECLARE_METATYPE (QList<LeechCraft::Snails::Message_ptr>);
