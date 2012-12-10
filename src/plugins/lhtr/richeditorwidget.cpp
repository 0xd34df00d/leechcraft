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

#include "richeditorwidget.h"
#include <functional>
#include <QWebFrame>
#include <QWebPage>
#include <QWebElement>
#include <QToolBar>
#include <QMenu>
#include <QColorDialog>
#include <QFontDialog>
#include <QInputDialog>
#include <QTextDocument>
#include <QXmlStreamWriter>
#include <QNetworkRequest>
#include <QtDebug>
#include <util/util.h>
#include <interfaces/core/ientitymanager.h>
#include "hyperlinkdialog.h"
#include "imagedialog.h"

namespace LeechCraft
{
namespace LHTR
{
	namespace
	{
		class Addable
		{
			enum class Type
			{
				Menu,
				Bar
			} Type_;

			QMenu *Menu_;
			QToolBar *Bar_;
		public:
			explicit Addable (QMenu *menu)
			: Type_ (Type::Menu)
			, Menu_ (menu)
			{
			}

			explicit Addable (QToolBar *bar)
			: Type_ (Type::Bar)
			, Bar_ (bar)
			{
			}

			QAction *addAction (const QString& text, const QObject *receiver, const char *member)
			{
				switch (Type_)
				{
				case Type::Menu:
					return Menu_->addAction (text, receiver, member);
				case Type::Bar:
					return Bar_->addAction (text, receiver, member);
				}

				qWarning () << Q_FUNC_INFO
						<< "unknown addable type";
				return 0;
			}
		};

		class EditorPage : public QWebPage
		{
		public:
			EditorPage (QObject *parent)
			: QWebPage (parent)
			{
			}
		protected:
			bool acceptNavigationRequest (QWebFrame*, const QNetworkRequest& request, NavigationType type)
			{
				if (type == NavigationTypeLinkClicked || type == NavigationTypeOther)
					emit linkClicked (request.url ());
				return false;
			}
		};
	}

