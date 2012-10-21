/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2012  Georg Rudoy
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

#include "sb2.h"
#include <QIcon>
#include <QMainWindow>
#include <QStatusBar>
#include <QtDebug>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/imwproxy.h>
#include "viewmanager.h"
#include "sbview.h"
#include "traycomponent.h"

namespace LeechCraft
{
namespace SB2
{
	void Plugin::Init (ICoreProxy_ptr proxy)
	{
		Mgr_ = new ViewManager (this);
		auto view = Mgr_->GetView ();
		proxy->GetMWProxy ()->AddSideWidget (view);

		proxy->GetMainWindow ()->statusBar ()->hide ();

		auto tray = new TrayComponent (proxy);
		Mgr_->AddComponent (tray->GetComponent ());
		connect (this,
				SIGNAL (pluginsAvailable ()),
				tray,
				SLOT (handlePluginsAvailable ()));
	}

	void Plugin::SecondInit ()
	{
		emit pluginsAvailable ();
	}

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.SB2";
	}

	void Plugin::Release ()
	{
	}

	QString Plugin::GetName () const
	{
		return "SB2";
	}

	QString Plugin::GetInfo () const
	{
		return tr ("Next-generation fluid sidebar.");
	}

	QIcon Plugin::GetIcon () const
	{
		return QIcon ();
	}
}
}

LC_EXPORT_PLUGIN (leechcraft_sb2, LeechCraft::SB2::Plugin);

