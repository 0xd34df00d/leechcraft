/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "richeditorwidget.h"
#include <functional>
#include <map>
#include <QWebFrame>
#include <QWebPage>
#include <QWebElement>
#include <QToolBar>
#include <QMenu>
#include <QColorDialog>
#include <QFontDialog>
#include <QInputDialog>
#include <QTextDocument>
#include <QToolButton>
#include <QKeyEvent>
#include <QXmlStreamWriter>
#include <QNetworkRequest>
#include <QtDebug>
#include <QDomDocument>

#ifdef WITH_HTMLTIDY
#include <tidy.h>
#include <tidybuffio.h>
#endif

#include <util/util.h>
#include <util/xpc/util.h>
#include <util/sll/util.h>
#include <util/sll/unreachable.h>
#include <interfaces/core/ientitymanager.h>
#include <interfaces/core/ipluginsmanager.h>
#include <interfaces/core/iiconthememanager.h>
#include <interfaces/data/iimgsource.h>
#include "hyperlinkdialog.h"
#include "imagedialog.h"
#include "finddialog.h"
#include "inserttabledialog.h"
#include "xmlsettingsmanager.h"
#include "htmlhighlighter.h"
#include "imagecollectiondialog.h"

namespace LC
{
namespace LHTR
{
	const QString MIMEType = "application/xhtml+xml";

	namespace
	{
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

		enum TabWidgetIdx
		{
			TWIVisualView,
			TWIHTMLView
		};
	}

