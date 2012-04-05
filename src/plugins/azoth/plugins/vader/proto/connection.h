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

#include <functional>
#include <QObject>
#include <QMap>
#include <QAbstractSocket>
#include <QStringList>
#include <interfaces/azoth/iclentry.h>
#include "packetfactory.h"
#include "packetextractor.h"
#include "contactinfo.h"
#include "balancer.h"

class QTimer;
class QSslSocket;

namespace LeechCraft
{
namespace Azoth
{
namespace Vader
{
namespace Proto
{
	struct Message;
	class TypingManager;

	class Connection : public QObject
	{
		Q_OBJECT

		Balancer Balancer_;

		QSslSocket *Socket_;
		QTimer *PingTimer_;
		TypingManager *TM_;

		PacketFactory PF_;
		PacketExtractor PE_;
		QMap<quint16, std::function<void (HalfPacket)>> PacketActors_;
		QHash<quint32, QString> RequestedInfos_;

		QString Host_;
		int Port_;

		QString Login_;
		QString Pass_;
		QString UA_;

		bool IsConnected_;

		EntryStatus PendingStatus_;
	public:
		Connection (QObject* = 0);

		void SetTarget (const QString&, int);
		void SetCredentials (const QString&, const QString&);
		void SetUA (const QString&);

		bool IsConnected () const;
		void Connect ();

		void SetState (const EntryStatus&);
		void RequestInfo (const QString&);
		quint32 SendMessage (const QString& to, const QString& message);
		quint32 SendSMS (const QString& to, const QString& message);
		quint32 SendSMS2Number (const QString& phone, const QString& message);
		void SendAttention (const QString& to, const QString& message);
		void SetTypingState (const QString& to, bool isTyping);
		void PublishTune (const QString& tune);
		void Authorize (const QString& email);
		quint32 AddContact (quint32 group, const QString& email, const QString& name);
		void ModifyContact (quint32 contactId, quint32 groupId,
				const QString& email, const QString& name);
		void RemoveContact (quint32 id, const QString& email, const QString& name);
		void RequestAuth (const QString& email, const QString& msg);
		quint32 AddGroup (const QString& group, int groupNum);
		void RequestPOPKey ();
	private:
		void HandleHello (HalfPacket);
		void Login ();
		void CorrectAuth (HalfPacket);
		void IncorrectAuth (HalfPacket);
		void ConnParams (HalfPacket);

		void UserInfo (HalfPacket);
		void UserStatus (HalfPacket);
		void ContactList (HalfPacket);

		void HandleWPInfo (HalfPacket, const QString&);

		void IncomingMsg (HalfPacket);
		void MsgStatus (HalfPacket);
		void SMSAck (HalfPacket);
		void OfflineMsg (HalfPacket);
		void MicroblogRecv (HalfPacket);

		void AuthAck (HalfPacket);
		void ContactAdded (HalfPacket);

		void NewMail (HalfPacket);
		void MPOPKey (HalfPacket);

		void Disconnect ();

		QByteArray Read ();
		void Write (const QByteArray&);
	private slots:
		void handleGotServer (const QString&, int);
		void connectToStored ();
		void tryRead ();
		void greet ();
		void handlePing ();
		void handleOutTypingNotify (const QString&);
		void handleSocketError (QAbstractSocket::SocketError);
	signals:
		void authenticationError (const QString&);

		void gotGroups (const QStringList&);
		void gotContacts (const QList<Proto::ContactInfo>&);

		void gotUserInfoError (const QString& email, Proto::AnketaInfoStatus);
		void gotUserInfoResult (const QString& email, const QMap<QString, QString>& vals);

		void gotMessage (const Proto::Message&);
		void gotOfflineMessage (const Proto::Message&);

		void gotAuthRequest (const QString& from, const QString& msg);
		void gotAuthAck (const QString& from);

		void gotAttentionRequest (const QString& from, const QString& msg);
		void messageDelivered (quint32);
		void smsDelivered (quint32);
		void smsServiceUnavailable (quint32);
		void smsBadParms (quint32);

		void statusChanged (EntryStatus);
		void contactAdded (quint32 seq, quint32 cid);
		void contactAdditionError (quint32 seq, quint32 status);
		void userStatusChanged (const Proto::ContactInfo&);
		void gotUserTune (const QString& from, const QString& tune);

		void userStartedTyping (const QString&);
		void userStoppedTyping (const QString&);

		void gotNewMail (const QString& from, const QString& subj);
		void gotPOPKey (const QString&);
	};
}
}
}
}
