/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "lineeditbuttonmanager.h"
#include <stdexcept>
#include <QLineEdit>
#include <QStyle>
#include <QToolButton>
#include <QEvent>
#include <QtDebug>

namespace LC
{
namespace Util
{
	LineEditButtonManager::LineEditButtonManager (QLineEdit *edit)
	: QObject { edit }
	, Edit_ { edit }
	, FrameWidth_ { edit->style ()->pixelMetric (QStyle::PM_DefaultFrameWidth) }
	, Pad_ { 1 + FrameWidth_ }
	{
		edit->installEventFilter (this);

		if (edit->findChildren<LineEditButtonManager*> ().size () > 1)
		{
			std::string str { "LineEditButtonManager is already installed on the edit" };

			const auto& name = edit->objectName ();
			if (!name.isEmpty ())
				str += " " + name.toStdString ();

			throw std::runtime_error (str);
		}
	}

	void LineEditButtonManager::Add (QToolButton *button)
	{
		Buttons_ << button;

		const auto& buttonSH = button->sizeHint ();
		Pad_ += buttonSH.width ();

		Edit_->setStyleSheet (QString ("QLineEdit { padding-right: %1px; }")
					.arg (Pad_));

		UpdatePos ();
	}

	bool LineEditButtonManager::eventFilter (QObject *obj, QEvent *event)
	{
		if (event->type () == QEvent::Resize ||
			event->type () == QEvent::Move)
			UpdatePos ();

		return QObject::eventFilter (obj, event);
	}

	void LineEditButtonManager::UpdatePos ()
	{
		int sumWidth = 0;

		for (const auto button : Buttons_)
		{
			const auto& hint = button->sizeHint ();

			sumWidth += hint.width ();

			const auto& rect = Edit_->rect ();
			const int frameWidth = Edit_->style ()->pixelMetric (QStyle::PM_DefaultFrameWidth);
			button->move (rect.right () - frameWidth - sumWidth,
					(rect.bottom () + 1 - hint.height ()) / 2);
		}
	}
}
}
