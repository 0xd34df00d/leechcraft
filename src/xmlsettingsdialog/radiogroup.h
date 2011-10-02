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

#ifndef XMLSETTINGSDIALOG_RADIOGROUP_H
#define XMLSETTINGSDIALOG_RADIOGROUP_H
#include <QWidget>

class QButtonGroup;
class QRadioButton;

namespace LeechCraft
{
	class RadioGroup : public QWidget
	{
		Q_OBJECT

		QString Value_;
		QButtonGroup *Group_;
	public:
		RadioGroup (QWidget *parent = 0);
		void AddButton (QRadioButton*, bool def = false);
		QString GetValue () const;
		void SetValue (const QString&);
	private slots:
		void handleToggled (bool);
	signals:
		void valueChanged ();
	};
};

#endif

