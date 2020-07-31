/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#ifndef XMLSETTINGSDIALOG_RADIOGROUP_H
#define XMLSETTINGSDIALOG_RADIOGROUP_H
#include <QWidget>

class QButtonGroup;
class QRadioButton;

namespace LC
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

