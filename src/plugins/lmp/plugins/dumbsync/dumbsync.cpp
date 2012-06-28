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

#include "dumbsync.h"
#include <QIcon>
#include <QFileInfo>
#include <QDir>
#include "dumbsyncparamswidget.h"

namespace LeechCraft
{
namespace LMP
{
namespace DumbSync
{
	void Plugin::Init (ICoreProxy_ptr)
	{
	}

	void Plugin::SecondInit ()
	{
	}

	void Plugin::Release ()
	{

	}

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.LMP.DumbSync";
	}

	QString Plugin::GetName () const
	{
		return "LMP DumbSync";
	}

	QString Plugin::GetInfo () const
	{
		return tr ("Adds support for synchronization with portable players that show themselves as Flash drives, like Rockbox players.");
	}

	QIcon Plugin::GetIcon () const
	{
		return QIcon ();
	}

	QSet<QByteArray> Plugin::GetPluginClasses () const
	{
		QSet<QByteArray> result;
		result << "org.LeechCraft.LMP.CollectionSync";
		return result;
	}

	void Plugin::SetLMPProxy (ILMPProxy*)
	{
	}

	QString Plugin::GetSyncSystemName () const
	{
		return tr ("dumb copying");
	}

	SyncConfLevel Plugin::CouldSync (const QString& path)
	{
		QFileInfo fi (path);
		if (!fi.isDir () || !fi.isWritable ())
			return SyncConfLevel::None;

		if (fi.dir ().entryList (QDir::Dirs).contains (".rockbox", Qt::CaseInsensitive) ||
			fi.dir ().entryList (QDir::Dirs).contains ("music", Qt::CaseInsensitive))
			return SyncConfLevel::High;

		return SyncConfLevel::Medium;
	}

	QWidget* Plugin::MakeSyncParamsWidget ()
	{
		return new DumbSyncParamsWidget ();
	}

	void Plugin::Upload (const QStringList& paths, QWidget *w)
	{
		auto paramsWidget = qobject_cast<DumbSyncParamsWidget*> (w);
		if (!paramsWidget)
		{
			qWarning () << Q_FUNC_INFO
					<< "incorrect widget passed"
					<< w;
			return;
		}
	}
}
}
}

LC_EXPORT_PLUGIN (leechcraft_lmp_dumbsync, LeechCraft::LMP::DumbSync::Plugin);
