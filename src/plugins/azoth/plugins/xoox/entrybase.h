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

#ifndef PLUGINS_AZOTH_PLUGINS_XOOX_ENTRYBASE_H
#define PLUGINS_AZOTH_PLUGINS_XOOX_ENTRYBASE_H
#include <QObject>
#include <QImage>
#include <QMap>
#include <QVariant>
#include <QXmppMessage.h>
#include <QXmppVCardIq.h>
#include <interfaces/iclentry.h>
#include <interfaces/iadvancedclentry.h>

class QXmppPresence;

namespace LeechCraft
{
namespace Azoth
{
namespace Xoox
{
	class GlooxMessage;
	class VCardDialog;
	class GlooxAccount;

	/** Common base class for GlooxCLEntry, which reprensents usual
	 * entries in the contact list, and RoomCLEntry, which represents
	 * participants in MUCs.
	 *
	 * This class tries to unify and provide a common implementation of
	 * what those classes, well, have in common.
	 */
	class EntryBase : public QObject
					, public ICLEntry
					, public IAdvancedCLEntry
	{
		Q_OBJECT
		Q_INTERFACES (LeechCraft::Azoth::ICLEntry LeechCraft::Azoth::IAdvancedCLEntry)
	protected:
		QList<QObject*> AllMessages_;
		QMap<QString, EntryStatus> CurrentStatus_;
		QList<QAction*> Actions_;

		QImage Avatar_;
		QString RawInfo_;
		GlooxAccount *Account_;
		QXmppVCardIq VCardIq_;
		QPointer<VCardDialog> VCardDialog_;

		QMap<QString, QMap<QString, QVariant> > Variant2ClientInfo_;
		QMap<QString, QByteArray> Variant2VerString_;
	public:
		EntryBase (GlooxAccount* = 0);

		// ICLEntry
		QObject* GetObject ();
		QList<QObject*> GetAllMessages () const;
		void PurgeMessages (const QDateTime&);
		void SetChatPartState (ChatPartState, const QString&);
		EntryStatus GetStatus (const QString&) const;
		QList<QAction*> GetActions () const;
		QImage GetAvatar () const;
		QString GetRawInfo () const;
		void ShowInfo ();
		QMap<QString, QVariant> GetClientInfo (const QString&) const;
		
		// IAdvancedCLEntry
		AdvancedFeatures GetAdvancedFeatures () const;
		void DrawAttention (const QString&, const QString&);

		virtual QString GetJID () const = 0;

		void HandleMessage (GlooxMessage*);
		void HandleAttentionMessage (const QXmppMessage&);
		void UpdateChatState (QXmppMessage::State, const QString&);
		void SetStatus (const EntryStatus&, const QString&);
		void SetAvatar (const QByteArray&);
		void SetAvatar (const QImage&);
		QXmppVCardIq GetVCard () const;
		void SetVCard (const QXmppVCardIq&);
		void SetRawInfo (const QString&);

		void SetClientInfo (const QString&, const QString&, const QByteArray&);
		void SetClientInfo (const QString&, const QXmppPresence&);
		
		QByteArray GetVariantVerString (const QString&) const;
	private:
		QString FormatRawInfo (const QXmppVCardIq&);
	signals:
		void gotMessage (QObject*);
		void statusChanged (const EntryStatus&, const QString&);
		void avatarChanged (const QImage&);
		void rawinfoChanged (const QString&);
		void availableVariantsChanged (const QStringList&);
		void nameChanged (const QString&);
		void groupsChanged (const QStringList&);
		void chatPartStateChanged (const ChatPartState&, const QString&);
		void permsChanged ();
		
		void attentionDrawn (const QString&, const QString&);
	};
}
}
}

#endif
