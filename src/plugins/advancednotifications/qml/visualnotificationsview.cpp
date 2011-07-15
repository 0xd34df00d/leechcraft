/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2011  Georg Rudoy
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
#include <QDeclarativeError>
#include <QtDebug>

namespace LeechCraft
{
namespace AdvancedNotifications
{
	VisualNotificationsView::VisualNotificationsView (QWidget *parent)
	: QDeclarativeView (parent)
	{
		connect (this,
				SIGNAL (statusChanged (QDeclarativeView::Status)),
				this,
				SLOT (handleStatusChanged (QDeclarativeView::Status)));
		
		QStringList candidates;
#ifdef Q_WS_X11
		candidates << "/usr/local/share/leechcraft/qml/"
				<< "/usr/share/leechcraft/qml/";
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

		setSource (QUrl::fromLocalFile (fileLocation));
	}
	
	void VisualNotificationsView::SetEvents (const QList<EventData>&)
	{
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
