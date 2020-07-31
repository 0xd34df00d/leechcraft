/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/poshuku/iwebplugin.h>

namespace LC
{
namespace Poshuku
{
namespace FOC
{
	class FlashOnClickWhitelist;

	class FlashOnClickPlugin : public QObject
							 , public IWebPlugin
	{
		Q_OBJECT
		Q_INTERFACES (LC::Poshuku::IWebPlugin)

		const ICoreProxy_ptr Proxy_;
		FlashOnClickWhitelist * const WL_;
	public:
		FlashOnClickPlugin (const ICoreProxy_ptr&,
				FlashOnClickWhitelist*, QObject* = nullptr);

		boost::optional<QWebPluginFactory::Plugin> Plugin (bool) const override;
		QWidget* Create (const QString&,
				const QUrl&,
				const QStringList&,
				const QStringList&) override;
	};
}
}
}
