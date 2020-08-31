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
#include <QDir>
#include <QHash>
#include <QSet>

extern "C"
{
#include <libotr/proto.h>
#include <libotr/message.h>
}

#include <interfaces/azoth/imessage.h>
#include <interfaces/structures.h>
#include <interfaces/core/ihookproxy.h>
#include <interfaces/core/icoreproxy.h>

class QTimer;
class QMenu;
class QAction;

namespace LC
{
namespace Azoth
{
class IProxyObject;

namespace OTRoid
{
	class Authenticator;
	enum class SmpMethod;

	class OtrHandler : public QObject
	{
		Q_OBJECT

		const ICoreProxy_ptr CoreProxy_;
		IProxyObject * const AzothProxy_;

		const QDir OtrDir_;

		const OtrlUserState UserState_;
		OtrlMessageAppOps OtrOps_;

		QTimer *PollTimer_;
		struct EntryActions
		{
			std::shared_ptr<QMenu> CtxMenu_;
			std::shared_ptr<QMenu> ButtonMenu_;

			std::shared_ptr<QAction> ToggleOtr_;
			std::shared_ptr<QAction> ToggleOtrCtx_;
			std::shared_ptr<QAction> Authenticate_;
		};
		QHash<QObject*, EntryActions> Entry2Action_;

		QHash<QObject*, QString> Msg2OrigText_;

		QSet<QObject*> PendingInjectedMessages_;

		bool IsGenerating_ = false;

		QHash<ICLEntry*, Authenticator*> Auths_;
	public:
		OtrHandler (const ICoreProxy_ptr&, IProxyObject*);
		~OtrHandler ();

		void HandleMessageCreated (const IHookProxy_ptr&, IMessage*);
		void HandleGotMessage (const IHookProxy_ptr&, QObject*);
		void HandleEntryActionsRemoved (QObject*);
		void HandleEntryActionsRequested (const IHookProxy_ptr&, QObject*);
		void HandleEntryActionAreasRequested (const IHookProxy_ptr&, QObject*);

		OtrlUserState GetUserState () const;

		int IsLoggedIn (const QString& accId, const QString& entryId);
		void InjectMsg (const QString& accId, const QString& entryId,
				const QString& msg, bool hidden, IMessage::Direction,
				IMessage::Type = IMessage::Type::ChatMessage);
		void InjectMsg (ICLEntry *entry,
				const QString& msg, bool hidden, IMessage::Direction,
				IMessage::Type = IMessage::Type::ChatMessage);
		void Notify (const QString& accId, const QString& entryId,
				Priority, const QString& title,
				const QString& primary, const QString& secondary);
		QString GetAccountName (const QString& accId);
		QString GetVisibleEntryName (const QString& accId, const QString& entryId);

		void CreatePrivkey (const char*, const char*, bool confirm = true);
		void CreateInstag (const char*, const char*);

		void SetPollTimerInterval (unsigned int seconds);
		void HandleSmpEvent (OtrlSMPEvent, ConnContext*, unsigned short, const QString&);
	private:
		QByteArray GetOTRFilename (const QString&) const;

		void CreateActions (QObject*);
		void SetOtrState (ICLEntry*, bool);

		void CreateAuthForEntry (ICLEntry*);
		void HandleAuthRequested (ICLEntry*);

		void StartAuth (ICLEntry*, SmpMethod, const QString&, const QString&);
	public slots:
		void writeFingerprints ();
		void writeKeys ();

		void generateKeys (const QString&, const QString&);
	signals:
		void privKeysChanged ();
	};
}
}
}
