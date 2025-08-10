/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "customwebview.h"
#include <QIcon>
#include <QFile>
#include <QWebEngineSettings>
#include <QWebEngineHistory>
#include <QWebEngineContextMenuRequest>
#include <QWebChannel>
#include <QPrinter>
#include <QPrintDialog>
#include <QContextMenuEvent>
#include <QMouseEvent>
#include <QAction>
#include <util/gui/findnotificationwe.h>
#include <util/sll/unreachable.h>
#include <util/sll/qtutil.h>
#include <interfaces/poshuku/iwebviewhistory.h>
#include <interfaces/poshuku/iproxyobject.h>
#include "customwebpage.h"

namespace LC::Poshuku::WebEngineView
{
	CustomWebView::CustomWebView (IProxyObject *poshukuProxy)
	{
		const auto page = new CustomWebPage { poshukuProxy, this };
		setPage (page);

		connect (page,
				&CustomWebPage::loadFinished,
				this,
				&CustomWebView::earliestViewLayout);
		connect (page,
				&CustomWebPage::loadFinished,
				this,
				&CustomWebView::loadFinished);
		connect (page,
				&CustomWebPage::iconChanged,
				this,
				&CustomWebView::iconChanged);
		connect (page,
				&CustomWebPage::windowCloseRequested,
				this,
				&CustomWebView::closeRequested);
		connect (page,
				&QWebEnginePage::linkHovered,
				this,
				[this] (const QString& url) { emit linkHovered (url, {}, {}); });
		connect (page,
				&CustomWebPage::webViewCreated,
				this,
				&CustomWebView::webViewCreated);
	}

	void CustomWebView::SurroundingsInitialized ()
	{
		FindDialog_ = new Util::FindNotificationWE { GetProxyHolder (), this };
		FindDialog_->hide ();
	}

	QWidget* CustomWebView::GetQWidget ()
	{
		return this;
	}

	QList<QAction*> CustomWebView::GetActions (ActionArea) const
	{
		return {};
	}

	QAction* CustomWebView::GetPageAction (PageAction action) const
	{
#define ACT(x) \
		case PageAction::x: \
			return pageAction (QWebEnginePage::x);

		switch (action)
		{
		ACT (Reload)
		ACT (ReloadAndBypassCache)
		ACT (Stop)
		ACT (Back)
		ACT (Forward)
		ACT (Cut)
		ACT (Copy)
		ACT (Paste)
		ACT (CopyLinkToClipboard)
		ACT (DownloadLinkToDisk)
		ACT (DownloadImageToDisk)
		ACT (CopyImageToClipboard)
		ACT (CopyImageUrlToClipboard)
		ACT (InspectElement)
		case PageAction::OpenImageInNewWindow:
			return nullptr;
		}

#undef ACT

		Util::Unreachable ();
	}

	QString CustomWebView::GetTitle () const
	{
		return title ();
	}

	QUrl CustomWebView::GetUrl () const
	{
		return url ();
	}

	QString CustomWebView::GetHumanReadableUrl () const
	{
		return url ().toDisplayString ();
	}

	QIcon CustomWebView::GetIcon () const
	{
		return icon ();
	}

	void CustomWebView::Load (const QUrl& url, const QString& title)
	{
		if (title.isEmpty ())
			emit titleChanged (tr ("Loading..."));
		else
			emit titleChanged (title);

		load (url);
		emit loadStarted ();
		emit urlChanged (url);
	}

	void CustomWebView::SetContent (const QByteArray& data, const QByteArray& mime, const QUrl& base)
	{
		setContent (data, mime, base);
	}

	void CustomWebView::ToHtml (const std::function<void (QString)>& handler) const
	{
		page ()->toHtml (handler);
	}

	void CustomWebView::EvaluateJS (const QString& js,
			const std::function<void (QVariant)>& handler,
			Util::BitFlags<EvaluateJSFlag> flags)
	{
		QString jsToRun;
		std::function<void (QVariant)> modifiedHandler;
		if (flags & EvaluateJSFlag::RecurseSubframes)
		{
			jsToRun = R"(
					(function(){
						var result = [];
						var f = function(document) {
							var r = %1;
							result.push(r);
						};
						f(document);
						var recurse = function(document) {
							var frames = document.querySelectorAll('iframe');
							for (var i = 0; i < frames.length; ++i) {
								try {
									var child = frames[i].contentDocument.children[0];
									f(child);
									recurse(child)
								} catch(e) { console.log("frame read failure: " + e); };
							}
						};
						recurse(document);
						return result;
					})();
				)"_qs.arg (js);

			modifiedHandler = [handler] (const QVariant& result)
			{
				for (const auto& item : result.toList ())
					handler (item);
			};
		}
		else
		{
			jsToRun = js;
			modifiedHandler = handler;
		}

