/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2009  Georg Rudoy
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

#ifndef TABCONTENTS_H
#define TABCONTENTS_H
#include <QWidget>
#include "ui_tabcontents.h"

class QTimer;
class QToolBar;

namespace LeechCraft
{
	class TabContents : public QWidget
	{
		Q_OBJECT

		Ui::TabContents Ui_;
		QTimer *FilterTimer_;
		QList<QComboBox*> AdditionalBoxes_;
	public:
		TabContents (QWidget* = 0);
		virtual ~TabContents ();

		void AllowPlugins ();
		Ui::TabContents GetUi () const;
		void SmartDeselect (TabContents*);
		void SetQuery (QStringList);
	private:
		void FillCombobox (QComboBox*);
		QString GetQuery () const;
	private slots:
		void updatePanes (const QModelIndex&, const QModelIndex&);
		void filterParametersChanged ();
		void filterReturnPressed ();
		void feedFilterParameters ();
		void on_PluginsTasksTree__customContextMenuRequested (const QPoint&);
		void on_Add__released ();
		void handleCategoriesChanged (const QStringList&, const QStringList&);
		void on_SimpleSearch__toggled (bool);
		void removeCategoryBox ();
		void syncSelection (const QModelIndex&);
	signals:
		void filterUpdated ();
		void queryUpdated (const QString&);
	};
};

#endif