	RichEditorWidget::RichEditorWidget (ICoreProxy_ptr proxy, QWidget *parent)
	: QWidget (parent)
	, Proxy_ (proxy)
	, ViewBar_ (new QToolBar (tr ("Editor bar")))
	, FindAction_ (new QAction (tr ("Find"), this))
	, ReplaceAction_ (new QAction (tr ("Replace"), this))
	{
		FindAction_->setProperty ("ActionIcon", "edit-find");
		FindAction_->setShortcut (QKeySequence::Find);
		connect (FindAction_,
				SIGNAL (triggered ()),
				this,
				SLOT (handleFind ()));
		ReplaceAction_->setProperty ("ActionIcon", "edit-find-replace");
		ReplaceAction_->setShortcut (QKeySequence::Replace);
		connect (ReplaceAction_,
				SIGNAL (triggered ()),
				this,
				SLOT (handleReplace ()));

		Ui_.setupUi (this);

		connect (Ui_.HTML_,
				SIGNAL (textChanged ()),
				this,
				SIGNAL (textChanged ()));
		connect (Ui_.View_->page (),
				SIGNAL (contentsChanged ()),
				this,
				SIGNAL (textChanged ()));

		handleBgColorSettings ();
		XmlSettingsManager::Instance ().RegisterObject ({ "BgColor", "HTMLBgColor" },
				this, "handleBgColorSettings");

		Ui_.View_->installEventFilter (this);

		Ui_.View_->setPage (new EditorPage (Ui_.View_));
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

		qobject_cast<QVBoxLayout*> (Ui_.ViewTab_->layout ())->insertWidget (0, ViewBar_);

		auto fwdCmd = [this] (const QString& name,
				const QString& icon,
				QWebPage::WebAction action,
				auto addable)
		{
			auto act = addable->addAction (name, Ui_.View_->pageAction (action), SLOT (trigger ()));
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
				auto addable,
				const QString& arg)
		{
			auto act = addable->addAction (name, this, SLOT (handleCmd ()));
			act->setProperty ("ActionIcon", icon);
			act->setProperty ("Editor/Command", cmd);
			act->setProperty ("Editor/Args", arg);
			Cmd2Action_ [cmd] [arg] = act;
			return act;
		};

		Bold_ = fwdCmd (tr ("Bold"), "format-text-bold",
				QWebPage::ToggleBold, ViewBar_);
		Bold_->setCheckable (true);
		Italic_ = fwdCmd (tr ("Italic"), "format-text-italic",
				QWebPage::ToggleItalic, ViewBar_);
		Italic_->setCheckable (true);
		Underline_ = fwdCmd (tr ("Underline"), "format-text-underline",
				QWebPage::ToggleUnderline, ViewBar_);
		Underline_->setCheckable (true);

		addCmd (tr ("Strikethrough"), "format-text-strikethrough",
				"strikeThrough", ViewBar_, QString ())->setCheckable (true);
		fwdCmd (tr ("Subscript"), "format-text-subscript",
				QWebPage::ToggleSubscript, ViewBar_)->setCheckable (true);
		fwdCmd (tr ("Superscript"), "format-text-superscript",
				QWebPage::ToggleSuperscript, ViewBar_)->setCheckable (true);

		auto addInlineCmd = [this] (const QString& name,
				const QString& icon,
				const QString& cmd,
				auto addable)
		{
			auto act = addable->addAction (name, this, SLOT (handleInlineCmd ()));
			act->setProperty ("ActionIcon", icon);
			act->setProperty ("Editor/Command", cmd);
			return act;
		};

		addInlineCmd (tr ("Code"), "code-context", "code", ViewBar_);

		ViewBar_->addSeparator ();

		auto alignActs =
		{
			fwdCmd (tr ("Align left"), "format-justify-left", QWebPage::AlignLeft, ViewBar_),
			fwdCmd (tr ("Align center"), "format-justify-center", QWebPage::AlignCenter, ViewBar_),
			fwdCmd (tr ("Align right"), "format-justify-right", QWebPage::AlignRight, ViewBar_),
			fwdCmd (tr ("Align justify"), "format-justify-fill", QWebPage::AlignJustified, ViewBar_)
		};
		QActionGroup *alignGroup = new QActionGroup (this);
		for (QAction *act : alignActs)
		{
			act->setCheckable (true);
			alignGroup->addAction (act);
		}

		ViewBar_->addSeparator ();

		QMenu *headMenu = new QMenu (tr ("Headings"));
		headMenu->setIcon (Proxy_->GetIconThemeManager ()->GetIcon ("view-list-details"));
		ViewBar_->addAction (headMenu->menuAction ());
		for (int i = 1; i <= 6; ++i)
		{
			const auto& num = QString::number (i);
			addCmd (tr ("Heading %1").arg (i), QString (), "formatBlock", headMenu, "h" + num);
		}
		headMenu->addSeparator ();
		addCmd (tr ("Paragraph"), QString (), "formatBlock", headMenu, "p");

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

		addCmd (tr ("Mark as quote"), "mail-reply-sender", "formatBlock", ViewBar_, "blockquote");

		ViewBar_->addSeparator ();

		addCmd (tr ("Indent more"), "format-indent-more", "indent", ViewBar_, QString ());
		addCmd (tr ("Indent less"), "format-indent-less", "outdent", ViewBar_, QString ());

		ViewBar_->addSeparator ();

		addCmd (tr ("Ordered list"), "format-list-ordered", "insertOrderedList", ViewBar_, QString ());
		addCmd (tr ("Unordered list"), "format-list-unordered", "insertUnorderedList", ViewBar_, QString ());

		ViewBar_->addSeparator ();

		InsertLink_ = ViewBar_->addAction (tr ("Insert link..."),
					this,
					SLOT (handleInsertLink ()));
		InsertLink_->setProperty ("ActionIcon", "insert-link");

		SetupImageMenu ();
		SetupTableMenu ();

		setupJS ();
		connect (Ui_.View_->page ()->mainFrame (),
				SIGNAL (javaScriptWindowObjectCleared ()),
				this,
				SLOT (setupJS ()));

		ToggleView_ = new QAction (tr ("Toggle view"), this);
		connect (ToggleView_,
				SIGNAL (triggered ()),
				this,
				SLOT (toggleView ()));

		SetContents ("", ContentType::HTML);

		new HtmlHighlighter (Ui_.HTML_->document ());

		setFocusProxy (Ui_.View_);
	}

	QString RichEditorWidget::GetContents (ContentType type) const
	{
		switch (type)
		{
		case ContentType::HTML:
			if (Ui_.TabWidget_->currentIndex () != TabWidgetIdx::TWIHTMLView)
				SyncHTMLToView ();
			return Ui_.HTML_->toPlainText ();
		case ContentType::PlainText:
			return Ui_.View_->page ()->mainFrame ()->toPlainText ();
		}

		Util::Unreachable ();
	}

	void RichEditorWidget::SetContents (QString contents, ContentType type)
	{
		QString content;
		content += "<!DOCTYPE html PUBLIC";
		content += "	\"-//W3C//DTD XHTML 1.0 Transitional//EN\" \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd\">";
		content += "	<html dir=\"ltr\" xmlns=\"http://www.w3.org/1999/xhtml\">";
		content += "<head><title></title></head><body>";
		switch (type)
		{
		case ContentType::HTML:
			content += contents;
			break;
		case ContentType::PlainText:
			contents = contents.toHtmlEscaped ();
			contents.replace ("\r\n", "<br/>");
			contents.replace ("\n", "<br/>");
			contents.replace ("\r", "<br/>");
			content += "<pre>" + contents + "</pre>";
			break;
		}
		content += "</body></html>";

		if (type == ContentType::HTML)
			content = ExpandCustomTags (content);

		Ui_.View_->setContent (content.toUtf8 (), MIMEType);

		setupJS ();
	}

