/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <memory>
#include <QObject>
#include <QImage>
#include <QPointer>
#include <QStringList>
#include <util/sll/util.h>
#include <interfaces/azoth/imetainfoentry.h>
#include <interfaces/azoth/ihaveavatars.h>
#include "structures.h"
#include "entrybase.h"

class QTimer;

namespace LC
{
namespace Azoth
{
namespace Murm
{
	class VkAccount;
	class VkMessage;
	class VCardDialog;
	class VkChatEntry;

	class VkEntry : public EntryBase
				  , public IMetaInfoEntry
				  , public IHaveAvatars
	{
		Q_OBJECT
		Q_INTERFACES (LC::Azoth::IMetaInfoEntry LC::Azoth::IHaveAvatars)

		UserInfo Info_;

		QTimer * const RemoteTypingTimer_;
		QTimer * const LocalTypingTimer_;

		bool IsSelf_ = false;
		bool IsNonRoster_ = false;

		QImage AppImage_;

		QPointer<VCardDialog> VCardDialog_;

		QStringList Groups_;

		QList<VkChatEntry*> Chats_;
	public:
		VkEntry (const UserInfo&, VkAccount*);

		void UpdateInfo (const UserInfo&, bool spontaneous = true);
		const UserInfo& GetInfo () const;

		void UpdateAppInfo (const AppInfo&, const QImage&);

		void Send (VkMessage*) override;

		void SetSelf ();
		void SetNonRoster ();
		bool IsNonRoster () const;

		Util::DefaultScopeGuard RegisterIn (VkChatEntry*);
		void ReemitGroups ();

		VkMessage* FindMessage (qulonglong) const;
		void HandleMessage (MessageInfo, const FullMessageInfo&);

		void HandleTypingNotification ();

		Features GetEntryFeatures () const override;
		EntryType GetEntryType () const override;
		QString GetEntryName () const override;
		void SetEntryName (const QString& name) override;
		QString GetEntryID () const override;
		QString GetHumanReadableID () const override;
		QStringList Groups () const override;
		void SetGroups (const QStringList& groups) override;
		QStringList Variants () const override;
		void SetChatPartState (ChatPartState state, const QString& variant) override;
		EntryStatus GetStatus (const QString& variant = QString ()) const override;
		void ShowInfo () override;
		QList<QAction*> GetActions () const override;
		QMap<QString, QVariant> GetClientInfo (const QString&) const override;
		void ChatTabClosed () override;

		QVariant GetMetaInfo (DataField) const override;
		QList<QPair<QString, QVariant>> GetVCardRepresentation () const override;

		QFuture<QImage> RefreshAvatar (Size) override;
		bool HasAvatar () const override;
		bool SupportsSize (Size) const override;
	private:
		void CheckPhotoChange ();
	private slots:
		void handleTypingTimeout ();
		void sendTyping ();

		void handleEntryNameFormat ();
	signals:
		void vcardUpdated () override;
		void avatarChanged (QObject*) override;
	};
}
}
}
