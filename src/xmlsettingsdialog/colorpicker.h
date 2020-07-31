/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QWidget>
#include <QColor>

class QLabel;
class QPushButton;

namespace LC
{
	class ColorPicker : public QWidget
	{
		Q_OBJECT

		const QString Title_;
		QColor Color_;
		QLabel * const Label_;
		QPushButton * const ChooseButton_;
	public:
		ColorPicker (const QString& = {}, QWidget* = nullptr);

		void SetCurrentColor (const QColor&);
		QColor GetCurrentColor () const;
	private slots:
		void chooseColor ();
	signals:
		void currentColorChanged (const QColor&);
	};
}
