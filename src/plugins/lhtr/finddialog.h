/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QDialog>
#include "ui_finddialog.h"

class QTextEdit;
class QWebEngineView;

namespace LC::LHTR
{
	class FindObjectProxy
	{
		QWebEngineView * const View_ = nullptr;
		QTextEdit * const HTML_ = nullptr;
	public:
		explicit FindObjectProxy (QWebEngineView*);
		explicit FindObjectProxy (QTextEdit*);

		void Next (const QString&, bool cs);
		void Previous (const QString&, bool cs);

		void Replace (const QString& text, const QString& with, bool cs, bool all);
	private:
		auto Alt (auto viewF, auto htmlF)
		{
			return View_ ? viewF (View_) : htmlF (HTML_);
		}
	};

	class FindDialog : public QDialog
	{
		Ui::FindDialog Ui_;
		FindObjectProxy Proxy_;
	public:
		explicit FindDialog (const FindObjectProxy&, QWidget* = nullptr);
	private:
		bool IsCaseSensitive () const;
	};
}
