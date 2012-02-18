/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2011  Oleg Linkin
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
#include "interfaces/structures.h"

namespace LeechCraft
{
	namespace Util
	{
		class ResourceLoader;
	}

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
				QStringList ActionsNames_;
				NotificationAction *Action_;
				std::shared_ptr<Util::ResourceLoader> ThemeLoader_;
				QPixmap OverridePixmap_;
				QObject_ptr HandlerGuard_;

				static QMap<QString, QString> ThemeCache_;

				Entity E_;
			public:
				KinotifyWidget (int timeout = 0, QWidget *widget = 0, int animationTimeout = 300);
				void SetThemeLoader (std::shared_ptr<Util::ResourceLoader>);

				static void ClearThemeCache ();

				void SetEntity (const Entity&);

				void SetContent (const QString&, const QString&,
						const QString&, const QSize& size = QSize (350, 70));
				void OverrideImage (const QPixmap&);
				void PrepareNotification ();
				void SetActions (const QStringList&, QObject_ptr);
			protected:
				virtual void mousePressEvent (QMouseEvent*);
				virtual void showEvent (QShowEvent*);
			private:
				const QByteArray MakeImage (const QString& imgPath = QString ());
				const QByteArray MakeImage (const QPixmap&);
				void CreateWidget ();
				void LoadTheme (const QString&);
				void SetData ();
				void SetWidgetPlace ();
				void ShowNotification ();
			public slots:
				void stateMachinePause ();
				void closeNotification ();
				void closeNotificationWidget ();
				void initJavaScript ();
				void handleLinkClicked (const QUrl&);
			signals:
				void initiateCloseNotification ();
				void checkNotificationQueue ();
				void gotEntity (const LeechCraft::Entity&);
			};
		}
	}
}

#endif // PLUGINS_KINOTIFY_KINOTIFYWIDGET_H
