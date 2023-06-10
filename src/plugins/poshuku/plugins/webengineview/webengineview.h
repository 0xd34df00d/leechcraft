/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <memory>
#include <QObject>
#include <interfaces/iinfo.h>
#include <interfaces/iplugin2.h>
#include <interfaces/poshuku/iwebviewprovider.h>
#include <interfaces/poshuku/iinterceptablerequests.h>

namespace LC::Poshuku
{
class IProxyObject;

namespace WebEngineView
{
	class RequestInterceptor;
	class CustomWebView;
	class IconDatabase;

	class Plugin : public QObject
				 , public IInfo
				 , public IPlugin2
				 , public IWebViewProvider
				 , public IInterceptableRequests
	{
		Q_OBJECT
		Q_INTERFACES (IInfo
				IPlugin2
				LC::Poshuku::IWebViewProvider
				LC::Poshuku::IInterceptableRequests)

		Q_PLUGIN_METADATA (IID "org.LeechCraft.Poshuku.WebEngineView" FILE "manifest.json")
		DEFINE_PROXY

		std::shared_ptr<RequestInterceptor> Interceptor_;

		IProxyObject *PoshukuProxy_ = nullptr;

		std::shared_ptr<IconDatabase> IconDB_;
	public:
		void Init (ICoreProxy_ptr) override;
		void SecondInit () override;
		void Release () override;
		QByteArray GetUniqueID () const override;
		QString GetName () const override;
		QString GetInfo () const override;
		QIcon GetIcon () const override;

		QSet<QByteArray> GetPluginClasses () const override;
		std::shared_ptr<IWebView> CreateWebView () override;
		QIcon GetIconForUrl (const QUrl&) const override;
		QIcon GetDefaultUrlIcon () const override;

		void AddInterceptor (const Interceptor_t&) override;
	private:
		void HandleView (CustomWebView*);
	public slots:
		void initPlugin (QObject*);
	signals:
		void webViewCreated (const std::shared_ptr<IWebView>&, bool) override;
	};
}
}
