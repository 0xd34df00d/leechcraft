/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "elidinglabel.h"

#include <QEvent>

namespace LC::Util
{
	ElidingLabel::ElidingLabel (QWidget *parent)
	: QLabel { parent }
	{
		setTextFormat (Qt::PlainText);
		setSizePolicy (QSizePolicy::Expanding, sizePolicy ().verticalPolicy ());
	}

	void ElidingLabel::SetFullText (const QString& text)
	{
		if (FullText_ == text)
			return;

		FullText_ = text;
		UpdateElide ();
	}

	QString ElidingLabel::GetFullText () const
	{
		return FullText_;
	}

	void ElidingLabel::SetElideMode (Qt::TextElideMode mode)
	{
		if (ElideMode_ == mode)
			return;

		ElideMode_ = mode;
		UpdateElide ();
	}

	Qt::TextElideMode ElidingLabel::GetElideMode () const
	{
		return ElideMode_;
	}

	void ElidingLabel::changeEvent (QEvent *ev)
	{
		QLabel::changeEvent (ev);
		switch (ev->type ())
		{
		case QEvent::FontChange:
		case QEvent::StyleChange:
		case QEvent::ApplicationFontChange:
		case QEvent::LayoutDirectionChange:
			UpdateElide ();
			break;
		default:
			break;
		}
	}

	void ElidingLabel::resizeEvent (QResizeEvent *ev)
	{
		QLabel::resizeEvent (ev);
		UpdateElide ();
	}

	void ElidingLabel::UpdateElide ()
	{
		const auto& fm = fontMetrics ();

		const auto& cm = contentsMargins ();
		setMaximumWidth (fm.horizontalAdvance (FullText_) + cm.left () + cm.right ());

		const auto& elided = fm.elidedText (FullText_, ElideMode_, contentsRect ().width ());
		setText (elided);
	}
}
