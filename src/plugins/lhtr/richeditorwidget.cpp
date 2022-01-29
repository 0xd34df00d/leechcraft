/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "richeditorwidget.h"
#include <memory>
#include <optional>
#include <QWebEnginePage>
#include <QWebEngineScriptCollection>
#include <QWebEngineSettings>
#include <QWebEngineUrlRequestInterceptor>
#include <QWebEngineView>
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
#include <QFutureWatcher>
#include <QRandomGenerator>
#include <QUrlQuery>

#ifdef WITH_HTMLTIDY
#include <tidy.h>
#include <tidybuffio.h>
#endif

#include <util/util.h>
#include <util/xpc/util.h>
#include <util/sll/util.h>
#include <util/sll/qtutil.h>
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

namespace LC::LHTR
{
	namespace
	{
		class EditorPage : public QWebEnginePage
		{
		public:
			using QWebEnginePage::QWebEnginePage;
		protected:
			bool acceptNavigationRequest (const QUrl& url, NavigationType type, bool) override
			{
				if (type == NavigationTypeTyped || url.scheme () == "data"_ql)
					return true;

				if (type == NavigationTypeLinkClicked || type == NavigationTypeOther)
				{
					const auto& e = Util::MakeEntity (url, {}, FromUserInitiated | OnlyHandle);
					GetProxyHolder ()->GetEntityManager ()->HandleEntity (e);
				}
				return false;
			}
		};

		class RequestInterceptor final : public QWebEngineUrlRequestInterceptor
		{
			RichEditorWidget& Editor_;
			const QByteArray PageKey_;
		public:
			RequestInterceptor (QByteArray pageKey, RichEditorWidget& editor)
			: QWebEngineUrlRequestInterceptor { &editor }
			, Editor_ { editor }
			, PageKey_ { std::move (pageKey) }
			{
			}
		protected:
			void interceptRequest (QWebEngineUrlRequestInfo& info) override
			{
				const auto& url = info.requestUrl ();
				if (url.host () != "lhtr"_ql)
					return;

				info.block (true);

				const QUrlQuery query { url };
				if (query.queryItemValue (QStringLiteral ("key")) != PageKey_)
				{
					qWarning () << "wrong page key"
							<< url;
					return;
				}

				if (url.path () == "/notify"_ql)
					emit Editor_.textChanged ();
			}
		};

		enum TabWidgetIdx
		{
			TWIVisualView,
			TWIHTMLView
		};

		auto GetInjectableScript (const QByteArray& pageKey)
		{
			QWebEngineScript script;
			script.setInjectionPoint (QWebEngineScript::DocumentReady);
			script.setWorldId (QWebEngineScript::MainWorld);
			script.setSourceCode (uR"(
const findParent = (item, name) => {
    while (item && (item.tagName || "").toLowerCase() != name)
        item = item.parentNode;
    return item;
}

(() => {
    document.body.contentEditable = true;

    let notificationPending = false;
    let counter = 0;

    const notifier = (e) => {
        if (notificationPending) {
            return;
        }

        notificationPending = true;
        setTimeout(async () => {
            notificationPending = false;
            try {
                await fetch("https://lhtr/notify?key=%1", {mode: "no-cors"});
            } catch (e) {
                // fetch error is expected, do nothing
            }
        }, 1000);
    };

    window.addEventListener('DOMContentLoaded', notifier);
    window.addEventListener('DOMSubtreeModified', notifier);
    window.addEventListener('DOMAttrModified', notifier);
    window.addEventListener('DOMNodeInserted', notifier);
    window.addEventListener('DOMNodeRemoved', notifier);
})();
)"_qsv.arg (pageKey));
			return script;
		}
	}

