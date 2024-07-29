/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <interfaces/iinfo.h>
#include <interfaces/iplugin2.h>
#include <interfaces/monocle/iknowfileextensions.h>
#include <interfaces/monocle/iredirectorplugin.h>

namespace LC::Monocle::Postrus
{
	class Plugin : public QObject
				 , public IInfo
				 , public IPlugin2
				 , public IRedirectorPlugin
				 , public IKnowFileExtensions
	{
		Q_OBJECT
		Q_INTERFACES (IInfo
				IPlugin2
				LC::Monocle::IRedirectorPlugin
				LC::Monocle::IKnowFileExtensions)

		LC_PLUGIN_METADATA ("org.LeechCraft.Monocle.Postrus")
	public:
		void Init (ICoreProxy_ptr) override;
		void SecondInit () override;
		QByteArray GetUniqueID () const override;
		void Release () override;
		QString GetName () const override;
		QString GetInfo () const override;
		QIcon GetIcon () const override;

		QSet<QByteArray> GetPluginClasses () const override;

		bool CanRedirectDocument (const QString&) const override;
		QString GetRedirectionMime (const QString&) const override;
		Util::Task<std::optional<RedirectionResult>> GetRedirection (const QString&) override;

		QList<ExtInfo> GetKnownFileExtensions () const override;
	};
}
