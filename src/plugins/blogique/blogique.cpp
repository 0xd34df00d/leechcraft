/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2012  Oleg Linkin
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

#include "blogique.h"
#include <QIcon>
#include <util/util.h>
#include "accountslistwidget.h"
#include "backupmanager.h"
#include "blogiquewidget.h"
#include "core.h"
#include "xmlsettingsmanager.h"

namespace LeechCraft
{
namespace Blogique
{
	void Plugin::Init (ICoreProxy_ptr proxy)
	{
		Util::InstallTranslator ("blogique");
		XmlSettingsDialog_.reset (new Util::XmlSettingsDialog ());
		XmlSettingsDialog_->RegisterObject (&XmlSettingsManager::Instance (),
				"blogiquesettings.xml");
		XmlSettingsDialog_->SetCustomWidget ("AccountsWidget", new AccountsListWidget);

		Core::Instance ().SetCoreProxy (proxy);

		BlogiqueWidget::SetParentMultiTabs (this);

		TabClassInfo tabClass =
		{
			"Blogique",
			"Blogique",
			GetInfo (),
			GetIcon (),
			50,
			TabFeatures (TFOpenableByRequest)
		};
		TabClasses_ << tabClass;

		connect (&Core::Instance (),
				SIGNAL (gotEntity (LeechCraft::Entity)),
				this,
				SIGNAL (gotEntity (LeechCraft::Entity)));
		connect (&Core::Instance (),
				SIGNAL (delegateEntity (LeechCraft::Entity, int*, QObject**)),
				this,
				SIGNAL (delegateEntity (LeechCraft::Entity, int*, QObject**)));
		connect (&Core::Instance (),
				SIGNAL (addNewTab (QString,QWidget*)),
				this,
				SIGNAL (addNewTab (QString,QWidget*)));
		connect (&Core::Instance (),
				SIGNAL (removeTab (QWidget*)),
				this,
				SIGNAL (removeTab (QWidget*)));

		BackupBlog_ = new QAction (tr ("Backup"), this);
		BackupBlog_->setProperty ("ActionIcon", "document-export");

		connect (BackupBlog_,
				SIGNAL (triggered ()),
				Core::Instance ().GetBackupManager (),
				SLOT (backup ()));

		ToolMenu_ = new QMenu ("Blogique");
		ToolMenu_->setIcon (GetIcon ());
		ToolMenu_->addAction (BackupBlog_);
	}

	void Plugin::SecondInit ()
	{
		Core::Instance ().DelayedProfilesUpdate ();
	}

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.Blogique";
	}

	void Plugin::Release ()
	{
	}

	QString Plugin::GetName () const
	{
		return "Blogique";
	}

	QString Plugin::GetInfo () const
	{
		return tr ("Blogging client");
	}

	QIcon Plugin::GetIcon () const
	{
		static QIcon icon (":/plugins/blogique/resources/images/blogique.svg");
		return icon;
	}

	TabClasses_t Plugin::GetTabClasses () const
	{
		return TabClasses_;
	}

	void Plugin::TabOpenRequested (const QByteArray& tabClass)
	{
		if (tabClass == "Blogique")
			CreateTab ();
		else
			qWarning () << Q_FUNC_INFO
					<< "unknown tab class"
					<< tabClass;
	}

	Util::XmlSettingsDialog_ptr Plugin::GetSettingsDialog () const
	{
		return XmlSettingsDialog_;
	}

	QSet<QByteArray> Plugin::GetExpectedPluginClasses () const
	{
		return Core::Instance ().GetExpectedPluginClasses ();
	}

	void Plugin::AddPlugin (QObject* plugin)
	{
		Core::Instance ().AddPlugin (plugin);
	}

	QList<QAction*> Plugin::GetActions (ActionsEmbedPlace area) const
	{
		QList<QAction*> result;

		switch (area)
		{
			case ActionsEmbedPlace::ToolsMenu:
				result << ToolMenu_->menuAction ();
				break;
			default:
				break;
		}

		return result;
	}

	void Plugin::CreateTab ()
	{
		BlogiqueWidget *blogPage = Core::Instance ().CreateBlogiqueWidget ();
		emit addNewTab ("Blogique", blogPage);
		emit changeTabIcon (blogPage, GetIcon ());
		emit raiseTab (blogPage);
	}
}
}

LC_EXPORT_PLUGIN (leechcraft_blogique, LeechCraft::Blogique::Plugin);
