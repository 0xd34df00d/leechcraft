/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QWebEngineView>
#include <interfaces/poshuku/iwebview.h>
#include <interfaces/poshuku/iwebviewprovider.h>
#include <interfaces/iwkfontssettable.h>

namespace LC::Util
{
	class FindNotificationWE;
}

namespace LC::Poshuku
{
class IProxyObject;

namespace WebEngineView
{
	class CustomWebView final : public QWebEngineView
							  , public IWebView
							  , public IWkFontsSettable
	{
		Q_OBJECT
		Q_INTERFACES (IWkFontsSettable LC::Poshuku::IWebView)

		Util::FindNotificationWE *FindDialog_ = nullptr;
	public:
		explicit CustomWebView (IProxyObject*);

		void SurroundingsInitialized () override;

		QWidget* GetQWidget () override;

		QList<QAction*> GetActions (ActionArea area) const override;
		QAction* GetPageAction (PageAction action) const override;

		QString GetTitle () const override;
		QUrl GetUrl () const override;
		QString GetHumanReadableUrl () const override;
		QIcon GetIcon () const override;

		void Load (const QUrl& url, const QString& title) override;

		void SetContent (const QByteArray& data, const QByteArray& mime, const QUrl& base) override;
		void ToHtml (const std::function<void (QString)>&) const override;

		void EvaluateJS (const QString& js,
				const std::function<void (QVariant)>& handler,
				Util::BitFlags<EvaluateJSFlag>) override;
		void AddJavaScriptObject (const QString& id, QObject* object) override;

		void Print (bool withPreview) override;
		QPixmap MakeFullPageSnapshot () override;

		QPoint GetScrollPosition () const override;
		void SetScrollPosition (const QPoint& point) override;
		double GetZoomFactor () const override;
		void SetZoomFactor (double d) override;

		QString GetDefaultTextEncoding () const override;
		void SetDefaultTextEncoding (const QString& encoding) override;

		void InitiateFind (const QString& text) override;

		QMenu* CreateStandardContextMenu () override;

		IWebViewHistory_ptr GetHistory () override;

		void SetAttribute (Attribute attribute, bool b) override;

		void SetFontFamily (FontFamily family, const QFont& font) override;
		void SetFontSize (FontSize type, int size) override;
		QObject* GetQObject () override;
	protected:
		void childEvent (QChildEvent*) override;
		void contextMenuEvent (QContextMenuEvent*) override;
		bool eventFilter (QObject*, QEvent*) override;
	signals:
		void earliestViewLayout () override;
		void loadFinished (bool) override;
		void linkHovered (const QString&, const QString&, const QString&) override;
		void storeFormData (const PageFormsData_t&) override;
		void featurePermissionRequested (const IWebView::IFeatureSecurityOrigin_ptr&, IWebView::Feature) override;
		void zoomChanged () override;
		void closeRequested () override;

		void contextMenuRequested (const QPoint& globalPos, const ContextMenuInfo&) override;

		void iconChanged ();
		void statusBarMessage (const QString&);

		void webViewCreated (const std::shared_ptr<CustomWebView>&, NewWebViewBehavior::Enum);
	};
}
}
