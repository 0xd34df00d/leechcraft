/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2011  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "kinotifywidget.h"
#include <algorithm>
#include <QApplication>
#include <QMouseEvent>
#include <QPainterPath>
#include <QTimer>
#include <QState>
#include <QPropertyAnimation>
#include <QFinalState>
#include <QMainWindow>
#include <QPushButton>
#include <util/gui/geometry.h>
#include <util/sll/qtutil.h>
#include <util/xpc/util.h>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/irootwindowsmanager.h>
#include <interfaces/core/ientitymanager.h>
#include "xmlsettingsmanager.h"

namespace LC::Kinotify
{
	KinotifyWidget::KinotifyWidget (int timeout, QWidget *widget)
	: QWidget (widget)
	, Timeout_ (timeout)
	{
		Ui_.setupUi (this);

#ifndef Q_OS_MAC
		setWindowFlags (Qt::ToolTip | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
#else
		setWindowFlags (Qt::SubWindow | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
		setAttribute (Qt::WA_ShowWithoutActivating);
#endif

		setAttribute (Qt::WA_DeleteOnClose);

		connect (Ui_.Body_,
				&QLabel::linkActivated,
				[] (const QUrl& url)
				{
					const auto& e = Util::MakeEntity (url,
							{},
							FromUserInitiated | OnlyHandle);
					GetProxyHolder ()->GetEntityManager ()->HandleEntity (e);
				});

		auto showStartState = new QState { &Machine_ };
		auto showFinishState = new QState { &Machine_ };
		auto closeStartState = new QState { &Machine_ };
		auto closeFinishState = new QState { &Machine_ };
		auto finalState = new QFinalState { &Machine_ };

		auto opacityAmination = new QPropertyAnimation (this, "opacity", this);
		const auto AnimationTime = 300;
		opacityAmination->setDuration (AnimationTime);

		const auto startOpacity = windowOpacity ();
		const auto endOpacity = 0.8;
		showStartState->assignProperty (this, "opacity", startOpacity);
		showFinishState->assignProperty (this, "opacity", endOpacity);
		closeStartState->assignProperty (this, "opacity", endOpacity);
		closeFinishState->assignProperty (this, "opacity", startOpacity);

		showStartState->addTransition (showFinishState);
		showFinishState->addTransition (this,
				&KinotifyWidget::initiateCloseNotification,
				closeStartState);
		closeStartState->addTransition (closeFinishState);
		closeFinishState->addTransition (closeFinishState,
				&QState::propertiesAssigned,
				finalState);

		Machine_.addDefaultAnimation (opacityAmination);
		Machine_.setInitialState (showStartState);

		connect (&Machine_,
				&QStateMachine::finished,
				this,
				&QObject::deleteLater);

		connect (showFinishState,
				&QState::entered,
				this,
				[this] { QTimer::singleShot (Timeout_, this, &KinotifyWidget::initiateCloseNotification); });
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

	void KinotifyWidget::SetContent (const QString& title, const QString& body)
	{
		Title_ = title;
		Body_ = body;
	}

	void KinotifyWidget::SetPixmap (QPixmap px)
	{
		Pixmap_ = std::move (px);
	}

	void KinotifyWidget::showEvent (QShowEvent*)
	{
		SetWidgetPlace ();
	}

	void KinotifyWidget::mouseReleaseEvent (QMouseEvent *event)
	{
		QWidget::mouseReleaseEvent (event);
		if (!event->isAccepted ())
			emit initiateCloseNotification ();
	}

	void KinotifyWidget::PrepareNotification ()
	{
		SetData ();

		show ();
		Machine_.start ();
	}

	void KinotifyWidget::SetActions (const QStringList& actions, QObject_ptr object)
	{
		ActionsNames_ = actions;
		ActionHandler_ = std::move (object);
	}

	namespace
	{
		QRect PreferredGeometry ()
		{
			const bool followMouse = XmlSettingsManager::Instance ().property ("FollowMouse").toBool ();

			auto rootWM = GetProxyHolder ()->GetRootWindowsManager ();
			return followMouse ?
					Util::AvailableGeometry (QCursor::pos ()) :
					rootWM->GetPreferredWindow ()->screen ()->availableGeometry ();
		}
	}

	void KinotifyWidget::SetData ()
	{
		Ui_.Title_->setText ("<h3>" + Title_ + "</h3>");
		Ui_.Body_->setText (Body_);
		Ui_.Image_->setPixmap (Pixmap_);

		for (int i = 0; i < ActionsNames_.size (); ++i)
		{
			auto button = new QPushButton { ActionsNames_ [i] };
			Ui_.ButtonsLayout_->addWidget (button);
			connect (button,
					&QPushButton::released,
					[this, i]
					{
						emit initiateCloseNotification ();
						QMetaObject::invokeMethod (ActionHandler_.get (),
								"notificationActionTriggered",
								Qt::QueuedConnection,
								Q_ARG (int, i));
					});
		}

		const auto maxWidth = PreferredGeometry ().width () / 3;
		auto twoLineWidth = Ui_.Body_->fontMetrics ().horizontalAdvance (Body_) / 2;

		const auto& theseMargins = layout ()->contentsMargins ();
		const auto layoutPixels = Ui_.TopDisplayLayout_->spacing () + theseMargins.left () + theseMargins.right ();
		auto targetWidth = std::max (twoLineWidth + Ui_.Image_->width () + layoutPixels, width ());
		setFixedWidth (std::min (targetWidth, maxWidth));
	}

	void KinotifyWidget::SetWidgetPlace ()
	{
		QPainterPath path;
		const auto& theseMargins = layout ()->contentsMargins ();
		path.addRoundedRect (rect (), theseMargins.left (), theseMargins.top (), Qt::AbsoluteSize);
		setMask (QRegion { path.toFillPolygon ().toPolygon () });

		const auto& geometry = PreferredGeometry ();

		QPoint point;
		const auto& placeStr = XmlSettingsManager::Instance ().property ("NotifyPosition").toString ();
		const auto margin = 20;
		if (placeStr.startsWith ("Top"_ql))
			point.setY (geometry.top () + margin);
		else
			point.setY (geometry.bottom () - margin);

		if (placeStr.endsWith ("Left"_ql))
			point.setX (geometry.left () + margin);
		else
			point.setX (geometry.right () - margin);

		QRect place (Util::FitRectScreen (point, size (), Util::FitFlag::NoOverlap), size ());
		setGeometry (place);
	}

	void KinotifyWidget::SetOpacity (qreal value)
	{
		setWindowOpacity (value);
		update ();
	}
}
