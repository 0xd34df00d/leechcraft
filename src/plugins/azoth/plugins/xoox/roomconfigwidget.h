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
#include <QXmppDataForm.h>
#include <QXmppMucIq.h>
#include <interfaces/iconfigurablemuc.h>
#include "ui_roomconfigwidget.h"

class QStandardItemModel;
class QStandardItem;
class QXmppMucManager;

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
		boost::shared_ptr<FormBuilder> FB_;
		RoomCLEntry *Room_;
		QString JID_;
		
		QStandardItemModel *PermsModel_;
		QMap<QXmppMucAdminIq::Item::Affiliation, QStandardItem*> Aff2Cat_;
		
		QXmppMucManager *MUCManager_;
	public:
		RoomConfigWidget (RoomCLEntry*, QWidget* = 0);
	private:
		QMap<QXmppMucAdminIq::Item::Affiliation, QStandardItem*> InitModel () const;
	public slots:
		void accept ();
	private slots:
		void handleConfigurationReceived (const QString&, const QXmppDataForm&);
		void handlePermsReceived (const QString&, const QList<QXmppMucAdminIq::Item>&);
	signals:
		void dataReady ();
	};
}
}
}

#endif
