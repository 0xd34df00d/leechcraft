/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "finddialog.h"
#include <QWebEnginePage>
#include <QWebEngineView>
#include <QTextEdit>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/ientitymanager.h>
#include <util/xpc/util.h>

namespace LC::LHTR
{
	FindObjectProxy::FindObjectProxy (QWebEngineView *view)
	: View_ { view }
	{
	}

	FindObjectProxy::FindObjectProxy (QTextEdit *edit)
	: HTML_ { edit }
	{
	}

	void FindObjectProxy::Next (const QString& str, bool cs)
	{
		QWebEnginePage::FindFlags viewFlags {};
		QTextDocument::FindFlags htmlFlags {};
		if (cs)
		{
			viewFlags |= QWebEnginePage::FindCaseSensitively;
			htmlFlags |= QTextDocument::FindCaseSensitively;
		}

		Alt ([&str, viewFlags] (QWebEngineView *view) { view->findText (str, viewFlags); },
				[&str, htmlFlags] (QTextEdit *edit) { edit->find (str, htmlFlags); });
	}

	void FindObjectProxy::Previous (const QString& str, bool cs)
	{
		QWebEnginePage::FindFlags viewFlags = QWebEnginePage::FindBackward;
		QTextDocument::FindFlags htmlFlags = QTextDocument::FindBackward;
		if (cs)
		{
			viewFlags |= QWebEnginePage::FindCaseSensitively;
			htmlFlags |= QTextDocument::FindCaseSensitively;
		}

		Alt ([&str, viewFlags] (QWebEngineView *view) { view->findText (str, viewFlags); },
				[&str, htmlFlags] (QTextEdit *edit) { edit->find (str, htmlFlags); } );
	}

	namespace
	{
		std::optional<QString> HandleHtml (const QString& text, const QString& with, bool cs, bool all, const QString& origHtml)
		{
			auto html = origHtml;

			auto csFlag = cs ? Qt::CaseSensitive : Qt::CaseInsensitive;
			if (all)
			{
				const int encounters = html.count (text, csFlag);
				const auto e = Util::MakeNotification (FindDialog::tr ("Text editor"),
						FindDialog::tr ("%n replacement(s) have been made", nullptr, encounters),
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
				return {};
			}

			return html;
		}
	}

	void FindObjectProxy::Replace (const QString& text, const QString& with, bool cs, bool all)
	{
		auto viewWorker = [&] (QWebEngineView *view)
		{
			QPointer viewPtr { view };

			view->page ()->toHtml ([=] (const QString& origHtml)
					{
						if (auto maybeHtml = HandleHtml (text, with, cs, all, origHtml);
							maybeHtml && viewPtr)
							viewPtr->page ()->setHtml (*maybeHtml);
					});
		};
		auto editWorker = [&] (QTextEdit *edit)
		{
			if (auto maybeHtml = HandleHtml (text, with, cs, all, edit->toPlainText ()))
				edit->setPlainText (*maybeHtml);
		};

		Alt (viewWorker, editWorker);
	}

	FindDialog::FindDialog (FindObjectProxy proxy, QWidget *parent)
	: QDialog { parent }
	, Proxy_ { proxy }
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
