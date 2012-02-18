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

#include "newlife.h"
#include <QIcon>
#include <QMenu>
#include <QAction>
#include <QTranslator>
#include <util/util.h>
#include "common/imimportpage.h"
#include "importwizard.h"

namespace LeechCraft
{
namespace NewLife
{
	void Plugin::Init (ICoreProxy_ptr)
	{
		Util::InstallTranslator ("newlife");

		Common::IMImportPage::SetPluginInstance (this);

		ImporterAction_ = new QAction (tr ("Import settings..."), this);
		ImporterAction_->setProperty ("ActionIcon", "document-import");
		connect (ImporterAction_,
				SIGNAL (triggered ()),
				this,
				SLOT (runWizard ()));
	}

	void Plugin::SecondInit ()
	{
	}

	void Plugin::Release ()
	{
	}

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.NewLife";
	}

	QString Plugin::GetName () const
	{
		return "New Life";
	}

	QString Plugin::GetInfo () const
	{
		return tr ("The settings importer.");
	}

	QIcon Plugin::GetIcon () const
	{
		return QIcon (":/resources/images/newlife.svg");
	}

	QStringList Plugin::Provides () const
	{
		return QStringList ();
	}

	QStringList Plugin::Needs () const
	{
		return QStringList ();
	}

	QStringList Plugin::Uses () const
	{
		return QStringList ();
	}

	void Plugin::SetProvider (QObject*, const QString&)
	{
	}

	QList<QAction*> Plugin::GetActions (ActionsEmbedPlace place) const
	{
		QList<QAction*> result;

		if (place == AEPToolsMenu)
			result << ImporterAction_;

		return result;
	}

	void Plugin::runWizard ()
	{
		ImportWizard *wiz = new ImportWizard (this);
		connect (wiz,
				SIGNAL (gotEntity (const LeechCraft::Entity&)),
				this,
				SIGNAL (gotEntity (const LeechCraft::Entity&)));
		wiz->show ();
	}
}
}

LC_EXPORT_PLUGIN (leechcraft_newlife, LeechCraft::NewLife::Plugin);
