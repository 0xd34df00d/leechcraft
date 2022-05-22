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
#include <QDBusMetaType>
#include <QMenu>
#include <QtDebug>
#include <QtEndian>
#include <util/sll/prelude.h>
#include <util/sll/qtutil.h>
#include <util/sll/visitor.h>

namespace LC::Util::detail
{
	IconFrame IconFrame::FromPixmap (const QPixmap& px)
	{
		const auto& img = px.toImage ().convertToFormat (QImage::Format_ARGB32);

		QByteArray data;
		const auto pixelsCnt = img.width () * img.height ();
		data.resize (pixelsCnt * sizeof (quint32));
		qToBigEndian<quint32> (img.bits (), pixelsCnt, data.data ());

		return { .Width_ = img.width (), .Height_ = img.height (), .Data_ = data };
	}

	QList<IconFrame> IconToFrames (const QIcon& icon)
	{
		if (icon.isNull ())
			return {};

		auto sizes = icon.availableSizes ();
		constexpr auto fallbackSize = 128;
		if (sizes.isEmpty ())
			sizes << QSize { fallbackSize, fallbackSize };

		return Util::Map (sizes, [&] (QSize size) { return IconFrame::FromPixmap (icon.pixmap (size)); });
	}

	QDBusArgument& operator<< (QDBusArgument& out, const IconFrame& frame)
	{
		out.beginStructure ();
		out << frame.Width_ << frame.Height_ << frame.Data_;
		out.endStructure ();
		return out;
	}

	const QDBusArgument& operator>> (const QDBusArgument& in, IconFrame& frame)
	{
		in.beginStructure ();
		in >> frame.Width_ >> frame.Height_ >> frame.Data_;
		in.endStructure ();
		return in;
	}

	QDBusArgument& operator<< (QDBusArgument& out, const DBusTooltip& tooltip)
	{
		out.beginStructure ();
		out << QString {};
		out << IconToFrames (QIcon {});
		out << tooltip.Title_;
		out << tooltip.Subtitle_;
		out.endStructure ();
		return out;
	}

	const QDBusArgument& operator>> (const QDBusArgument& in, DBusTooltip& tooltip)
	{
		QString iconName;
		QList<IconFrame> frames;

		in.beginStructure ();
		in >> iconName;
		in >> frames;
		in >> tooltip.Title_;
		in >> tooltip.Subtitle_;
		in.endStructure ();

		return in;
	}
}

namespace LC::Util
{
	FancyTrayIconFreedesktop::FancyTrayIconFreedesktop (FancyTrayIcon& icon)
	: FancyTrayIconImpl { &icon }
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

		if (const auto reply = watcher.call ("RegisterStatusNotifierItem", serviceName);
			reply.type () == QDBusMessage::ErrorMessage)
			throw std::runtime_error { "unable to register the SNI with the watcher: " + reply.errorMessage ().toStdString () };
	}

	void FancyTrayIconFreedesktop::UpdateIcon ()
	{
		emit Adaptor_.NewIcon ();
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
		qDBusRegisterMetaType<IconFrame> ();
		qDBusRegisterMetaType<QList<IconFrame>> ();
		qDBusRegisterMetaType<DBusTooltip> ();
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
		return Impl_.FTI_.GetInfo ().Id_;
	}

	QString SNIAdaptor::GetTitle () const
	{
		return Impl_.FTI_.GetInfo ().Title_;
	}

	QString SNIAdaptor::GetIconName () const
	{
		return Util::Visit (Impl_.FTI_.GetIcon (),
				[] (const QString& path) { return path; },
				[] (const QIcon&) { return QString {}; });
	}

	QList<IconFrame> SNIAdaptor::GetIconPixmap () const
	{
		return Util::Visit (Impl_.FTI_.GetIcon (),
				[] (const QString&) { return QList<IconFrame> {}; },
				[] (const QIcon& icon) { return IconToFrames (icon); });
	}

	DBusTooltip SNIAdaptor::GetTooltip () const
	{
		return
		{
			.Title_ = Impl_.FTI_.GetInfo ().Title_,
			.Subtitle_ = Impl_.FTI_.GetTooltip ().HTML_
		};
	}
}

