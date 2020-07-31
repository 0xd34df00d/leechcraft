/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2011  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#ifndef PLUGINS_POSHUKU_PLUGINS_ONLINEBOOKMARKS_PLUGINS_READITLATER_READITLATER_H
#define PLUGINS_POSHUKU_PLUGINS_ONLINEBOOKMARKS_PLUGINS_READITLATER_READITLATER_H

#include <QObject>
#include <interfaces/iinfo.h>
#include <interfaces/iplugin2.h>
#include <interfaces/iserviceplugin.h>

namespace LC
{
namespace Poshuku
{
namespace OnlineBookmarks
{
namespace ReadItLater
{

	class ReadItLaterService;

	class Plugin : public QObject
				, public IPlugin2
				, public IInfo
				, public IServicePlugin
	{
		Q_OBJECT
		Q_INTERFACES (IInfo IPlugin2
				LC::Poshuku::OnlineBookmarks::IServicePlugin)

		LC_PLUGIN_METADATA ("org.LeechCraft.Poshuku.OnlineBookmarks.ReadItLater")

		std::shared_ptr<ReadItLaterService> ReadItLaterService_;
	public:
		void Init (ICoreProxy_ptr proxy) override;
		void SecondInit () override;
		void Release () override;
		QByteArray GetUniqueID () const override;
		QString GetName () const override;
		QString GetInfo () const override;
		QIcon GetIcon () const override;

		QSet<QByteArray> GetPluginClasses () const override;

		QObject* GetQObject () override;
		QObject* GetBookmarksService () const override;
	};
}
}
}
}

#endif // PLUGINS_POSHUKU_PLUGINS_ONLINEBOOKMARKS_PLUGINS_READITLATER_READITLATER_H
