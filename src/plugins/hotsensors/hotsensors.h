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

#pragma once

#include <memory>
#include <QObject>
#include <interfaces/iinfo.h>
#include <interfaces/iquarkcomponentprovider.h>

namespace LeechCraft
{
namespace HotSensors
{
	class SensorsManager;
	class HistoryManager;
	class PlotManager;

	class Plugin : public QObject
				 , public IInfo
				 , public IQuarkComponentProvider
	{
		Q_OBJECT
		Q_INTERFACES (IInfo IQuarkComponentProvider)

		std::shared_ptr<SensorsManager> SensorsMgr_;
		std::unique_ptr<HistoryManager> HistoryMgr_;
		std::unique_ptr<PlotManager> PlotMgr_;

		QuarkComponent ComponentTemplate_;
	public:
		void Init (ICoreProxy_ptr);
		void SecondInit ();
		QByteArray GetUniqueID () const;
		void Release ();
		QString GetName () const;
		QString GetInfo () const;
		QIcon GetIcon () const;

		QuarkComponents_t GetComponents () const;
	};
}
}
