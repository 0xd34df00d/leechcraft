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

#include "depester.h"
#include <QIcon>
#include <QAction>
#include <QTranslator>
#include <plugininterface/util.h>

namespace LeechCraft
{
namespace Azoth
{
namespace Depester
{
	void Plugin::Init (ICoreProxy_ptr)
	{
		Translator_.reset (Util::InstallTranslator ("azoth_depester"));
	}

	void Plugin::SecondInit ()
	{
	}

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.Azoth.Depester";
	}

	void Plugin::Release ()
	{
	}

	QString Plugin::GetName () const
	{
		return "Azoth Depester";
	}

	QString Plugin::GetInfo () const
	{
		return tr ("Allows to block messages from unwanted contacts in MUCs.");
	}

	QIcon Plugin::GetIcon () const
	{
		return QIcon ();
	}

	QSet<QByteArray> Plugin::GetPluginClasses () const
	{
		QSet<QByteArray> result;
		result << "org.LeechCraft.Plugins.Azoth.Plugins.IGeneralPlugin";
		return result;
	}
	
	void Plugin::hookEntryActionAreasRequested (IHookProxy_ptr proxy,
			QObject *action, QObject*)
	{
	}
	
	void Plugin::hookEntryActionsRequested (IHookProxy_ptr proxy, QObject *entry)
	{
	}

	void Plugin::hookGotMessage (LeechCraft::IHookProxy_ptr,
				QObject *message)
	{
	}
}
}
}

Q_EXPORT_PLUGIN2 (leechcraft_azoth_depester, LeechCraft::Azoth::Depester::Plugin);
