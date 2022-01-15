/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "customwebview.h"
#include <cmath>
#include <limits>
#include <optional>
#include <qwebframe.h>
#include <qwebinspector.h>
#include <QMenu>
#include <QApplication>
#include <QBuffer>
#include <QClipboard>
#include <QFile>
#include <QWebElement>
#include <QWebHistory>
#include <QTextCodec>
#include <QMouseEvent>
#include <QPrinter>
#include <QPrintDialog>
#include <QPrintPreviewDialog>
#include <QFileDialog>
#include <QTimer>
#include <QtDebug>
#include <util/xpc/util.h>
#include <util/xpc/defaulthookproxy.h>
#include <util/gui/findnotificationwk.h>
#include <util/sll/unreachable.h>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/iiconthememanager.h>
#include "interfaces/poshuku/ibrowserwidget.h"
#include "interfaces/poshuku/iwebviewhistory.h"
#include "interfaces/poshuku/poshukutypes.h"
#include "customwebpage.h"
#include "customwebview.h"
#include "webviewsmoothscroller.h"
#include "webviewrendersettingshandler.h"
#include "webviewsslwatcherhandler.h"
#include "settingsinstancehandler.h"
#include "xmlsettingsmanager.h"

namespace LC
{
namespace Poshuku
{
namespace WebKitView
{
	CustomWebView::CustomWebView (const ICoreProxy_ptr& proxy,
			IProxyObject *poshukuProxy, QWidget *parent)
	: QWebView { parent }
	, Proxy_ { proxy }
	, WebInspector_
	{
		new QWebInspector,
		[] (QWebInspector *insp)
		{
			insp->hide ();
			insp->deleteLater ();
		}
	}
	{
		if (XmlSettingsManager::Instance ().property ("FixNonWhitePalettes").toBool ())
		{
			auto p = palette ();
			if (p.color (QPalette::Window) != Qt::white)
			{
				p.setColor (QPalette::Window, Qt::white);
				setPalette (p);
			}
		}

		const auto page = new CustomWebPage { proxy, poshukuProxy, this };
		setPage (page);
		page->HandleViewReady ();
		connect (page,
				&CustomWebPage::webViewCreated,
				this,
				&CustomWebView::webViewCreated);

		new WebViewSmoothScroller { this };
		new WebViewRenderSettingsHandler { this };

		new SettingsInstanceHandler { settings (), this };

		SslWatcherHandler_ = new WebViewSslWatcherHandler { this, proxy->GetIconThemeManager () };

		WebInspector_->setPage (page);

		connect (page,
				SIGNAL (printRequested (QWebFrame*)),
				this,
				SLOT (handlePrintRequested (QWebFrame*)));
		connect (page,
				SIGNAL (windowCloseRequested ()),
				this,
				SIGNAL (closeRequested ()));
		connect (page,
				SIGNAL (storeFormData (PageFormsData_t)),
				this,
				SIGNAL (storeFormData (PageFormsData_t)));
		connect (page,
				SIGNAL (loadFinished (bool)),
				this,
				SIGNAL (loadFinished (bool)));

		connect (page,
				SIGNAL (linkHovered (QString, QString, QString)),
				this,
				SIGNAL (linkHovered (QString, QString, QString)));

		connect (page->mainFrame (),
				SIGNAL (initialLayoutCompleted ()),
				this,
				SIGNAL (earliestViewLayout ()));

		connect (page,
				SIGNAL (featurePermissionRequested (QWebFrame*, QWebPage::Feature)),
				this,
				SLOT (handleFeaturePermissionReq (QWebFrame*, QWebPage::Feature)));
	}