	void RichEditorWidget::AppendAction (QAction *act)
	{
		ViewBar_->addAction (act);
	}

	void RichEditorWidget::AppendSeparator ()
	{
		ViewBar_->addSeparator ();
	}

	void RichEditorWidget::RemoveAction (QAction *act)
	{
		ViewBar_->removeAction (act);
	}

	QAction* RichEditorWidget::GetEditorAction (EditorAction action)
	{
		switch (action)
		{
		case EditorAction::Find:
			return FindAction_;
		case EditorAction::Replace:
			return ReplaceAction_;
		case EditorAction::Bold:
			return Bold_;
		case EditorAction::Italic:
			return Italic_;
		case EditorAction::Underline:
			return Underline_;
		case EditorAction::InsertLink:
			return InsertLink_;
		case EditorAction::InsertImage:
			return InsertImage_;
		case EditorAction::ToggleView:
			return ToggleView_;
		}

		return 0;
	}

	void RichEditorWidget::SetBackgroundColor (const QColor& color, ContentType type)
	{
		if (!XmlSettingsManager::Instance ().property ("OverrideBgColor").toBool ())
			InternalSetBgColor (color, type);
	}

	QWidget* RichEditorWidget::GetWidget ()
	{
		return this;
	}

	QObject* RichEditorWidget::GetQObject ()
	{
		return this;
	}

	void RichEditorWidget::InsertHTML (const QString& html)
	{
		auto expanded = ExpandCustomTags (html, ExpandMode::PartialHTML);

		expanded.replace ('\n', "\\n");
		expanded.replace ('\'', "\\'");

		auto frame = Ui_.View_->page ()->mainFrame ();
		frame->evaluateJavaScript (R"delim(
			var s = window.getSelection();
			if (!s.rangeCount || !s.getRangeAt(0).endContainer)
				document.body.focus();

			var wrapper = document.createElement("div");
			wrapper.innerHTML = ')delim" + expanded + R"delim(';
			var node = wrapper.childNodes[0];

			var textNode = document.createTextNode(' ');
			s.getRangeAt(0).insertNode(textNode);

			s.getRangeAt(0).insertNode(node);

			s.removeAllRanges();

			var r = document.createRange();
			r.setStartAfter(textNode);
			r.setEndAfter(textNode);
			s.addRange(r);
			)delim");
	}

	void RichEditorWidget::SetCustomTags (const CustomTags_t& tags)
	{
		CustomTags_ = tags;
	}

	QAction* RichEditorWidget::AddInlineTagInserter (const QString& tagName, const QVariantMap& params)
	{
		auto act = ViewBar_->addAction (QString (), this, SLOT (handleInlineCmd ()));
		act->setProperty ("Editor/Command", tagName);
		act->setProperty ("Editor/Attrs", params);
		return act;
	}

	void RichEditorWidget::ExecJS (const QString& js)
	{
		Ui_.View_->page ()->mainFrame ()->evaluateJavaScript (js);
	}

	void RichEditorWidget::SetFontFamily (FontFamily family, const QFont& font)
	{
		Ui_.View_->settings ()->setFontFamily (static_cast<QWebSettings::FontFamily> (family), font.family ());
	}

	void RichEditorWidget::SetFontSize (FontSize font, int size)
	{
		Ui_.View_->settings ()->setFontSize (static_cast<QWebSettings::FontSize> (font), size);
	}

	bool RichEditorWidget::eventFilter (QObject*, QEvent *event)
	{
		if (event->type () != QEvent::KeyPress && event->type () != QEvent::KeyRelease)
			return false;

		auto keyEv = static_cast<QKeyEvent*> (event);
		if (keyEv->key () != Qt::Key_Tab)
			return false;

		auto frame = Ui_.View_->page ()->mainFrame ();
		const auto isParagraph = frame->evaluateJavaScript ("findParent(window.getSelection().getRangeAt(0).endContainer, 'p') != null");
		if (!isParagraph.toBool ())
			return false;

		if (event->type () == QEvent::KeyRelease)
			return true;

		QString js;
		js += "var p = findParent(window.getSelection().getRangeAt(0).endContainer, 'p');";
		js += "p.style.textIndent = '2em';";

		frame->evaluateJavaScript (js);

		return true;
	}

	void RichEditorWidget::InternalSetBgColor (const QColor& color, ContentType type)
	{
		QWidget *widget = 0;
		switch (type)
		{
		case ContentType::PlainText:
			widget = Ui_.HTML_;
			break;
		case ContentType::HTML:
			widget = Ui_.View_;
			break;
		}

		auto palette = widget->palette ();
		palette.setColor (QPalette::Base, color);
		widget->setPalette (palette);
	}