	RichEditorWidget::RichEditorWidget (QWidget *parent)
	: QWidget (parent)
	, ViewBar_ (new QToolBar (tr ("Editor bar")))
	, FindAction_ (new QAction (tr ("Find"), this))
	, ReplaceAction_ (new QAction (tr ("Replace"), this))
	{
		FindAction_->setProperty ("ActionIcon", "edit-find");
		FindAction_->setShortcut (QKeySequence::Find);
		connect (FindAction_,
				&QAction::triggered,
				[this] { OpenFindReplace (true); });
		ReplaceAction_->setProperty ("ActionIcon", "edit-find-replace");
		ReplaceAction_->setShortcut (QKeySequence::Replace);
		connect (ReplaceAction_,
				&QAction::triggered,
				[this] { OpenFindReplace (false); });

		const auto pageKey = QByteArray::number (QRandomGenerator::global ()->generate ());
		Ui_.setupUi (this);
		Ui_.View_->setPage (new EditorPage (Ui_.View_));
		Ui_.View_->page ()->setUrlRequestInterceptor (new RequestInterceptor { pageKey, *this });

		connect (Ui_.HTML_,
				&QTextEdit::textChanged,
				this,
				&RichEditorWidget::HandleHtmlEditChanged);

		connect (Ui_.TabWidget_,
				&QTabWidget::currentChanged,
				this,
				&RichEditorWidget::SyncTabs);

		XmlSettingsManager::Instance ().RegisterObject ({ "BgColor", "HTMLBgColor" },
				this,
				[this]
				{
					auto& xsm = XmlSettingsManager::Instance ();
					InternalSetBgColor (xsm.property ("BgColor").value<QColor> (), ContentType::HTML);
					InternalSetBgColor (xsm.property ("HTMLBgColor").value<QColor> (), ContentType::PlainText);
				});

		Ui_.View_->installEventFilter (this);

		connect (Ui_.View_->page (),
				&QWebEnginePage::selectionChanged,
				[this]
				{
					for (const auto& [cmd, act] : HtmlActions_)
						act->setChecked (QueryCommandState (cmd));
				});

		qobject_cast<QVBoxLayout*> (Ui_.ViewTab_->layout ())->insertWidget (0, ViewBar_);

		auto addCmd = [this] (const QString& name,
				const QString& icon,
				QStringView cmd,
				auto addable,
				const QString& arg = {})
		{
			auto act = addable->addAction (name,
					this,
					[=] { HandleHtmlCmdAction (cmd, arg); });
			act->setProperty ("ActionIcon", icon);
			if (arg.isEmpty ())
				HtmlActions_.append ({ cmd, act });
			return act;
		};

		addCmd (tr ("Bold"), QStringLiteral ("format-text-bold"), u"bold", ViewBar_)->setCheckable (true);
		addCmd (tr ("Italic"), QStringLiteral ("format-text-italic"), u"italic", ViewBar_)->setCheckable (true);
		addCmd (tr ("Underline"), QStringLiteral ("format-text-underline"), u"underline", ViewBar_)->setCheckable (true);

		addCmd (tr ("Strikethrough"), QStringLiteral ("format-text-strikethrough"), u"strikeThrough", ViewBar_)->setCheckable (true);
		addCmd (tr ("Subscript"), QStringLiteral ("format-text-subscript"), u"subscript", ViewBar_)->setCheckable (true);
		addCmd (tr ("Superscript"), QStringLiteral ("format-text-superscript"), u"superscript", ViewBar_)->setCheckable (true);

		auto addInlineCmd = [this] (const QString& name,
				const QString& icon,
				QStringView cmd,
				auto addable)
		{
			auto act = addable->addAction (name,
					this,
					[=] { HandleInlineCmd (cmd); });
			act->setProperty ("ActionIcon", icon);
			return act;
		};

		addInlineCmd (tr ("Code"), QStringLiteral ("code-context"), u"code", ViewBar_);

		ViewBar_->addSeparator ();

		auto alignActs =
		{
			addCmd (tr ("Align left"), QStringLiteral ("format-justify-left"), u"justifyLeft", ViewBar_, {}),
			addCmd (tr ("Align center"), QStringLiteral ("format-justify-center"), u"justifyCenter", ViewBar_, {}),
			addCmd (tr ("Align right"), QStringLiteral ("format-justify-right"), u"justifyRight", ViewBar_, {}),
			addCmd (tr ("Align justify"), QStringLiteral ("format-justify-fill"), u"justifyFull", ViewBar_, {})
		};
		const auto alignGroup = new QActionGroup (this);
		for (QAction *act : alignActs)
		{
			act->setCheckable (true);
			alignGroup->addAction (act);
		}

		ViewBar_->addSeparator ();

		const auto headMenu = new QMenu (tr ("Headings"));
		headMenu->setIcon (GetProxyHolder ()->GetIconThemeManager ()->GetIcon (QStringLiteral ("view-list-details")));
		ViewBar_->addAction (headMenu->menuAction ());
		const int maxHeadingLevel = 6;
		for (int i = 1; i <= maxHeadingLevel; ++i)
		{
			const auto& num = QString::number (i);
			addCmd (tr ("Heading %1").arg (i), {}, u"formatBlock", headMenu, "h" + num);
		}
		headMenu->addSeparator ();
		addCmd (tr ("Paragraph"), QString (), u"formatBlock", headMenu, QStringLiteral ("p"));

		ViewBar_->addSeparator ();

		QAction *bgColor = ViewBar_->addAction (tr ("Background color..."),
					this,
					[this]
					{
						const auto& color = QColorDialog::getColor (Qt::white, this);
						if (color.isValid ())
							ExecCommand (u"hiliteColor", color.name ());
					});
		bgColor->setProperty ("ActionIcon", "format-fill-color");

		QAction *fgColor = ViewBar_->addAction (tr ("Text color..."),
					this,
					[this]
					{
						const auto& color = QColorDialog::getColor (Qt::black, this);
						if (color.isValid ())
							ExecCommand (u"foreColor", color.name ());
					});
		fgColor->setProperty ("ActionIcon", "format-text-color");

		QAction *font = ViewBar_->addAction (tr ("Font..."),
					this,
					&RichEditorWidget::HandleFont);
		font->setProperty ("ActionIcon", "list-add-font");
		ViewBar_->addSeparator ();

		addCmd (tr ("Mark as quote"), QStringLiteral ("mail-reply-sender"), u"formatBlock", ViewBar_, QStringLiteral ("blockquote"));

		ViewBar_->addSeparator ();

		addCmd (tr ("Indent more"), QStringLiteral ("format-indent-more"), u"indent", ViewBar_);
		addCmd (tr ("Indent less"), QStringLiteral ("format-indent-less"), u"outdent", ViewBar_);

		ViewBar_->addSeparator ();

		addCmd (tr ("Ordered list"), QStringLiteral ("format-list-ordered"), u"insertOrderedList", ViewBar_);
		addCmd (tr ("Unordered list"), QStringLiteral ("format-list-unordered"), u"insertUnorderedList", ViewBar_);

		ViewBar_->addSeparator ();

		const auto insertLink = ViewBar_->addAction (tr ("Insert link..."),
				this,
				&RichEditorWidget::HandleInsertLink);
		insertLink->setProperty ("ActionIcon", "insert-link");

		SetupImageMenu ();
		SetupTableMenu ();

		Ui_.View_->page ()->scripts ().insert (GetInjectableScript (pageKey));

		SetContents ({}, ContentType::HTML);

		new HtmlHighlighter (Ui_.HTML_->document ());

		setFocusProxy (Ui_.View_);
	}

