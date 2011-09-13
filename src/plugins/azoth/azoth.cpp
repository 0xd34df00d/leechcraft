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

#include "azoth.h"
#include <QIcon>
#include <QDockWidget>
#include <QMainWindow>
#include <QVBoxLayout>
#include <QMenu>
#include <QStringListModel>
#include <QAudioDeviceInfo>
#include <interfaces/entitytesthandleresult.h>
#include <interfaces/imwproxy.h>
#include <interfaces/core/icoreproxy.h>
#include <xmlsettingsdialog/xmlsettingsdialog.h>
#include <util/resourceloader.h>
#include <util/util.h>
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

		XmlSettingsDialog_.reset (new Util::XmlSettingsDialog ());
		XmlSettingsDialog_->RegisterObject (&XmlSettingsManager::Instance (),
				"azothsettings.xml");

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

		XmlSettingsDialog_->SetCustomWidget ("AccountsWidget", new AccountsListWidget);

		QDockWidget *dw = new QDockWidget ();
		MW_ = new MainWidget ();
		dw->setWidget (MW_);
		dw->setWindowTitle ("Azoth");
		proxy->GetMWProxy ()->AddDockWidget (Qt::RightDockWidgetArea, dw);

		connect (&Core::Instance (),
				SIGNAL (gotEntity (const LeechCraft::Entity&)),
				this,
				SIGNAL (gotEntity (const LeechCraft::Entity&)));
		connect (&Core::Instance (),
				SIGNAL (delegateEntity (const LeechCraft::Entity&, int*, QObject**)),
				this,
				SIGNAL (delegateEntity (const LeechCraft::Entity&, int*, QObject**)));

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

		TabClassInfo chatTab =
		{
			"ChatTab",
			tr ("Chat"),
			tr ("A tab with a chat session"),
			QIcon (),
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
			tr ("A search tab allows to search within IM services"),
			QIcon (),
			55,
			TFOpenableByRequest
		};
		TabClassInfo sdTab =
		{
			"SD",
			tr ("Service discovery"),
			tr ("A service discovery tab that allows one to discover "
				"capabilities of remote entries"),
			QIcon (),
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

	QMap<QString, QList<QAction*> > Plugin::GetMenuActions () const
	{
		QMap<QString, QList<QAction*> > result;
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
		{
			ServiceDiscoveryWidget *sd = new ServiceDiscoveryWidget;
			connect (sd,
					SIGNAL (removeTab (QWidget*)),
					this,
					SIGNAL (removeTab (QWidget*)));
			emit addNewTab (tr ("Service discovery"), sd);
		}
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

	QList<ANFieldData> Plugin::GetANFields () const
	{
		return Core::Instance ().GetANFields ();
	}

	void Plugin::handleTasksTreeSelectionCurrentRowChanged (const QModelIndex& index, const QModelIndex&)
	{
		QModelIndex si = Core::Instance ().GetProxy ()->MapToSource (index);
		TransferJobManager *mgr = Core::Instance ().GetTransferJobManager ();
		mgr->SelectionChanged (si.model () == mgr->GetSummaryModel () ? si : QModelIndex ());
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

Q_EXPORT_PLUGIN2 (leechcraft_azoth, LeechCraft::Azoth::Plugin);