	void RichEditorWidget::SetupImageMenu ()
	{
		auto imagesMenu = new QMenu (tr ("Insert image..."), this);

		auto imagesButton = new QToolButton;
		imagesButton->setMenu (imagesMenu);
		imagesButton->setPopupMode (QToolButton::InstantPopup);
		imagesButton->setIcon (Proxy_->GetIconThemeManager ()->GetIcon ("insert-image"));
		ViewBar_->addWidget (imagesButton);

		InsertImage_ = imagesMenu->addAction (tr ("Insert image by link..."),
					this,
					SLOT (handleInsertImage ()));

		auto fromCollection = imagesMenu->addMenu (tr ("Insert image from collection"));

		bool added = false;
		for (const auto pluginObj : Proxy_->GetPluginsManager ()->GetAllCastableRoots<IImgSource*> ())
		{
			const auto plugin = qobject_cast<IImgSource*> (pluginObj);
			for (const auto& service : plugin->GetServices ())
			{
				const auto act = fromCollection->addAction (service.Name_,
						this,
						SLOT (handleInsertImageFromCollection ()));
				act->setProperty ("LHTR/Plugin", QVariant::fromValue (pluginObj));
				act->setProperty ("LHTR/Service", service.ID_);
				added = true;
			}
		}

		fromCollection->setEnabled (added);
	}

	void RichEditorWidget::SetupTableMenu ()
	{
		auto tablesMenu = new QMenu (tr ("Tables..."), this);

		auto tablesButton = new QToolButton;
		tablesButton->setMenu (tablesMenu);
		tablesButton->setPopupMode (QToolButton::InstantPopup);
		tablesButton->setIcon (Proxy_->GetIconThemeManager ()->GetIcon ("view-form-table"));
		ViewBar_->addWidget (tablesButton);

		auto table = tablesMenu->addAction (tr ("Insert table..."),
					this,
					SLOT (handleInsertTable ()));
		table->setProperty ("ActionIcon", "insert-table");

		tablesMenu->addSeparator ();

		auto addRowAbove = tablesMenu->addAction (tr ("Insert row above"),
					this,
					SLOT (handleInsertRow ()));
		addRowAbove->setProperty ("ActionIcon", "edit-table-insert-row-above");
		addRowAbove->setProperty ("LHTR/Shift", 0);

		auto addRowBelow = tablesMenu->addAction (tr ("Insert row below"),
					this,
					SLOT (handleInsertRow ()));
		addRowBelow->setProperty ("ActionIcon", "edit-table-insert-row-below");
		addRowBelow->setProperty ("LHTR/Shift", 1);

		auto addColumnLeft = tablesMenu->addAction (tr ("Insert column to the left"),
					this,
					SLOT (handleInsertColumn ()));
		addColumnLeft->setProperty ("ActionIcon", "edit-table-insert-column-left");
		addColumnLeft->setProperty ("LHTR/Shift", 0);

		auto addColumnRight = tablesMenu->addAction (tr ("Insert column to the right"),
					this,
					SLOT (handleInsertColumn ()));
		addColumnRight->setProperty ("ActionIcon", "edit-table-insert-column-right");
		addColumnRight->setProperty ("LHTR/Shift", 1);

		tablesMenu->addSeparator ();

		auto removeRow = tablesMenu->addAction (tr ("Remove row"),
					this,
					SLOT (handleRemoveRow ()));
		removeRow->setProperty ("ActionIcon", "edit-table-delete-row");

		auto removeColumn = tablesMenu->addAction (tr ("Remove column"),
					this,
					SLOT (handleRemoveColumn ()));
		removeColumn->setProperty ("ActionIcon", "edit-table-delete-column");
	}

	void RichEditorWidget::ExecCommand (const QString& cmd, QString arg)
	{
		if (cmd == "insertHTML")
		{
			InsertHTML (arg);
			return;
		}

		auto frame = Ui_.View_->page ()->mainFrame ();
		const QString& js = arg.isEmpty () ?
				QString ("document.execCommand('%1', false, null)").arg (cmd) :
				QString ("document.execCommand('%1', false, '%2')").arg (cmd, arg.replace ('\n', "\\n"));
		frame->evaluateJavaScript (js);
	}

	bool RichEditorWidget::QueryCommandState (const QString& cmd)
	{
		auto frame = Ui_.View_->page ()->mainFrame ();
		const QString& js = QString ("document.queryCommandState(\"%1\", false, null)").arg (cmd);
		auto res = frame->evaluateJavaScript (js);
		return res.toString ().simplified ().toLower () == "true";
	}

