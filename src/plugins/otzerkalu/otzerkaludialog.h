/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2011  Minh Ngo
 * Copyright (C) 2006-2011  Georg Rudoy
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

#ifndef PLUGINS_OTZERKALU_OTZERKALUWIDGET_H
#define PLUGINS_OTZERKALU_OTZERKALUWIDGET_H
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
		Ui::OtzerkaluDialog Ui_;
	public:
		OtzerkaluDialog (QWidget *parent = 0);
		
		int GetRecursionLevel () const;
		QString GetDir () const;
		bool FetchFromExternalHosts () const;
	private slots:
		void on_ChooseDirButton__clicked ();
	};
}
}
#endif //PLUGINS_OTZERKALU_OTZERKALUWIDGET_H