	void CustomWebView::Load (const QUrl& url, const QString& title)
	{
		if (url.isEmpty () || !url.isValid ())
			return;

		if (url.scheme () == "javascript")
		{
			const auto& result = page ()->mainFrame ()->evaluateJavaScript (url.toString ().mid (11));
			if (result.canConvert (QVariant::String))
				setHtml (result.toString ());
			return;
		}

		emit navigateRequested (url);

		if (url.scheme () == "about")
		{
			if (url.path () == "plugins")
				NavigatePlugins ();
			else if (url.path () == "home")
				NavigateHome ();
			return;
		}

		if (title.isEmpty ())
			emit titleChanged (tr ("Loading..."));
		else
			emit titleChanged (title);
		load (url);
		emit urlChanged (url);
	}

	void CustomWebView::Load (const QNetworkRequest& req,
			QNetworkAccessManager::Operation op, const QByteArray& ba)
	{
		emit titleChanged (tr ("Loading..."));
		QWebView::load (req, op, ba);
	}

	QString CustomWebView::URLToProperString (const QUrl& url) const
	{
		QString string = url.toString ();
		QWebElement equivs = page ()->mainFrame ()->
				findFirstElement ("meta[http-equiv=\"Content-Type\"]");
		if (!equivs.isNull ())
		{
			QString content = equivs.attribute ("content", "text/html; charset=UTF-8");
			const QString charset = "charset=";
			int pos = content.indexOf (charset);
			if (pos >= 0)
				PreviousEncoding_ = content.mid (pos + charset.length ()).toLower ();
		}

		if (PreviousEncoding_ != "utf-8" &&
				PreviousEncoding_ != "utf8" &&
				!PreviousEncoding_.isEmpty ())
			string = url.toEncoded ();

		return string;
	}

	void CustomWebView::Print (bool preview)
	{
		PrintImpl (preview, page ()->mainFrame ());
	}

	QPixmap CustomWebView::MakeFullPageSnapshot ()
	{
		QSize contentsSize = page ()->mainFrame ()->contentsSize ();
		QSize oldSize = page ()->viewportSize ();
		QRegion clip (0, 0, contentsSize.width (), contentsSize.height ());

		QPixmap image (contentsSize);
		QPainter painter (&image);
		page ()->setViewportSize (contentsSize);
		page ()->mainFrame ()->render (&painter, clip);
		page ()->setViewportSize (oldSize);
		return image;
	}

	void CustomWebView::SurroundingsInitialized ()
	{
		FindDialog_ = new Util::FindNotificationWk { Proxy_, this };
		FindDialog_->hide ();
	}

	QWidget* CustomWebView::GetQWidget ()
	{
		return this;
	}

	QList<QAction*> CustomWebView::GetActions (ActionArea area) const
	{
		switch (area)
		{
		case ActionArea::UrlBar:
			return { SslWatcherHandler_->GetStateAction () };
		}

		Util::Unreachable ();
	}

	QAction* CustomWebView::GetPageAction (PageAction action) const
	{
#define ACT(x) \
		case PageAction::x: \
			return pageAction (QWebPage::x);

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
		ACT (OpenImageInNewWindow)
		ACT (DownloadImageToDisk)
		ACT (CopyImageToClipboard)
		ACT (CopyImageUrlToClipboard)
		ACT (InspectElement)
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
		return URLToProperString (url ());
	}

	QIcon CustomWebView::GetIcon () const
	{
		return icon ();
	}

	void CustomWebView::SetContent (const QByteArray& data, const QByteArray& mime, const QUrl& base)
	{
		setContent (data, mime, base);
	}

	void CustomWebView::ToHtml (const std::function<void (QString)>& handler) const
	{
		handler (page ()->mainFrame ()->toHtml ());
	}

	void CustomWebView::EvaluateJS (const QString& js,
			const std::function<void (QVariant)>& callback,
			Util::BitFlags<EvaluateJSFlag> flags)
	{
		auto eval = std::make_shared<std::function<void (QWebFrame*)>> ();
		*eval = [=] (QWebFrame *frame)
		{
			const auto& res = frame->evaluateJavaScript (js);
			if (callback)
				callback (res);

			if (flags & EvaluateJSFlag::RecurseSubframes)
				QTimer::singleShot (0,
						[eval, framePtr = QPointer { frame }]
						{
							if (!framePtr)
								return;

							for (const auto child : framePtr->childFrames ())
								(*eval) (child);
						});
		};

		(*eval) (page ()->mainFrame ());
	}

