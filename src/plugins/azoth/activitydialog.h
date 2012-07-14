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

#ifndef PLUGINS_AZOTH_ACTIVITYDIALOG_H
#define PLUGINS_AZOTH_ACTIVITYDIALOG_H
#include <QDialog>
#include "ui_activitydialog.h"

namespace LeechCraft
{
namespace Azoth
{
	class ActivityDialog : public QDialog
	{
		Q_OBJECT
		
		Ui::ActivityDialog Ui_;
		QMap<QString, QStringList> Gen2Specific_;
	public:
		static QString ToHumanReadable (const QString&);

		ActivityDialog (QWidget* = 0);
		
		QString GetGeneral () const;
		void SetGeneral (const QString&);
		
		QString GetSpecific () const;
		void SetSpecific (const QString&);
		
		QString GetText () const;
		void SetText (const QString&);
	private slots:
		void on_General__currentIndexChanged (int);
	};
}
}

#endif
