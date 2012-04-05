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

#include "chathistory.h"
#include <QDir>
#include <QIcon>
#include <QAction>
#include <QTranslator>
#include <util/util.h>
#include <interfaces/azoth/imessage.h>
#include <interfaces/azoth/iclentry.h>
#include <interfaces/azoth/iaccount.h>
#include <interfaces/azoth/azothcommon.h>
#include "core.h"
#include "chathistorywidget.h"
#include "historymessage.h"

namespace LeechCraft
{
namespace Azoth
{
namespace ChatHistory
{
	void Plugin::Init (ICoreProxy_ptr)
	{
		Translator_.reset (Util::InstallTranslator ("azoth_chathistory"));

		ChatHistoryWidget::SetParentMultiTabs (this);

		Guard_.reset (new STGuard<Core> ());
		ActionHistory_ = new QAction (tr ("IM history"), this);
		connect (ActionHistory_,
				SIGNAL (triggered ()),
				this,
				SLOT (handleHistoryRequested ()));

		connect (Core::Instance ().get (),
				SIGNAL (gotChatLogs (QString, QString, int, int, QVariant)),
				this,
				SLOT (handleGotChatLogs (QString, QString, int, int, QVariant)));
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
		return QIcon (":/azoth/chathistory/resources/images/chathistory.svg");
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

		if (entry->GetEntryType () != ICLEntry::ETChat)
			return;

		IAccount *account = qobject_cast<IAccount*> (entry->GetParentAccount ());
		if (!account)
		{
			qWarning () << Q_FUNC_INFO
					<< entry->GetParentAccount ()
					<< "doesn't implement IAccount";
			return;
		}

		const QString& accId = account->GetAccountID ();
		const QString& entryId = entry->GetEntryID ();
		Core::Instance ()->GetChatLogs (accId, entryId, 0, num);

		RequestedLogs_ [accId] [entryId] = entryObj;
	}

	void Plugin::AddRawMessage (const QVariantMap& map)
	{
		Core::Instance ()->Process (map);
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

		QList<QVariant> list = proxy->GetReturnValue ().toList ();
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

	void Plugin::handleGotChatLogs (const QString& accId, const QString& entryId,
			int backPages, int amount, const QVariant& logs)
	{
		if (!RequestedLogs_.contains (accId) ||
				!RequestedLogs_ [accId].contains (entryId))
			return;

		QObject *entryObj = RequestedLogs_ [accId].take (entryId);

		QList<QObject*> result;
		Q_FOREACH (const QVariant& messageVar, logs.toList ())
		{
			const QVariantMap& msgMap = messageVar.toMap ();

			const IMessage::Direction dir =
				msgMap ["Direction"].toString () == "IN" ?
						IMessage::DIn :
						IMessage::DOut;
			HistoryMessage *msg = new HistoryMessage (dir,
					entryObj,
					msgMap ["Variant"].toString (),
					msgMap ["Message"].toString (),
					msgMap ["Date"].toDateTime ());
			result << msg;
		}

		emit gotLastMessages (entryObj, result);
	}

	void Plugin::handleHistoryRequested ()
	{
		ChatHistoryWidget *wh = new ChatHistoryWidget;
		connect (wh,
				SIGNAL (removeSelf (QWidget*)),
				this,
				SIGNAL (removeTab (QWidget*)));
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

		ChatHistoryWidget *wh = new ChatHistoryWidget (entry);
		connect (wh,
				SIGNAL (removeSelf (QWidget*)),
				this,
				SIGNAL (removeTab (QWidget*)));
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
