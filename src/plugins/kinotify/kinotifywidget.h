/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010  Oleg Linkin
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


#ifndef PLUGINS_KINOTIFY_KINOTIFYWIDGET_H
#define PLUGINS_KINOTIFY_KINOTIFYWIDGET_H

#include "kinotify.h"
#include <QWebView>
#include <QStateMachine>

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Kinotify
		{
			class NotificationAction;

			class KinotifyWidget : public QWebView
			{
				Q_OBJECT
				Q_PROPERTY (qreal opacity READ windowOpacity WRITE setWindowOpacity);

				QString Title_;
				QString Body_;
				QString ImagePath_;
				QString Theme_;
				QSize DefaultSize_;
				int Timeout_;
				int AnimationTime_;
				QTimer *CloseTimer_;
				QTimer *CheckTimer_;
				QStateMachine Machine_;
				const QByteArray MakeImage (const QString& imgPath = QString ());
				QStringList ActionsNames_;
				NotificationAction *Action_;
			public:
				KinotifyWidget (int timeout = 0, QWidget *widget = 0, int animationTimeout = 300);
				void SetContent (const QString&, const QString&, const QString&, const QSize& size = QSize (350, 70));
				void PrepareNotification ();
				void SetActions (const QStringList&, QObject*);
			protected:
				virtual void mousePressEvent (QMouseEvent*);
			public slots:
				void stateMachinePause ();
				void closeNotification ();
				void closeNotificationWidget ();
				void initJavaScript ();
			private:
				void CreateWidget ();
				void LoadTheme (const QString&);
				QSize SetData ();
				void SetWidgetPlace ();
				void ShowNotification ();
			signals:
				void initiateCloseNotification ();
				void checkNotificationQueue ();
			};
		};
	};
};

#endif // PLUGINS_KINOTIFY_KINOTIFYWIDGET_H
