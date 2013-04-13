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

#include <QObject>
#include <QPointer>
#include <interfaces/core/icoreproxy.h>

class QAbstractItemModel;

namespace LeechCraft
{
namespace Util
{
	class UnhideListViewBase;
}

namespace HotSensors
{
	class PlotManager;
	class SensorsFilterModel;

	class ContextWrapper : public QObject
	{
		Q_OBJECT

		const ICoreProxy_ptr Proxy_;

		QString Context_;
		SensorsFilterModel *Filter_;
		QPointer<Util::UnhideListViewBase> CurrentList_;
	public:
		ContextWrapper (PlotManager*, ICoreProxy_ptr);
	private:
		QStringList LoadHiddenNames () const;
		void SaveHiddenNames (const QStringList&) const;
	public slots:
		void setContext (const QString&);
		QObject* getModel () const;

		void sensorUnhideListRequested (int x, int y, const QRect&);
		void unhideSensor (const QString&);
		void hideSensor (const QString&);
	};
}
}