	RichEditorWidget::RichEditorWidget (ICoreProxy_ptr proxy, QWidget *parent)
	: QWidget (parent)
	, Proxy_ (proxy)
	, ViewBar_ (0)
	, HTMLDirty_ (false)
	{
		Ui_.setupUi (this);

		Ui_.View_->setPage (new EditorPage (Ui_.View_));
		Ui_.View_->page ()->setContentEditable (true);
		Ui_.View_->settings ()->setAttribute (QWebSettings::DeveloperExtrasEnabled, true);
		Ui_.View_->page ()->setLinkDelegationPolicy (QWebPage::DelegateAllLinks);
		connect (Ui_.View_->page (),
				SIGNAL (linkClicked (QUrl)),
				this,
				SLOT (handleLinkClicked (QUrl)));
		connect (Ui_.View_->page (),
				SIGNAL (selectionChanged ()),
				this,
				SLOT (updateActions ()));

		ViewBar_ = new QToolBar (tr ("Editor bar"));
		qobject_cast<QVBoxLayout*> (Ui_.ViewTab_->layout ())->insertWidget (0, ViewBar_);

		auto fwdCmd = [this] (const QString& name,
				const QString& icon,
				QWebPage::WebAction action,
				Addable addable) -> QAction*
		{
			QAction *act = addable.addAction (name,
					Ui_.View_->pageAction (action), SLOT (trigger ()));
			act->setProperty ("ActionIcon", icon);
			connect (Ui_.View_->pageAction (action),
					SIGNAL (changed ()),
					this,
					SLOT (updateActions ()));
			WebAction2Action_ [action] = act;
			return act;
		};
		auto addCmd = [this] (const QString& name,
				const QString& icon,
				const QString& cmd,
				Addable addable,
				const QString& arg) -> QAction*
		{
			QAction *act = addable.addAction (name, this, SLOT (handleCmd ()));
			act->setProperty ("ActionIcon", icon);
			act->setProperty ("Editor/Command", cmd);
			act->setProperty ("Editor/Args", arg);
			Cmd2Action_ [cmd] [arg] = act;
			return act;
		};

		Addable barAdd (ViewBar_);

		fwdCmd (tr ("Bold"), "format-text-bold",
				QWebPage::ToggleBold, barAdd)->setCheckable (true);
		fwdCmd (tr ("Italic"), "format-text-italic",
				QWebPage::ToggleItalic, barAdd)->setCheckable (true);
		fwdCmd (tr ("Underline"), "format-text-underline",
				QWebPage::ToggleUnderline, barAdd)->setCheckable (true);
		addCmd (tr ("Strikethrough"), "format-text-strikethrough",
				"strikeThrough", barAdd, QString ())->setCheckable (true);
		fwdCmd (tr ("Subscript"), "format-text-subscript",
				QWebPage::ToggleSubscript, barAdd)->setCheckable (true);
		fwdCmd (tr ("Superscript"), "format-text-superscript",
				QWebPage::ToggleSuperscript, barAdd)->setCheckable (true);

		ViewBar_->addSeparator ();

		auto addInlineCmd = [this] (const QString& name,
				const QString& icon,
				const QString& cmd,
				const QStringList& args,
				Addable addable) -> QAction*
		{
			auto act = addable.addAction (name, this, SLOT (handleInlineCmd ()));
			act->setProperty ("ActionIcon", icon);
			act->setProperty ("Editor/Command", cmd);
			act->setProperty ("Editor/Args", args);
			return act;
		};

		addInlineCmd (tr ("Code"), "code-context", "code", QStringList (), barAdd);

		ViewBar_->addSeparator ();

		QList<QAction*> alignActs;
		alignActs << fwdCmd (tr ("Align left"), "format-justify-left",
				QWebPage::AlignLeft, barAdd);
		alignActs << fwdCmd (tr ("Align center"), "format-justify-center",
				QWebPage::AlignCenter, barAdd);
		alignActs << fwdCmd (tr ("Align right"), "format-justify-right",
				QWebPage::AlignRight, barAdd);
		alignActs << fwdCmd (tr ("Align justify"), "format-justify-fill",
				QWebPage::AlignJustified, barAdd);
		QActionGroup *alignGroup = new QActionGroup (this);
		Q_FOREACH (QAction *act, alignActs)
		{
			act->setCheckable (true);
			alignGroup->addAction (act);
		}

		ViewBar_->addSeparator ();

		QMenu *headMenu = new QMenu (tr ("Headings"));
		headMenu->setIcon (Proxy_->GetIcon ("view-list-details"));
		ViewBar_->addAction (headMenu->menuAction ());
		for (int i = 1; i <= 6; ++i)
		{
			const auto& num = QString::number (i);
			addCmd (tr ("Heading %1").arg (i), QString (), "formatBlock",
					Addable (headMenu), "h" + num);
		}
		headMenu->addSeparator ();
		addCmd (tr ("Paragraph"), QString (), "formatBlock",
				Addable (headMenu), "p");

		ViewBar_->addSeparator ();

		QAction *bgColor = ViewBar_->addAction (tr ("Background color..."),
					this,
					SLOT (handleBgColor ()));
		bgColor->setProperty ("ActionIcon", "format-fill-color");

		QAction *fgColor = ViewBar_->addAction (tr ("Text color..."),
					this,
					SLOT (handleFgColor ()));
		fgColor->setProperty ("ActionIcon", "format-text-color");

		QAction *font = ViewBar_->addAction (tr ("Font..."),
					this,
					SLOT (handleFont ()));
		font->setProperty ("ActionIcon", "list-add-font");
		ViewBar_->addSeparator ();

		addCmd (tr ("Indent more"), "format-indent-more", "indent", barAdd, QString ());
		addCmd (tr ("Indent less"), "format-indent-less", "outdent", barAdd, QString ());

		ViewBar_->addSeparator ();

		addCmd (tr ("Ordered list"), "format-list-ordered", "insertOrderedList", barAdd, QString ());
		addCmd (tr ("Unordered list"), "format-list-unordered", "insertUnorderedList", barAdd, QString ());

		ViewBar_->addSeparator ();
		QAction *link = ViewBar_->addAction (tr ("Insert link..."),
					this,
					SLOT (handleInsertLink ()));
		link->setProperty ("ActionIcon", "insert-link");

		QAction *img = ViewBar_->addAction (tr ("Insert image..."),
					this,
					SLOT (handleInsertImage ()));
		img->setProperty ("ActionIcon", "insert-image");
	}

