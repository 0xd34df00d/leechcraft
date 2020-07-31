/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <QNetworkAccessManager>
#include <interfaces/iinfo.h>
#include <interfaces/iplugin2.h>
#include <interfaces/ihavesettings.h>
#include <interfaces/ihavediaginfo.h>
#include <interfaces/core/ihookproxy.h>
#include <interfaces/poshuku/iwebviewprovider.h>
#include <interfaces/poshuku/iinterceptablerequests.h>

namespace LC
{
namespace Poshuku
{
class IProxyObject;

namespace WebKitView
{
	class WebPluginFactory;
	class InterceptAdaptor;
	class CustomWebView;

	class Plugin : public QObject
				 , public IInfo
				 , public IPlugin2
				 , public IHaveSettings
				 , public IHaveDiagInfo
				 , public IWebViewProvider
				 , public IInterceptableRequests
	{
		Q_OBJECT
		Q_INTERFACES (IInfo
				IPlugin2
				IHaveSettings
				IHaveDiagInfo
				LC::Poshuku::IWebViewProvider
				LC::Poshuku::IInterceptableRequests)

		LC_PLUGIN_METADATA ("org.LeechCraft.Poshuku.WebKitView")

		ICoreProxy_ptr Proxy_;
		IProxyObject *PoshukuProxy_ = nullptr;

		WebPluginFactory *WebPluginFactory_ = nullptr;

		Util::XmlSettingsDialog_ptr XSD_;

		std::shared_ptr<InterceptAdaptor> Interceptor_;
	public:
		void Init (ICoreProxy_ptr) override;
		void SecondInit () override;
		void Release () override;
		QByteArray GetUniqueID () const override;
		QString GetName () const override;
		QString GetInfo () const override;
		QIcon GetIcon () const override;

		QSet<QByteArray> GetPluginClasses () const override;

		Util::XmlSettingsDialog_ptr GetSettingsDialog () const override;

		QString GetDiagInfoString () const override;

		IWebView_ptr CreateWebView () override;
		QIcon GetIconForUrl (const QUrl&) const override;
		QIcon GetDefaultUrlIcon () const override;

		void AddInterceptor (const Interceptor_t&) override;
	private:
		void HandleView (CustomWebView*);
	private slots:
		void handleWebViewCreated (const std::shared_ptr<CustomWebView>&, bool);
	public slots:
		void hookNAMCreateRequest (LC::IHookProxy_ptr,
				QNetworkAccessManager*,
				QNetworkAccessManager::Operation*,
				QIODevice**);

		void initPlugin (QObject*);
	signals:
		void webViewCreated (const std::shared_ptr<IWebView>&, bool) override;
	};
}
}
}
