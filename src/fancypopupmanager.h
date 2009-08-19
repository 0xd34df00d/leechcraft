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
#include <deque>
#include <map>
#include <QObject>
#include <QDateTime>
#include <QPoint>

class QSystemTrayIcon;

namespace LeechCraft
{
	class FancyPopupManager : public QObject
	{
		Q_OBJECT

		typedef std::deque<QString> popups_t;
		popups_t Popups_;

		typedef std::map<QDateTime, QString> dates_t;
		dates_t Dates_;

		QSystemTrayIcon *TrayIcon_;
	public:
		FancyPopupManager (QSystemTrayIcon*,QObject* = 0);
		~FancyPopupManager ();

		void ShowMessage (const QString&);
	private slots:
		void timerTimeout ();
		void handleMessageClicked ();
	private:
		void UpdateMessage ();
	};
};

#endif

