/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2011-2012  Alexander Konovalov
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

#ifndef PLUGINS_SECMAN_PLUGINS_SECURESTORAGE_SETTINGSWIDGET_H
#define	PLUGINS_SECMAN_PLUGINS_SECURESTORAGE_SETTINGSWIDGET_H

#include <QWidget>
#include <QLineEdit>
#include "ui_settingswidget.h"

namespace LeechCraft
{
namespace Plugins
{
namespace SecMan
{
namespace StoragePlugins
{
namespace SecureStorage
{
	class SettingsWidget : public QWidget
	{
		Q_OBJECT

		Ui::SettingsWidget Ui_;
	public:
		SettingsWidget ();
		
		QString GetOldPassword ();
		QString GetNewPassword ();
		void ClearPasswordFields ();
	signals:
		void clearSettingsRequested ();
		void changePasswordRequested ();
	};
}
}
}
}
}

#endif
