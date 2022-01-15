/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <memory>
#include <functional>
#include <util/sll/bitflags.h>
#include "poshukutypes.h"

class QUrl;
class QAction;
class QString;
class QWidget;
class QPixmap;

class QMenu;

template<typename>
class QList;

namespace LC
{
namespace Poshuku
{
	class IWebViewHistory;
	using IWebViewHistory_ptr = std::shared_ptr<IWebViewHistory>;

	/** @brief Interface for QWebView-like widgets displaying HTML content.
	 *
	 * The implementations of this interface are also expected to have the
	 * following signals, following QWebView API:
	 * - loadStarted()
	 * - loadProgress(int)
	 * - loadFinished(bool)
	 * - iconChanged()
	 * - titleChanged(QString)
	 * - urlChanged(QUrl)
	 * - urlChanged(QString)
	 * - zoomChanged()
	 * - statusBarMessage(QString)
	 * - closeRequested()
	 *
	 * The objects implementing this interface may also implement
	 * IWkFontsSettable, in which case their fonts will be configurable
	 * in Poshuku settings.
	 *
	 * @sa IWkFontSettable
	 */
	class IWebView
	{
	public:
		virtual ~IWebView () = default;

		enum class ActionArea
		{
			UrlBar
		};

		enum class PageAction
		{
			Reload,
			ReloadAndBypassCache,
			Stop,

			Back,
			Forward,

			Cut,
			Copy,
			Paste,

			CopyLinkToClipboard,
			DownloadLinkToDisk,

			OpenImageInNewWindow,
			DownloadImageToDisk,
			CopyImageToClipboard,
			CopyImageUrlToClipboard,

			InspectElement
		};

		enum class Feature
		{
			Notifications,
			Geolocation
		};

		enum class Permission
		{
			Grant,
			Deny
		};

		enum class Attribute
		{
			AutoLoadImages,
			JavascriptEnabled,
			PluginsEnabled,
			JavascriptCanOpenWindows,
			JavascriptCanAccessClipboard,
			LocalStorageEnabled,
			XSSAuditingEnabled,
			HyperlinkAuditingEnabled,
			WebGLEnabled,
			ScrollAnimatorEnabled
		};

		enum class NavigationType
		{
			LinkClicked,
			Typed,
			FormSubmitted,
			BackForward,
			Reload,
			Redirect,
			Other,
		};

		class IFeatureSecurityOrigin
		{
		protected:
			virtual ~IFeatureSecurityOrigin () = default;
		public:
			virtual QString GetName () const = 0;

			virtual void SetPermission (Permission) = 0;
		};

		using IFeatureSecurityOrigin_ptr = std::shared_ptr<IFeatureSecurityOrigin>;

		virtual void SurroundingsInitialized () = 0;

		virtual QWidget* GetQWidget () = 0;

		virtual QList<QAction*> GetActions (ActionArea) const = 0;

		virtual QAction* GetPageAction (PageAction) const = 0;

		virtual QString GetTitle () const = 0;
		virtual QUrl GetUrl () const = 0;
		virtual QString GetHumanReadableUrl () const = 0;
		virtual QIcon GetIcon () const = 0;

		virtual void Load (const QUrl& url, const QString& title = {}) = 0;

		virtual void SetContent (const QByteArray& data, const QByteArray& mime, const QUrl& base = {}) = 0;

		virtual void ToHtml (const std::function<void (QString)>&) const = 0;

		enum class EvaluateJSFlag
		{
			None			 = 0b00,
			RecurseSubframes = 0b01
		};

		virtual void EvaluateJS (const QString& js,
				const std::function<void (QVariant)>& handler = {},
				Util::BitFlags<EvaluateJSFlag> = {}) = 0;
		virtual void AddJavaScriptObject (const QString& id, QObject *object) = 0;

		virtual void Print (bool withPreview) = 0;
		virtual QPixmap MakeFullPageSnapshot () = 0;

		virtual QPoint GetScrollPosition () const = 0;
		virtual void SetScrollPosition (const QPoint&) = 0;

		virtual double GetZoomFactor () const = 0;
		virtual void SetZoomFactor (double) = 0;

		virtual QString GetDefaultTextEncoding () const = 0;
		virtual void SetDefaultTextEncoding (const QString& encoding) = 0;

		virtual void InitiateFind (const QString& text) = 0;

		virtual QMenu* CreateStandardContextMenu () = 0;

		virtual IWebViewHistory_ptr GetHistory () = 0;

		virtual void SetAttribute (Attribute, bool) = 0;
	protected:
		virtual void loadFinished (bool) = 0;

		virtual void earliestViewLayout () = 0;

		virtual void linkHovered (const QString& link,
				const QString& title, const QString& textContent) = 0;

		virtual void storeFormData (const PageFormsData_t&) = 0;

		virtual void featurePermissionRequested (const IWebView::IFeatureSecurityOrigin_ptr& origin,
				IWebView::Feature feature) = 0;

		virtual void zoomChanged () = 0;

		virtual void closeRequested () = 0;

		virtual void contextMenuRequested (const QPoint& globalPos,
				const ContextMenuInfo& info) = 0;
	};

	using IWebView_ptr = std::shared_ptr<IWebView>;
}
}

Q_DECLARE_INTERFACE (LC::Poshuku::IWebView,
		"org.LeechCraft.Poshuku.IWebView/1.0")
