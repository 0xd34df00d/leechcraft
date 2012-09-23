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

#ifndef PLUGINMANAGERDIALOG_H
#define PLUGINMANAGERDIALOG_H
#include <QWidget>
#include "ui_pluginmanagerdialog.h"

class QSortFilterProxyModel;

namespace LeechCraft
{
	class PluginManagerHeader;
	
	class PluginManagerDialog : public QWidget
	{
		Q_OBJECT

		Ui::PluginManagerDialog Ui_;
		QSortFilterProxyModel *FilterProxy_;
		PluginManagerHeader *Header_;
	public:
		PluginManagerDialog (QWidget* = 0);
	public slots:
		void readjustColumns ();

		void accept ();
		void reject ();
		void selectAll (Qt::CheckState selectState);
	signals:
		void needSelectAll (int);
	};
}

#endif
