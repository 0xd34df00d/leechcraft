/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QWidget>

class QLineEdit;
class QPushButton;

namespace LC
{
	class FilePicker : public QWidget
	{
		Q_OBJECT

		QLineEdit& LineEdit_;
		QPushButton& BrowseButton_;
		bool ClearOnCancel_ = false;
		QString Filter_;
	public:
		enum class Type
		{
			ExistingDirectory,
			OpenFileName,
			SaveFileName
		};
	private:
		const Type Type_;
	public:
		explicit FilePicker (Type = Type::ExistingDirectory, QWidget* = nullptr);

		void SetText (const QString&);
		QString GetText () const;

		void SetClearOnCancel (bool);

		void SetFilter (const QString&);
	private:
		void ChooseFile ();
	signals:
		void textChanged (const QString&);
	};
}
