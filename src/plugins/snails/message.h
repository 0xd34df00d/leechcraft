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

#ifndef PLUGINS_SNAILS_MESSAGE_H
#define PLUGINS_SNAILS_MESSAGE_H
#include <memory>
#include <QObject>
#include <QDateTime>
#include <QStringList>
#include <QMetaType>

namespace LeechCraft
{
namespace Snails
{
	class Message : public QObject
	{
		Q_OBJECT

		QByteArray ID_;
		quint64 Size_;
		QString From_;
		QString FromEmail_;
		QDateTime Date_;
		QStringList Recipients_;
		QString Subject_;

		QString Body_;
		QString HTMLBody_;
	public:
		Message (QObject* = 0);

		bool IsFullyFetched () const;

		QByteArray GetID () const;
		void SetID (const QByteArray&);

		quint64 GetSize () const;
		void SetSize (quint64);

		QString GetFrom () const;
		void SetFrom (const QString&);

		QString GetFromEmail () const;
		void SetFromEmail (const QString&);

		QDateTime GetDate () const;
		void SetDate (const QDateTime&);

		QStringList GetRecipients () const;
		void SetRecipients (const QStringList&);

		QString GetSubject () const;
		void SetSubject (const QString&);

		QString GetBody () const;
		void SetBody (const QString&);

		QString GetHTMLBody () const;
		void SetHTMLBody (const QString&);

		void Dump () const;

		QByteArray Serialize () const;
		void Deserialize (const QByteArray&);
	};

	typedef std::shared_ptr<Message> Message_ptr;
}
}

Q_DECLARE_METATYPE (LeechCraft::Snails::Message_ptr);
Q_DECLARE_METATYPE (QList<LeechCraft::Snails::Message_ptr>);

#endif
