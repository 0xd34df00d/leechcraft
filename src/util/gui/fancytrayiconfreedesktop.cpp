/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "fancytrayiconfreedesktop.h"
#include <QCoreApplication>
#include <QDBusArgument>
#include <QDBusInterface>
#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QMenu>
#include <QtDebug>
#include <util/sll/qtutil.h>

namespace LC::Util
{
	FancyTrayIconFreedesktop::FancyTrayIconFreedesktop (FancyTrayIcon& icon, const FancyTrayIcon::IconInfo& info)
	: FancyTrayIconImpl { &icon }
	, Info_ { info }
	, FTI_ { icon }
	, Adaptor_ { *this }
	{
		auto sb = QDBusConnection::sessionBus ();

		const auto& watchers = sb.interface ()->registeredServiceNames ().value ().filter ("StatusNotifierWatcher");
		if (watchers.isEmpty ())
			throw std::runtime_error { "no SNI watchers available" };
		const auto& watcherService = watchers.value (0);
		QDBusInterface watcher { watcherService, "/StatusNotifierWatcher", {}, sb };
		if (!watcher.isValid ())
			throw std::runtime_error { "interface to the SNI watcher " + watcherService.toStdString () + "is invalid" };

		static int uniqueId = 0;
		const auto& serviceName = u"org.freedesktop.StatusNotifierItem-%1-%2"_qsv
				.arg (QByteArray::number (QCoreApplication::applicationPid ()), QByteArray::number (++uniqueId));
		if (!sb.registerService (serviceName))
			throw std::runtime_error { "unable to register SNI service" };
		if (!sb.registerObject ("/StatusNotifierItem", this))
			throw std::runtime_error { "unable to register SNI object" };

		qDebug () << watcher.call ("RegisterStatusNotifierItem", serviceName);
	}

	void FancyTrayIconFreedesktop::UpdateIcon ()
	{
	}

	void FancyTrayIconFreedesktop::UpdateTooltip ()
	{
		emit Adaptor_.NewTooltip ();
	}

	void FancyTrayIconFreedesktop::UpdateMenu ()
	{
	}
}

namespace LC::Util::detail
{
	SNIAdaptor::SNIAdaptor (FancyTrayIconFreedesktop& impl)
	: QDBusAbstractAdaptor { &impl }
	, Impl_ { impl }
	{
	}

	void SNIAdaptor::ContextMenu (int x, int y)
	{
		if (const auto menu = Impl_.FTI_.GetContextMenu ())
			menu->popup ({ x, y });
	}

	void SNIAdaptor::Activate (int, int)
	{
		emit Impl_.FTI_.activated ();
	}

	void SNIAdaptor::SecondaryActivate (int, int)
	{
		emit Impl_.FTI_.secondaryActivated ();
	}

	QString SNIAdaptor::GetId () const
	{
		return Impl_.Info_.Id_;
	}

	QString SNIAdaptor::GetTitle () const
	{
		return Impl_.Info_.Title_;
	}
}

