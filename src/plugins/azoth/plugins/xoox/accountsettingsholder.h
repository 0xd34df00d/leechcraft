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

#include <QObject>
#include <QString>
#include <QPair>
#include <QXmppTransferManager.h>

namespace LeechCraft
{
namespace Azoth
{
namespace Xoox
{
	class GlooxAccount;
	class GlooxAccountConfigurationWidget;

	class AccountSettingsHolder : public QObject
	{
		Q_OBJECT

		bool ReconnectScheduled_;

		GlooxAccount *Account_;

		QString JID_;
		QString Nick_;
		QString Resource_;
		QString Host_;
		int Port_;

		QByteArray OurPhotoHash_;

		QPair<int, int> KAParams_;
		bool FileLogEnabled_;

		int Priority_;

		QXmppTransferJob::Methods FTMethods_;
		bool UseSOCKS5Proxy_;
		QString SOCKS5Proxy_;
	public:
		AccountSettingsHolder (GlooxAccount* = 0);

		void Serialize (QDataStream&) const;
		void Deserialize (QDataStream&, quint16);

		void OpenConfigDialog ();
		void FillSettings (GlooxAccountConfigurationWidget*);

		QString GetJID () const;
		void SetJID (const QString&);

		QString GetNick () const;
		void SetNick (const QString&);

		QString GetResource () const;
		void SetResource (const QString&);

		QString GetFullJID () const;

		QString GetHost () const;
		void SetHost (const QString&);

		int GetPort () const;
		void SetPort (int);

		QByteArray GetPhotoHash () const;
		void SetPhotoHash (const QByteArray&);

		QPair<int, int> GetKAParams () const;
		void SetKAParams (const QPair<int, int>&);

		bool GetFileLogEnabled () const;
		void SetFileLogEnabled (bool);

		int GetPriority () const;
		void SetPriority (int);

		QXmppTransferJob::Methods GetFTMethods () const;
		void SetFTMethods (QXmppTransferJob::Methods);

		bool GetUseSOCKS5Proxy () const;
		void SetUseSOCKS5Proxy (bool);

		QString GetSOCKS5Proxy () const;
		void SetSOCKS5Proxy (const QString&);
	private slots:
		void scheduleReconnect ();
		void handleReconnect ();
	signals:
		void jidChanged (const QString&);
		void resourceChanged (const QString&);
		void nickChanged (const QString&);
		void hostChanged (const QString&);
		void portChanged (int);
		void photoHashChanged (const QByteArray&);
		void kaParamsChanged (const QPair<int, int>&);
		void fileLogChanged (bool);
		void priorityChanged (int);

		void fileTransferSettingsChanged ();

		void accountSettingsChanged ();
	};
}
}
}
