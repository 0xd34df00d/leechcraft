/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2013  Georg Rudoy
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

#include "hotsensors.h"
#include <QIcon>
#include "sensorsmanager.h"
#include "historymanager.h"

namespace LeechCraft
{
namespace HotSensors
{
	void Plugin::Init (ICoreProxy_ptr proxy)
	{
		SensorsMgr_.reset (new SensorsManager (this));
		HistoryMgr_.reset (new HistoryManager (this));
		connect (SensorsMgr_.get (),
				SIGNAL (gotReadings (Readings_t)),
				HistoryMgr_.get (),
				SLOT (handleReadings (Readings_t)));
	}

	void Plugin::SecondInit ()
	{
	}

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.HotSensors";
	}

	void Plugin::Release ()
	{
		SensorsMgr_.reset ();
	}

	QString Plugin::GetName () const
	{
		return "HotSensors";
	}

	QString Plugin::GetInfo () const
	{
		return tr ("Temperature sensors information quark.");
	}

	QIcon Plugin::GetIcon () const
	{
		return QIcon ();
	}
}
}

LC_EXPORT_PLUGIN (leechcraft_hotsensors, LeechCraft::HotSensors::Plugin);

