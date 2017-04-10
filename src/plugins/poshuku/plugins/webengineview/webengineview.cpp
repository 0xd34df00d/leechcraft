/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Boost Software License - Version 1.0 - August 17th, 2003
 *
 * Permission is hereby granted, free of charge, to any person or organization
 * obtaining a copy of the software and accompanying documentation covered by
 * this license (the "Software") to use, reproduce, display, distribute,
 * execute, and transmit the Software, and to prepare derivative works of the
 * Software, and to permit third-parties to whom the Software is furnished to
 * do so, all subject to the following:
 *
 * The copyright notices in the Software and this entire statement, including
 * the above license grant, this restriction and the following disclaimer,
 * must be included in all copies of the Software, in whole or in part, and
 * all derivative works of the Software, unless such copies or derivative
 * works are solely in the form of machine-executable object code generated by
 * a source language processor.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
 * SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
 * FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 **********************************************************************/

#include "webengineview.h"
#include <QIcon>
#include <QWebEngineProfile>
#include <util/sys/sysinfo.h>
#include <util/network/customcookiejar.h>
#include <interfaces/core/icoreproxy.h>
#include "customwebview.h"
#include "requestinterceptor.h"
#include "cookiessyncer.h"

namespace LeechCraft
{
namespace Poshuku
{
namespace WebEngineView
{
	namespace
	{
		QString GetDefaultUserAgent (const ICoreProxy_ptr& proxy,
				const QString& wkVer, const QString& chromeVer)
		{
#if defined(Q_OS_WIN32)
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

			const auto& lcVersion = proxy->GetVersion ();

			return QString { "Mozilla/5.0 (%1; %2) AppleWebKit/%3 (KHTML, like Gecko) Leechcraft/%5 Chrome/%4 Safari/%3" }
					.arg (platform)
					.arg (osVersion)
					.arg (wkVer)
					.arg (chromeVer)
					.arg (lcVersion.section ('-', 0, 0));
		}
	}

	void Plugin::Init (ICoreProxy_ptr proxy)
	{
		const auto prof = QWebEngineProfile::defaultProfile ();
		const auto& uaParts = prof->httpUserAgent ().split (' ');
		auto getVer = [&uaParts] (const QByteArray& marker)
		{
			return uaParts.filter (marker).value (0).section ('/', 1, 1);
		};
		const auto& wkVer = getVer ("AppleWebKit/");
		const auto& chromeVer = getVer ("Chrome/");
		prof->setHttpUserAgent (GetDefaultUserAgent (proxy, wkVer, chromeVer));

		Interceptor_ = std::make_shared<RequestInterceptor> ();
		prof->setRequestInterceptor (Interceptor_.get ());

		const auto cookieJar = proxy->GetNetworkAccessManager ()->cookieJar ();
		new CookiesSyncer { qobject_cast<Util::CustomCookieJar*> (cookieJar), prof->cookieStore () };
	}

	void Plugin::SecondInit ()
	{
	}

	void Plugin::Release ()
	{
		QWebEngineProfile::defaultProfile ()->setRequestInterceptor (nullptr);
		Interceptor_.reset ();
	}

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.Poshuku.WebEngineView";
	}

	QString Plugin::GetName () const
	{
		return "Poshuku WebEngineView";
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
		QSet<QByteArray> result;
		result << "org.LeechCraft.Poshuku.Plugins/1.0";
		return result;
	}

	IWebView* Plugin::CreateWebView ()
	{
		auto view = new CustomWebView;
		HandleView (view);
		return view;
	}

	QIcon Plugin::GetIconForUrl (const QUrl& url) const
	{
		return {};
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
				&CustomWebView::webViewCreated,
				this,
				[this] (CustomWebView *view, bool invert)
				{
					HandleView (view);
					emit webViewCreated (view, invert);
				});
	}
}
}
}

LC_EXPORT_PLUGIN (leechcraft_poshuku_webengineview, LeechCraft::Poshuku::WebEngineView::Plugin);
