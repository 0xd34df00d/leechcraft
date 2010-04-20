/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2009  Georg Rudoy
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

#ifndef FANCYPOPUPMANAGER_H
#define FANCYPOPUPMANAGER_H
#include <QObject>
#include <QDateTime>
#include <QPoint>
#include <QList>
#include <QMap>
#include "plugininterface/structuresops.h"

class QSystemTrayIcon;

namespace LeechCraft
{
	/** Manages notifications from IInfo::downloadFinished(). Shows nice
	 * balloon tip with all the notifications collected and removes
	 * older ones based on timer.
	 */
	class FancyPopupManager : public QObject
	{
		Q_OBJECT

		typedef QList<DownloadEntity> popups_t;
		popups_t Popups_;

		typedef QMap<DownloadEntity, QDateTime> dates_t;
		dates_t Dates_;

		QSystemTrayIcon *TrayIcon_;
	public:
		/** Constructs the manager.
		 */
		FancyPopupManager (QSystemTrayIcon*,QObject* = 0);
		~FancyPopupManager ();

		/** Pushes the message to the message array. Shows the popup if
		 * it was hidden, appends the message otherwise.
		 * 
		 * @param[in] n The notification.
		 */
		void ShowMessage (const DownloadEntity&);
	private slots:
		/** Checks the messages and deletes too old ones.
		 */
		void timerTimeout ();

		/** Clears the message queue.
		 */
		void handleMessageClicked ();
	private:
		/** The main function to show the message and update the
		 * corresponding string.
		 */
		void UpdateMessage ();
	};
};

#endif

