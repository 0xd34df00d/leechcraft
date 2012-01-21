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

#ifndef PLUGINS_AZOTH_PLUGINS_XOOX_ROOMCONFIGWIDGET_H
#define PLUGINS_AZOTH_PLUGINS_XOOX_ROOMCONFIGWIDGET_H
#include <memory>
#include <QWidget>
#include <QXmppDataForm.h>
#include <QXmppMucIq.h>
#include <interfaces/iconfigurablemuc.h>
#include "ui_roomconfigwidget.h"

class QStandardItemModel;
class QStandardItem;
class QXmppMucManager;
class QXmppMucRoom;

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
		Q_INTERFACES (LeechCraft::Azoth::IMUCConfigWidget)

		Ui::RoomConfigWidget Ui_;

		QWidget *FormWidget_;
		std::shared_ptr<FormBuilder> FB_;
		RoomCLEntry *Room_;
		QString JID_;
		QXmppMucRoom *RoomHandler_;

		QStandardItemModel *PermsModel_;
		QMap<QXmppMucItem::Affiliation, QStandardItem*> Aff2Cat_;
	public:
		RoomConfigWidget (RoomCLEntry*, QWidget* = 0);
	private:
		QMap<QXmppMucItem::Affiliation, QStandardItem*> InitModel () const;
		void SendItem (const QXmppMucItem&);
		QStandardItem* GetCurrentItem () const;
	public slots:
		void accept ();
	private slots:
		void on_AddPerm__released ();
		void on_ModifyPerm__released ();
		void on_RemovePerm__released ();
		void handleConfigurationReceived (const QXmppDataForm&);
		void handlePermsReceived (const QList<QXmppMucItem>&);
	signals:
		void dataReady ();
	};
}
}
}

#endif
