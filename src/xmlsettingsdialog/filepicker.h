/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#ifndef XMLSETTINGSDIALOG_FILEPICKER_H
#define XMLSETTINGSDIALOG_FILEPICKER_H
#include <QWidget>

class QLineEdit;
class QPushButton;

namespace LC
{
	class FilePicker : public QWidget
	{
		Q_OBJECT

		QLineEdit *LineEdit_;
		QPushButton *BrowseButton_;
		bool ClearOnCancel_;
		QString Filter_;
	public:
		enum class Type
		{
			ExistingDirectory,
			OpenFileName,
			SaveFileName
		};
	private:
		Type Type_;
	public:
		FilePicker (Type = Type::ExistingDirectory, QWidget* = 0);
		void SetText (QString);
		QString GetText () const;
		void SetClearOnCancel (bool);
		void SetFilter (const QString&);
	private slots:
		void chooseFile ();
	signals:
		void textChanged (const QString&);
	};
};

#endif

