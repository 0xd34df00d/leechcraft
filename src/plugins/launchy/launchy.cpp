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

#include "launchy.h"
#include <QIcon>
#include <QAction>
#include "itemsfinder.h"
#include "fsdisplayer.h"

namespace LeechCraft
{
namespace Launchy
{
	void Plugin::Init (ICoreProxy_ptr proxy)
	{
		Proxy_ = proxy;

		Finder_ = new ItemsFinder (proxy);

		FSLauncher_ = new QAction (tr ("Open fullscreen launcher..."), this);
		FSLauncher_->setProperty ("ActionIcon", "system-run");
		connect (FSLauncher_,
				SIGNAL (triggered ()),
				this,
				SLOT (handleFSRequested ()));
	}

	void Plugin::SecondInit ()
	{
	}

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.Launchy";
	}

	void Plugin::Release ()
	{
	}

	QString Plugin::GetName () const
	{
		return "Launchy";
	}

	QString Plugin::GetInfo () const
	{
		return tr ("Neat application launcher for LeechCraft.");
	}

	QIcon Plugin::GetIcon () const
	{
		return QIcon ();
	}

	QList<QAction*> Plugin::GetActions (ActionsEmbedPlace aep) const
	{
		QList<QAction*> result;
		if (aep == ActionsEmbedPlace::LCTray)
			result << FSLauncher_;
		return result;
	}

	void Plugin::handleFSRequested ()
	{
		auto dis = new FSDisplayer (Proxy_, Finder_, this);
		connect (dis,
				SIGNAL (gotEntity (LeechCraft::Entity)),
				this,
				SIGNAL (gotEntity (LeechCraft::Entity)));
	}
}
}

LC_EXPORT_PLUGIN (leechcraft_launchy, LeechCraft::Launchy::Plugin);
