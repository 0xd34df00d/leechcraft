/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#ifndef PLUGINS_AZOTH_SERVICEDISCOVERYWIDGET_H
#define PLUGINS_AZOTH_SERVICEDISCOVERYWIDGET_H
#include <memory>
#include <QWidget>
#include <interfaces/ihavetabs.h>
#include "ui_servicediscoverywidget.h"

class QComboBox;
class QSortFilterProxyModel;

namespace LC
{
namespace Azoth
{
	class ISDSession;

	class ServiceDiscoveryWidget : public QWidget
								 , public ITabWidget
	{
		Q_OBJECT
		Q_INTERFACES (ITabWidget)

		static QObject *S_ParentMultiTabs_;

		Ui::ServiceDiscoveryWidget Ui_;

		QToolBar *Toolbar_;
		QComboBox *AccountBox_;
		QLineEdit *AddressLine_;
		QLineEdit *FilterLine_;
		QSortFilterProxyModel *FilterModel_;

		QTimer *DiscoveryTimer_;
		std::shared_ptr<ISDSession> SDSession_;
	public:
		static void SetParentMultiTabs (QObject*);

		ServiceDiscoveryWidget (QWidget* = 0);

		void SetAccount (QObject*);
		void SetSDSession (ISDSession*);

		TabClassInfo GetTabClassInfo () const;
		QObject* ParentMultiTabs ();
		void Remove ();
		QToolBar* GetToolBar () const;
	private slots:
		void handleDiscoveryAddressChanged ();
		void on_DiscoveryTree__customContextMenuRequested (const QPoint&);
		void discover ();
	signals:
		void removeTab (QWidget*);
	};
}
}

#endif
