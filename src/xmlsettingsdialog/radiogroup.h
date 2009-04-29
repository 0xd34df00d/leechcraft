/*
    Copyright (c) 2008 by Rudoy Georg <0xd34df00d@gmail.com>

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************
*/
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

