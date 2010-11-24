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


#include "kinotifywidget.h"

#include <QApplication>
#include <QDesktopWidget>
#include <QWebFrame>
#include <QFrame>
#include <QBuffer>
#include <QFile>
#include <QMouseEvent>
#include <QTimer>
#include <QState>
#include <QPropertyAnimation>
#include <QFinalState>

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Kinotify
		{
			KinotifyWidget::KinotifyWidget (int timeout, QWidget *widget, int animationTimout)
			: QWebView (widget)
			, Timeout_ (timeout)
			, AnimationTime_ (animationTimout)
			{
				setWindowOpacity (0.0);

				CloseTimer_ = new QTimer (this);
				CheckTimer_ = new QTimer (this);
				CloseTimer_->setSingleShot (true);
				CheckTimer_->setSingleShot (true);

				QState *showStartState = new QState;
				QState *showFinishState = new QState;
				QState *closeStartState = new QState;
				QState *closeFinishState = new QState;
				QFinalState *finalState = new QFinalState;

				QPropertyAnimation *opacityAmination = new QPropertyAnimation (this, "opacity");
				opacityAmination->setDuration (AnimationTime_);

				showStartState->assignProperty (this, "opacity", 0.0);
				showFinishState->assignProperty (this, "opacity", 0.8);
				closeStartState->assignProperty (this, "opacity", 0.8);
				closeFinishState->assignProperty (this, "opacity", 0.0);

				showStartState->addTransition (showFinishState);
				showFinishState->addTransition (this,
						SIGNAL (initiateCloseNotification ()), closeStartState);
				closeStartState->addTransition (closeFinishState);
				closeFinishState->addTransition (closeFinishState,
						SIGNAL (propertiesAssigned ()), finalState);

				Machine_.addState (showStartState);
				Machine_.addState (showFinishState);
				Machine_.addState (closeStartState);
				Machine_.addState (closeFinishState);
				Machine_.addState (finalState);

				Machine_.addDefaultAnimation (opacityAmination);
				Machine_.setInitialState (showStartState);

				connect (&Machine_,
						SIGNAL (finished ()),
						this,
						SLOT (closeNotification ()));

				connect (showFinishState,
						SIGNAL (entered ()),
						this,
						SLOT (stateMachinePause ()));

				connect (CloseTimer_,
						SIGNAL (timeout ()),
						this,
						SIGNAL (initiateCloseNotification ()));

				connect (CheckTimer_,
						SIGNAL (timeout ()),
						this,
						SIGNAL (checkNotificationQueue ()));
			}

			void KinotifyWidget::SetContent (const QString& title, const QString& body,
					const QString& imgPath, const QSize& size)
			{
				Title_ = title;
				Body_ = body;
				ImagePath_ = imgPath;
				DefaultSize_ = size;
			}

			const QByteArray KinotifyWidget::MakeImage (const QString& imgPath)
			{
				QBuffer iconBuffer;
				QPixmap pixmap;
				iconBuffer.open (QIODevice::ReadWrite);

				if (imgPath.isNull ())
					pixmap.load (ImagePath_);
				else
					pixmap.load (imgPath);
				pixmap.save (&iconBuffer, "PNG");

				return QByteArray ("data:image/png;base64,") +
						iconBuffer.buffer ().toBase64 ();
			}

			void KinotifyWidget::CreateWidget ()
			{
				SetTheme (":/plugins/kinotify/resources/notification/commie");
				setStyleSheet ("background: transparent");
				page ()->mainFrame ()->setScrollBarPolicy (Qt::Horizontal, Qt::ScrollBarAlwaysOff);
				page ()->mainFrame ()->setScrollBarPolicy (Qt::Vertical, Qt::ScrollBarAlwaysOff);

				setWindowFlags (Qt::ToolTip | Qt::FramelessWindowHint);

				QPalette pal = palette ();
				pal.setBrush (QPalette::Base, Qt::transparent);
				page ()->setPalette (pal);
				setAttribute (Qt::WA_OpaquePaintEvent, false);
				settings ()->setAttribute (QWebSettings::AutoLoadImages, true);
				setAttribute (Qt::WA_TranslucentBackground);

				resize (DefaultSize_);
				setSizePolicy (QSizePolicy::Fixed, QSizePolicy::Preferred);
				page ()->setPreferredContentsSize (size ());
			}

			void KinotifyWidget::SetTheme (const QString& themePath)
			{
				QFile content (themePath + "/tmp.html");
				QString output;

				if (content.open (QIODevice::ReadOnly))
				{
					output = content.readAll ();
					QStringList elements;
					elements << "back";
					Q_FOREACH (QString elem, elements)
						output.replace (QString ("{%1}").arg (elem),
								MakeImage (QString (themePath + "/img/%1.png").arg (elem)));
					content.close ();
					Theme_ = output;
				}
			}

			QSize KinotifyWidget::SetData ()
			{
				QString data = Theme_;
				data.replace ("{title}", Title_);
				data.replace ("{body}", Body_);
				data.replace ("{imagepath}", MakeImage (ImagePath_));
				setHtml (data);

				int width = size ().width ();
				int height = size ().height ();

				QSize contents = page ()->mainFrame ()->contentsSize ();
				int cheight = contents.height ();

				if (cheight > height)
					height = cheight;

				return QSize (width, height);
			}

			void KinotifyWidget::SetWidgetPlace ()
			{
				const QRect& geometry = QApplication::desktop ()->
						availableGeometry (parentWidget ());
				const QSize& desktopSize = geometry.size ();

				QPoint point (desktopSize.width () - DefaultSize_.width () - 5,
						desktopSize.height () - DefaultSize_.height () - 20);
				point += geometry.topLeft ();

				QRect place (point, DefaultSize_);
				setGeometry (place);
			}

			void KinotifyWidget::mouseReleaseEvent (QMouseEvent *event)
			{
				disconnect (CheckTimer_,
						SIGNAL (timeout ()),
						this,
						SIGNAL (checkNotificationQueue ()));

				disconnect (&Machine_,
						SIGNAL (finished ()),
						this,
						SLOT (closeNotification()));

				emit checkNotificationQueue ();
				closeNotification ();
			}

			void KinotifyWidget::PrepareNotification ()
			{
				CreateWidget ();
				DefaultSize_ = SetData ();
				SetWidgetPlace ();
				ShowNotification ();
			}

			void KinotifyWidget::stateMachinePause ()
			{
				CloseTimer_->start (Timeout_);
				CheckTimer_->start (Timeout_);
			}

			void KinotifyWidget::ShowNotification ()
			{
				show ();
				Machine_.start ();
			}

			void KinotifyWidget::closeNotification ()
			{
				close ();
			}
		};
	};
};
