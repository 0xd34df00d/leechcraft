/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#ifndef PLUGINMANAGERDIALOG_H
#define PLUGINMANAGERDIALOG_H
#include <QWidget>
#include "ui_pluginmanagerdialog.h"

class QSortFilterProxyModel;

namespace LC
{
	class PluginManagerDialog : public QWidget
	{
		Q_OBJECT

		Ui::PluginManagerDialog Ui_;
		QSortFilterProxyModel *FilterProxy_;
	public:
		PluginManagerDialog (QWidget* = 0);
	public slots:
		void readjustColumns ();

		void accept ();
		void reject ();
	};
}

#endif
