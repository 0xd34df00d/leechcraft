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

#include "blackdash.h"
#include <QIcon>
#include "dashtab.h"

namespace LeechCraft
{
namespace BlackDash
{
	void Plugin::Init (ICoreProxy_ptr proxy)
	{		
		DashTab::SetParentPlugin (this);
		TabClasses_ << DashTab::GetStaticTabClassInfo ();
	}

	void Plugin::SecondInit ()
	{
	}

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.BlackDash";
	}

	void Plugin::Release ()
	{
	}

	QString Plugin::GetName () const
	{
		return "BlackDash";
	}

	QString Plugin::GetInfo () const
	{
		return tr ("BlackDash is a dashboard plugin for LeechCraft.");
	}

	QIcon Plugin::GetIcon () const
	{
		return QIcon ();
	}
	
	TabClasses_t Plugin::GetTabClasses () const
	{
		return TabClasses_;
	}
	
	void Plugin::TabOpenRequested (const QByteArray& tabClass)
	{
		if (tabClass == DashTab::GetStaticTabClassInfo ().TabClass_)
		{
			DashTab *tab = new DashTab;
			
			connect (tab,
					SIGNAL (removeTab (QWidget*)),
					this,
					SIGNAL (removeTab (QWidget*)));
			
			emit addNewTab (tr ("Dashboard"), tab);
		}
		else
			qWarning () << Q_FUNC_INFO
					<< "unknown tab class"
					<< tabClass;
	}
}
}

LC_EXPORT_PLUGIN (leechcraft_blackdash, LeechCraft::BlackDash::Plugin);
