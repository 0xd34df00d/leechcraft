/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2010  Georg Rudoy
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

#ifndef XMLSETTINGSDIALOG_ITEMHANDLERS_DATAVIEWWIDGET_H
#define XMLSETTINGSDIALOG_ITEMHANDLERS_DATAVIEWWIDGET_H
#include <QWidget>
#include "ui_dataviewwidget.h"

namespace LeechCraft
{
	class DataViewWidget : public QWidget
	{
		Q_OBJECT

		Ui::DataViewWidget Ui_;
	public:
		DataViewWidget (QWidget* = 0);

		void SetModel (QAbstractItemModel*);
		QAbstractItemModel* GetModel () const;
		QModelIndexList GetSelectedRows () const;
	signals:
		void addRequested ();
		void removeRequested ();
	};
}

#endif