	void RichEditorWidget::OpenFindReplace (bool)
	{
		const bool isView = Ui_.TabWidget_->currentIndex () == 0;

		auto dia = isView ?
				new FindDialog (Ui_.View_, Proxy_, this) :
				new FindDialog (Ui_.HTML_, Proxy_, this);
		dia->setAttribute (Qt::WA_DeleteOnClose);
		dia->show ();
	}

	namespace
	{
		void TryFixHTML (QString& html, bool prependDoctype)
		{
#ifdef WITH_HTMLTIDY
			TidyBuffer output {};

			auto tdoc = tidyCreate ();

			const auto guard = Util::MakeScopeGuard ([&tdoc, &output]
					{
						tidyRelease (tdoc);
						tidyBufFree (&output);
					});

			if (!tidyOptSetBool (tdoc, TidyXmlOut, yes) ||
					!tidyOptSetBool (tdoc, TidyForceOutput, yes) ||
					!tidyOptSetValue (tdoc, TidyCharEncoding, "utf8"))
			{
				qWarning () << Q_FUNC_INFO
						<< "cannot set xhtml output";
				return;
			}

			tidyOptSetInt (tdoc, TidyWrapLen, std::numeric_limits<int>::max ());

			if (tidyParseString (tdoc, html.toUtf8 ().constData ()) < 0)
			{
				qWarning () << Q_FUNC_INFO
						<< "failed to parse"
						<< html;
				return;
			}

			if (tidyCleanAndRepair (tdoc) < 0)
			{
				qWarning () << Q_FUNC_INFO
						<< "failed to clean and repair"
						<< html;
				return;
			}

			if (tidySaveBuffer (tdoc, &output) < 0)
			{
				qWarning () << Q_FUNC_INFO
						<< "cannot save buffer";
				return;
			}

			html = QString::fromUtf8 (reinterpret_cast<char*> (output.bp));
#endif

			if (prependDoctype && !html.startsWith ("<!DOCTYPE "))
			{
				html.prepend ("	\"-//W3C//DTD XHTML 1.0 Transitional//EN\" \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd\">");
				html.prepend ("<!DOCTYPE html PUBLIC");
			}
		}
	}

	QString RichEditorWidget::ExpandCustomTags (QString html, ExpandMode mode) const
	{
		TryFixHTML (html, mode == ExpandMode::FullHTML);
		if (CustomTags_.isEmpty ())
			return html;

		html.remove ('\n');

		QDomDocument doc;
		if (!doc.setContent (html))
		{
			qWarning () << Q_FUNC_INFO
					<< "unable to set content from"
					<< html;
			return html;
		}

		if (!doc.documentElement ().hasAttribute ("xmlns"))
			doc.documentElement ().setAttribute ("xmlns", "http://www.w3.org/1999/xhtml");

		for (const auto& tag : CustomTags_)
		{
			const auto& elems = doc.elementsByTagName (tag.TagName_);
			for (int i = elems.size () - 1; i >= 0; --i)
			{
				auto elem = elems.at (i).toElement ();

				QString origContents;
				QTextStream str (&origContents);
				elem.save (str, 1);

				tag.ToKnown_ (elem);

				elem.setAttribute ("__tagname__", tag.TagName_);

				elem.setAttribute ("__original__", origContents.trimmed ());
				if (!tag.FromKnown_)
					elem.setAttribute ("contenteditable", "false");
			}
		}

		return doc.toString (-1);
	}

	QString RichEditorWidget::RevertCustomTags () const
	{
		auto frame = Ui_.View_->page ()->mainFrame ();
		auto root = frame->documentElement ().clone ();
		for (const auto& tag : CustomTags_)
		{
			const auto& elems = root.findAll ("*[__tagname__='" + tag.TagName_ + "']");
			for (auto elem : elems)
			{
				QDomDocument doc;
				if (!tag.FromKnown_)
					elem.setOuterXml (elem.attribute ("__original__"));
				else if (!doc.setContent (elem.toOuterXml ()))
				{
					qWarning () << Q_FUNC_INFO
							<< "unable to parse"
							<< elem.toOuterXml ();
					elem.setOuterXml (elem.attribute ("__original__"));
				}
				else
				{
					auto docElem = doc.documentElement ();
					const auto& original = docElem.attribute ("__original__");
					docElem.removeAttribute ("__original__");
					docElem.removeAttribute ("__tagname__");

					if (tag.FromKnown_ (docElem))
					{
						QString contents;
						QTextStream str (&contents);
						docElem.save (str, 1);

						elem.setOuterXml (contents);
					}
					else
					{
						qWarning () << Q_FUNC_INFO
								<< "FromKnown_ failed";
						elem.setOuterXml (original);
					}
				}
			}
		}
		return root.toOuterXml ();
	}

