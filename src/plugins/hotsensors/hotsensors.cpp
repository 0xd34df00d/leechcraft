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
#include <QAbstractItemModel>
#include <util/sys/paths.h>
#include "sensorsmanager.h"
#include "historymanager.h"
#include "plotmanager.h"

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

		PlotMgr_.reset (new PlotManager (SensorsMgr_, this));
		connect (HistoryMgr_.get (),
				SIGNAL (historyChanged (ReadingsHistory_t)),
				PlotMgr_.get (),
				SLOT (handleHistoryUpdated (ReadingsHistory_t)));

		const auto& path = Util::GetSysPath (Util::SysPath::QML, "hotsensors", "HSQuark.qml");
		Component_.Url_ = QUrl::fromLocalFile (path);
		Component_.DynamicProps_.append ({ "HS_sensorsModel", PlotMgr_->GetModel () });
		Component_.ImageProviders_.append ({ "HS_sensorsGraph", PlotMgr_->GetImageProvider () });
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

	QuarkComponents_t Plugin::GetComponents () const
	{
		return { Component_ };
	}
}
}

LC_EXPORT_PLUGIN (leechcraft_hotsensors, LeechCraft::HotSensors::Plugin);
