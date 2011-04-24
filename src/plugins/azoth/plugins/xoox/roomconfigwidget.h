/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2011  Georg Rudoy
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

#ifndef PLUGINS_AZOTH_PLUGINS_XOOX_ROOMCONFIGWIDGET_H
#define PLUGINS_AZOTH_PLUGINS_XOOX_ROOMCONFIGWIDGET_H
#include <boost/shared_ptr.hpp>
#include <QWidget>
#include <interfaces/iconfigurablemuc.h>

namespace LeechCraft
{
namespace Azoth
{
namespace Xoox
{
	class RoomCLEntry;
	class FormBuilder;

	class RoomConfigWidget : public QWidget
						   , public IMUCConfigWidget
	{
		Q_OBJECT
		Q_INTERFACES (IMUCConfigWidget)
		
		QWidget *FormWidget_;
		boost::shared_ptr<FormBuilder> FB_;
		RoomCLEntry *Room_;
	public:
		RoomConfigWidget (RoomCLEntry*, QWidget* = 0);
	private slots:
		void handleRoomConfigurationReceived (const QString&, const QXmppDataForm&);
	signals:
		void dataReady ();
	};
}
}
}

#endif
