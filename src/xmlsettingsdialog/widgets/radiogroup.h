/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QGroupBox>

class QButtonGroup;
class QRadioButton;
class QVBoxLayout;

namespace LC
{
	class RadioGroup : public QGroupBox
	{
		Q_OBJECT

		QButtonGroup& Group_;
		QVBoxLayout& Layout_;
		QString Value_;

		QHash<QString, QRadioButton*> Value2Button_;
	public:
		explicit RadioGroup (const QString& title, QWidget *parent = nullptr);

		void AddButton (const QString& value, const QString& label, const QString& descr, bool isSelected);
		QString GetValue () const;
		void SetValue (const QString&);
	signals:
		void valueChanged ();
	};
}
