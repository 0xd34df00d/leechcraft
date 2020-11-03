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

namespace LC
{
namespace AdvancedNotifications
{
	VisualNotificationsView::VisualNotificationsView (const ICoreProxy_ptr& proxy)
	{
		setWindowFlags (Qt::WindowStaysOnTopHint | Qt::ToolTip);
		Util::EnableTransparency (this);

		Util::WatchQmlErrors (this);

		const auto& fileLocation = Util::GetSysPath (Util::SysPath::QML, "advancednotifications", "visualnotificationsview.qml");

		if (fileLocation.isEmpty ())
		{
			qWarning () << Q_FUNC_INFO
					<< "visualnotificationsview.qml isn't found";
			return;
		}

		qDebug () << Q_FUNC_INFO << "created";

		Location_ = QUrl::fromLocalFile (fileLocation);

		rootContext ()->setContextProperty ("colorProxy",
				new Util::ColorThemeProxy (proxy->GetColorThemeManager (), this));
		engine ()->addImageProvider ("ThemeIcons", new Util::ThemeImageProvider (proxy));

		for (const auto& cand : Util::GetPathCandidates (Util::SysPath::QML, ""))
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
					SIGNAL (actionTriggered (const QString&, int)),
					this,
					SIGNAL (actionTriggered (const QString&, int)));
			connect (obj,
					SIGNAL (dismissEventRequested (const QString&)),
					this,
					SIGNAL (dismissEvent (const QString&)));
			LastEvents_ << obj;
		}

		rootContext ()->setContextProperty ("eventsModel",
				QVariant::fromValue<QList<QObject*>> (LastEvents_));

		setSource (Location_);

		qDeleteAll (oldEvents);
	}
}
}
