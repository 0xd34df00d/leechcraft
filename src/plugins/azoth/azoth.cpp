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

#include "azoth.h"
#include <QIcon>
#include <QDockWidget>
#include <QMainWindow>
#include <QVBoxLayout>
#include <QMenu>
#include <QStringListModel>

#ifdef ENABLE_MEDIACALLS
#include <QAudioDeviceInfo>
#endif

#include <interfaces/entitytesthandleresult.h>
#include <interfaces/imwproxy.h>
#include <interfaces/core/icoreproxy.h>
#include <xmlsettingsdialog/xmlsettingsdialog.h>
#include <util/resourceloader.h>
#include <util/util.h>
#include <util/shortcuts/shortcutmanager.h>
#include "core.h"
#include "mainwidget.h"
#include "chattabsmanager.h"
#include "chattab.h"
#include "xmlsettingsmanager.h"
#include "transferjobmanager.h"
#include "servicediscoverywidget.h"
#include "accountslistwidget.h"
#include "consolewidget.h"
#include "searchwidget.h"

namespace LeechCraft
{
namespace Azoth
{
	void Plugin::Init (ICoreProxy_ptr proxy)
	{
		Translator_.reset (Util::InstallTranslator ("azoth"));

		ChatTab::SetParentMultiTabs (this);
		ServiceDiscoveryWidget::SetParentMultiTabs (this);
		SearchWidget::SetParentMultiTabs (this);

		Core::Instance ().SetProxy (proxy);
		InitShortcuts ();

		XmlSettingsDialog_.reset (new Util::XmlSettingsDialog ());
		XmlSettingsDialog_->RegisterObject (&XmlSettingsManager::Instance (),
				"azothsettings.xml");

		connect (XmlSettingsDialog_.get (),
				SIGNAL (moreThisStuffRequested (const QString&)),
				this,
				SLOT (handleMoreThisStuff (const QString&)));

		XmlSettingsDialog_->SetDataSource ("StatusIcons",
				Core::Instance ().GetResourceLoader (Core::RLTStatusIconLoader)->
					GetSubElemModel ());
		XmlSettingsDialog_->SetDataSource ("ClientIcons",
				Core::Instance ().GetResourceLoader (Core::RLTClientIconLoader)->
					GetSubElemModel ());
		XmlSettingsDialog_->SetDataSource ("AffIcons",
				Core::Instance ().GetResourceLoader (Core::RLTAffIconLoader)->
					GetSubElemModel ());
		XmlSettingsDialog_->SetDataSource ("MoodIcons",
				Core::Instance ().GetResourceLoader (Core::RLTMoodIconLoader)->
					GetSubElemModel ());
		XmlSettingsDialog_->SetDataSource ("ActivityIcons",
				Core::Instance ().GetResourceLoader (Core::RLTActivityIconLoader)->
					GetSubElemModel ());
		XmlSettingsDialog_->SetDataSource ("SystemIcons",
				Core::Instance ().GetResourceLoader (Core::RLTSystemIconLoader)->
					GetSubElemModel ());

		QList<QByteArray> iconsPropList;
		iconsPropList << "StatusIcons"
				<< "ClientIcon"
				<< "AffIcons"
				<< "MoodIcons"
				<< "ActivityIcons"
				<< "SystemIcons";
		XmlSettingsManager::Instance ().RegisterObject (iconsPropList,
				&Core::Instance (),
				"flushIconCaches");

#ifdef ENABLE_MEDIACALLS
		QStringList audioIns (tr ("Default input device"));
		Q_FOREACH (const QAudioDeviceInfo& info,
				QAudioDeviceInfo::availableDevices (QAudio::AudioInput))
			audioIns << info.deviceName ();
		XmlSettingsDialog_->SetDataSource ("InputAudioDevice", new QStringListModel (audioIns));

		QStringList audioOuts (tr ("Default output device"));
		Q_FOREACH (const QAudioDeviceInfo& info,
				QAudioDeviceInfo::availableDevices (QAudio::AudioOutput))
			audioOuts << info.deviceName ();
		XmlSettingsDialog_->SetDataSource ("OutputAudioDevice", new QStringListModel (audioOuts));
#endif

		XmlSettingsDialog_->SetCustomWidget ("AccountsWidget", new AccountsListWidget);

		QDockWidget *dw = new QDockWidget ();
		MW_ = new MainWidget ();
		dw->setWidget (MW_);
		dw->setWindowTitle ("Azoth");
		proxy->GetMWProxy ()->AddDockWidget (Qt::RightDockWidgetArea, dw);
		proxy->GetMWProxy ()->SetViewActionShortcut (dw, QString ("Ctrl+J,A"));

		connect (&Core::Instance (),
				SIGNAL (gotEntity (const LeechCraft::Entity&)),
				this,
				SIGNAL (gotEntity (const LeechCraft::Entity&)));
		connect (&Core::Instance (),
				SIGNAL (delegateEntity (const LeechCraft::Entity&, int*, QObject**)),
				this,
				SIGNAL (delegateEntity (const LeechCraft::Entity&, int*, QObject**)));
		connect (&Core::Instance (),
				SIGNAL (gotSDWidget (ServiceDiscoveryWidget*)),
				this,
				SLOT (handleSDWidget (ServiceDiscoveryWidget*)));

		connect (Core::Instance ().GetChatTabsManager (),
				SIGNAL (addNewTab (const QString&, QWidget*)),
				this,
				SIGNAL (addNewTab (const QString&, QWidget*)));
		connect (Core::Instance ().GetChatTabsManager (),
				SIGNAL (changeTabName (QWidget*, const QString&)),
				this,
				SIGNAL (changeTabName (QWidget*, const QString&)));
		connect (Core::Instance ().GetChatTabsManager (),
				SIGNAL (changeTabIcon (QWidget*, const QIcon&)),
				this,
				SIGNAL (changeTabIcon (QWidget*, const QIcon&)));
		connect (Core::Instance ().GetChatTabsManager (),
				SIGNAL (removeTab (QWidget*)),
				this,
				SIGNAL (removeTab (QWidget*)));
		connect (Core::Instance ().GetChatTabsManager (),
				SIGNAL (raiseTab (QWidget*)),
				this,
				SIGNAL (raiseTab (QWidget*)));
		connect (MW_,
				SIGNAL (gotConsoleWidget (ConsoleWidget*)),
				this,
				SLOT (handleConsoleWidget (ConsoleWidget*)));
		connect (MW_,
				SIGNAL (gotSDWidget (ServiceDiscoveryWidget*)),
				this,
				SLOT (handleSDWidget (ServiceDiscoveryWidget*)));

		TabClassInfo chatTab =
		{
			"ChatTab",
			tr ("Chat"),
			tr ("A tab with a chat session"),
			QIcon (":/plugins/azoth/resources/images/chattabclass.svg"),
			0,
			TFEmpty
		};
		TabClassInfo mucTab =
		{
			"MUCTab",
			tr ("MUC"),
			tr ("A multiuser conference"),
			QIcon (),
			50,
			TFOpenableByRequest
		};
		TabClassInfo searchTab =
		{
			"Search",
			tr ("Search"),
			tr ("A search tab allows one to search within IM services"),
			QIcon (":/plugins/azoth/resources/images/searchtab.svg"),
			55,
			TFOpenableByRequest
		};
		TabClassInfo sdTab =
		{
			"SD",
			tr ("Service discovery"),
			tr ("A service discovery tab that allows one to discover "
				"capabilities of remote entries"),
			QIcon (":/plugins/azoth/resources/images/sdtab.svg"),
			55,
			TFOpenableByRequest
		};
		TabClassInfo consoleTab =
		{
			"ConsoleTab",
			tr ("IM console"),
			tr ("Protocol console, for example, XML console for a XMPP "
				"client protocol"),
			QIcon (),
			0,
			TFEmpty
		};

		TabClasses_ << chatTab;
		TabClasses_ << mucTab;
		TabClasses_ << searchTab;
		TabClasses_ << sdTab;
		TabClasses_ << consoleTab;
	}

