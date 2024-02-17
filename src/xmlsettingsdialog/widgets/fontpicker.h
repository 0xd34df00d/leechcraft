/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#ifndef XMLSETTINGSDIALOG_FONTPICKER_H
#define XMLSETTINGSDIALOG_FONTPICKER_H
#include <QWidget>
#include <QFont>

class QLabel;
class QPushButton;

namespace LC
{
	class FontPicker : public QWidget
	{
		Q_OBJECT

		QString Title_;
		QFont Font_;
		QLabel *Label_;
		QPushButton *ChooseButton_;
	public:
		FontPicker (const QString& = QString (), QWidget* = 0);
		void SetCurrentFont (const QFont&);
		QFont GetCurrentFont () const;
	private slots:
		void chooseFont ();
	signals:
		void currentFontChanged (const QFont&);
	};
};

#endif

