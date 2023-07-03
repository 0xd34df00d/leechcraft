/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "webengineview.h"
#include <QIcon>
#include <QWebEngineProfile>
#include <util/network/customcookiejar.h>
#include <util/sll/qtutil.h>
#include <util/sys/sysinfo.h>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/poshuku/iproxyobject.h>
#include "customwebview.h"
#include "requestinterceptor.h"
#include "cookiessyncer.h"
#include "downloaditemhandler.h"
#include "icondatabase.h"
#include "lcschemehandler.h"

namespace LC::Poshuku::WebEngineView
{
	namespace
	{
		QString GetDefaultUserAgent (const QString& wkVer, const QString& chromeVer)
		{
#if defined (Q_OS_WIN32)
			const auto platform = "Windows";
#elif defined (Q_OS_MAC)
			const auto platform = "Macintosh";
#else
			const auto platform = "X11";
#endif

			const auto& osInfo = Util::SysInfo::GetOSInfo ();
			auto osVersion = osInfo.Flavour_;
			if (!osInfo.Arch_.isEmpty ())
				osVersion += " " + osInfo.Arch_;

			const auto& lcVersion = GetProxyHolder ()->GetVersion ();

			return "Mozilla/5.0 (%1; %2) AppleWebKit/%3 (KHTML, like Gecko) Leechcraft/%5 Chrome/%4 Safari/%3"_qs
					.arg (platform, osVersion, wkVer, chromeVer, lcVersion.section ('-', 0, 0));
		}
	}

	void Plugin::Init (ICoreProxy_ptr)
	{
		const auto prof = QWebEngineProfile::defaultProfile ();
		const auto& uaParts = prof->httpUserAgent ().split (' ');
		auto getVer = [&uaParts] (const QByteArray& marker)
		{
			return uaParts.filter (marker).value (0).section ('/', 1, 1);
		};
		const auto& wkVer = getVer ("AppleWebKit/");
		const auto& chromeVer = getVer ("Chrome/");
		prof->setHttpUserAgent (GetDefaultUserAgent (wkVer, chromeVer));

		Interceptor_ = std::make_shared<RequestInterceptor> ();
		prof->setUrlRequestInterceptor (Interceptor_.get ());

		LcSchemeHandler::Register (*prof);

		new DownloadItemHandler (prof);

		const auto cookieJar = GetProxyHolder ()->GetNetworkAccessManager ()->cookieJar ();
		new CookiesSyncer { qobject_cast<Util::CustomCookieJar*> (cookieJar), prof->cookieStore () };

		IconDB_ = std::make_shared<IconDatabase> ();
	}

	void Plugin::SecondInit ()
	{
	}

	void Plugin::Release ()
	{
		QWebEngineProfile::defaultProfile ()->setUrlRequestInterceptor (nullptr);
		Interceptor_.reset ();
	}

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.Poshuku.WebEngineView";
	}

	QString Plugin::GetName () const
	{
		return "Poshuku WebEngineView"_qs;
	}

	QString Plugin::GetInfo () const
	{
		return tr ("Provides QtWebEngine-based backend for Poshuku.");
	}

	QIcon Plugin::GetIcon () const
	{
		return {};
	}

	QSet<QByteArray> Plugin::GetPluginClasses () const
	{
		return { "org.LeechCraft.Poshuku.Plugins/1.0" };
	}

	IWebView_ptr Plugin::CreateWebView ()
	{
		auto view = std::make_shared<CustomWebView> (PoshukuProxy_);
		HandleView (view.get ());
		return view;
	}

	QIcon Plugin::GetIconForUrl (const QUrl& url) const
	{
		return IconDB_->GetIcon (url);
	}

	QIcon Plugin::GetDefaultUrlIcon () const
	{
		return {};
	}

	void Plugin::AddInterceptor (const Interceptor_t& interceptor)
	{
		Interceptor_->Add (interceptor);
	}

	void Plugin::HandleView (CustomWebView *view)
	{
		Interceptor_->RegisterView (view);
		connect (view,
				&QWebEngineView::iconChanged,
				this,
				[this, view] (const QIcon& icon)
				{
					IconDB_->UpdateIcon (view->url (), icon, view->iconUrl ());
				});

		connect (view,
				&CustomWebView::webViewCreated,
				this,
				[this] (const std::shared_ptr<CustomWebView>& view, bool invert)
				{
					HandleView (view.get ());
					emit webViewCreated (view, invert);
				});
	}

	void Plugin::initPlugin (QObject *proxyObj)
	{
		PoshukuProxy_ = qobject_cast<IProxyObject*> (proxyObj);
	}
}

LC_EXPORT_PLUGIN (leechcraft_poshuku_webengineview, LC::Poshuku::WebEngineView::Plugin);
