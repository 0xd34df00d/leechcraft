/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QWidget>
#include <interfaces/ihavetabs.h>
#include "ui_graphstab.h"

namespace LC
{
namespace Poleemery
{
	class GraphsTab : public QWidget
					, public ITabWidget
	{
		Q_OBJECT
		Q_INTERFACES (ITabWidget)

		const TabClassInfo TC_;
		QObject * const ParentPlugin_;

		Ui::GraphsTab Ui_;
	public:
		GraphsTab (const TabClassInfo&, QObject*);

		TabClassInfo GetTabClassInfo () const;
		QObject* ParentMultiTabs ();
		void Remove ();
		QToolBar* GetToolBar () const;
	private slots:
		void updateGraph ();
		void setPredefinedDate (int);
	signals:
		void removeTab (QWidget*);
	};
}
}
