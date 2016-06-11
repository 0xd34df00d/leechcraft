/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
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

#include "chathistory.h"
#include <QDir>
#include <QIcon>
#include <QAction>
#include <QTranslator>
#include <util/util.h>
#include <util/threads/futures.h>
#include <xmlsettingsdialog/xmlsettingsdialog.h>
#include <interfaces/azoth/imessage.h>
#include <interfaces/azoth/iclentry.h>
#include <interfaces/azoth/iaccount.h>
#include <interfaces/azoth/azothcommon.h>
#include <interfaces/azoth/imucentry.h>
#include "core.h"
#include "chathistorywidget.h"
#include "historymessage.h"
#include "xmlsettingsmanager.h"
#include "storagemanager.h"

namespace LeechCraft
{
namespace Azoth
{
namespace ChatHistory
{
	void Plugin::Init (ICoreProxy_ptr proxy)
	{
		Util::InstallTranslator ("azoth_chathistory");

		XSD_ = std::make_shared<Util::XmlSettingsDialog> ();
		XSD_->RegisterObject (&XmlSettingsManager::Instance (), "azothchathistorysettings.xml");
		connect (XSD_.get (),
				SIGNAL (pushButtonClicked (QString)),
				this,
				SLOT (handlePushButton (QString)));

		StorageMgr_ = std::make_shared<StorageManager> (Core::Instance ().get ());

		Core::Instance ()->SetCoreProxy (proxy);

		ChatHistoryWidget::SetParentMultiTabs (this);

		Guard_.reset (new STGuard<Core> ());
		ActionHistory_ = new QAction (tr ("IM history"), this);
		connect (ActionHistory_,
				SIGNAL (triggered ()),
				this,
				SLOT (handleHistoryRequested ()));

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
		Guard_.reset ();
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
		static QIcon icon ("lcicons:/azoth/chathistory/resources/images/chathistory.svg");
		return icon;
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
		TabClasses_t result;
		result << Core::Instance ()->GetTabClass ();
		return result;
	}

	void Plugin::TabOpenRequested (const QByteArray& tabClass)
	{
		if (tabClass == "Chathistory")
			handleHistoryRequested ();
		else
			qWarning () << Q_FUNC_INFO
					<< "unknown tab class"
					<< tabClass;
	}

	Util::XmlSettingsDialog_ptr Plugin::GetSettingsDialog () const
	{
		return XSD_;
	}

	bool Plugin::IsHistoryEnabledFor (QObject *entry) const
	{
		return Core::Instance ()->IsLoggingEnabled (entry);
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
		Util::Sequence (this, Core::Instance ()->GetChatLogs (accId, entryId, 0, num)) >>
				std::bind (&Plugin::HandleGotChatLogs, this, entryObj, std::placeholders::_1);
	}

	QFuture<Plugin::MaxTimestampResult_t> Plugin::RequestMaxTimestamp (IAccount *acc)
	{
		return Core::Instance ()->GetMaxTimestamp (acc->GetAccountID ());
	}

	void Plugin::AddRawMessages (const QString& accountId, const QString& entryId,
			const QString& visibleName, const QList<HistoryItem>& items)
	{
		Core::Instance ()->AddLogItems (accountId, entryId, visibleName, items, true);
	}

	void Plugin::InitWidget (ChatHistoryWidget *wh)
	{
		connect (wh,
				SIGNAL (removeSelf (QWidget*)),
				this,
				SIGNAL (removeTab (QWidget*)));
		connect (wh,
				SIGNAL (gotEntity (LeechCraft::Entity)),
				this,
				SIGNAL (gotEntity (LeechCraft::Entity)));
	}

	void Plugin::initPlugin (QObject *proxy)
	{
		Core::Instance ()->SetPluginProxy (proxy);
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

	void Plugin::hookEntryActionsRequested (IHookProxy_ptr proxy, QObject *entry)
	{
		if (!Entry2ActionHistory_.contains (entry))
		{
			QAction *action = new QAction (tr ("History..."), entry);
			action->setProperty ("ActionIcon", "view-history");
			action->setProperty ("Azoth/ChatHistory/IsGood", true);
			action->setProperty ("Azoth/ChatHistory/Entry",
					QVariant::fromValue<QObject*> (entry));
			connect (action,
					SIGNAL (triggered ()),
					this,
					SLOT (handleEntryHistoryRequested ()));
			Entry2ActionHistory_ [entry] = action;
		}
		if (!Entry2ActionEnableHistory_.contains (entry))
		{
			QAction *action = new QAction (tr ("Logging enabled"), entry);
			action->setCheckable (true);
			action->setChecked (Core::Instance ()->IsLoggingEnabled (entry));
			action->setProperty ("Azoth/ChatHistory/IsGood", true);
			action->setProperty ("Azoth/ChatHistory/Entry",
					QVariant::fromValue<QObject*> (entry));
			connect (action,
					SIGNAL (toggled (bool)),
					this,
					SLOT (handleEntryEnableHistoryRequested (bool)));
			Entry2ActionEnableHistory_ [entry] = action;
		}

		auto list = proxy->GetReturnValue ().toList ();
		list << QVariant::fromValue<QObject*> (SeparatorAction_);
		list << QVariant::fromValue<QObject*> (Entry2ActionHistory_ [entry]);
		list << QVariant::fromValue<QObject*> (Entry2ActionEnableHistory_ [entry]);
		proxy->SetReturnValue (list);
	}

	void Plugin::hookGotMessage2 (LeechCraft::IHookProxy_ptr,
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

		Core::Instance ()->Process (message);
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
			Core::Instance ()->RegenUsersCache ();
	}

	void Plugin::handleHistoryRequested ()
	{
		const auto wh = new ChatHistoryWidget { StorageMgr_.get () };
		InitWidget (wh);
		emit addNewTab (tr ("Chat history"), wh);
	}

	void Plugin::handleEntryHistoryRequested ()
	{
		if (!sender ())
		{
			qWarning () << Q_FUNC_INFO
					<< "null sender()";
			return;
		}

		QObject *obj = sender ()->property ("Azoth/ChatHistory/Entry")
				.value<QObject*> ();
		if (!obj)
		{
			qWarning () << Q_FUNC_INFO
					<< "null object for sender"
					<< sender ();
			return;
		}

		ICLEntry *entry = qobject_cast<ICLEntry*> (obj);
		if (!entry)
		{
			qWarning () << Q_FUNC_INFO
					<< "null entry for sender"
					<< sender ()
					<< "and object"
					<< obj;
			return;
		}

		const auto wh = new ChatHistoryWidget { StorageMgr_.get (), entry };
		InitWidget (wh);
		emit addNewTab (tr ("Chat history"), wh);
		emit raiseTab (wh);
	}

	void Plugin::handleEntryEnableHistoryRequested (bool enable)
	{
		if (!sender ())
		{
			qWarning () << Q_FUNC_INFO
					<< "null sender()";
			return;
		}

		QObject *obj = sender ()->property ("Azoth/ChatHistory/Entry")
				.value<QObject*> ();
		if (!obj)
		{
			qWarning () << Q_FUNC_INFO
					<< "null object for sender"
					<< sender ();
			return;
		}

		Core::Instance ()->SetLoggingEnabled (obj, enable);
	}
}
}
}

LC_EXPORT_PLUGIN (leechcraft_azoth_chathistory, LeechCraft::Azoth::ChatHistory::Plugin);