	void RichEditorWidget::SyncHTMLToView () const
	{
		const auto frame = Ui_.View_->page ()->mainFrame ();
		const auto& peElem = frame->findFirstElement ("parsererror");

		if (!peElem.isNull ())
		{
			qWarning () << Q_FUNC_INFO
					<< "there are parser errors, ignoring reverting";
			return;
		}

		Ui_.HTML_->setPlainText (RevertCustomTags ());
	}

	void RichEditorWidget::handleBgColorSettings ()
	{
		const auto& color = XmlSettingsManager::Instance ()
				.property ("BgColor").value<QColor> ();
		InternalSetBgColor (color, ContentType::HTML);

		const auto& plainColor = XmlSettingsManager::Instance ()
				.property ("HTMLBgColor").value<QColor> ();
		InternalSetBgColor (plainColor, ContentType::PlainText);
	}

	void RichEditorWidget::handleLinkClicked (const QUrl& url)
	{
		const auto& e = Util::MakeEntity (url, QString (), FromUserInitiated | OnlyHandle);
		Proxy_->GetEntityManager ()->HandleEntity (e);
	}

	void RichEditorWidget::on_TabWidget__currentChanged (int idx)
	{
		disconnect (Ui_.HTML_,
				SIGNAL (textChanged ()),
				this,
				SIGNAL (textChanged ()));
		disconnect (Ui_.View_->page (),
				SIGNAL (contentsChanged ()),
				this,
				SIGNAL (textChanged ()));

		switch (idx)
		{
		case TabWidgetIdx::TWIHTMLView:
			SyncHTMLToView ();
			break;
		case TabWidgetIdx::TWIVisualView:
			if (!HTMLDirty_)
				return;

			HTMLDirty_ = false;
			const auto& str = ExpandCustomTags (Ui_.HTML_->toPlainText ());
			Ui_.View_->setContent (str.toUtf8 (), MIMEType);

			auto frame = Ui_.View_->page ()->mainFrame ();
			if (frame->findFirstElement ("html > body").isNull ())
			{
				qWarning () << Q_FUNC_INFO
						<< "null html/body element";

				SetContents (str, ContentType::HTML);
			}

			setupJS ();

			break;
		}

		connect (Ui_.HTML_,
				SIGNAL (textChanged ()),
				this,
				SIGNAL (textChanged ()));
		connect (Ui_.View_->page (),
				SIGNAL (contentsChanged ()),
				this,
				SIGNAL (textChanged ()));
	}