	void CustomWebView::AddJavaScriptObject (const QString& id, QObject *object)
	{
		page ()->mainFrame ()->addToJavaScriptWindowObject (id, object);

		static const QString initter { "if (window.%1.init) window.%1.init();" };
		page ()->mainFrame ()->evaluateJavaScript (initter.arg (id));
	}

	QPoint CustomWebView::GetScrollPosition () const
	{
		return page ()->mainFrame ()->scrollPosition ();
	}

	void CustomWebView::SetScrollPosition (const QPoint& point)
	{
		page ()->mainFrame ()->setScrollPosition (point);
	}

	double CustomWebView::GetZoomFactor () const
	{
		return zoomFactor ();
	}

	void CustomWebView::SetZoomFactor (double factor)
	{
		setZoomFactor (factor);
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
		return page ()->createStandardContextMenu ();
	}

	namespace
	{
		class HistoryWrapper final : public IWebViewHistory
		{
			QWebHistory * const History_;

			class Item : public IItem
			{
				const QWebHistoryItem Item_;
				QWebHistory * const History_;
			public:
				Item (const QWebHistoryItem& item, QWebHistory* const history)
				: Item_ { item }
				, History_ { history }
				{
				}

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

			static constexpr quint64 Magic_ = 0xfb1cc74ce40369af ;
		public:
			HistoryWrapper (QWebHistory *history)
			: History_ { history }
			{
			}

			void Save (QDataStream& out) const override
			{
				out << Magic_ << *History_;
			}

			void Load (QDataStream& in) override
			{
				quint64 magic;

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
			settings ()->setAttribute (QWebSettings::x, enable); \
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
		settings ()->setFontFamily (static_cast<QWebSettings::FontFamily> (family), font.family ());
	}

	void CustomWebView::SetFontSize (FontSize type, int size)
	{
		settings ()->setFontSize (static_cast<QWebSettings::FontSize> (type), size);
	}

	QObject* CustomWebView::GetQObject ()
	{
		return this;
	}

	void CustomWebView::mousePressEvent (QMouseEvent *e)
	{
		const bool mBack = e->button () == Qt::XButton1;
		const bool mForward = e->button () == Qt::XButton2;
		if (mBack || mForward)
		{
			pageAction (mBack ? QWebPage::Back : QWebPage::Forward)->trigger ();
			e->accept ();
			return;
		}

		QWebView::mousePressEvent (e);
	}

	void CustomWebView::contextMenuEvent (QContextMenuEvent *e)
	{
		const auto& r = page ()->mainFrame ()->hitTestContent (e->pos ());
		emit contextMenuRequested (mapToGlobal (e->pos ()),
				{
					r.isContentEditable (),
					page ()->selectedText (),
					r.linkUrl (),
					r.linkText (),
					r.imageUrl (),
					r.pixmap ()
				});
	}

	void CustomWebView::keyReleaseEvent (QKeyEvent *event)
	{
		if (event->matches (QKeySequence::Copy))
			pageAction (QWebPage::Copy)->trigger ();
		else
			QWebView::keyReleaseEvent (event);
	}

	void CustomWebView::NavigatePlugins ()
	{
		QFile pef (":/resources/html/pluginsenum.html");
		pef.open (QIODevice::ReadOnly);
		QString contents = QString (pef.readAll ())
			.replace ("INSTALLEDPLUGINS", tr ("Installed plugins"))
			.replace ("NOPLUGINS", tr ("No plugins installed"))
			.replace ("FILENAME", tr ("File name"))
			.replace ("MIME", tr ("MIME type"))
			.replace ("DESCR", tr ("Description"))
			.replace ("SUFFIXES", tr ("Suffixes"))
			.replace ("ENABLED", tr ("Enabled"))
			.replace ("NO", tr ("No"))
			.replace ("YES", tr ("Yes"));
		setHtml (contents);
	}

