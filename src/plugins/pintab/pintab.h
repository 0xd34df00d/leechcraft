/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2012  Oleg Linkin
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

#ifndef LEECHCRAFT_PINTAB_PINTAB_H
#define LEECHCRAFT_PINTAB_PINTAB_H

#include <QObject>
#include <interfaces/iinfo.h>
#include <interfaces/core/icoretabwidget.h>

namespace LeechCraft
{
namespace PinTab
{
	class Plugin : public QObject
				, public IInfo
	{
		Q_OBJECT
		Q_INTERFACES (IInfo)

		ICoreTabWidget *MainTabWidget_;
	public:
		void Init(ICoreProxy_ptr proxy);
		void SecondInit();
		QByteArray GetUniqueID() const;
		void Release();
		QString GetName() const;
		QString GetInfo() const;
		QIcon GetIcon() const;
	private slots:
		void handleContextMenuRequested (const QPoint& point);
	};
}
}

#endif // LEECHCRAFT_PINTAB_PINTAB_H
