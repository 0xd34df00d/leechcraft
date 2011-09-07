/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2011  Oleg Linkin
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

#ifndef PLUGINS_AZOTH_PLUGINS_ACETAMIDE_SERVERINFOWIDGET_H
#define PLUGINS_AZOTH_PLUGINS_ACETAMIDE_SERVERINFOWIDGET_H

#include <QWidget>
#include <interfaces/iconfigurablemuc.h>
#include "ui_serverinfowidget.h"

namespace LeechCraft
{
namespace Azoth
{
namespace Acetamide
{

	class IrcServerCLEntry;

	class ServerInfoWidget : public QWidget
							, public IMUCConfigWidget
	{
		Q_OBJECT
		Q_INTERFACES (LeechCraft::Azoth::IMUCConfigWidget)

		Ui::ServerInfoWidget Ui_;
		IrcServerCLEntry *ISCLEntry_;
	public:
		ServerInfoWidget (IrcServerCLEntry*, QWidget* = 0);
		void dataReady ();
		void accept ();
	};
}
}
}

#endif // PLUGINS_AZOTH_PLUGINS_ACETAMIDE_SERVERINFOWIDGET_H
