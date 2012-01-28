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

#pragma once

#include <QDialog>
#include "ui_showconfigdialog.h"

class QStandardItemModel;

namespace LeechCraft
{
namespace Sidebar
{
	class ShowConfigDialog : public QDialog
	{
		Q_OBJECT

		Ui::ShowConfigDialog Ui_;

		const QString Context_;
		QStandardItemModel *Model_;

		typedef QHash<QString, QList<QAction*>> ID2Actions_t;
		ID2Actions_t HiddenActions_;
		ID2Actions_t AllActions_;

		enum Roles
		{
			ActionID = Qt::UserRole + 1
		};
	public:
		ShowConfigDialog (const QString& context, QWidget* = 0);

		bool CheckAction (const QString&, QAction*);
	private slots:
		void saveSettings ();
		void reloadSettings ();
		void handleActionDestroyed ();
	signals:
		void showActions (const QList<QAction*>&);
		void hideActions (const QList<QAction*>&);
	};
}
}