	void Plugin::SecondInit ()
	{
		XmlSettingsDialog_->SetDataSource ("SmileIcons",
				Core::Instance ().GetSmilesOptionsModel ());
		XmlSettingsDialog_->SetDataSource ("ChatWindowStyle",
				Core::Instance ().GetChatStylesOptionsModel ());
		XmlSettingsDialog_->SetDataSource ("MUCWindowStyle",
				Core::Instance ().GetChatStylesOptionsModel ());

		Entity e = Util::MakeEntity (QVariant (),
				QString (),
				OnlyHandle,
				"x-leechcraft/global-action-register");
		e.Additional_ ["ActionID"] = GetUniqueID () + "_ShowNextUnread";
		e.Additional_ ["Receiver"] = QVariant::fromValue<QObject*> (&Core::Instance ());
		e.Additional_ ["Method"] = SLOT (handleShowNextUnread ());
		e.Additional_ ["Shortcut"] = QKeySequence (QString ("Ctrl+Alt+Shift+M"));
		emit gotEntity (e);
	}

	void Plugin::Release ()
	{
		Core::Instance ().Release ();
	}

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.Azoth";
	}

	QString Plugin::GetName () const
	{
		return "Azoth";
	}

	QString Plugin::GetInfo () const
	{
		return tr ("Extensible IM client for LeechCraft.");
	}

