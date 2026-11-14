/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <memory>
#include <variant>
#include <QHash>
#include <QDir>
#include <QObject>
#include <QSet>
#include <util/sll/trackingpointer.h>
#include <util/threads/coro/task.h>
#include <util/threads/coro/corocontext.h>
#include "interfaces/azoth/iclentry.h"

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
	public:
		enum class KeyGenResult : std::uint8_t
		{
			Rejected,
			Created,
			Failed,
		};

		enum class OtrState : bool
		{
			Disabled,
			QuerySent,
		};
	private:
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

		QHash<ICLEntry*, Authenticator*> Auths_;

		QHash<AccountId, Util::SharedContextTask<KeyGenResult>> PrivkeyCreationTasks_;

		QSet<AccountId> PrivkeysRequested_;
	public:
		OtrHandler (const ICoreProxy_ptr&, IProxyObject*);
		~OtrHandler ();

		void HandleMessageCreated (bool& cancel, ICLEntry& other, OutgoingMessage& msg);
		void HandleGotMessage (const IHookProxy_ptr&, QObject*);
		void HandleEntryActionsRemoved (QObject*);
		void HandleEntryActionsRequested (const IHookProxy_ptr&, QObject*);
		void HandleEntryActionAreasRequested (const IHookProxy_ptr&, QObject*);

		OtrlUserState GetUserState () const;

		int IsLoggedIn (const AccountId& accId, const EntryStrongestId& entryId);

		void NotifyConvFlow (const ConnContext*, const QString& msg);
		void NotifyConvFlow (const AccountId& accId, const EntryStrongestId& entryId, const QString& msg);
		void NotifyConvFlow (ICLEntry& entry, const QString& msg);

		void Notify (Priority, const QString& title, const QString& primary, const QString& secondary);

		QString GetAccountName (const AccountId& accId);
		ICLEntry* GetEntry (const AccountId& accId, const EntryStrongestId& entryId);
		QString GetVisibleEntryName (const AccountId& accId, const EntryStrongestId& entryId);

		Util::SharedContextTask<KeyGenResult> CreatePrivkey (const AccountId&, const QString&, bool confirm = true);
		void CreateInstag (const char*, const char*);

		void SetPollTimerInterval (unsigned int seconds);
		void HandleSmpEvent (OtrlSMPEvent, ConnContext*, unsigned short, const QString&);
	private:
		Util::SharedContextTask<KeyGenResult> RunCreatePrivkey (AccountId, QString, bool);

		QByteArray GetOTRFilename (const QString&) const;

		void CreateActions (QObject*);
		Util::ContextTask<OtrState> SetOtrState (Util::TrackingPointer<ICLEntry>, bool);
		Util::ContextTask<void> ReplayWithPrivkey (Util::TrackingPointer<ICLEntry>, QString);

		struct ReceivingIgnore {};
		struct ReceivingNonOtr {};
		struct ReceivingOtrEnd { std::optional<QString> PlaintextMsg_; };
		struct ReceivingDecrypted { QString Body_; };
		using ReceivingResult_t = std::variant<ReceivingIgnore, ReceivingNonOtr, ReceivingOtrEnd, ReceivingDecrypted>;

		ReceivingResult_t RunOtrReceiving (ICLEntry&, const QString&);

		void SetOtrActionsState (ICLEntry&, bool);

		void CreateAuthForEntry (ICLEntry*);
		Util::ContextTask<void> HandleAuthRequested (Util::TrackingPointer<ICLEntry>);

		void StartAuth (ICLEntry*, SmpMethod, const QString&, const QString&);
	public slots:
		void writeFingerprints ();
		void writeKeys ();

		void generateKeys (const AccountId&, const QString&);
	signals:
		void privKeysChanged ();

		LC_CORO_CONTEXT
	};
}
}
}
