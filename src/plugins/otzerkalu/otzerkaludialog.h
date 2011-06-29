/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2011 Minh Ngo
 * Copyright (C) 2011  Georg Rudoy
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

#ifndef OTZERKALUWIDGET_H
#define OTZERKALUWIDGET_H

#include <QDialog>
#include <QUrl>
#include <QFileDialog>
#include "ui_otzerkaludialog.h"

namespace LeechCraft
{
	
namespace Otzerkalu
{

	class OtzerkaluDialog : public QDialog
	{
		Q_OBJECT
		Ui::OtzerkaluDialog *ui;
		
		QString saveDir;
		int recLevel;
		
		bool ok;
	public:
		OtzerkaluDialog (QWidget* parent = 0, Qt::WindowFlags f = 0);
		virtual ~OtzerkaluDialog ();
		
		int getRecursionLevel ();
		QString getDir ();
		
		bool isOk ();
		
	private slots:
		void getData ();
		void save ();
	
	};

}
}
#endif // OTZERKALUWIDGET_H
