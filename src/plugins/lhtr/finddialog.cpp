/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2013  Georg Rudoy
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

#include "finddialog.h"
#include <QWebView>
#include <QWebFrame>
#include <QTextEdit>
#include <interfaces/core/ientitymanager.h>
#include <util/util.h>

namespace LeechCraft
{
namespace LHTR
{
	FindObjectProxy::FindObjectProxy (QWebView *view)
	: View_ (view)
	, HTML_ (0)
	{
	}

	FindObjectProxy::FindObjectProxy (QTextEdit *edit)
	: View_ (0)
	, HTML_ (edit)
	{
	}

	void FindObjectProxy::SetProxy (ICoreProxy_ptr proxy)
	{
		Proxy_ = proxy;
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

		Alt<void> ([&str, viewFlags] (QWebView *view) { view->page ()->findText (str, viewFlags); },
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

		Alt<void> ([&str, viewFlags] (QWebView *view) { view->page ()->findText (str, viewFlags); },
				[&str, htmlFlags] (QTextEdit *edit) { edit->find (str, htmlFlags); } );
	}

	void FindObjectProxy::Replace (const QString& text, const QString& with, bool cs, bool all)
	{
		const auto& origHtml = Alt<QString> ([] (QWebView *view) { return view->page ()->mainFrame ()->toHtml (); },
				[] (QTextEdit *edit) { return edit->toPlainText (); });
		auto html = origHtml;

		auto csFlag = cs ? Qt::CaseSensitive : Qt::CaseInsensitive;
		if (all)
		{
			const int encounters = html.count (text, csFlag);
			const auto e = Util::MakeNotification (FindDialog::tr ("Text editor"),
					FindDialog::tr ("%n replacement(s) have been made", 0, encounters),
					PInfo_);
			Proxy_->GetEntityManager ()->HandleEntity (e);

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
					PWarning_);
			Proxy_->GetEntityManager ()->HandleEntity (e);
			return;
		}

		Alt<void> ([&html] (QWebView *view) { view->setHtml (html); },
				[&html] (QTextEdit *edit) { edit->setPlainText (html); });
	}

	FindDialog::FindDialog (const FindObjectProxy& proxy, ICoreProxy_ptr coreProxy, QWidget *parent)
	: QDialog (parent)
	, Proxy_ (proxy)
	{
		Ui_.setupUi (this);
		Proxy_.SetProxy (coreProxy);
		on_FindText__textChanged (QString ());
	}

	void FindDialog::on_FindText__textChanged (const QString& text)
	{
		const bool empty = text.isEmpty ();
		Ui_.Next_->setEnabled (!empty);
		Ui_.Previous_->setEnabled (!empty);

		on_ReplaceText__textChanged (Ui_.ReplaceText_->text ());
	}

	void FindDialog::on_ReplaceText__textChanged (const QString& text)
	{
		const auto& findText = Ui_.FindText_->text ();
		const bool empty = findText.isEmpty () || text.isEmpty ();

		Ui_.Replace_->setEnabled (!empty);
		Ui_.ReplaceAll_->setEnabled (!empty);
	}

	void FindDialog::on_Next__released ()
	{
		Proxy_.Next (Ui_.FindText_->text (), Ui_.CaseSensitive_->checkState () == Qt::Checked);
	}

	void FindDialog::on_Previous__released ()
	{
		Proxy_.Previous (Ui_.FindText_->text (), Ui_.CaseSensitive_->checkState () == Qt::Checked);
	}

	void FindDialog::on_Replace__released ()
	{
		Proxy_.Replace (Ui_.FindText_->text (), Ui_.ReplaceText_->text (),
				Ui_.CaseSensitive_->checkState () == Qt::Checked, false);
	}

	void FindDialog::on_ReplaceAll__released ()
	{
		Proxy_.Replace (Ui_.FindText_->text (), Ui_.ReplaceText_->text (),
				Ui_.CaseSensitive_->checkState () == Qt::Checked, true);
	}
}
}
