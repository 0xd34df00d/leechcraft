/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2011  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "delicious.h"
#include <QIcon>
#include <util/util.h>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/iiconthememanager.h>
#include "deliciousauthwidget.h"
#include "deliciousservice.h"

namespace LC
{
namespace Poshuku
{
namespace OnlineBookmarks
{
namespace Delicious
{
	void Plugin::Init (ICoreProxy_ptr proxy)
	{
		Util::InstallTranslator ("poshuku_onlinebookmarks_delicious");

		DeliciousService_ = std::make_shared<DeliciousService> (proxy);
	}

	void Plugin::SecondInit ()
	{
		DeliciousService_->Prepare ();
	}

	void Plugin::Release ()
	{
	}

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.Poshuku.OnlineBookmarks.Delicious";
	}

	QString Plugin::GetName () const
	{
		return "Poshuku OB: Delicious";
	}

	QString Plugin::GetInfo () const
	{
		return tr ("Sync local bookmarks with your Del.icio.us account.");
	}

	QIcon Plugin::GetIcon () const
	{
		return GetProxyHolder ()->GetIconThemeManager ()->GetPluginIcon ();
	}

	QSet<QByteArray> Plugin::GetPluginClasses () const
	{
		return { "org.LeechCraft.Plugins.Poshuku.Plugins.OnlineBookmarks.IServicePlugin" };
	}

	QObject* Plugin::GetQObject ()
	{
		return this;
	}

	QObject* Plugin::GetBookmarksService () const
	{
		return DeliciousService_.get ();
	}
}
}
}
}

LC_EXPORT_PLUGIN (leechcraft_poshuku_onlinebookmarks_delicious,
		LC::Poshuku::OnlineBookmarks::Delicious::Plugin);
