/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2011  Oleg Linkin
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

#include "glance.h"
#include <QAction>
#include <QTabWidget>
#include <QToolBar>
#include <QMainWindow>
#include "core.h"
#include "glanceshower.h"

namespace LeechCraft
{
namespace Plugins
{
namespace Glance
{
	void Plugin::Init (ICoreProxy_ptr proxy)
	{
		Core::Instance ().SetProxy (proxy);

		ActionGlance_ = new QAction (this);
		ActionGlance_->setToolTip ("Show the quick overview of tabs");
		ActionGlance_->setShortcut (QKeySequence ("Ctrl+G"));
		ActionGlance_->setShortcutContext (Qt::ApplicationShortcut);
		ActionGlance_->setIcon (proxy->GetIcon ("glance"));

		connect (ActionGlance_,
				SIGNAL (triggered ()),
				this,
				SLOT (on_ActionGlance__triggered ()));
	}
	
	void Plugin::SecondInit ()
	{
	}

	void Plugin::Release ()
	{
	}

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.Glance";
	}

	QString Plugin::GetName () const
	{
		return "Glance";
	}

	QString Plugin::GetInfo () const
	{
		return tr ("Quick overview of tabs.");
	}

	QIcon Plugin::GetIcon () const
	{
		return QIcon ();
	}
	
	void Plugin::on_ActionGlance__triggered ()
	{
		Glance_ = new GlanceShower;
		Glance_->SetTabWidget (Core::Instance ().GetProxy ()->GetTabWidget ());

		connect (Glance_,
				SIGNAL (finished (bool)),
				ActionGlance_,
				SLOT (setEnabled (bool)));

		ActionGlance_->setEnabled (false);
		Glance_->Start ();
	}
	
	QList<QAction*> Plugin::GetActions (ActionsEmbedPlace aep) const
	{
		QList<QAction*> result;
		if (aep == AEPQuickLaunch)
			result << ActionGlance_;
		return result;
	}
}
}
}

Q_EXPORT_PLUGIN2 (leechcraft_glance, LeechCraft::Plugins::Glance::Plugin);
