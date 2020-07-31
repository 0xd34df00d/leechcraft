/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#ifndef PLUGINS_AZOTH_PLUGINS_XOOX_ROOMCONFIGWIDGET_H
#define PLUGINS_AZOTH_PLUGINS_XOOX_ROOMCONFIGWIDGET_H
#include <memory>
#include <QWidget>
#include <QXmppDataForm.h>
#include <QXmppMucIq.h>
#include <interfaces/azoth/iconfigurablemuc.h>
#include "ui_roomconfigwidget.h"

class QStandardItemModel;
class QStandardItem;
class QXmppMucManager;
class QXmppMucRoom;

namespace LC
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
		Q_INTERFACES (LC::Azoth::IMUCConfigWidget)

		Ui::RoomConfigWidget Ui_;

		QWidget *FormWidget_ = nullptr;
		std::shared_ptr<FormBuilder> FB_;
		RoomCLEntry *Room_;
		QString JID_;
		QXmppMucRoom * const RoomHandler_;

		QStandardItemModel *PermsModel_;
		QMap<QXmppMucItem::Affiliation, QStandardItem*> Aff2Cat_;

		enum ItemRoles
		{
			Reason = Qt::UserRole + 1
		};
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
