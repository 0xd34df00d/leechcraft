/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "chathistory.h"
#include <QIcon>
#include <QAction>
#include <QTranslator>
#include <util/util.h>
#include <util/sll/visitor.h>
#include <xmlsettingsdialog/xmlsettingsdialog.h>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/irootwindowsmanager.h>
#include <interfaces/core/iiconthememanager.h>
#include <interfaces/azoth/imessage.h>
#include <interfaces/azoth/iclentry.h>
#include <interfaces/azoth/iaccount.h>
#include <interfaces/azoth/azothcommon.h>
#include <interfaces/azoth/imucentry.h>
#include <interfaces/azoth/iproxyobject.h>
#include "chathistorywidget.h"
#include "historymessage.h"
#include "loggingstatekeeper.h"
#include "storage2.h"
#include "xmlsettingsmanager.h"

namespace LC::Azoth::ChatHistory
{
	Plugin::Plugin () = default;

	Plugin::~Plugin () = default;

	void Plugin::Init (ICoreProxy_ptr)
	{
		TabClass_.TabClass_ = "Chathistory";
		TabClass_.VisibleName_ = tr ("Chat history");
		TabClass_.Description_ = tr ("Chat history viewer for the Azoth IM");
		TabClass_.Priority_ = 40;
		TabClass_.Features_ = TFOpenableByRequest;
		TabClass_.Icon_ = GetProxyHolder ()->GetIconThemeManager ()->GetPluginIcon ();

		XSD_ = std::make_shared<Util::XmlSettingsDialog> ();
		XSD_->RegisterObject (&XmlSettingsManager::Instance (), "azothchathistorysettings.xml");

		LoggingStateKeeper_ = std::make_unique<LoggingStateKeeper> ();
		StorageThread_ = std::make_unique<StorageThread> ();

		ActionHistory_ = new QAction (tr ("IM history"), this);
		connect (ActionHistory_,
				&QAction::triggered,
				this,
				&Plugin::HandleHistoryRequested);

		SeparatorAction_ = Util::CreateSeparator (this);
		SeparatorAction_->property ("Azoth/ChatHistory/IsGood").toBool ();
	}

