/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2011  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/


#pragma once

#include <variant>
#include <QWebView>
#include <QStateMachine>
#include <util/sll/void.h>
#include "interfaces/structures.h"
#include "kinotify.h"

namespace LC
{
namespace Util
{
	class ResourceLoader;
}

namespace Kinotify
{
	class NotificationAction;

	using ImageVar_t = std::variant<Util::Void, QPixmap, QImage>;

	class KinotifyWidget : public QWebView
	{
		Q_OBJECT
		Q_PROPERTY (qreal opacity READ windowOpacity WRITE setWindowOpacity)

		ICoreProxy_ptr Proxy_;

		QString ID_;

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
		ImageVar_t OverridePixmap_;
		QObject_ptr HandlerGuard_;

		static QMap<QString, QString> ThemeCache_;

		Entity E_;
	public:
		KinotifyWidget (ICoreProxy_ptr, int timeout = 0, QWidget *widget = 0, int animationTimeout = 300);
		void SetThemeLoader (std::shared_ptr<Util::ResourceLoader>);

		static void ClearThemeCache ();

		void SetEntity (const Entity&);

		QString GetTitle () const;
		QString GetBody () const;

		QString GetID () const;
		void SetID (const QString&);

		void SetContent (const QString&, const QString&,
				const QString&, const QSize& size = QSize (350, 70));
		void OverrideImage (const ImageVar_t&);
		void PrepareNotification ();
		void SetActions (const QStringList&, QObject_ptr);
	protected:
		virtual void mousePressEvent (QMouseEvent*);
		virtual void showEvent (QShowEvent*);
	private:
		const QByteArray MakeImage (const QString& imgPath = QString ());
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
	};
}
}
