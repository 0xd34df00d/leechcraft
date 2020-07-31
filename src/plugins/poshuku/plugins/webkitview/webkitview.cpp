/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "webkitview.h"
#include <QIcon>
#include <QDir>
#include <qwebsettings.h>
#include <qtwebkitversion.h>
#include <xmlsettingsdialog/xmlsettingsdialog.h>
#include <util/util.h>
#include <util/sys/paths.h>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/poshuku/iproxyobject.h>
#include "customwebview.h"
#include "customwebpage.h"
#include "webpluginfactory.h"
#include "linkhistory.h"
#include "xmlsettingsmanager.h"
#include "settingsglobalhandler.h"
#include "interceptadaptor.h"

namespace LC
{
namespace Poshuku
{
namespace WebKitView
{
	void Plugin::Init (ICoreProxy_ptr proxy)
	{
		Util::InstallTranslator ("poshuku_webkitview");

		Proxy_ = proxy;

		QWebHistoryInterface::setDefaultInterface (new LinkHistory);

		XSD_ = std::make_shared<Util::XmlSettingsDialog> ();
		XSD_->RegisterObject (&XmlSettingsManager::Instance (), "poshukuwebkitviewsettings.xml");

		const auto sgh = new SettingsGlobalHandler { this };
		connect (XSD_.get (),
				SIGNAL (pushButtonClicked (QString)),
				sgh,
				SLOT (handleSettingsClicked (QString)));

		try
		{
			const auto& path = Util::GetUserDir (Util::UserDir::Cache, "poshuku/favicons").absolutePath ();
			QWebSettings::setIconDatabasePath (path);
		}
		catch (const std::runtime_error& e)
		{
			qWarning () << Q_FUNC_INFO
					<< e.what ();
		}

		try
		{
			const auto& path = Util::CreateIfNotExists ("poshuku/offlinestorage").absolutePath ();
			QWebSettings::setOfflineStoragePath (path);
		}
		catch (const std::runtime_error& e)
		{
			qWarning () << Q_FUNC_INFO
					<< e.what ();
		}

		try
		{
			const auto& path = Util::GetUserDir (Util::UserDir::Cache, "poshuku/offlinewebappcache").absolutePath ();
			QWebSettings::setOfflineWebApplicationCachePath (path);
		}
		catch (const std::runtime_error& e)
		{
			qWarning () << Q_FUNC_INFO
					<< e.what ();
		}

		Interceptor_ = std::make_shared<InterceptAdaptor> ();
	}

	void Plugin::SecondInit ()
	{
		WebPluginFactory_ = new WebPluginFactory { Proxy_->GetPluginsManager () };
	}

	void Plugin::Release ()
	{
		Interceptor_.reset ();
	}

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.Poshuku.WebKitView";
	}

	QString Plugin::GetName () const
	{
		return "Poshuku WebKitView";
	}

	QString Plugin::GetInfo () const
	{
		return tr ("Provides QtWebKit-based backend for Poshuku.");
	}

	QIcon Plugin::GetIcon () const
	{
		return {};
	}

	QSet<QByteArray> Plugin::GetPluginClasses () const
	{
		QSet<QByteArray> result;
		result << "org.LeechCraft.Poshuku.Plugins/1.0";
		result << "org.LeechCraft.Core.Plugins/1.0";
		return result;
	}

	Util::XmlSettingsDialog_ptr Plugin::GetSettingsDialog () const
	{
		return XSD_;
	}

	QString Plugin::GetDiagInfoString () const
	{
		return QString ("Built with QtWebKit %1, running with QtWebKit %2")
#ifdef QTWEBKIT_VERSION_STR
				.arg (QTWEBKIT_VERSION_STR)
#else
				.arg ("unknown (QTWEBKIT_VERSION_STR is not defined)")
#endif
				.arg (qWebKitVersion ());
	}

	IWebView_ptr Plugin::CreateWebView ()
	{
		const auto view = std::make_shared<CustomWebView> (Proxy_, PoshukuProxy_);

		HandleView (view.get ());

		return view;
	}

	QIcon Plugin::GetIconForUrl (const QUrl& url) const
	{
		const auto& specific = QWebSettings::iconForUrl (url);
		if (!specific.isNull ())
			return specific;

		QUrl test;
		test.setScheme (url.scheme ());
		test.setHost (url.host ());
		return QWebSettings::iconForUrl (test);
	}

	QIcon Plugin::GetDefaultUrlIcon () const
	{
		return QWebSettings::webGraphic (QWebSettings::DefaultFrameIconGraphic);
	}

	void Plugin::AddInterceptor (const Interceptor_t& interceptor)
	{
		Interceptor_->AddInterceptor (interceptor);
	}

	void Plugin::HandleView (CustomWebView *view)
	{
		connect (view,
				&CustomWebView::webViewCreated,
				this,
				&Plugin::handleWebViewCreated);

		if (WebPluginFactory_)
			view->page ()->setPluginFactory (WebPluginFactory_);
		else
			qWarning () << Q_FUNC_INFO
					<< "web plugin factory isn't initialized yet";
	}

	void Plugin::handleWebViewCreated (const std::shared_ptr<CustomWebView>& view, bool invert)
	{
		HandleView (view.get ());
		emit webViewCreated (view, invert);
	}

	void Plugin::hookNAMCreateRequest (IHookProxy_ptr proxy,
			QNetworkAccessManager *manager,
			QNetworkAccessManager::Operation *op,
			QIODevice **dev)
	{
		Interceptor_->HandleNAM (proxy, manager, op, dev);
	}

	void Plugin::initPlugin (QObject *proxyObj)
	{
		PoshukuProxy_ = qobject_cast<IProxyObject*> (proxyObj);
	}
}
}
}

LC_EXPORT_PLUGIN (leechcraft_poshuku_webkitview, LC::Poshuku::WebKitView::Plugin);