	void CustomWebView::NavigateHome ()
	{
		QFile file (":/resources/html/home.html");
		if (!file.open (QIODevice::ReadOnly))
		{
			qCritical () << Q_FUNC_INFO
					<< "cannot open"
					<< file.fileName ()
					<< file.errorString ();
			return;
		}

		QString data = file.readAll ();
		data.replace ("{pagetitle}",
				tr ("Welcome to LeechCraft!"));
		data.replace ("{title}",
				tr ("Welcome to LeechCraft!"));
		data.replace ("{body}",
				tr ("Welcome to LeechCraft, the integrated internet-client.<br />"
					"More info is available on the <a href='https://leechcraft.org'>"
					"project's site</a>."));

		QBuffer iconBuffer;
		iconBuffer.open (QIODevice::ReadWrite);
		auto pixmap = Proxy_->GetIconThemeManager ()->GetPluginIcon ().pixmap (QSize { 256, 256 });
		pixmap.save (&iconBuffer, "PNG");

		data.replace ("{img}",
				QByteArray ("data:image/png;base64,") + iconBuffer.buffer ().toBase64 ());

		setHtml (data);
	}

	void CustomWebView::PrintImpl (bool preview, QWebFrame *frame)
	{
		QPrinter printer;
		if (preview)
		{
			QPrintPreviewDialog prevDialog (&printer, this);
			connect (&prevDialog,
					SIGNAL (paintRequested (QPrinter*)),
					frame,
					SLOT (print (QPrinter*)));

			if (prevDialog.exec () != QDialog::Accepted)
				return;
		}
		else
		{
			QPrintDialog dialog (&printer, this);
			dialog.setWindowTitle (tr ("Print web page"));

			if (dialog.exec () != QDialog::Accepted)
				return;

			frame->print (&printer);
		}
	}

	void CustomWebView::handlePrintRequested (QWebFrame *frame)
	{
		PrintImpl (false, frame);
	}

	namespace
	{
		QWebPage::PermissionPolicy ConvertPerm (IWebView::Permission perm)
		{
			switch (perm)
			{
			case IWebView::Permission::Grant:
				return QWebPage::PermissionGrantedByUser;
			case IWebView::Permission::Deny:
				return QWebPage::PermissionDeniedByUser;
			}

			Util::Unreachable ();
		}

		class FrameFeatureSecurityOrigin : public IWebView::IFeatureSecurityOrigin
		{
			QWebFrame * const Frame_;
			QWebPage * const Page_;
			const QWebPage::Feature Feature_;
		public:
			FrameFeatureSecurityOrigin (QWebFrame *frame,
					QWebPage *page, QWebPage::Feature feature)
			: Frame_ { frame }
			, Page_ { page }
			, Feature_ { feature }
			{
			}

			QString GetName () const override
			{
				return Frame_->url ().host ();
			}

			void SetPermission (IWebView::Permission perm) override
			{
				Page_->setFeaturePermission (Frame_, Feature_, ConvertPerm (perm));
			}
		};

		std::optional<IWebView::Feature> ConvertFeature (QWebPage::Feature feature)
		{
			switch (feature)
			{
			case QWebPage::Notifications:
				return IWebView::Feature::Notifications;
			case QWebPage::Geolocation:
				return IWebView::Feature::Geolocation;
			}

			qWarning () << Q_FUNC_INFO
					<< "unknown feature"
					<< feature;

			return {};
		}
	}

	void CustomWebView::handleFeaturePermissionReq (QWebFrame *frame, QWebPage::Feature feature)
	{
		if (const auto converted = ConvertFeature (feature))
		{
			const auto origin = std::make_shared<FrameFeatureSecurityOrigin> (frame, page (), feature);
			emit featurePermissionRequested (origin, *converted);
		}
	}
}
}
}