	QIcon Plugin::GetIcon () const
	{
		return QIcon (":/plugins/azoth/resources/images/azoth.svg");
	}

	QStringList Plugin::Provides () const
	{
		return QStringList (GetUniqueID ());
	}

	QSet<QByteArray> Plugin::GetExpectedPluginClasses () const
	{
		return Core::Instance ().GetExpectedPluginClasses ();
	}

	void Plugin::AddPlugin (QObject *object)
	{
		Core::Instance ().AddPlugin (object);
	}

	Util::XmlSettingsDialog_ptr Plugin::GetSettingsDialog () const
	{
		return XmlSettingsDialog_;
	}

	QAbstractItemModel* Plugin::GetRepresentation () const
	{
		return Core::Instance ().GetTransferJobManager ()->GetSummaryModel ();
	}

	QList<QAction*> Plugin::GetActions (ActionsEmbedPlace aep) const
	{
		QList<QAction*> result;
		switch (aep)
		{
		case AEPTrayMenu:
			result << MW_->GetChangeStatusMenu ()->menuAction ();
			break;
		default:
			break;
		}
		return result;
	}

	QMap<QString, QList<QAction*>> Plugin::GetMenuActions () const
	{
		QMap<QString, QList<QAction*>> result;
		result ["Azoth"] << MW_->GetMenuActions ();
		return result;
	}

	EntityTestHandleResult Plugin::CouldHandle (const Entity& e) const
	{
		return Core::Instance ().CouldHandle (e) ?
				EntityTestHandleResult (EntityTestHandleResult::PIdeal) :
				EntityTestHandleResult ();
	}

	void Plugin::Handle (Entity e)
	{
		Core::Instance ().Handle (e);
	}

	TabClasses_t Plugin::GetTabClasses () const
	{
		return TabClasses_;
	}

	void Plugin::TabOpenRequested (const QByteArray& tabClass)
	{
		if (tabClass == "MUCTab")
			Core::Instance ().handleMucJoinRequested ();
		else if (tabClass == "SD")
			handleSDWidget (new ServiceDiscoveryWidget);
		else if (tabClass == "Search")
		{
			SearchWidget *search = new SearchWidget;
			connect (search,
					SIGNAL (removeTab (QWidget*)),
					this,
					SIGNAL (removeTab (QWidget*)));
			emit addNewTab (tr ("Search"), search);
			emit raiseTab (search);
		}
	}

