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

#include "twifee.h"
#include <QIcon>

namespace LeechCraft
{
namespace Twifee
{
	void Plugin::Init (ICoreProxy_ptr proxy)
	{
	}

	void Plugin::SecondInit ()
	{
	}

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.Twifee";
	}

	void Plugin::Release ()
	{
	}

	QString Plugin::GetName () const
	{
		return "Twifee";
	}

	QString Plugin::GetInfo () const
	{
		return tr ("Twitter client for LeechCraft.");
	}

	QIcon Plugin::GetIcon () const
	{
		return QIcon ();
	}
}
}

LC_EXPORT_PLUGIN (leechcraft_twifee, LeechCraft::Twifee::Plugin);

