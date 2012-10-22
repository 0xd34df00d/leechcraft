/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2012  Like-all
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

#include "shaitan.h"
#include "terminalwidget.h"
#include <QIcon>

namespace LeechCraft
{
namespace Shaitan
{
	void Plugin::Init (ICoreProxy_ptr proxy)
	{
		TerminalTC_ =
		{
			GetUniqueID (),
			"Shaitan",
			GetInfo (),
			GetIcon (),
			40,
			TFOpenableByRequest
		};
	}

	void Plugin::SecondInit ()
	{
	}

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.Shaitan";
	}

	void Plugin::Release ()
	{
	}
	
	void Plugin::TabOpenRequested (const QByteArray& tabClass)
	{
		TerminalWidget *terminal = new TerminalWidget (TerminalTC_, this);
		emit addNewTab ("Shaitan", terminal);
		emit raiseTab (terminal);
		connect (terminal, 
				SIGNAL (removeTab (QWidget*)),
				this, 
				SIGNAL (removeTab (QWidget*)));
	}

	QString Plugin::GetName () const
	{
		return "Shaitan";
	}
	
	TabClasses_t Plugin::GetTabClasses () const
	{
		TabClasses_t tcs;
		tcs << TerminalTC_;
		return tcs;
	}


	QString Plugin::GetInfo () const
	{
		return tr ("");
	}

	QIcon Plugin::GetIcon () const
	{
		return QIcon ();
	}
	
}
}

LC_EXPORT_PLUGIN (leechcraft_shaitan, LeechCraft::Shaitan::Plugin);

