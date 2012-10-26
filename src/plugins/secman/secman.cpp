/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010  Vadim Misbakh-Soloviev, Georg Rudoy
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

#include "secman.h"
#include <QIcon>
#include <QAction>
#include <interfaces/entitytesthandleresult.h>
#include "core.h"
#include "contentsdisplaydialog.h"

namespace LeechCraft
{
namespace Plugins
{
namespace SecMan
{
	void Plugin::Init (ICoreProxy_ptr)
	{
		auto displayContentsAction = new QAction (tr ("Display storages' contents"), this);
		connect (displayContentsAction,
				SIGNAL (triggered ()),
				this,
				SLOT (handleDisplayContents ()));
		MenuActions_ ["tools"] << displayContentsAction;
	}

	void Plugin::SecondInit ()
	{
	}

	void Plugin::Release ()
	{
	}

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.SecMan";
	}

	QString Plugin::GetName () const
	{
		return "SecMan";
	}

	QString Plugin::GetInfo () const
	{
		return tr ("Security and personal data manager for LeechCraft");
	}

	QIcon Plugin::GetIcon () const
	{
		return QIcon ();
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

	QSet<QByteArray> Plugin::GetExpectedPluginClasses () const
	{
		return Core::Instance ().GetExpectedPluginClasses ();
	}

	void Plugin::AddPlugin (QObject *plugin)
	{
		Core::Instance ().AddPlugin (plugin);
	}

	QList<QAction*> Plugin::GetActions (ActionsEmbedPlace area) const
	{
		return QList<QAction*> ();
	}

	QMap<QString, QList<QAction*>> Plugin::GetMenuActions () const
	{
		return MenuActions_;
	}

	void Plugin::handleDisplayContents ()
	{
		auto dia = new ContentsDisplayDialog;
		dia->setAttribute (Qt::WA_DeleteOnClose);
		dia->show ();
	}
}
}
}

LC_EXPORT_PLUGIN (leechcraft_secman, LeechCraft::Plugins::SecMan::Plugin);