	void RichEditorWidget::setupJS ()
	{
		auto frame = Ui_.View_->page ()->mainFrame ();
		frame->evaluateJavaScript ("function findParent(item, name)"
				"{"
				"	while (item != null && (item.tagName == null || item.tagName.toLowerCase() != name))"
				"		item = item.parentNode; return item;"
				"}");

		frame->addToJavaScriptWindowObject ("LHTR", this);
		frame->evaluateJavaScript ("var f = function() { window.LHTR.textChanged() }; "
				"window.addEventListener('DOMContentLoaded', f);"
				"window.addEventListener('DOMSubtreeModified', f);"
				"window.addEventListener('DOMAttrModified', f);"
				"window.addEventListener('DOMNodeInserted', f);"
				"window.addEventListener('DOMNodeRemoved', f);");

		frame->findFirstElement ("body").setAttribute ("contenteditable", "true");
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

	void RichEditorWidget::toggleView ()
	{
		if (Ui_.TabWidget_->currentIndex () == 1)
			Ui_.TabWidget_->setCurrentIndex (0);
		else
			Ui_.TabWidget_->setCurrentIndex (1);
	}

	void RichEditorWidget::handleCmd ()
	{
		const auto& command = sender ()->property ("Editor/Command").toString ();
		const auto& args = sender ()->property ("Editor/Args").toString ();

		if (command.toLower () != "formatblock")
		{
			ExecCommand (command, args);
			return;
		}

		QString jstr;
		jstr += "var selection = window.getSelection().getRangeAt(0);"
				"var parentItem = findParent(selection.commonAncestorContainer.parentNode, '" + args + "');"
				"if (parentItem == null) {"
				"	document.execCommand('formatBlock', false, '" + args + "');"
				"} else {"
				"	parentItem.outerHTML = parentItem.innerHTML;"
				"}";

		auto frame = Ui_.View_->page ()->mainFrame ();
		frame->evaluateJavaScript (jstr);
	}

	void RichEditorWidget::handleInlineCmd ()
	{
		const auto& tag = sender ()->property ("Editor/Command").toString ();
		const auto& attrs = sender ()->property ("Editor/Attrs").toMap ();

		QString jstr;
		jstr += "var selection = window.getSelection().getRangeAt(0);"
				"var parentItem = findParent(selection.commonAncestorContainer.parentNode, '" + tag + "');"
				"if (parentItem == null) {"
				"	var selectedText = selection.extractContents();"
				"	var span = document.createElement('" + tag + "');";
		for (auto i = attrs.begin (), end = attrs.end (); i != end; ++i)
			jstr += QString ("	span.setAttribute ('%1', '%2');")
					.arg (i.key ())
					.arg (i->toString ());
		jstr += "	span.appendChild(selectedText);"
				"	selection.insertNode(span);"
				"} else {"
				"	parentItem.outerHTML = parentItem.innerHTML;"
			"}";

		auto frame = Ui_.View_->page ()->mainFrame ();
		frame->evaluateJavaScript (jstr);

		const auto& fullHtml = frame->documentElement ().toOuterXml ();
		Ui_.View_->setContent (ExpandCustomTags (fullHtml).toUtf8 (), MIMEType);
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

	void RichEditorWidget::handleInsertTable ()
	{
		InsertTableDialog dia;
		if (dia.exec () != QDialog::Accepted)
			return;

		QString html;
		QXmlStreamWriter w (&html);
		w.writeStartElement ("table");
		w.writeAttribute ("style", "border: 1px solid black; border-collapse: collapse;");

		const auto& caption = dia.GetCaption ().trimmed ();
		if (!caption.isEmpty ())
		{
			w.writeStartElement ("caption");
			w.writeCharacters (caption);
			w.writeEndElement ();
		}

		w.writeStartElement ("tbody");
		for (int i = 0; i < dia.GetRows (); ++i)
		{
			w.writeStartElement ("tr");
			for (int j = 0; j < dia.GetColumns (); ++j)
			{
				w.writeStartElement ("td");
				w.writeAttribute ("style", "border: 1px solid black; min-width: 1em; height: 1.5em;");
				w.writeEndElement ();
			}
			w.writeEndElement ();
		}
		w.writeEndElement ();
		w.writeEndElement ();
		ExecCommand ("insertHTML", html);
	}

	void RichEditorWidget::handleInsertRow ()
	{
		auto shift = sender ()->property ("LHTR/Shift").toInt ();

		QString js;
		js += "var row = findParent(window.getSelection().getRangeAt(0).endContainer, 'tr');";
		js += "var rowIdx = row.rowIndex;";
		js += "var table = findParent(row, 'table');";
		js += "var newRow = table.insertRow(rowIdx + " + QString::number (shift) + ");";
		js += "for (var j = 0; j < row.cells.length; ++j)";
		js += "{";
		js += "    var newCell = newRow.insertCell(j);";
		js += "    newCell.setAttribute('style', 'border: 1px solid black; min-width: 1em; height: 1.5em;');";
		js += "}";

		Ui_.View_->page ()->mainFrame ()->evaluateJavaScript (js);
	}

	void RichEditorWidget::handleInsertColumn ()
	{
		auto shift = sender ()->property ("LHTR/Shift").toInt ();

		QString js;
		js += "var cell = findParent(window.getSelection().getRangeAt(0).endContainer, 'td');";
		js += "var colIdx = cell.cellIndex + " + QString::number (shift) + ";";
		js += "var table = findParent(cell, 'table');";
		js += "for (var r = 0; r < table.rows.length; ++r)";
		js += "{";
		js += "    var newCell = table.rows[r].insertCell(colIdx);";
		js += "    newCell.setAttribute('style', 'border: 1px solid black; min-width: 1em; height: 1.5em;');";
		js += "}";

		Ui_.View_->page ()->mainFrame ()->evaluateJavaScript (js);
	}

	void RichEditorWidget::handleRemoveRow ()
	{
		QString js;
		js += "var row = findParent(window.getSelection().getRangeAt(0).endContainer, 'tr');";
		js += "var table = findParent(row, 'table');";
		js += "table.deleteRow(row.rowIndex);";

		Ui_.View_->page ()->mainFrame ()->evaluateJavaScript (js);
	}

	void RichEditorWidget::handleRemoveColumn ()
	{
		QString js;
		js += "var cell = findParent(window.getSelection().getRangeAt(0).endContainer, 'td');";
		js += "var colIdx = cell.cellIndex;";
		js += "var table = findParent(cell, 'table');";
		js += "for (var r = 0; r < table.rows.length; ++r)";
		js += "    table.rows[r].deleteCell(colIdx);";

		Ui_.View_->page ()->mainFrame ()->evaluateJavaScript (js);
	}

	void RichEditorWidget::handleInsertLink ()
	{
		if (Ui_.View_->hasSelection ())
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

	namespace
	{
		QUrl GetPreviewUrl (const RemoteImageInfo& info, ImageCollectionDialog::PreviewSize size)
		{
			switch (size)
			{
			case ImageCollectionDialog::PreviewSize::None:
				return {};
			case ImageCollectionDialog::PreviewSize::Thumb:
				return info.Thumb_;
			case ImageCollectionDialog::PreviewSize::Preview:
				return info.Preview_;
			case ImageCollectionDialog::PreviewSize::Full:
				return info.Full_;
			}

			qWarning () << Q_FUNC_INFO
					<< "unknown preview size"
					<< static_cast<int> (size);
			return {};
		}

		QSize GetPreviewSize (const RemoteImageInfo& info, ImageCollectionDialog::PreviewSize size)
		{
			switch (size)
			{
			case ImageCollectionDialog::PreviewSize::None:
				return {};
			case ImageCollectionDialog::PreviewSize::Thumb:
				return info.ThumbSize_;
			case ImageCollectionDialog::PreviewSize::Preview:
				return info.PreviewSize_;
			case ImageCollectionDialog::PreviewSize::Full:
				return info.FullSize_;
			}

			qWarning () << Q_FUNC_INFO
					<< "unknown preview size"
					<< static_cast<int> (size);
			return {};
		}
	}

	void RichEditorWidget::handleCollectionImageChosen ()
	{
		const auto chooser = qobject_cast<IPendingImgSourceRequest*> (sender ());
		const auto& rawInfos = chooser->GetInfos ();
		if (rawInfos.isEmpty ())
			return;

		ImageCollectionDialog dia { rawInfos, this };
		if (dia.exec () != QDialog::Accepted)
			return;

		const bool linkPreviews = dia.PreviewsAreLinks ();
		const auto previewSize = dia.GetPreviewSize ();

		QString html;
		QXmlStreamWriter w (&html);
		w.writeStartElement ("span");

		bool displayBlock = false;
		QString floatPos;
		QString align;
		switch (dia.GetPosition ())
		{
		case ImageCollectionDialog::Position::Center:
			align = "center";
			displayBlock = true;
			break;
		case ImageCollectionDialog::Position::Right:
			displayBlock = true;
			align = "right";
			break;
		case ImageCollectionDialog::Position::Left:
			displayBlock = true;
			align = "left";
			break;
		case ImageCollectionDialog::Position::RightWrap:
			floatPos = "right";
			break;
		case ImageCollectionDialog::Position::LeftWrap:
			floatPos = "left";
			break;
		}

		QStringList styleElems;
		if (displayBlock)
			styleElems << "display: block";
		if (!floatPos.isEmpty ())
			styleElems << "float: " + floatPos;
		if (!align.isEmpty ())
			styleElems << "text-align: " + align;

		if (!styleElems.isEmpty ())
			w.writeAttribute ("style", styleElems.join ("; "));

		for (const auto& image : dia.GetInfos ())
		{
			if (linkPreviews)
			{
				w.writeStartElement ("a");
				w.writeAttribute ("href", image.Full_.toString ());
			}

			w.writeStartElement ("img");
			w.writeAttribute ("src", GetPreviewUrl (image, previewSize).toString ());
			w.writeAttribute ("alt", image.Title_);

			const auto& size = GetPreviewSize (image, previewSize);
			if (size.isValid ())
			{
				w.writeAttribute ("width", QString::number (size.width ()));
				w.writeAttribute ("height", QString::number (size.height ()));
			}

			w.writeEndElement ();

			if (linkPreviews)
				w.writeEndElement ();

			w.writeEmptyElement ("br");
		}
		w.writeEndElement ();

		ExecCommand ("insertHTML", html);
	}

	void RichEditorWidget::handleInsertImageFromCollection ()
	{
		const auto pluginObj = sender ()->property ("LHTR/Plugin").value<QObject*> ();
		const auto& service = sender ()->property ("LHTR/Service").toByteArray ();

		const auto plugin = qobject_cast<IImgSource*> (pluginObj);
		const auto chooser = plugin->RequestImages (service);
		connect (chooser->GetQObject (),
				SIGNAL (ready ()),
				this,
				SLOT (handleCollectionImageChosen ()));
	}

	void RichEditorWidget::handleFind ()
	{
		OpenFindReplace (true);
	}

	void RichEditorWidget::handleReplace ()
	{
		OpenFindReplace (false);
	}
}
}
