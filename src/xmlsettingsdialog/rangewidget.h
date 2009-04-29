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
#ifndef XMLSETTINGSDIALOG_RANGEWIDGET_H
#define XMLSETTINGSDIALOG_RANGEWIDGET_h
#include <QWidget>

class QSpinBox;

namespace LeechCraft
{
	class RangeWidget : public QWidget
	{
		Q_OBJECT

		QSpinBox *Lower_, *Higher_;
	public:
		RangeWidget (QWidget *parent = 0);
		void SetMinimum (int);
		void SetMaximum (int);
		void SetLower (int);
		void SetHigher (int);
		void SetRange (const QVariant&);
		QVariant GetRange () const;
	private slots:
		void lowerChanged (int);
		void upperChanged (int);
	signals:
		void changed ();
	};
};

#endif

