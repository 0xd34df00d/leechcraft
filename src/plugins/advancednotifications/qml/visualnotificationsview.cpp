/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2012  Georg Rudoy
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

#include "visualnotificationsview.h"
#include <QFile>
#include <QDeclarativeContext>
#include <QDeclarativeError>
#include <QtDebug>
#include "eventproxyobject.h"
#include <QApplication>

namespace LeechCraft
{
namespace AdvancedNotifications
{
	VisualNotificationsView::VisualNotificationsView (QWidget *parent)
	: QDeclarativeView (parent)
	{
		setWindowFlags (Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint | Qt::ToolTip);
		setAttribute (Qt::WA_TranslucentBackground);
		setStyleSheet ("background: transparent");

		connect (this,
				SIGNAL (statusChanged (QDeclarativeView::Status)),
				this,
				SLOT (handleStatusChanged (QDeclarativeView::Status)));

		QStringList candidates;
#ifdef Q_OS_WIN32
		candidates << QApplication::applicationDirPath () + "/share/qml/advancednotifications/";
#else
		candidates << "/usr/local/share/leechcraft/qml/advancednotifications/"
				<< "/usr/share/leechcraft/qml/advancednotifications/";
#endif

		QString fileLocation;
		Q_FOREACH (const QString& cand, candidates)
			if (QFile::exists (cand + "visualnotificationsview.qml"))
			{
				fileLocation = cand + "visualnotificationsview.qml";
				break;
			}

		if (fileLocation.isEmpty ())
		{
			qWarning () << Q_FUNC_INFO
					<< "visualnotificationsview.qml isn't found";
			return;
		}

		qDebug () << Q_FUNC_INFO << "created";

		Location_ = QUrl::fromLocalFile (fileLocation);
	}

	void VisualNotificationsView::SetEvents (const QList<EventData>& events)
	{
		QObjectList oldEvents = LastEvents_;

		LastEvents_.clear ();
		Q_FOREACH (const EventData& ed, events)
		{
			EventProxyObject *obj = new EventProxyObject (ed, this);
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

	void VisualNotificationsView::handleStatusChanged (QDeclarativeView::Status status)
	{
		qDebug () << Q_FUNC_INFO
				<< status;

		if (status == Error)
		{
			qWarning () << Q_FUNC_INFO
					<< "got errors:"
					<< errors ().size ();
			Q_FOREACH (const QDeclarativeError& error, errors ())
				qWarning () << error.toString ()
						<< "["
						<< error.description ()
						<< "]";
		}
	}
}
}
