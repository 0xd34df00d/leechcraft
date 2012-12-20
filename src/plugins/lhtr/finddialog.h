/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2012  Georg Rudoy
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

#pragma once

#include <functional>
#include <QDialog>
#include <interfaces/core/icoreproxy.h>
#include "ui_finddialog.h"

class QTextEdit;
class QWebView;

namespace LeechCraft
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
