/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2011  Oleg Linkin
 *
 * Boost Software License - Version 1.0 - August 17th, 2003
 *
 * Permission is hereby granted, free of charge, to any person or organization
 * obtaining a copy of the software and accompanying documentation covered by
 * this license (the "Software") to use, reproduce, display, distribute,
 * execute, and transmit the Software, and to prepare derivative works of the
 * Software, and to permit third-parties to whom the Software is furnished to
 * do so, all subject to the following:
 *
 * The copyright notices in the Software and this entire statement, including
 * the above license grant, this restriction and the following disclaimer,
 * must be included in all copies of the Software, in whole or in part, and
 * all derivative works of the Software, unless such copies or derivative
 * works are solely in the form of machine-executable object code generated by
 * a source language processor.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
 * SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
 * FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 **********************************************************************/

#include "kinotifywidget.h"
#include <algorithm>
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
#include <QMainWindow>
#include <util/gui/geometry.h>
#include <util/sll/visitor.h>
#include <util/sys/resourceloader.h>
#include <util/xpc/util.h>
#include <util/gui/geometry.h>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/irootwindowsmanager.h>
#include <interfaces/core/ientitymanager.h>
#include "notificationaction.h"
#include "xmlsettingsmanager.h"

namespace LC
{
namespace Kinotify
{
	QMap<QString, QString> KinotifyWidget::ThemeCache_;

