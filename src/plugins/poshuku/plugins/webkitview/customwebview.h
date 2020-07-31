/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <memory>
#include <qwebview.h>
#include <interfaces/structures.h>
#include <interfaces/iwkfontssettable.h>
#include <interfaces/core/icoreproxyfwd.h>
#include <interfaces/core/ihookproxy.h>
#include "interfaces/poshuku/poshukutypes.h"
#include "interfaces/poshuku/iwebview.h"

class QTimer;
class QWebInspector;

class IEntityManager;

namespace LC
{
namespace Util
{
	class FindNotificationWk;
}

namespace Poshuku
{
class IProxyObject;

namespace WebKitView
{
	class WebViewSslWatcherHandler;

	class CustomWebView : public QWebView
						, public IWebView
						, public IWkFontsSettable
	{
		Q_OBJECT
		Q_INTERFACES (IWkFontsSettable LC::Poshuku::IWebView)

		const ICoreProxy_ptr Proxy_;

		mutable QString PreviousEncoding_;

		std::shared_ptr<QWebInspector> WebInspector_;

		const WebViewSslWatcherHandler *SslWatcherHandler_;

		Util::FindNotificationWk *FindDialog_ = nullptr;
	public:
		CustomWebView (const ICoreProxy_ptr&, IProxyObject*, QWidget* = nullptr);

		void SurroundingsInitialized () override;

		QWidget* GetQWidget () override;
		QList<QAction*> GetActions (ActionArea) const override;

		QAction* GetPageAction (PageAction) const override;

		QString GetTitle () const override;
		QUrl GetUrl () const override;
		QString GetHumanReadableUrl () const override;
		QIcon GetIcon () const override;

		void Load (const QUrl&, const QString&) override;

		void SetContent (const QByteArray&, const QByteArray&, const QUrl& = {}) override;
		void ToHtml (const std::function<void (QString)>&) const override;
		void EvaluateJS (const QString&,
				const std::function<void (QVariant)>&,
				Util::BitFlags<EvaluateJSFlag>) override;
		void AddJavaScriptObject (const QString&, QObject*) override;

		void Print (bool preview) override;
		QPixmap MakeFullPageSnapshot () override;

		QPoint GetScrollPosition () const override;
		void SetScrollPosition (const QPoint&) override;
		double GetZoomFactor () const override;
		void SetZoomFactor (double) override;

		QString GetDefaultTextEncoding () const override;
		void SetDefaultTextEncoding (const QString&) override;

		void InitiateFind (const QString&) override;

		QMenu* CreateStandardContextMenu () override;

		IWebViewHistory_ptr GetHistory () override;

		void SetAttribute (Attribute, bool) override;

		void SetFontFamily (FontFamily family, const QFont& font) override;
		void SetFontSize (FontSize type, int size) override;
		QObject* GetQObject () override;

		void Load (const QNetworkRequest&,
				QNetworkAccessManager::Operation = QNetworkAccessManager::GetOperation,
				const QByteArray& = QByteArray ());

		/** This function is equivalent to url.toString() if the url is
		 * all in UTF-8. But if the site is in another encoding,
		 * QUrl::toString() returns a bad, unreadable and, moreover,
		 * unusable string. In this case, this function converts the url
		 * to its percent-encoding representation.
		 *
		 * @param[in] url The possibly non-UTF-8 URL.
		 * @return The \em url converted to Unicode.
		 */
		QString URLToProperString (const QUrl& url) const;
	protected:
		void mousePressEvent (QMouseEvent*) override;
		void contextMenuEvent (QContextMenuEvent*) override;
		void keyReleaseEvent (QKeyEvent*) override;
	private:
		void NavigatePlugins ();
		void NavigateHome ();
		void PrintImpl (bool, QWebFrame*);
	private slots:
		void handlePrintRequested (QWebFrame*);

		void handleFeaturePermissionReq (QWebFrame*, QWebPage::Feature);
	signals:
		void closeRequested () override;

		void navigateRequested (const QUrl&);

		void zoomChanged () override;

		void contextMenuRequested (const QPoint& globalPos, const ContextMenuInfo&) override;

		void earliestViewLayout () override;
		void linkHovered (const QString& link, const QString& title, const QString& textContent) override;
		void storeFormData (const PageFormsData_t&) override;
		void featurePermissionRequested (const IWebView::IFeatureSecurityOrigin_ptr&,
				IWebView::Feature) override;

		void webViewCreated (const std::shared_ptr<CustomWebView>&, bool);
	};
}
}
}
