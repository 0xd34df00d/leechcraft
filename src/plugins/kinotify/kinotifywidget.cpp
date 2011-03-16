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
#include <QWebElement>
#include <QFrame>
#include <QBuffer>
#include <QFile>
#include <QMouseEvent>
#include <QTimer>
#include <QState>
#include <QPropertyAnimation>
#include <QFinalState>
#include <QWebHitTestResult>
#include <plugininterface/resourceloader.h>
#include "notificationaction.h"
#include "xmlsettingsmanager.h"

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
			, Action_ (new NotificationAction (this))
			{
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

				initJavaScript ();
				connect (page ()->mainFrame (),
						SIGNAL (javaScriptWindowObjectCleared ()),
						this,
						SLOT (initJavaScript ()));

				connect (Action_,
						SIGNAL (actionPressed ()),
						this,
						SLOT (closeNotificationWidget ()));
			}

			void KinotifyWidget::SetThemeLoader (boost::shared_ptr<Util::ResourceLoader> loader)
			{
				ThemeLoader_ = loader;
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

			void KinotifyWidget::mousePressEvent (QMouseEvent *event)
			{
				QWebElement elem = page ()->mainFrame ()->hitTestContent (event->pos ()).element ();
				if (elem.isNull () || elem.attribute ("type") != "button")
				{
					disconnect (CheckTimer_,
							SIGNAL (timeout ()),
							this,
							SIGNAL (checkNotificationQueue ()));

					disconnect (&Machine_,
							SIGNAL (finished ()),
							this,
							SLOT (closeNotification ()));

					emit checkNotificationQueue ();
					closeNotification ();
				}
				else
					mouseReleaseEvent (event);
			}

			void KinotifyWidget::showEvent (QShowEvent *event)
			{
				DefaultSize_ = page ()->mainFrame ()->contentsSize ();
				resize (DefaultSize_);
				SetWidgetPlace ();
			}

			void KinotifyWidget::PrepareNotification ()
			{
				CreateWidget ();
				SetData ();
				ShowNotification ();
			}

			void KinotifyWidget::SetActions (const QStringList& actions, QObject *object)
			{
				ActionsNames_ = actions;
				Action_->SetActionObject (object);
			}

			void KinotifyWidget::stateMachinePause ()
			{
				CloseTimer_->start (Timeout_);
				CheckTimer_->start (Timeout_);
			}

			void KinotifyWidget::closeNotificationWidget()
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

			void KinotifyWidget::closeNotification ()
			{
				close ();
			}

			void KinotifyWidget::initJavaScript ()
			{
				page ()->mainFrame ()->addToJavaScriptWindowObject ("Action", Action_);
			}

			void KinotifyWidget::CreateWidget ()
			{
				QStringList variants;
				variants << XmlSettingsManager::Instance ()->
						property ("NotificatorStyle").toString ();
				LoadTheme (ThemeLoader_->GetPath (variants));

				setStyleSheet ("background: transparent");
				page ()->mainFrame ()->setScrollBarPolicy (Qt::Horizontal, Qt::ScrollBarAlwaysOff);
				page ()->mainFrame ()->setScrollBarPolicy (Qt::Vertical, Qt::ScrollBarAlwaysOff);

				setWindowFlags (Qt::ToolTip | Qt::FramelessWindowHint);

				QPalette pal = palette ();
				pal.setBrush (QPalette::Base, Qt::transparent);
				page ()->setPalette (pal);
				setAttribute (Qt::WA_OpaquePaintEvent, false);
				setAttribute (Qt::WA_DeleteOnClose);
				settings ()->setAttribute (QWebSettings::AutoLoadImages, true);
				setAttribute (Qt::WA_TranslucentBackground);

				resize (DefaultSize_);
				setSizePolicy (QSizePolicy::Fixed, QSizePolicy::Preferred);
				page ()->setPreferredContentsSize (size ());
			}

			void KinotifyWidget::LoadTheme (const QString& themePath)
			{
				Theme_.clear ();
				QFile content (themePath + "/tmp.html");
				if (!content.open (QIODevice::ReadOnly))
				{
					qWarning () << Q_FUNC_INFO
							<< "could not open theme file at"
							<< content.fileName ()
							<< content.errorString ();
					return;
				}

				Theme_ = content.readAll ();

				const QPalette& palette = QApplication::palette ();

#define REPLACE1(a) { const QColor& c = palette.color (QPalette::a); \
					  Theme_.replace ("{Color"#a "}", QString ("%1, %2, %3").arg (c.red ()).arg (c.green ()).arg (c.blue ())); }
				REPLACE1 (Window);
				REPLACE1 (WindowText);
				REPLACE1 (Base);
				REPLACE1 (AlternateBase);
				REPLACE1 (ToolTipBase);
				REPLACE1 (ToolTipText);
				REPLACE1 (Text);
				REPLACE1 (Button);
				REPLACE1 (ButtonText);
				REPLACE1 (BrightText);
#undef REPLACE1

				QDir imgDir (themePath + "/img");
				Q_FOREACH (QString elem, imgDir.entryList (QStringList ("*.png")))
					Theme_.replace (QString ("{%1}").arg (elem.left (elem.size () - 4)),
							MakeImage (themePath + "/img/" + elem));
			}

			void KinotifyWidget::SetData ()
			{
				QString data = Theme_;
				data.replace ("{title}", Title_);
				data.replace ("{body}", Body_);
				data.replace ("{imagepath}", MakeImage (ImagePath_));
				setHtml (data);

				if (!ActionsNames_.isEmpty ())
				{
					QWebElement button = page ()->mainFrame ()->documentElement ().findFirst ("form");
					if (!button.isNull ())
					{
						QStringList reversed = ActionsNames_;
						std::reverse (reversed.begin (), reversed.end ());
						Q_FOREACH (const QString& name, reversed)
							button.appendInside (QString ("<input type=\"button\" id=\"%1\" value=\"%2\""
									" onclick=\"Action.sendActionOnClick(id)\" />")
											.arg (ActionsNames_.indexOf (name))
											.arg (name));
					}
				}
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

			void KinotifyWidget::ShowNotification ()
			{
				show ();
				setWindowOpacity (0.0);
				Machine_.start ();
			}
		};
	};
};
