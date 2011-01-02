/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
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

#ifndef LOGTOOLBOX_H
#define LOGTOOLBOX_H
#include "ui_logtoolbox.h"

namespace LeechCraft
{
	class LogToolBox : public QDialog
	{
		Q_OBJECT
		
		Ui::LogToolBox Ui_;
	public:
		LogToolBox (QWidget* = 0);
		virtual ~LogToolBox ();
	public slots:
		void log (const QString&);
		void handleMaxLogLines ();
	private slots:
		void on_Clear__released ();
	};
};

#endif