	QString RichEditorWidget::GetContents (ContentType type) const
	{
		switch (type)
		{
		case ContentType::HTML:
		{
			auto body = Ui_.View_->page ()->mainFrame ()->findFirstElement ("body");
			QString xml = body.toOuterXml ();

			const int lb = 10;
			QString init = xml.left (lb);
			init.replace ("body", "div");
			xml.replace (0, lb, init);

			QString tail = xml.right (lb);
			tail.replace ("body", "div");
			xml.chop (lb);
			xml.append (tail);

			return xml;
		}
		case ContentType::PlainText:
			return Ui_.View_->page ()->mainFrame ()->toPlainText ();
		}

		qWarning () << Q_FUNC_INFO
				<< "unknown content type";
		return QString ();
	}

	void RichEditorWidget::SetContents (const QString& contents, ContentType type)
	{
		switch (type)
		{
		case ContentType::HTML:
			Ui_.View_->setHtml (contents);
			break;
		case ContentType::PlainText:
			Ui_.View_->setHtml ("<html><head><meta http-equiv='content-type' content='text/html; charset=utf-8' /><title></title></head><body><pre>" + contents + "</pre></body></html>");
			break;
		}
	}

	void RichEditorWidget::AppendAction (QAction *act)
	{
		ViewBar_->addAction (act);
	}

	void RichEditorWidget::RemoveAction (QAction *act)
	{
		ViewBar_->removeAction (act);
	}

	void RichEditorWidget::InsertHTML (const QString& html)
	{
		ExecCommand ("insertHTML", html);
	}

	void RichEditorWidget::SetTagsMappings (const Replacements_t& rich2html, const Replacements_t& html2rich)
	{
		Rich2HTML_ = rich2html;
		HTML2Rich_ = html2rich;
	}

	void RichEditorWidget::ExecCommand (const QString& cmd, const QString& arg)
	{
		auto frame = Ui_.View_->page ()->mainFrame ();
		const QString& js = arg.isEmpty () ?
				QString ("document.execCommand('%1', false, null)").arg (cmd) :
				QString ("document.execCommand('%1', false, '%2')").arg (cmd, arg);
		frame->evaluateJavaScript (js);
	}

	bool RichEditorWidget::QueryCommandState (const QString& cmd)
	{
		auto frame = Ui_.View_->page ()->mainFrame ();
		const QString& js = QString ("document.queryCommandState(\"%1\", false, null)").arg (cmd);
		auto res = frame->evaluateJavaScript (js);
		return res.toString ().simplified ().toLower () == "true";
	}

	void RichEditorWidget::handleLinkClicked (const QUrl& url)
	{
		const auto& e = Util::MakeEntity (url, QString (), FromUserInitiated | OnlyHandle);
		Proxy_->GetEntityManager ()->HandleEntity (e);
	}

	namespace
	{
		QString ProcessWith (QString html, const IAdvancedHTMLEditor::Replacements_t& rxs)
		{
			for (const auto& rx : rxs)
				html.replace (rx.first, rx.second);
			return html;
		}
	}

	void RichEditorWidget::on_TabWidget__currentChanged (int idx)
	{
		switch (idx)
		{
		case 1:
			Ui_.HTML_->setPlainText (ProcessWith (Ui_.View_->page ()->mainFrame ()->toHtml (), Rich2HTML_));
			break;
		case 0:
			if (HTMLDirty_)
			{
				Ui_.View_->setHtml (ProcessWith (Ui_.HTML_->toPlainText (), HTML2Rich_));
				HTMLDirty_ = false;
			}
			break;
		}
	}

	void RichEditorWidget::on_HTML__textChanged ()
	{
		HTMLDirty_ = true;
	}

	void RichEditorWidget::updateActions ()
	{
		auto upAct = [this] (const QString& cmd)
		{
			Cmd2Action_ [cmd] [QString ()]->setChecked (QueryCommandState (cmd));
		};

		upAct ("strikeThrough");
		upAct ("insertOrderedList");
		upAct ("insertUnorderedList");

		auto upWebAct = [this] (QWebPage::WebAction action)
		{
			WebAction2Action_ [action]->setChecked (Ui_.View_->pageAction (action)->isChecked ());
		};

		upWebAct (QWebPage::ToggleBold);
		upWebAct (QWebPage::ToggleItalic);
		upWebAct (QWebPage::ToggleUnderline);
		upWebAct (QWebPage::ToggleSubscript);
		upWebAct (QWebPage::ToggleSuperscript);

		upWebAct (QWebPage::AlignLeft);
		upWebAct (QWebPage::AlignCenter);
		upWebAct (QWebPage::AlignRight);
		upWebAct (QWebPage::AlignJustified);
	}