	KinotifyWidget::KinotifyWidget (ICoreProxy_ptr proxy, int timeout, QWidget *widget, int animationTimout)
	: QWebView (widget)
	, Proxy_ (proxy)
	, Timeout_ (timeout)
	, AnimationTime_ (animationTimout)
	, Action_ (new NotificationAction (this))
	{
		page ()->setLinkDelegationPolicy (QWebPage::DelegateAllLinks);
		connect (this,
				SIGNAL (linkClicked (const QUrl&)),
				this,
				SLOT (handleLinkClicked (const QUrl&)));
		CloseTimer_ = new QTimer (this);
		CheckTimer_ = new QTimer (this);
		CloseTimer_->setSingleShot (true);
		CheckTimer_->setSingleShot (true);

		QState *showStartState = new QState;
		QState *showFinishState = new QState;
		QState *closeStartState = new QState;
		QState *closeFinishState = new QState;
		QFinalState *finalState = new QFinalState;

		QPropertyAnimation *opacityAmination = new QPropertyAnimation (this, "opacity", this);
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

	void KinotifyWidget::SetThemeLoader (std::shared_ptr<Util::ResourceLoader> loader)
	{
		ThemeLoader_ = loader;
	}

	void KinotifyWidget::ClearThemeCache ()
	{
		ThemeCache_.clear ();
	}

	void KinotifyWidget::SetEntity (const Entity& e)
	{
		E_ = e;
	}

	QString KinotifyWidget::GetTitle () const
	{
		return Title_;
	}

	QString KinotifyWidget::GetBody () const
	{
		return Body_;
	}

	QString KinotifyWidget::GetID () const
	{
		return ID_;
	}

	void KinotifyWidget::SetID (const QString& id)
	{
		ID_ = id;
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
		QFile file { imgPath };
		if (!file.open (QIODevice::ReadOnly))
			return {};

		return QByteArray ("data:image/png;base64,") + file.readAll ().toBase64 ();
	}

	void KinotifyWidget::mousePressEvent (QMouseEvent *event)
	{
		const QWebHitTestResult& r = page ()->mainFrame ()->hitTestContent (event->pos ());
		if (!r.linkUrl ().isEmpty ())
		{
			QWebView::mousePressEvent (event);
			return;
		}

		QWebElement elem = r.element ();
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
			QWebView::mousePressEvent (event);
	}

	void KinotifyWidget::showEvent (QShowEvent*)
	{
		DefaultSize_ = page ()->mainFrame ()->contentsSize ();
		resize (DefaultSize_);
		SetWidgetPlace ();
	}

	void KinotifyWidget::OverrideImage (const ImageVar_t& px)
	{
		OverridePixmap_ = px;
	}

	void KinotifyWidget::PrepareNotification ()
	{
		CreateWidget ();
		SetData ();
		ShowNotification ();
	}

	void KinotifyWidget::SetActions (const QStringList& actions, QObject_ptr object)
	{
		ActionsNames_ = actions;
		Action_->SetActionObject (object.get ());
		HandlerGuard_ = object;
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
#ifndef Q_OS_MAC
		setWindowFlags (Qt::ToolTip | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
#else
		setWindowFlags (Qt::SubWindow | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
		setAttribute (Qt::WA_ShowWithoutActivating);
#endif

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
		if (ThemeCache_.contains (themePath))
		{
			Theme_ = ThemeCache_ [themePath];
			return;
		}

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
		REPLACE1 (Light);
		REPLACE1 (Midlight);
		REPLACE1 (Dark);
		REPLACE1 (Mid);
		REPLACE1 (Shadow);
		REPLACE1 (Link);
		REPLACE1 (LinkVisited);
		REPLACE1 (Highlight);
		REPLACE1 (HighlightedText);
#undef REPLACE1

		QDir imgDir (themePath + "/img");
		for (const auto& elem : imgDir.entryList (QStringList ("*.png")))
			Theme_.replace (QString ("{%1}").arg (elem.left (elem.size () - 4)),
					MakeImage (themePath + "/img/" + elem));

		if (ThemeCache_.size () > 3)
			ThemeCache_.clear ();

		ThemeCache_ [themePath] = Theme_;
	}

	void KinotifyWidget::SetData ()
	{
		QString data = Theme_;
		data.replace ("{title}", Title_);
		data.replace ("{body}", Body_);

		const auto& overrideData = Util::Visit (OverridePixmap_,
				[] (Util::Void) { return QByteArray {}; },
				[] (const auto& pixmap)
				{
					QBuffer iconBuffer;
					iconBuffer.open (QIODevice::ReadWrite);
					pixmap.save (&iconBuffer, "PNG");
					return "data:image/png;base64," + iconBuffer.buffer ().toBase64 ();
				});

		if (overrideData.isNull ())
			data.replace ("{imagepath}", MakeImage (ImagePath_));
		else
			data.replace ("{imagepath}", overrideData);

		setHtml (data);

		if (!ActionsNames_.isEmpty ())
		{
			QWebElement button = page ()->mainFrame ()->documentElement ().findFirst ("form");
			if (!button.isNull ())
			{
				auto reversed = ActionsNames_;
				std::reverse (reversed.begin (), reversed.end ());
				for (const auto& name : reversed)
					button.appendInside (QString ("<input type=\"button\" id=\"%1\" value=\"%2\""
							" onclick=\"Action.sendActionOnClick(id)\" />")
									.arg (ActionsNames_.indexOf (name))
									.arg (name));
			}
		}
	}

	void KinotifyWidget::SetWidgetPlace ()
	{
		const bool followMouse = XmlSettingsManager::Instance ()->
				property ("FollowMouse").toBool ();

		auto desktop = QApplication::desktop ();
		auto rootWM = Proxy_->GetRootWindowsManager ();
		const auto& geometry = followMouse ?
				Util::AvailableGeometry (QCursor::pos ()) :
				desktop->availableGeometry (rootWM->GetPreferredWindow ());

		QPoint point;
		const auto& placeStr = XmlSettingsManager::Instance ()->
				property ("NotifyPosition").toString ();
		if (placeStr.startsWith ("Top"))
			point.setY (geometry.top () + 20);
		else
			point.setY (geometry.bottom () - 20);

		if (placeStr.endsWith ("Left"))
			point.setX (geometry.left () + 5);
		else
			point.setX (geometry.right () - 5);

		QRect place (Util::FitRectScreen (point, DefaultSize_, Util::FitFlag::NoOverlap), DefaultSize_);
		setGeometry (place);
	}

	void KinotifyWidget::ShowNotification ()
	{
		setWindowOpacity (0.0);
		show ();
		Machine_.start ();
	}

	void KinotifyWidget::stateMachinePause ()
	{
		CloseTimer_->start (Timeout_);
		CheckTimer_->start (Timeout_);
	}

	void KinotifyWidget::closeNotificationWidget ()
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

	void KinotifyWidget::handleLinkClicked (const QUrl& url)
	{
		if (!url.isValid ())
			return;

		const auto& e = Util::MakeEntity (url,
				{},
				FromUserInitiated | OnlyHandle);
		Proxy_->GetEntityManager ()->HandleEntity (e);
	}
}
}