	namespace
	{
		template<typename R>
		R Blocking (QWebEngineView *view, auto func)
		{
			QFutureInterface<R> promise;
			promise.reportStarted ();

			std::invoke (func, view->page (), [&] (const R& result) { promise.reportFinished (&result); });

			QEventLoop loop;
			QFutureWatcher<R> watcher;
			QObject::connect (&watcher,
					&QFutureWatcher<R>::finished,
					&loop,
					&QEventLoop::quit);
			watcher.setFuture (promise.future ());

			loop.exec (QEventLoop::ExcludeUserInputEvents);

			return watcher.result ();
		}
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
			return Blocking<QString> (Ui_.View_, &QWebEnginePage::toPlainText);
		}

		Util::Unreachable ();
	}

	void RichEditorWidget::SetContents (QString contents, ContentType type)
	{
		auto content = QStringLiteral (R"(
<!DOCTYPE html>
<html>
<head><title>Blogique</title></head><body>)");
		switch (type)
		{
		case ContentType::HTML:
			content += contents;
			break;
		case ContentType::PlainText:
			contents = contents.toHtmlEscaped ();
			contents.replace ("\r\n"_ql, "<br/>"_ql);
			contents.replace ('\n', "<br/>"_ql);
			contents.replace ('\r', "<br/>"_ql);
			content += "<pre>" + contents + "</pre>";
			break;
		}
		content += "</body></html>"_ql;

		if (type == ContentType::HTML)
			content = ExpandCustomTags (content);

		Ui_.View_->page ()->setHtml (content);
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
		}

		return nullptr;
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

		expanded.replace ('\n', "\\n"_ql);
		expanded.replace ('\'', "\\'"_ql);

		ExecJS (R"delim(
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
		CustomTags_.clear ();
		QMap<CustomTag::TagType, QByteArrayList> tagNames;
		for (const auto& tag : tags)
		{
			CustomTags_ [tag.TagName_] = tag;
			tagNames [tag.TagType_] << tag.TagName_.toUtf8 ();
		}

		CustomTagNames_.clear ();
		for (const auto& [type, names] : Util::Stlize (tagNames))
			CustomTagNames_ [type] = names.join (',');
	}

	QAction* RichEditorWidget::AddInlineTagInserter (const QString& tagName, const QVariantMap& params)
	{
		auto act = ViewBar_->addAction ({},
				this,
				[=] { HandleInlineCmd (tagName, params); });
		return act;
	}

	void RichEditorWidget::ExecJS (const QString& js)
	{
		Ui_.View_->page ()->runJavaScript (js);
	}

	void RichEditorWidget::SetFontFamily (FontFamily family, const QFont& font)
	{
		Ui_.View_->settings ()->setFontFamily (static_cast<QWebEngineSettings::FontFamily> (family), font.family ());
	}

	void RichEditorWidget::SetFontSize (FontSize font, int size)
	{
		Ui_.View_->settings ()->setFontSize (static_cast<QWebEngineSettings::FontSize> (font), size);
	}

	bool RichEditorWidget::eventFilter (QObject*, QEvent *event)
	{
		if (event->type () != QEvent::KeyPress && event->type () != QEvent::KeyRelease)
			return false;

		auto keyEv = static_cast<QKeyEvent*> (event);
		if (keyEv->key () != Qt::Key_Tab)
			return false;

		const auto act = keyEv->modifiers () & Qt::ShiftModifier ?
				QWebEnginePage::Outdent :
				QWebEnginePage::Indent;
		Ui_.View_->page ()->action (act)->trigger ();
		return true;
	}

	void RichEditorWidget::InternalSetBgColor (const QColor& color, ContentType type)
	{
		QWidget *widget = nullptr;
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
		imagesButton->setIcon (GetProxyHolder ()->GetIconThemeManager ()->GetIcon (QStringLiteral ("insert-image")));
		ViewBar_->addWidget (imagesButton);

		imagesMenu->addAction (tr ("Insert image by link..."),
				this,
				&RichEditorWidget::HandleInsertImage);

		auto fromCollection = imagesMenu->addMenu (tr ("Insert image from collection"));

		bool added = false;
		for (const auto pluginObj : GetProxyHolder ()->GetPluginsManager ()->GetAllCastableRoots<IImgSource*> ())
		{
			const auto plugin = qobject_cast<IImgSource*> (pluginObj);
			for (const auto& service : plugin->GetServices ())
			{
				fromCollection->addAction (service.Name_,
						this,
						[=]
						{
							connect (plugin->RequestImages (service.ID_)->GetQObject (),
									SIGNAL (ready ()),
									this,
									SLOT (handleCollectionImageChosen ()));
						});
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
		tablesButton->setIcon (GetProxyHolder ()->GetIconThemeManager ()->GetIcon (QStringLiteral ("view-form-table")));
		ViewBar_->addWidget (tablesButton);

		auto table = tablesMenu->addAction (tr ("Insert table..."),
					this,
					&RichEditorWidget::HandleInsertTable);
		table->setProperty ("ActionIcon", "insert-table");

		tablesMenu->addSeparator ();

		auto addRowAbove = tablesMenu->addAction (tr ("Insert row above"),
					this,
					[this] { HandleInsertRow (0); });
		addRowAbove->setProperty ("ActionIcon", "edit-table-insert-row-above");

		auto addRowBelow = tablesMenu->addAction (tr ("Insert row below"),
					this,
					[this] { HandleInsertRow (1); });
		addRowBelow->setProperty ("ActionIcon", "edit-table-insert-row-below");

		auto addColumnLeft = tablesMenu->addAction (tr ("Insert column to the left"),
					this,
					[this] { HandleInsertColumn (0); });
		addColumnLeft->setProperty ("ActionIcon", "edit-table-insert-column-left");

		auto addColumnRight = tablesMenu->addAction (tr ("Insert column to the right"),
					this,
					[this] { HandleInsertColumn (1); });
		addColumnRight->setProperty ("ActionIcon", "edit-table-insert-column-right");

		tablesMenu->addSeparator ();

		auto removeRow = tablesMenu->addAction (tr ("Remove row"),
					this,
					&RichEditorWidget::HandleRemoveRow);
		removeRow->setProperty ("ActionIcon", "edit-table-delete-row");

		auto removeColumn = tablesMenu->addAction (tr ("Remove column"),
					this,
					&RichEditorWidget::HandleRemoveColumn);
		removeColumn->setProperty ("ActionIcon", "edit-table-delete-column");
	}

	QVariant RichEditorWidget::ExecJSBlocking (const QString& js)
	{
		return Blocking<QVariant> (Ui_.View_,
				[&] (QWebEnginePage *page, auto callback) { page->runJavaScript (js, callback); });
	}

	void RichEditorWidget::ExecCommand (QStringView cmd, QStringView arg)
	{
		if (cmd == u"insertHTML"_qsv)
		{
			InsertHTML (arg.toString ());
			return;
		}

		const auto& js = arg.isEmpty () ?
				u"document.execCommand('%1', false, null)"_qsv.arg (cmd) :
				u"document.execCommand('%1', false, '%2')"_qsv.arg (cmd, arg.toString ().replace ('\n', R"(\n)"_ql));
		ExecJS (js);
	}

	bool RichEditorWidget::QueryCommandState (QStringView cmd)
	{
		return ExecJSBlocking (uR"(document.queryCommandState("%1", false, null))"_qsv.arg (cmd)).toBool ();
	}

	void RichEditorWidget::OpenFindReplace (bool)
	{
		const bool isView = Ui_.TabWidget_->currentIndex () == 0;

		auto dia = isView ?
				new FindDialog (FindObjectProxy { Ui_.View_ }, this) :
				new FindDialog (FindObjectProxy { Ui_.HTML_ }, this);
		dia->setAttribute (Qt::WA_DeleteOnClose);
		dia->show ();
	}

	namespace
	{
		void TryFixHTML (QString& html, const QMap<RichEditorWidget::CustomTag::TagType, QByteArray>& tagNames)
		{
#ifdef WITH_HTMLTIDY
			TidyBuffer output {};

			auto tdoc = tidyCreate ();

			const auto guard = Util::MakeScopeGuard ([&tdoc, &output]
					{
						tidyRelease (tdoc);
						tidyBufFree (&output);
					});

			auto setTagType = [tdoc] (TidyOptionId option, const QByteArray& names) -> bool
			{
				if (!names.isEmpty ())
					return tidyOptSetValue (tdoc, option, names.constData ());
				return true;
			};

			using enum RichEditorWidget::CustomTag::TagType;
			if (!tidyOptSetBool (tdoc, TidyXmlOut, yes) ||
					!tidyOptSetBool (tdoc, TidyForceOutput, yes) ||
					!tidyOptSetValue (tdoc, TidyCharEncoding, "utf8") ||
					!setTagType (TidyEmptyTags, tagNames [Empty]) ||
					!setTagType (TidyInlineTags, tagNames [Inline]) ||
					!setTagType (TidyBlockTags, tagNames [Block]))
			{
				qWarning () << "cannot set xhtml output";
				return;
			}

			tidyOptSetInt (tdoc, TidyWrapLen, std::numeric_limits<int>::max ());

			if (tidyParseString (tdoc, html.toUtf8 ().constData ()) < 0)
			{
				qWarning () << "failed to parse"
						<< html;
				return;
			}

			if (tidyCleanAndRepair (tdoc) < 0)
			{
				qWarning () << "failed to clean and repair"
						<< html;
				return;
			}

			if (tidySaveBuffer (tdoc, &output) < 0)
			{
				qWarning () << "cannot save buffer";
				return;
			}

			html = QString::fromUtf8 (std::bit_cast<char*> (output.bp));
#endif

			if (!html.startsWith ("<!DOCTYPE "_ql))
				html.prepend (u"<!DOCTYPE html>"_qsv);
		}
	}

	namespace Lits
	{
		const QString Tagname = QStringLiteral ("__tagname__");
		const QString Original = QStringLiteral ("__original__");
		const QString ContentEditable = QStringLiteral ("contenteditable");
	}

	QString RichEditorWidget::ExpandCustomTags (QString html, ExpandMode mode) const
	{
		if (CustomTags_.isEmpty ())
			return html;

		if (mode == ExpandMode::FullHTML)
			TryFixHTML (html, CustomTagNames_);

		QDomDocument doc;
		if (!doc.setContent (html))
		{
			qWarning () << "unable to set content from"
					<< html;
			return html;
		}

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

				elem.setAttribute (Lits::Tagname, tag.TagName_);
				elem.setAttribute (Lits::Original, origContents.trimmed ());
				if (!tag.FromKnown_)
					elem.setAttribute (Lits::ContentEditable, QStringLiteral ("false"));
			}
		}

		return doc.toString (-1);
	}

	namespace
	{
		std::optional<QDomDocument> ParseXml (const QString& xml)
		{
			QDomDocument doc;
			QString errMsg;
			int errLine = -1;
			int errCol = -1;
			if (!doc.setContent (xml, &errMsg, &errLine, &errCol))
			{
				qWarning () << "unable to parse xml:"
						<< errMsg
						<< "at"
						<< errLine
						<< ":"
						<< errCol;
				return {};
			}

			return doc;
		}

		// Returns whether this is a custom element whose children shall not be processed.
		bool TryRevertCustomTag (const QHash<QString, IAdvancedHTMLEditor::CustomTag>& customTags, QDomElement elem)
		{
			const auto& tagName = elem.attribute (Lits::Tagname);
			if (tagName.isEmpty ())
				return false;

			if (!customTags.contains (tagName))
			{
				qWarning () << "unknown tag"
						<< tagName
						<< "; known tags:"
						<< customTags.keys ();
				return true;
			}

			const auto& customTag = customTags [tagName];

			const auto& original = elem.attribute (Lits::Original);
			const auto setOriginal = [&]
			{
				if (const auto& maybeOrigDoc = ParseXml (original))
					elem.parentNode ().replaceChild (maybeOrigDoc->documentElement (), elem);
			};

			if (!customTag.FromKnown_)
			{
				setOriginal ();
				return true;
			}

			elem.removeAttribute (Lits::Tagname);
			elem.removeAttribute (Lits::Original);
			elem.removeAttribute (Lits::ContentEditable);
			if (!customTag.FromKnown_ (elem))
			{
				QString elemRepr;
				QTextStream stream { &elemRepr };
				elem.save (stream, 2);

				qWarning () << "FromKnown failed for"
						<< tagName
						<< elemRepr;
				setOriginal ();
			}

			return true;
		}

		void RevertCustomTagsRec (const QHash<QString, IAdvancedHTMLEditor::CustomTag>& customTags, QDomElement elem)
		{
			while (!elem.isNull ())
			{
				if (!TryRevertCustomTag (customTags, elem))
					RevertCustomTagsRec (customTags, elem.firstChildElement ());

				elem = elem.nextSiblingElement ();
			}
		}
	}

	QString RichEditorWidget::RevertCustomTags (const QString& xml) const
	{
		const auto& maybeDoc = ParseXml (xml);
		if (!maybeDoc)
			return xml;
		const auto& doc = *maybeDoc;
		RevertCustomTagsRec (CustomTags_, doc.documentElement ());
		return doc.toString ();
	}

	void RichEditorWidget::SyncHTMLToView () const
	{
		Ui_.View_->page ()->runJavaScript (QStringLiteral (R"(
(() => {
    if (document.querySelectorAll("parsererror").length) {
        return false;
    }
    return new XMLSerializer().serializeToString(document);
})();
)"),
			[pThis = QPointer { this }] (const QVariant& var)
			{
				if (!pThis)
				{
					qWarning () << "editor died";
					return;
				}

				if (var.type () == QVariant::Bool && !var.toBool ())
				{
					qWarning () << "there are parser errors, ignoring reverting";
					return;
				}

				disconnect (pThis->Ui_.HTML_,
						&QTextEdit::textChanged,
						&*pThis,
						&RichEditorWidget::HandleHtmlEditChanged);
				pThis->Ui_.HTML_->setPlainText (pThis->RevertCustomTags (var.toString ()));
				connect (pThis->Ui_.HTML_,
						&QTextEdit::textChanged,
						&*pThis,
						&RichEditorWidget::HandleHtmlEditChanged);
			});
	}

	void RichEditorWidget::HandleHtmlEditChanged ()
	{
		HTMLDirty_ = true;
		emit textChanged ();
	}

	void RichEditorWidget::SyncTabs (int idx)
	{
		// Unfortunate to use a shared_ptr where a stack-allocated QSignalBlocker would do,
		// but, sadly, QtWebEngine requires a copyable callback below.
		auto blocker = std::make_shared<QSignalBlocker> (this);

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
			Ui_.View_->setHtml (str.toUtf8 ());

			Ui_.View_->page ()->runJavaScript (QStringLiteral (R"(document.querySelectorAll("html > body").length)"),
					[str, blocker, pThis = QPointer { this }] (const QVariant& result)
					{
						if (!result.toInt () && pThis)
						{
							qWarning () << "null html/body element";
							pThis->SetContents (str, ContentType::HTML);
						}
					});
			break;
		}
	}

	void RichEditorWidget::HandleHtmlCmdAction (QStringView command, QStringView args)
	{
		if (command.compare (u"formatblock", Qt::CaseInsensitive))
		{
			ExecCommand (command, args);
			return;
		}

		ExecJS (uR"(
var selection = window.getSelection().getRangeAt(0);
var parentItem = findParent(selection.commonAncestorContainer.parentNode, '%1');
if (parentItem == null) {
	document.execCommand('formatBlock', false, '%1');
} else {
	parentItem.outerHTML = parentItem.innerHTML;
}
)"_qsv.arg (args));
	}

	void RichEditorWidget::HandleInlineCmd (QStringView tag, const QVariantMap& attrs)
	{
		QString attrsStr;
		for (const auto& [name, val] : Util::Stlize (attrs))
			for (auto i = attrs.begin (), end = attrs.end (); i != end; ++i)
				attrsStr += u"	span.setAttribute ('%1', '%2');"_qsv.arg (name, val.toString ());

		ExecJS (uR"(
var selection = window.getSelection().getRangeAt(0);
var parentItem = findParent(selection.commonAncestorContainer.parentNode, '%1');
if (parentItem == null) {
	var selectedText = selection.extractContents();
	var span = document.createElement('%1');
	%2
	span.appendChild(selectedText);
	selection.insertNode(span);
} else {
	parentItem.outerHTML = parentItem.innerHTML;
}
)"_qsv.arg (tag, attrsStr));
	}

	void RichEditorWidget::HandleFont ()
	{
		bool ok = false;
		const QFont& font = QFontDialog::getFont (&ok, this);
		if (!ok)
			return;

		ExecCommand (u"fontName", font.family ());

		auto checkWebAct = [this, &font] (auto f, QStringView cmd)
		{
			const bool state = std::invoke (f, &font);
			if (QueryCommandState (cmd) != state)
				ExecCommand (cmd);
		};

		checkWebAct (&QFont::bold, u"bold");
		checkWebAct (&QFont::italic, u"italic");
		checkWebAct (&QFont::underline, u"underline");
	}

	void RichEditorWidget::HandleInsertTable ()
	{
		InsertTableDialog dia;
		if (dia.exec () != QDialog::Accepted)
			return;

		QString html;
		QXmlStreamWriter w (&html);
		w.writeStartElement (QStringLiteral ("table"));
		w.writeAttribute (QStringLiteral ("style"), QStringLiteral ("border: 1px solid black; border-collapse: collapse;"));

		const auto& caption = dia.GetCaption ().trimmed ();
		if (!caption.isEmpty ())
			w.writeTextElement (QStringLiteral ("caption"), caption);

		w.writeStartElement (QStringLiteral ("tbody"));
		for (int i = 0; i < dia.GetRows (); ++i)
		{
			w.writeStartElement (QStringLiteral ("tr"));
			for (int j = 0; j < dia.GetColumns (); ++j)
			{
				w.writeStartElement (QStringLiteral ("td"));
				w.writeAttribute (QStringLiteral ("style"), QStringLiteral ("border: 1px solid black; min-width: 1em; height: 1.5em;"));
				w.writeEndElement ();
			}
			w.writeEndElement ();
		}
		w.writeEndElement ();
		w.writeEndElement ();
		ExecCommand (u"insertHTML", html);
	}

	void RichEditorWidget::HandleInsertRow (int shift)
	{
		ExecJS (uR"(
const row = findParent(window.getSelection().getRangeAt(0).endContainer, 'tr');
const rowIdx = row.rowIndex;
const table = findParent(row, 'table');
const newRow = table.insertRow(rowIdx + %1);
for (let j = 0; j < row.cells.length; ++j) {
    const newCell = newRow.insertCell(j);
    newCell.setAttribute('style', 'border: 1px solid black; min-width: 1em; height: 1.5em;');
}
)"_qsv.arg (QString::number (shift)));
	}

	void RichEditorWidget::HandleInsertColumn (int shift)
	{
		ExecJS (uR"(
const cell = findParent(window.getSelection().getRangeAt(0).endContainer, 'td');
const colIdx = cell.cellIndex + %1;
const table = findParent(cell, 'table');
for (let r = 0; r < table.rows.length; ++r) {
    const newCell = table.rows[r].insertCell(colIdx);
    newCell.setAttribute('style', 'border: 1px solid black; min-width: 1em; height: 1.5em;');
}
)"_qsv.arg (QString::number (shift)));
	}

	void RichEditorWidget::HandleRemoveRow ()
	{
		ExecJS (QStringLiteral (uR"(
const row = findParent(window.getSelection().getRangeAt(0).endContainer, 'tr');
const table = findParent(row, 'table');
table.deleteRow(row.rowIndex);
)"));
	}

	void RichEditorWidget::HandleRemoveColumn ()
	{
		ExecJS (QStringLiteral (uR"(
const cell = findParent(window.getSelection().getRangeAt(0).endContainer, 'td');
const colIdx = cell.cellIndex;
const table = findParent(cell, 'table');
for (let r = 0; r < table.rows.length; ++r)
    table.rows[r].deleteCell(colIdx);
)"));
	}

	void RichEditorWidget::HandleInsertLink ()
	{
		if (Ui_.View_->hasSelection ())
		{
			const QString& url = QInputDialog::getText (this,
					tr ("Insert link"), tr ("Enter URL:"));
			const QUrl& guess = QUrl::fromUserInput (url);
			if (guess.isValid ())
				ExecCommand (u"createLink", guess.toString ());

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
		w.writeStartElement (QStringLiteral ("a"));
		w.writeAttribute (QStringLiteral ("href"), link);
		if (!dia.GetTitle ().isEmpty ())
			w.writeAttribute (QStringLiteral ("title"), dia.GetTitle ());
		if (!dia.GetTarget ().isEmpty ())
			w.writeAttribute (QStringLiteral ("target"), dia.GetTarget ());
		w.writeCharacters (text);
		w.writeEndElement ();

		ExecCommand (u"insertHTML", html);
	}

	void RichEditorWidget::HandleInsertImage ()
	{
		ImageDialog dia (this);
		if (dia.exec () != QDialog::Accepted)
			return;

		const QString& path = dia.GetPath ();
		const QUrl& url = QUrl::fromEncoded (path.toUtf8 ());
		const QString& src = url.scheme () == "file"_ql ?
				Util::GetAsBase64Src (QImage (url.toLocalFile ())) :
				path;

		QString html;
		QXmlStreamWriter w (&html);
		w.writeStartElement (QStringLiteral ("img"));
		w.writeAttribute (QStringLiteral ("src"), src);
		w.writeAttribute (QStringLiteral ("alt"), dia.GetAlt ());
		if (dia.GetWidth () > 0)
			w.writeAttribute (QStringLiteral ("width"), QString::number (dia.GetWidth ()));
		if (dia.GetHeight () > 0)
			w.writeAttribute (QStringLiteral ("height"), QString::number (dia.GetHeight ()));
		w.writeAttribute (QStringLiteral ("style"), "float:" + dia.GetFloat ());
		w.writeEndElement ();

		ExecCommand (u"insertHTML", html);
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

			qWarning () << "unknown preview size"
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

			qWarning () << "unknown preview size"
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
		w.writeStartElement (QStringLiteral ("span"));

		bool displayBlock = false;
		QStringView floatPos;
		QStringView align;
		switch (dia.GetPosition ())
		{
		case ImageCollectionDialog::Position::Center:
			align = u"center";
			displayBlock = true;
			break;
		case ImageCollectionDialog::Position::Right:
			displayBlock = true;
			align = u"right";
			break;
		case ImageCollectionDialog::Position::Left:
			displayBlock = true;
			align = u"left";
			break;
		case ImageCollectionDialog::Position::RightWrap:
			floatPos = u"right";
			break;
		case ImageCollectionDialog::Position::LeftWrap:
			floatPos = u"left";
			break;
		}

		QStringList styleElems;
		if (displayBlock)
			styleElems << QStringLiteral ("display: block");
		if (!floatPos.isEmpty ())
			styleElems << QStringLiteral ("float: ").append (floatPos);
		if (!align.isEmpty ())
			styleElems << QStringLiteral ("text-align: ").append (align);

		if (!styleElems.isEmpty ())
			w.writeAttribute (QStringLiteral ("style"), styleElems.join (';'));

		for (const auto& image : dia.GetInfos ())
		{
			if (linkPreviews)
			{
				w.writeStartElement (QStringLiteral ("a"));
				w.writeAttribute (QStringLiteral ("href"), image.Full_.toString ());
			}

			w.writeStartElement (QStringLiteral ("img"));
			w.writeAttribute (QStringLiteral ("src"), GetPreviewUrl (image, previewSize).toString ());
			w.writeAttribute (QStringLiteral ("alt"), image.Title_);

			const auto& size = GetPreviewSize (image, previewSize);
			if (size.isValid ())
			{
				w.writeAttribute (QStringLiteral ("width"), QString::number (size.width ()));
				w.writeAttribute (QStringLiteral ("height"), QString::number (size.height ()));
			}

			w.writeEndElement ();

			if (linkPreviews)
				w.writeEndElement ();

			w.writeEmptyElement (QStringLiteral ("br"));
		}
		w.writeEndElement ();

		ExecCommand (u"insertHTML", html);
	}
}
