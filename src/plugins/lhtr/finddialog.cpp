/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "finddialog.h"
#include <QWebView>
#include <QWebFrame>
#include <QTextEdit>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/ientitymanager.h>
#include <util/xpc/util.h>

namespace LC::LHTR
{
	FindObjectProxy::FindObjectProxy (QWebView *view)
	: View_ { view }
	{
	}

	FindObjectProxy::FindObjectProxy (QTextEdit *edit)
	: HTML_ { edit }
	{
	}

	void FindObjectProxy::Next (const QString& str, bool cs)
	{
		QWebPage::FindFlags viewFlags = QWebPage::FindWrapsAroundDocument;
		QTextDocument::FindFlags htmlFlags;
		if (cs)
		{
			viewFlags |= QWebPage::FindCaseSensitively;
			htmlFlags |= QTextDocument::FindCaseSensitively;
		}

		Alt ([&str, viewFlags] (QWebView *view) { view->page ()->findText (str, viewFlags); },
				[&str, htmlFlags] (QTextEdit *edit) { edit->find (str, htmlFlags); });
	}

	void FindObjectProxy::Previous (const QString& str, bool cs)
	{
		QWebPage::FindFlags viewFlags = QWebPage::FindWrapsAroundDocument | QWebPage::FindBackward;
		QTextDocument::FindFlags htmlFlags = QTextDocument::FindBackward;;
		if (cs)
		{
			viewFlags |= QWebPage::FindCaseSensitively;
			htmlFlags |= QTextDocument::FindCaseSensitively;
		}

		Alt ([&str, viewFlags] (QWebView *view) { view->page ()->findText (str, viewFlags); },
				[&str, htmlFlags] (QTextEdit *edit) { edit->find (str, htmlFlags); } );
	}

	void FindObjectProxy::Replace (const QString& text, const QString& with, bool cs, bool all)
	{
		const auto& origHtml = Alt ([] (QWebView *view) { return view->page ()->mainFrame ()->toHtml (); },
				[] (QTextEdit *edit) { return edit->toPlainText (); });
		auto html = origHtml;

		auto csFlag = cs ? Qt::CaseSensitive : Qt::CaseInsensitive;
		if (all)
		{
			const int encounters = html.count (text, csFlag);
			const auto e = Util::MakeNotification (FindDialog::tr ("Text editor"),
					FindDialog::tr ("%n replacement(s) have been made", 0, encounters),
					Priority::Info);
			GetProxyHolder ()->GetEntityManager ()->HandleEntity (e);

			html.replace (text, with, csFlag);
		}
		else
		{
			const int pos = html.indexOf (text, 0, csFlag);
			if (pos >= 0)
				html.replace (pos, text.size (), with);
		}

		if (origHtml == html)
		{
			const auto& e = Util::MakeNotification (FindDialog::tr ("Text editor"),
					FindDialog::tr ("No replacements were made"),
					Priority::Warning);
			GetProxyHolder ()->GetEntityManager ()->HandleEntity (e);
			return;
		}

		Alt ([&html] (QWebView *view) { view->setHtml (html); },
				[&html] (QTextEdit *edit) { edit->setPlainText (html); });
	}

	FindDialog::FindDialog (const FindObjectProxy& proxy, QWidget *parent)
	: QDialog (parent)
	, Proxy_ (proxy)
	{
		Ui_.setupUi (this);

		for (const auto button : { Ui_.Next_, Ui_.Previous_, Ui_.Replace_, Ui_.ReplaceAll_ })
			button->setEnabled (false);

		auto replaceChangeHandler = [this] (const QString& text)
		{
			const auto& findText = Ui_.FindText_->text ();
			const bool empty = findText.isEmpty () || text.isEmpty ();

			Ui_.Replace_->setEnabled (!empty);
			Ui_.ReplaceAll_->setEnabled (!empty);
		};

		connect (Ui_.ReplaceText_,
				&QLineEdit::textChanged,
				replaceChangeHandler);
		connect (Ui_.FindText_,
				&QLineEdit::textChanged,
				[this, replaceChangeHandler] (const QString& text)
				{
					const bool empty = text.isEmpty ();
					Ui_.Next_->setEnabled (!empty);
					Ui_.Previous_->setEnabled (!empty);

					replaceChangeHandler (Ui_.ReplaceText_->text ());
				});
		connect (Ui_.Next_,
				&QPushButton::released,
				[this] { Proxy_.Next (Ui_.FindText_->text (), IsCaseSensitive ()); });
		connect (Ui_.Previous_,
				&QPushButton::released,
				[this] { Proxy_.Previous (Ui_.FindText_->text (), IsCaseSensitive ()); });
		connect (Ui_.Replace_,
				&QPushButton::released,
				[this] { Proxy_.Replace (Ui_.FindText_->text (), Ui_.ReplaceText_->text (), IsCaseSensitive (), false); });
		connect (Ui_.ReplaceAll_,
				&QPushButton::released,
				[this] { Proxy_.Replace (Ui_.FindText_->text (), Ui_.ReplaceText_->text (), IsCaseSensitive (), true); });
	}

	bool FindDialog::IsCaseSensitive () const
	{
		return Ui_.CaseSensitive_->checkState () == Qt::Checked;
	}
}
