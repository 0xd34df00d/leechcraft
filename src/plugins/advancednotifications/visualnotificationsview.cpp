/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "visualnotificationsview.h"
#include <QQmlContext>
#include <QQmlError>
#include <QtDebug>
#include <util/sys/paths.h>
#include <util/qml/colorthemeproxy.h>
#include <util/qml/themeimageprovider.h>
#include <util/qml/qmlerrorwatcher.h>
#include <util/qml/util.h>
#include <interfaces/core/icoreproxy.h>
#include "eventproxyobject.h"
#include "generalhandler.h"

namespace LC::AdvancedNotifications
{
	VisualNotificationsView::VisualNotificationsView (const ICoreProxy_ptr& proxy)
	{
		setWindowFlags (Qt::WindowStaysOnTopHint | Qt::ToolTip);
		Util::EnableTransparency (this);

		Util::WatchQmlErrors (this);

		const auto& fileLocation = Util::GetSysPath (Util::SysPath::QML,
				QStringLiteral ("advancednotifications"),
				QStringLiteral ("visualnotificationsview.qml"));

		if (fileLocation.isEmpty ())
		{
			qWarning () << Q_FUNC_INFO
					<< "visualnotificationsview.qml isn't found";
			return;
		}

		qDebug () << Q_FUNC_INFO << "created";

		Location_ = QUrl::fromLocalFile (fileLocation);

		rootContext ()->setContextProperty (QStringLiteral ("colorProxy"),
				new Util::ColorThemeProxy (proxy->GetColorThemeManager (), this));
		engine ()->addImageProvider (QStringLiteral ("ThemeIcons"), new Util::ThemeImageProvider (proxy));

		for (const auto& cand : Util::GetPathCandidates (Util::SysPath::QML, {}))
			engine ()->addImportPath (cand);
	}

	void VisualNotificationsView::SetEvents (const QList<EventData>& events)
	{
		auto oldEvents { std::move (LastEvents_) };

		LastEvents_.clear ();
		for (const auto& ed : events)
		{
			const auto obj = new EventProxyObject (ed, this);
			connect (obj,
					&EventProxyObject::actionTriggered,
					this,
					&VisualNotificationsView::actionTriggered);
			connect (obj,
					&EventProxyObject::dismissEventRequested,
					this,
					&VisualNotificationsView::dismissEvent);
			LastEvents_ << obj;
		}

		rootContext ()->setContextProperty (QStringLiteral ("eventsModel"),
				QVariant::fromValue<QList<QObject*>> (LastEvents_));

		setSource (Location_);

		qDeleteAll (oldEvents);
	}
}