	void Plugin::RecoverTabs (const QList<TabRecoverInfo>& infos)
	{
		Q_FOREACH (const TabRecoverInfo& recInfo, infos)
		{
			QDataStream str (recInfo.Data_);
			QByteArray context;
			str >> context;

			qDebug () << Q_FUNC_INFO << context;

			if (context == "chattab")
			{
				ChatTabsManager::RestoreChatInfo info;
				info.Props_ = recInfo.DynProperties_;
				str >> info.EntryID_
					>> info.Variant_;

				QList<ChatTabsManager::RestoreChatInfo> infos;
				infos << info;
				Core::Instance ().GetChatTabsManager ()->EnqueueRestoreInfos (infos);
			}
			else
				qWarning () << Q_FUNC_INFO
						<< "unknown context"
						<< context;
		}
	}

	void Plugin::SetShortcut (const QString& id, const QKeySequences_t& seqs)
	{
		Core::Instance ().GetShortcutManager ()->SetShortcut (id, seqs);
	}

	QMap<QString, ActionInfo> Plugin::GetActionInfo () const
	{
		return Core::Instance ().GetShortcutManager ()->GetActionInfo ();
	}

	QList<ANFieldData> Plugin::GetANFields () const
	{
		return Core::Instance ().GetANFields ();
	}

	void Plugin::InitShortcuts ()
	{
		auto proxy = Core::Instance ().GetProxy ();

		auto sm = Core::Instance ().GetShortcutManager ();
		sm->SetObject (this);

		sm->RegisterActionInfo ("org.LeechCraft.Azoth.ClearChat",
				ActionInfo (tr ("Clear chat window"),
						QString ("Ctrl+L"),
						proxy->GetIcon ("edit-clear-history")));
		sm->RegisterActionInfo ("org.LeechCraft.Azoth.QuoteSelected",
				ActionInfo (tr ("Quote selected in chat tab"),
						QString ("Ctrl+Q"),
						proxy->GetIcon ("mail-reply-sender")));
	}

	void Plugin::handleSDWidget (ServiceDiscoveryWidget *sd)
	{
		connect (sd,
				SIGNAL (removeTab (QWidget*)),
				this,
				SIGNAL (removeTab (QWidget*)));
		emit addNewTab (tr ("Service discovery"), sd);
		emit raiseTab (sd);
	}

	void Plugin::handleTasksTreeSelectionCurrentRowChanged (const QModelIndex& index, const QModelIndex&)
	{
		QModelIndex si = Core::Instance ().GetProxy ()->MapToSource (index);
		TransferJobManager *mgr = Core::Instance ().GetTransferJobManager ();
		mgr->SelectionChanged (si.model () == mgr->GetSummaryModel () ? si : QModelIndex ());
	}

	void Plugin::handleMoreThisStuff (const QString& id)
	{
		QMap<QString, QStringList> id2tags;
		id2tags ["StatusIcons"] << "azoth" << "status icons";
		id2tags ["MoodIcons"] << "azoth" << "mood icons";
		id2tags ["Smiles"] << "azoth" << "emoticons";
		id2tags ["ClientIcons"] << "azoth" << "client icons";
		id2tags ["AffIcons"] << "azoth" << "affiliation icons";
		id2tags ["ActivityIcons"] << "azoth" << "activity icons";
		id2tags ["SystemIcons"] << "azoth" << "system icons";
		id2tags ["ChatWindowStyles"] << "azoth" << "chat styles";

		const QStringList& tags = id2tags [id];
		if (tags.isEmpty ())
			return;

		Entity e = Util::MakeEntity ("ListPackages",
				QString (),
				FromUserInitiated,
				"x-leechcraft/package-manager-action");
		e.Additional_ ["Tags"] = tags;

		emit gotEntity (e);
	}

	void Plugin::handleConsoleWidget (ConsoleWidget *cw)
	{
		cw->SetParentMultiTabs (this);
		connect (cw,
				SIGNAL (removeTab (QWidget*)),
				this,
				SIGNAL (removeTab (QWidget*)),
				Qt::UniqueConnection);
		emit addNewTab (cw->GetTitle (), cw);
		emit raiseTab (cw);
	}
}
}

LC_EXPORT_PLUGIN (leechcraft_azoth, LeechCraft::Azoth::Plugin);
