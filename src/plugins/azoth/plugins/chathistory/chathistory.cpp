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
#include <util/threads/futures.h>
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
#include "xmlsettingsmanager.h"
#include "storagemanager.h"
#include "loggingstatekeeper.h"

namespace LC
{
namespace Azoth
{
namespace ChatHistory
{
	void Plugin::Init (ICoreProxy_ptr proxy)
	{
		CoreProxy_ = proxy;

		TabClass_.TabClass_ = "Chathistory";
		TabClass_.VisibleName_ = tr ("Chat history");
		TabClass_.Description_ = tr ("Chat history viewer for the Azoth IM");
		TabClass_.Priority_ = 40;
		TabClass_.Features_ = TFOpenableByRequest;
		TabClass_.Icon_ = proxy->GetIconThemeManager ()->GetPluginIcon ();

		XSD_ = std::make_shared<Util::XmlSettingsDialog> ();
		XSD_->RegisterObject (&XmlSettingsManager::Instance (), "azothchathistorysettings.xml");
		connect (XSD_.get (),
				SIGNAL (pushButtonClicked (QString)),
				this,
				SLOT (handlePushButton (QString)));

		LoggingStateKeeper_ = std::make_shared<LoggingStateKeeper> ();
		StorageMgr_ = std::make_shared<StorageManager> (LoggingStateKeeper_.get ());

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

	bool Plugin::IsHistoryEnabledFor (QObject *entryObj) const
	{
		const auto entry = qobject_cast<ICLEntry*> (entryObj);
		if (!entry)
		{
			qWarning () << Q_FUNC_INFO
					<< entryObj
					<< "could not be casted to ICLEntry";
			return true;
		}

		return LoggingStateKeeper_->IsLoggingEnabled (entry);
	}

	void Plugin::RequestLastMessages (QObject *entryObj, int num)
	{
		ICLEntry *entry = qobject_cast<ICLEntry*> (entryObj);
		if (!entry)
		{
			qWarning () << Q_FUNC_INFO
					<< entryObj
					<< "doesn't implement ICLEntry";
			return;
		}

		const auto account = entry->GetParentAccount ();
		const QString& accId = account->GetAccountID ();
		const QString& entryId = entry->GetEntryID ();
		Util::Sequence (this, StorageMgr_->GetChatLogs (accId, entryId, 0, num)) >>
				std::bind (&Plugin::HandleGotChatLogs,
						this, QPointer<QObject> { entryObj }, std::placeholders::_1);
	}

	QFuture<Plugin::MaxTimestampResult_t> Plugin::RequestMaxTimestamp (IAccount *acc)
	{
		return StorageMgr_->GetMaxTimestamp (acc->GetAccountID ());
	}

	void Plugin::AddRawMessages (const QString& accountId, const QString& entryId,
			const QString& visibleName, const QList<HistoryItem>& items)
	{
		StorageMgr_->AddLogItems (accountId, entryId, visibleName, items, true);
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

	void Plugin::hookGotMessage2 (LC::IHookProxy_ptr,
				QObject *message)
	{
		if (message->property ("Azoth/HiddenMessage").toBool () == true)
			return;

		IMessage *msg = qobject_cast<IMessage*> (message);
		if (!msg)
		{
			qWarning () << Q_FUNC_INFO
					<< message
					<< "doesn't implement IMessage"
					<< sender ();
			return;
		}

		StorageMgr_->Process (message);
	}

	void Plugin::HandleGotChatLogs (const QPointer<QObject>& entryObj,
			const ChatLogsResult_t& result)
	{
		if (!entryObj)
		{
			qWarning () << Q_FUNC_INFO
					<< entryObj
					<< "is dead already";
			return;
		}

		if (const auto err = result.MaybeLeft ())
		{
			qWarning () << Q_FUNC_INFO
					<< "unable to request logs:"
					<< *err;
			return;
		}

		auto mucEntry = qobject_cast<IMUCEntry*> (entryObj);
		const auto& parts = mucEntry ?
				mucEntry->GetParticipants () :
				QObjectList ();

		QList<QObject*> logs;
		for (const auto& item : result.GetRight ())
		{
			QObject *participantObj = nullptr;
			for (auto part : parts)
				if (qobject_cast<ICLEntry*> (part)->GetEntryName () == item.Variant_)
				{
					participantObj = part;
					break;
				}

			const auto msg = new HistoryMessage (item.Dir_,
					participantObj ? participantObj : entryObj.data (),
					item.Type_,
					participantObj ? QString {} : item.Variant_,
					item.Message_,
					item.Date_,
					item.RichMessage_,
					item.EscPolicy_);

			logs << msg;
		}

		emit gotLastMessages (entryObj, logs);
	}

	void Plugin::handlePushButton (const QString& name)
	{
		if (name == "RegenUsersCache")
			StorageMgr_->RegenUsersCache ();
	}

	void Plugin::HandleHistoryRequested ()
	{
		GetProxyHolder ()->GetRootWindowsManager ()->AddTab (tr ("Chat history"),
				new ChatHistoryWidget { { StorageMgr_.get (), PluginProxy_, CoreProxy_, this, TabClass_ } },
				IRootWindowsManager::AddTabFlag::Background);
	}

	void Plugin::HandleEntryHistoryRequested (ICLEntry *entry)
	{
		GetProxyHolder ()->GetRootWindowsManager ()->AddTab (tr ("Chat history"),
				new ChatHistoryWidget { { StorageMgr_.get (), PluginProxy_, CoreProxy_, this, TabClass_ }, entry });
	}

}
}
}

LC_EXPORT_PLUGIN (leechcraft_azoth_chathistory, LC::Azoth::ChatHistory::Plugin);