	void Plugin::SecondInit ()
	{
	}

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.Azoth.ChatHistory";
	}

	void Plugin::Release ()
	{
	}

	QString Plugin::GetName () const
	{
		return "Azoth ChatHistory";
	}

	QString Plugin::GetInfo () const
	{
		return tr ("Stores message history in Azoth.");
	}

	QIcon Plugin::GetIcon () const
	{
		return TabClass_.Icon_;
	}

	QSet<QByteArray> Plugin::GetPluginClasses () const
	{
		QSet<QByteArray> result;
		result << "org.LeechCraft.Plugins.Azoth.Plugins.IGeneralPlugin";
		return result;
	}

	QList<QAction*> Plugin::GetActions (ActionsEmbedPlace) const
	{
		return QList<QAction*> ();
	}

	QMap<QString, QList<QAction*>> Plugin::GetMenuActions () const
	{
		QMap<QString, QList<QAction*>> result;
		result ["Azoth"] << ActionHistory_;
		return result;
	}

	TabClasses_t Plugin::GetTabClasses () const
	{
		return { TabClass_ };
	}

	void Plugin::TabOpenRequested (const QByteArray& tabClass)
	{
		if (tabClass == "Chathistory")
			HandleHistoryRequested ();
		else
			qWarning () << Q_FUNC_INFO
					<< "unknown tab class"
					<< tabClass;
	}

	Util::XmlSettingsDialog_ptr Plugin::GetSettingsDialog () const
	{
		return XSD_;
	}

	bool Plugin::IsHistoryEnabledFor (ICLEntry& entry) const
	{
		return LoggingStateKeeper_->IsLoggingEnabled (&entry);
	}

	namespace
	{
		QHash<QString, ICLEntry*> BuildNick2Participant (const QList<ICLEntry*>& parts)
		{
			QHash<QString, ICLEntry*> result;
			result.reserve (parts.size ());
			for (const auto& part : parts)
				result [part->GetEntryName ()] = part;
			return result;
		}
	}

	Util::ContextTask<void> Plugin::RequestLastMessages (ICLEntry& entry, int count)
	{
		const auto account = entry.GetParentAccount ();
		const auto accId = account->GetAccountID ();
		const auto entryHRId = entry.GetHumanReadableID ();

		co_await Util::AddContextObject { *this };
		co_await Util::AddContextObject { *entry.GetQObject () };
		const auto messages = co_await StorageThread_->Run (&Storage2::GetLastMessages, accId, entryHRId, count);

		const auto mucEntry = qobject_cast<IMUCEntry*> (entry.GetQObject ());
		const auto& nick2part = BuildNick2Participant (mucEntry ? mucEntry->GetParticipants () : QList<ICLEntry*> {});

		QList<QObject*> logs;
		for (const auto& message : messages)
		{
			const auto otherPart = [&]
			{
				switch (entry.GetEntryType ())
				{
				case ICLEntry::EntryType::Chat:
				case ICLEntry::EntryType::UnauthEntry:
					return &entry;
				case ICLEntry::EntryType::MUC:
				case ICLEntry::EntryType::PrivateChat:
					return nick2part.value (message.DisplayName_, &entry);
				}
			} ();
			const auto variant = message.Variant_.value_or ({});

			logs << new HistoryMessage (message.Direction_,
					otherPart,
					mucEntry ? IMessage::Type::MUCMessage : IMessage::Type::ChatMessage,
					variant,
					message.Body_,
					message.TS_,
					message.RichBody_.value_or ({}),
					IMessage::EscapePolicy::Escape);
		}

		emit gotLastMessages (entry.GetQObject (), logs);
	}

	Util::ContextTask<std::optional<QDateTime>> Plugin::RequestMaxTimestamp (IAccount& acc)
	{
		return StorageThread_->Run (&Storage2::GetLastTimestamp, acc.GetAccountID ());
	}

	void Plugin::AddMessages (const History::SomeEntryWithMessages& messages)
	{
		StorageThread_->Run (&Storage2::AddMessage, messages);
	}

	void Plugin::initPlugin (QObject *proxy)
	{
		PluginProxy_ = qobject_cast<IProxyObject*> (proxy);
	}

	void Plugin::hookEntryActionAreasRequested (IHookProxy_ptr proxy,
			QObject *action, QObject*)
	{
		if (!action->property ("Azoth/ChatHistory/IsGood").toBool ())
			return;

		QStringList ours;
		ours << "contactListContextMenu"
			<< "tabContextMenu";
		if (action->property ("ActionIcon") == "view-history")
			ours << "toolbar";

		proxy->SetReturnValue (proxy->GetReturnValue ().toStringList () + ours);
	}

	void Plugin::hookEntryActionsRemoved (IHookProxy_ptr, QObject *entry)
	{
		delete Entry2ActionHistory_.take (entry);
		delete Entry2ActionEnableHistory_.take (entry);
	}

	void Plugin::hookEntryActionsRequested (IHookProxy_ptr proxy, QObject *entryObj)
	{
		const auto entry = qobject_cast<ICLEntry*> (entryObj);
		if (!entry)
		{
			qWarning () << Q_FUNC_INFO
					<< entryObj
					<< "isn't an ICLEntry";
			return;
		}

		if (!Entry2ActionHistory_.contains (entryObj))
		{
			QAction *action = new QAction (tr ("History..."), entryObj);
			action->setProperty ("ActionIcon", "view-history");
			action->setProperty ("Azoth/ChatHistory/IsGood", true);
			connect (action,
					&QAction::triggered,
					this,
					[this, entry] { HandleEntryHistoryRequested (entry); });
			Entry2ActionHistory_ [entryObj] = action;
		}
		if (!Entry2ActionEnableHistory_.contains (entryObj))
		{
			QAction *action = new QAction (tr ("Logging enabled"), entryObj);
			action->setCheckable (true);
			action->setChecked (LoggingStateKeeper_->IsLoggingEnabled (entry));
			action->setProperty ("Azoth/ChatHistory/IsGood", true);
			connect (action,
					&QAction::toggled,
					this,
					[this, entry] (bool enable) { LoggingStateKeeper_->SetLoggingEnabled (entry, enable); });
			Entry2ActionEnableHistory_ [entryObj] = action;
		}

		auto list = proxy->GetReturnValue ().toList ();
		list << QVariant::fromValue<QObject*> (SeparatorAction_);
		list << QVariant::fromValue<QObject*> (Entry2ActionHistory_ [entryObj]);
		list << QVariant::fromValue<QObject*> (Entry2ActionEnableHistory_ [entryObj]);
		proxy->SetReturnValue (list);
	}

	namespace
	{
		ICLEntry& GetEntry (const IMessage& msg)
		{
			switch (msg.GetMessageType ())
			{
			case IMessage::Type::MUCMessage:
				return *qobject_cast<ICLEntry*> (msg.ParentCLEntry ());
			case IMessage::Type::ChatMessage:
			default:
				return *qobject_cast<ICLEntry*> (msg.OtherPart ());
			}
		}

		History::SomeEntryDescrWithEndpoint GetDescription (const ICLEntry& entry, const IMessage& msg)
		{
			const auto& name = entry.GetEntryName ();

			using namespace History;
			using enum EntryKind;

			// TODO fill out self variant when the API will allow that
			const SelfEndpoint self { .Name_ = entry.GetParentAccount ()->GetOurNick (), .Variant_ = {} };

			const auto selfOr = [&]<EntryKind K> (EntryEndpoint<K>&& incoming)
			{
				return msg.GetDirection () == IMessage::Direction::Out ? Endpoint<K> { self } : std::move (incoming);
			};

			switch (entry.GetEntryType ())
			{
			case ICLEntry::EntryType::UnauthEntry: [[fallthrough]];
			case ICLEntry::EntryType::Chat:
			{
				const auto& variant = msg.GetOtherVariant ().isEmpty () ? std::nullopt : std::optional { msg.GetOtherVariant () };
				return EntryDescrWithEndpoint
				{
					EntryDescr<Chat> { name },
					selfOr (EntryEndpoint<Chat> { .Variant_ = variant }),
				};
			}
			case ICLEntry::EntryType::MUC:
				return EntryDescrWithEndpoint<MUC>
				{
					EntryDescr<MUC> { name },
					EntryEndpoint<MUC> { .Nick_ = msg.GetOtherVariant (), .PersistentId_ = {} },
				};
			case ICLEntry::EntryType::PrivateChat:
				// TODO XEP-0421-style persistent IDs
				return EntryDescrWithEndpoint
				{
					EntryDescr<PrivateChat> { .MucName_ = entry.GetParentCLEntry ()->GetEntryName (), .Nick_ = name, .PersistentId_ = {} },
					selfOr (EntryEndpoint<PrivateChat> {}),
				};
			}

			qCritical () << "unknown entry type" << static_cast<int> (entry.GetEntryType ());
			return EntryDescrWithEndpoint
			{
				EntryDescr<Chat> { name },
				selfOr (EntryEndpoint<Chat> { .Variant_ = msg.GetOtherVariant () }),
			};
		}
	}

	void Plugin::hookGotMessage2 (IHookProxy_ptr, QObject *msgObj)
	{
		if (msgObj->property ("Azoth/HiddenMessage").toBool ())
			return;

		const auto msg = qobject_cast<IMessage*> (msgObj);
		if (!msg)
		{
			qWarning () << msgObj << "doesn't implement IMessage" << sender ();
			return;
		}

		if (msg->GetMessageType () != IMessage::Type::ChatMessage &&
			msg->GetMessageType () != IMessage::Type::MUCMessage)
			return;

		if (msg->GetBody ().isEmpty ())
			return;

		if (msg->GetDirection () == IMessage::Direction::Out &&
				msg->GetMessageType () == IMessage::Type::MUCMessage)
			return;

		const double secsDiff = msg->GetDateTime ().secsTo (QDateTime::currentDateTime ());
		if (msg->GetMessageType () == IMessage::Type::MUCMessage &&
				std::abs (secsDiff) >= 2)
			return;

		auto& entry = GetEntry (*msg);
		if (!LoggingStateKeeper_->IsLoggingEnabled (&entry))
			return;

		const auto acc = entry.GetParentAccount ();

		const auto irtm = qobject_cast<IRichTextMessage*> (msgObj);
		const auto richBody = irtm ? irtm->GetRichBody () : QString {};

		StorageThread_->Run (&Storage2::AddMessage, Util::Visit (GetDescription (entry, *msg),
				[&] (const auto& descrWithEndpoint)
				{
					return History::SomeEntryWithMessages
					{
						History::EntryWithMessages
						{
							History::Entry
							{
								.AccountId_ = acc->GetAccountID (),
								.AccountName_ = acc->GetAccountName (),
								.EntryHumanReadableId_ = entry.GetHumanReadableID (),
								.Description_ = descrWithEndpoint.Description_,
							},
							{
								History::NewMessage
								{
									.Endpoint_ = descrWithEndpoint.Endpoint_,
									.TS_ = msg->GetDateTime (),
									.Body_ = msg->GetBody (),
									.RichBody_ = richBody.isEmpty () ? std::nullopt : std::optional { richBody },
								}
							}
						}
					};
				}));
	}

	void Plugin::HandleHistoryRequested ()
	{
		GetProxyHolder ()->GetRootWindowsManager ()->AddTab (tr ("Chat history"),
				new ChatHistoryWidget { { *StorageThread_, PluginProxy_, this, TabClass_ } },
				IRootWindowsManager::AddTabFlag::Background);
	}

	void Plugin::HandleEntryHistoryRequested (ICLEntry *entry)
	{
		GetProxyHolder ()->GetRootWindowsManager ()->AddTab (tr ("Chat history"),
				new ChatHistoryWidget { { *StorageThread_, PluginProxy_, this, TabClass_ }, entry });
	}

}

LC_EXPORT_PLUGIN (leechcraft_azoth_chathistory, LC::Azoth::ChatHistory::Plugin);