		if (handler)
			page ()->runJavaScript (jsToRun, modifiedHandler);
		else
			page ()->runJavaScript (jsToRun);
	}

	void CustomWebView::AddJavaScriptObject (const QString& id, QObject *object)
	{
		auto channel = page ()->webChannel ();
		if (!channel)
		{
			channel = new QWebChannel;
			page ()->setWebChannel (channel);
		}

		EvaluateJS ("typeof QWebChannel === 'undefined'"_qs,
				[=, page = page ()] (const QVariant& res)
				{
					if (res.toBool ())
					{
						QFile file { ":/qtwebchannel/qwebchannel.js"_qs };
						if (!file.open (QIODevice::ReadOnly))
						{
							qWarning () << "unable to open WebChannel setup file"
									<< file.errorString ();
							return;
						}

						page->runJavaScript (file.readAll ());
					}

					channel->registerObject (id, object);

					auto js = R"(
								new QWebChannel(qt.webChannelTransport,
									function(channel) {
										window.%1 = channel.objects.%1;
										if (window.%1.init)
											window.%1.init();
									});
							)"_qs.arg (id);
					page->runJavaScript (js);
				},
				{});
	}

	void CustomWebView::Print (bool preview)
	{
		if (preview)
			qWarning () << "printing with preview is not supported yet";

		auto printer = std::make_shared<QPrinter> ();
		QPrintDialog dialog (printer.get (), this);
		dialog.setWindowTitle (tr ("Print web page"));

		if (dialog.exec () != QDialog::Accepted)
			return;

		// TODO port to Qt 6
		// page ()->print (printer.get (), [printer] (bool) {});
	}

	QPixmap CustomWebView::MakeFullPageSnapshot ()
	{
		// TODO
		return {};
	}

	QPoint CustomWebView::GetScrollPosition () const
	{
		return page ()->scrollPosition ().toPoint ();
	}

	void CustomWebView::SetScrollPosition (const QPoint& point)
	{
		page ()->runJavaScript ("window.scrollTo(%1, %2);"_qs.arg (point.x ()).arg (point.y ()));
	}

	double CustomWebView::GetZoomFactor () const
	{
		return page ()->zoomFactor ();
	}

	void CustomWebView::SetZoomFactor (double factor)
	{
		page ()->setZoomFactor (factor);
	}

	QString CustomWebView::GetDefaultTextEncoding () const
	{
		return settings ()->defaultTextEncoding ();
	}

	void CustomWebView::SetDefaultTextEncoding (const QString& encoding)
	{
		settings ()->setDefaultTextEncoding (encoding);
	}

	void CustomWebView::InitiateFind (const QString& text)
	{
		if (!text.isEmpty ())
			FindDialog_->SetText (text);
		FindDialog_->show ();
		FindDialog_->setFocus ();
	}

	QMenu* CustomWebView::CreateStandardContextMenu ()
	{
		return createStandardContextMenu ();
	}

	namespace
	{
		class HistoryWrapper : public IWebViewHistory
		{
			QWebEngineHistory * const History_;

			class Item : public IItem
			{
				const QWebEngineHistoryItem Item_;
				QWebEngineHistory * const History_;
			public:
				Item (const QWebEngineHistoryItem& item, QWebEngineHistory * const history)
				: Item_ { item }
				, History_ { history }
				{
				}

				~Item () override = default;

				Item (const Item&) = delete;
				Item (Item&&) = delete;

				Item& operator= (const Item&) = delete;
				Item& operator= (Item&&) = delete;

				bool IsValid () const override
				{
					return Item_.isValid ();
				}

				QString GetTitle () const override
				{
					return Item_.title ();
				}

				QUrl GetUrl () const override
				{
					return Item_.url ();
				}

				QIcon GetIcon () const override
				{
					return {};
				}

				void Navigate () override
				{
					History_->goToItem (Item_);
				}
			};

			static constexpr quint64 Magic_ = 0x62067d73bc85b872;
		public:
			explicit HistoryWrapper (QWebEngineHistory *history)
			: History_ { history }
			{
			}

			~HistoryWrapper () override = default;

			HistoryWrapper (const HistoryWrapper&) = delete;
			HistoryWrapper (HistoryWrapper&&) = delete;

			HistoryWrapper& operator= (const HistoryWrapper&) = delete;
			HistoryWrapper& operator= (HistoryWrapper&&) = delete;

			void Save (QDataStream& out) const override
			{
				out << Magic_ << *History_;
			}

			void Load (QDataStream& in) override
			{
				quint64 magic = 0;

				in.startTransaction ();
				in >> magic;
				if (magic == Magic_)
					in.commitTransaction ();
				else
				{
					in.abortTransaction ();
					return;
				}

				in >> *History_;
			}

			QList<IItem_ptr> GetItems (Direction dir, int maxItems) const override
			{
				const auto& srcItems = [&]
				{
					switch (dir)
					{
					case Direction::Forward:
						return History_->forwardItems (maxItems);
					case Direction::Backward:
						return History_->backItems (maxItems);
					}

					Util::Unreachable ();
				} ();

				QList<IItem_ptr> result;
				result.reserve (srcItems.size ());

				for (const auto& item : srcItems)
					result << std::make_shared<Item> (item, History_);

				return result;
			}
		};
	}

	IWebViewHistory_ptr CustomWebView::GetHistory ()
	{
		return std::make_shared<HistoryWrapper> (history ());
	}

	void CustomWebView::SetAttribute (Attribute attribute, bool enable)
	{
#define ATTR(x) \
		case Attribute::x: \
			settings ()->setAttribute (QWebEngineSettings::x, enable); \
			break;

		switch (attribute)
		{
		ATTR (AutoLoadImages)
		ATTR (PluginsEnabled)
		ATTR (JavascriptEnabled)
		ATTR (JavascriptCanOpenWindows)
		ATTR (JavascriptCanAccessClipboard)
		ATTR (LocalStorageEnabled)
		ATTR (XSSAuditingEnabled)
		ATTR (HyperlinkAuditingEnabled)
		ATTR (WebGLEnabled)
		ATTR (ScrollAnimatorEnabled)
		}

#undef ATTR
	}

	void CustomWebView::SetFontFamily (FontFamily family, const QFont& font)
	{
		const auto webFamily = static_cast<QWebEngineSettings::FontFamily> (family);
		settings ()->setFontFamily (webFamily, font.family ());
	}

	void CustomWebView::SetFontSize (FontSize type, int size)
	{
		const auto webSize = static_cast<QWebEngineSettings::FontSize> (type);
		settings ()->setFontSize (webSize, size);
	}

	QObject* CustomWebView::GetQObject ()
	{
		return this;
	}

	void CustomWebView::childEvent (QChildEvent *event)
	{
		const auto child = event->child ();
		switch (event->type ())
		{
		case QEvent::ChildAdded:
			child->installEventFilter (this);
			break;
		case QEvent::ChildRemoved:
			child->removeEventFilter (this);
			break;
		default:
			break;
		}
	}

	void CustomWebView::contextMenuEvent (QContextMenuEvent *event)
	{
		const auto& data = *lastContextMenuRequest ();
		emit contextMenuRequested (event->globalPos (),
				{
					data.isContentEditable (),
					data.selectedText (),
					data.linkUrl (),
					data.linkText (),
					data.mediaUrl (),
					{}
				});
	}

	bool CustomWebView::eventFilter (QObject *src, QEvent *event)
	{
		if (event->type () != QEvent::MouseButtonPress)
			return QWebEngineView::eventFilter (src, event);

		switch (dynamic_cast<QMouseEvent&> (*event).button ())
		{
		case Qt::BackButton:
			pageAction (QWebEnginePage::Back)->trigger ();
			return true;
		case Qt::ForwardButton:
			pageAction (QWebEnginePage::Forward)->trigger ();
			return true;
		default:
			return QWebEngineView::eventFilter (src, event);
		}
	}
}
