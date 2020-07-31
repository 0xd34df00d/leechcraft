/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <QString>
#include <QPair>
#include <QXmppTransferManager.h>
#include <QXmppConfiguration.h>

namespace LC
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

		QXmppConfiguration::StreamSecurityMode TLSMode_;

		QXmppTransferJob::Methods FTMethods_;
		bool UseSOCKS5Proxy_;
		QString SOCKS5Proxy_;

		QString StunHost_;
		int StunPort_;
		QString TurnHost_;
		int TurnPort_;
		QString TurnUser_;
		QString TurnPass_;

		bool EnableMessageCarbons_ = true;
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

		QXmppConfiguration::StreamSecurityMode GetTLSMode () const;
		void SetTLSMode (QXmppConfiguration::StreamSecurityMode);

		bool GetUseSOCKS5Proxy () const;
		void SetUseSOCKS5Proxy (bool);

		QString GetSOCKS5Proxy () const;
		void SetSOCKS5Proxy (const QString&);

		QString GetStunHost () const;
		int GetStunPort () const;
		void SetStunParams (const QString& host, int port);

		QString GetTurnHost () const;
		int GetTurnPort () const;
		QString GetTurnUser () const;
		QString GetTurnPass () const;
		void SetTurnParams (const QString& host, int port, const QString& user, const QString& pass);

		bool IsMessageCarbonsEnabled () const;
		void SetMessageCarbonsEnabled (bool);
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
		void tlsModeChanged (QXmppConfiguration::StreamSecurityMode);
		void messageCarbonsSettingsChanged ();

		void fileTransferSettingsChanged ();

		void stunSettingsChanged ();
		void turnSettingsChanged ();

		void accountSettingsChanged ();
	};
}
}
}