	void RichEditorWidget::handleCmd ()
	{
		ExecCommand (sender ()->property ("Editor/Command").toString (),
				sender ()->property ("Editor/Args").toString ());
	}

	void RichEditorWidget::handleInlineCmd ()
	{
		const auto& tag = sender ()->property ("Editor/Command").toString ();
		const auto& args = sender ()->property ("Editor/Args").toStringList ();

		QString jstr;
		jstr += "var selection = window.getSelection().getRangeAt(0);"
				"var selectedText = selection.extractContents();"
				"var span = document.createElement('" + tag + "');";
		for (const auto& arg : args)
			jstr += "span." + arg + ';';
		jstr += "span.appendChild(selectedText);"
				"selection.insertNode(span);";

		auto frame = Ui_.View_->page ()->mainFrame ();
		frame->evaluateJavaScript (jstr);
	}

	void RichEditorWidget::handleBgColor ()
	{
		const auto& color = QColorDialog::getColor (Qt::white, this);
		if (color.isValid ())
			ExecCommand ("hiliteColor", color.name ());
	}

	void RichEditorWidget::handleFgColor ()
	{
		const auto& color = QColorDialog::getColor (Qt::black, this);
		if (color.isValid ())
			ExecCommand ("foreColor", color.name ());
	}

	void RichEditorWidget::handleFont ()
	{
		bool ok = false;
		const QFont& font = QFontDialog::getFont (&ok, this);
		if (!ok)
			return;

		ExecCommand ("fontName", font.family ());

		auto checkWebAct = [this, &font] (std::function<bool (const QFont*)> f,
				QWebPage::WebAction act)
		{
			const bool state = f (&font);
			if (state != WebAction2Action_ [act]->isChecked ())
			{
				WebAction2Action_ [act]->setChecked (state);
				WebAction2Action_ [act]->trigger ();
			}
		};

		checkWebAct (&QFont::bold, QWebPage::ToggleBold);
		checkWebAct (&QFont::italic, QWebPage::ToggleItalic);
		checkWebAct (&QFont::underline, QWebPage::ToggleUnderline);
	}

	void RichEditorWidget::handleInsertLink ()
	{
		if (!Ui_.View_->selectedText ().isEmpty ())
		{
			const QString& url = QInputDialog::getText (this,
					tr ("Insert link"), tr ("Enter URL:"));
			const QUrl& guess = QUrl::fromUserInput (url);
			if (guess.isValid ())
				ExecCommand ("createLink", guess.toString ());

			return;
		}

		HyperlinkDialog dia (this);
		if (dia.exec () != QDialog::Accepted)
			return;

		const QString& link = dia.GetLink ();
		const QString& text = dia.GetText ();
		if (link.isEmpty () || text.isEmpty ())
			return;

		QString html;
		QXmlStreamWriter w (&html);
		w.writeStartElement ("a");
		w.writeAttribute ("href", link);
		if (!dia.GetTitle ().isEmpty ())
			w.writeAttribute ("title", dia.GetTitle ());
		if (!dia.GetTarget ().isEmpty ())
			w.writeAttribute ("target", dia.GetTarget ());
		w.writeCharacters (text);
		w.writeEndElement ();

		ExecCommand ("insertHTML", html);
	}

	void RichEditorWidget::handleInsertImage ()
	{
		ImageDialog dia (this);
		if (dia.exec () != QDialog::Accepted)
			return;

		const QString& path = dia.GetPath ();
		const QUrl& url = QUrl::fromEncoded (path.toUtf8 ());
		const QString& src = url.scheme () == "file" ?
				Util::GetAsBase64Src (QImage (url.toLocalFile ())) :
				path;

		QStringList styles;
		styles << "float:" + dia.GetFloat ();

		QString html;
		QXmlStreamWriter w (&html);
		w.writeStartElement ("img");
		w.writeAttribute ("src", src);
		w.writeAttribute ("alt", dia.GetAlt ());
		if (dia.GetWidth () > 0)
			w.writeAttribute ("width", QString::number (dia.GetWidth ()));
		if (dia.GetHeight () > 0)
			w.writeAttribute ("height", QString::number (dia.GetHeight ()));
		w.writeAttribute ("style", styles.join (";"));
		w.writeEndElement ();

		ExecCommand ("insertHTML", html);
	}
}
}
