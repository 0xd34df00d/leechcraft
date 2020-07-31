/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <functional>
#include <QDialog>
#include <interfaces/core/icoreproxy.h>
#include "ui_finddialog.h"

class QTextEdit;
class QWebView;

namespace LC
{
namespace LHTR
{
	class FindObjectProxy
	{
		ICoreProxy_ptr Proxy_;

		QWebView *View_;
		QTextEdit *HTML_;
	public:
		FindObjectProxy (QWebView*);
		FindObjectProxy (QTextEdit*);

		void SetProxy (ICoreProxy_ptr);

		void Next (const QString&, bool cs);
		void Previous (const QString&, bool cs);

		void Replace (const QString& text, const QString& with, bool cs, bool all);
	private:
		template<typename T>
		T Alt (std::function<T (QWebView*)> viewF, std::function<T (QTextEdit*)> htmlF)
		{
			return View_ ? viewF (View_) : htmlF (HTML_);
		}
	};

	class FindDialog : public QDialog
	{
		Q_OBJECT

		Ui::FindDialog Ui_;

		FindObjectProxy Proxy_;
	public:
		FindDialog (const FindObjectProxy&, ICoreProxy_ptr, QWidget* = 0);
	private slots:
		void on_FindText__textChanged (const QString&);
		void on_ReplaceText__textChanged (const QString&);

		void on_Next__released ();
		void on_Previous__released ();

		void on_Replace__released ();
		void on_ReplaceAll__released ();
	};
}
}
