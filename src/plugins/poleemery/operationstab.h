/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <memory>
#include <QWidget>
#include <interfaces/ihavetabs.h>
#include "ui_operationstab.h"

namespace LC
{
namespace Poleemery
{
	class OperationsManager;

	class OperationsTab : public QWidget
						, public ITabWidget
	{
		Q_OBJECT
		Q_INTERFACES (ITabWidget)

		OperationsManager * const OpsManager_;

		Ui::OperationsTab Ui_;

		const TabClassInfo TC_;
		QObject * const ParentPlugin_;

		QToolBar *Toolbar_;
	public:
		OperationsTab (const TabClassInfo&, QObject*);

		TabClassInfo GetTabClassInfo () const;
		QObject* ParentMultiTabs ();
		void Remove ();
		QToolBar* GetToolBar () const;
	private slots:
		void add ();
		void remove ();
	signals:
		void removeTab ();
	};
}
}
