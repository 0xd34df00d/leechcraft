/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2009  Georg Rudoy
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

#include "historyholder.h"
#include <QIcon>
#include "core.h"
#include "findproxy.h"

using namespace LeechCraft::Plugins::HistoryHolder;

void Plugin::Init (ICoreProxy_ptr proxy)
{
	Core::Instance ().SetCoreProxy (proxy);
	connect (&Core::Instance (),
			SIGNAL (gotEntity (const LeechCraft::Entity&)),
			this,
			SIGNAL (gotEntity (const LeechCraft::Entity&)));
}

void Plugin::SecondInit ()
{
}

void Plugin::Release ()
{
	Core::Instance ().Release ();
}

QByteArray Plugin::GetUniqueID () const
{
	return "org.LeechCraft.HistoryHolder";
}

QString Plugin::GetName () const
{
	return "History holder";
}

QString Plugin::GetInfo () const
{
	return tr ("Holds history from various plugins");
}

QIcon Plugin::GetIcon () const
{
	return QIcon (":/resources/images/historyholder.svg");
}

QStringList Plugin::Provides () const
{
	return QStringList ("history");
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

QStringList Plugin::GetCategories () const
{
	return QStringList ("history");
}

QList<IFindProxy_ptr> Plugin::GetProxy (const LeechCraft::Request& r)
{
	QList<IFindProxy_ptr> result;
	result << IFindProxy_ptr (new FindProxy (r));
	return result;
}

bool Plugin::CouldHandle (const LeechCraft::Entity& e) const
{
	Core::Instance ().Handle (e);
	return false;
}

void Plugin::Handle (LeechCraft::Entity)
{
}

void Plugin::SetShortcut (const QString& id, const QKeySequences_t& seqs)
{
	Core::Instance ().SetShortcut (id, seqs);
}

QMap<QString, LeechCraft::ActionInfo> Plugin::GetActionInfo () const
{
	return Core::Instance ().GetActionInfo ();
}

void Plugin::handleTasksTreeActivated (const QModelIndex& index)
{
	Core::Instance ().handleTasksTreeActivated (index);
}

Q_EXPORT_PLUGIN2 (leechcraft_historyholder, Plugin);

