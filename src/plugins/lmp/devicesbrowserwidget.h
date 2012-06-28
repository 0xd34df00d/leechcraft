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

#pragma once

#include <QWidget>
#include "ui_devicesbrowserwidget.h"

class IRemovableDevManager;

namespace LeechCraft
{
namespace LMP
{
	class ISyncPlugin;

	class DevicesBrowserWidget : public QWidget
	{
		Q_OBJECT

		Ui::DevicesBrowserWidget Ui_;
		IRemovableDevManager *DevMgr_;

		ISyncPlugin *CurrentSyncer_;
	public:
		DevicesBrowserWidget (QWidget* = 0);

		void InitializeDevices ();
	private slots:
		void handleDevDataChanged (const QModelIndex&, const QModelIndex&);
		void on_UploadButton__released ();
		void on_DevicesSelector__activated (int);
		void on_MountButton__released ();
	};
}
}
