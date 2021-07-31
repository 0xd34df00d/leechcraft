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
#include <QDesktopWidget>
#include <QMouseEvent>
#include <QTimer>
#include <QState>
#include <QPropertyAnimation>
#include <QFinalState>
#include <QMainWindow>
#include <QPushButton>
#include <util/gui/geometry.h>
#include <util/sll/visitor.h>
#include <util/xpc/util.h>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/irootwindowsmanager.h>
#include <interfaces/core/ientitymanager.h>
#include "xmlsettingsmanager.h"

namespace LC
{
namespace Kinotify
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
				[this]
				{
					QTimer::singleShot (Timeout_, this, &KinotifyWidget::initiateCloseNotification);
				});
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

	void KinotifyWidget::showEvent (QShowEvent*)
	{
		SetWidgetPlace ();
	}

	void KinotifyWidget::OverrideImage (const ImageVar_t& px)
	{
		OverridePixmap_ = px;
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
			const bool followMouse = XmlSettingsManager::Instance ()->property ("FollowMouse").toBool ();

			auto desktop = QApplication::desktop ();
			auto rootWM = GetProxyHolder ()->GetRootWindowsManager ();
			return followMouse ?
					Util::AvailableGeometry (QCursor::pos ()) :
					desktop->availableGeometry (rootWM->GetPreferredWindow ());
		}
	}

	void KinotifyWidget::SetData ()
	{
		Ui_.Title_->setText ("<h3>" + Title_ + "</h3>");
		Ui_.Body_->setText (Body_);
		Util::Visit (OverridePixmap_,
				[] (Util::Void) {},
				[this] (const QPixmap& pixmap) { Ui_.Image_->setPixmap (pixmap); });

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
		const auto& geometry = PreferredGeometry ();

		QPoint point;
		const auto& placeStr = XmlSettingsManager::Instance ()->property ("NotifyPosition").toString ();
		if (placeStr.startsWith ("Top"))
			point.setY (geometry.top () + 20);
		else
			point.setY (geometry.bottom () - 20);

		if (placeStr.endsWith ("Left"))
			point.setX (geometry.left () + 5);
		else
			point.setX (geometry.right () - 5);

		QRect place (Util::FitRectScreen (point, size (), Util::FitFlag::NoOverlap), size ());
		setGeometry (place);
	}

	void KinotifyWidget::SetOpacity (qreal value)
	{
		setWindowOpacity (value);
		update ();
	}
}
}
