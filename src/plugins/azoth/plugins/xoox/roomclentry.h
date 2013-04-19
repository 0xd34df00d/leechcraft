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

#ifndef PLUGINS_AZOTH_PLUGINS_XOOX_ROOMCLENTRY_H
#define PLUGINS_AZOTH_PLUGINS_XOOX_ROOMCLENTRY_H
#include <memory>
#include <QObject>
#include <QStringList>
#include <QXmppMucIq.h>
#include <interfaces/azoth/iclentry.h>
#include <interfaces/azoth/imucentry.h>
#include <interfaces/azoth/imucperms.h>
#include <interfaces/azoth/iconfigurablemuc.h>

class QXmppBookmarkSet;

namespace LeechCraft
{
namespace Azoth
{
namespace Xoox
{
	class GlooxAccount;
	class RoomPublicMessage;
	class RoomHandler;

	class RoomCLEntry : public QObject
					  , public ICLEntry
					  , public IMUCEntry
					  , public IMUCPerms
					  , public IConfigurableMUC
	{
		Q_OBJECT
		Q_INTERFACES (LeechCraft::Azoth::ICLEntry
						LeechCraft::Azoth::IMUCEntry
						LeechCraft::Azoth::IMUCPerms
						LeechCraft::Azoth::IConfigurableMUC)

		friend class RoomHandler;

		const bool IsAutojoined_;
		GlooxAccount *Account_;
		QList<QObject*> AllMessages_;
		RoomHandler *RH_;
		QMap<QByteArray, QList<QByteArray>> Perms_;
		QMap<QXmppMucItem::Role, QByteArray> Role2Str_;
		QMap<QXmppMucItem::Affiliation, QByteArray> Aff2Str_;
		QMap<QByteArray, QString> Translations_;

		mutable QAction *ActionRequestVoice_;
	public:
		RoomCLEntry (RoomHandler*, bool, GlooxAccount*);

		RoomHandler* GetRoomHandler () const;

		// ICLEntry
		QObject* GetQObject ();
		QObject* GetParentAccount () const ;
		Features GetEntryFeatures () const;
		EntryType GetEntryType () const;
		QString GetEntryName () const;
		void SetEntryName (const QString&);
		QString GetEntryID () const;
		QString GetHumanReadableID () const;
		QStringList Groups () const;
		void SetGroups (const QStringList&);
		QStringList Variants () const;
		QObject* CreateMessage (IMessage::MessageType,
				const QString&, const QString&);
		QList<QObject*> GetAllMessages () const;
		void PurgeMessages (const QDateTime&);
		void SetChatPartState (ChatPartState, const QString&);
		EntryStatus GetStatus (const QString&) const;
		QList<QAction*> GetActions () const;
		QImage GetAvatar () const;
		QString GetRawInfo () const;
		void ShowInfo ();
		QMap<QString, QVariant> GetClientInfo (const QString&) const;
		void MarkMsgsRead ();

		// IMUCEntry
		MUCFeatures GetMUCFeatures () const;
		QString GetMUCSubject () const;
		void SetMUCSubject (const QString&);
		QList<QObject*> GetParticipants ();
		bool IsAutojoined () const;
		void Join ();
		void Leave (const QString&);
		QString GetNick () const;
		void SetNick (const QString&);
		QString GetGroupName () const;
		QVariantMap GetIdentifyingData () const;
		QString GetRealID (QObject*) const;
		void InviteToMUC (const QString&, const QString&);

		// IMUCPerms
		QMap<QByteArray, QList<QByteArray>> GetPossiblePerms () const;
		QMap<QByteArray, QList<QByteArray>> GetPerms (QObject *object) const;
		QPair<QByteArray, QByteArray> GetKickPerm () const;
		QPair<QByteArray, QByteArray> GetBanPerm () const;
		QByteArray GetAffName (QObject*) const;
		bool MayChangePerm (QObject*, const QByteArray&, const QByteArray&) const;
		void SetPerm (QObject*, const QByteArray&, const QByteArray&, const QString&);
		bool IsLessByPerm (QObject*, QObject*) const;
		bool IsMultiPerm (const QByteArray&) const;
		QString GetUserString (const QByteArray&) const;

		// IConfigurableMUC
		QWidget* GetConfigurationWidget ();
		void AcceptConfiguration (QWidget*);

		void HandleMessage (RoomPublicMessage*);
		void HandleNewParticipants (const QList<ICLEntry*>&);
		void HandleSubjectChanged (const QString&);
	private slots:
		void handleBookmarks (const QXmppBookmarkSet&);
		void reemitStatusChange (const EntryStatus&);
	signals:
		void gotMessage (QObject*);
		void statusChanged (const EntryStatus&, const QString&);
		void availableVariantsChanged (const QStringList&);
		void avatarChanged (const QImage&);
		void rawinfoChanged (const QString&);
		void nameChanged (const QString&);
		void groupsChanged (const QStringList&);
		void chatPartStateChanged (const ChatPartState&, const QString&);
		void permsChanged ();
		void entryGenerallyChanged ();

		void gotNewParticipants (const QList<QObject*>&);
		void mucSubjectChanged (const QString&);
		void nicknameConflict (const QString&);
		void beenKicked (const QString&);
		void beenBanned (const QString&);
	};
}
}
}

#endif
