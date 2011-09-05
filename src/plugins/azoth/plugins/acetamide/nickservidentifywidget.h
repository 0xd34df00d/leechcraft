/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2011  Oleg Linkin
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

#ifndef PLUGINS_AZOTH_PLUGINS_ACETAMIDE_NICKSERVIDENTIFYWIDGET_H
#define PLUGINS_AZOTH_PLUGINS_ACETAMIDE_NICKSERVIDENTIFYWIDGET_H

#include <QWidget>
#include "ui_nickservidentifywidget.h"

class QStandardItemModel;

namespace LeechCraft
{
namespace Azoth
{
namespace Acetamide
{
	class NickServIdentifyWidget : public QWidget
	{
		Q_OBJECT

		Ui::NickServIdentifyWidget Ui_;
		QStandardItemModel* Model_;
	public:
		NickServIdentifyWidget (QStandardItemModel*, QWidget* = 0, Qt::WindowFlags = 0);
	private:
		void ReadSettings ();
	public slots:
		void accept ();
	private slots:
		void on_Add__clicked ();
		void on_Edit__clicked ();
		void on_Delete__clicked ();
	};
};
};
};

#endif // PLUGINS_AZOTH_PLUGINS_ACETAMIDE_NICKSERVIDENTIFYWIDGET_H
