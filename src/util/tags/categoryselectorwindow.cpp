/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "categoryselectorwindow.h"
#include <QDialogButtonBox>
#include <QMoveEvent>
#include <QScreen>
#include <QVBoxLayout>
#include "categoryselector.h"

namespace LC::Util
{
	CategorySelectorWindow::CategorySelectorWindow (Buttons buttons, QWidget *parent)
	: QDialog { parent }
	, Selector_ { *new CategorySelector { this } }
	{
		setWindowTitle (tr ("Tags selector"));
		setWindowFlags (Qt::Dialog | Qt::WindowStaysOnTopHint);
		setMinimumHeight (screen ()->availableGeometry ().height () / 3 * 2);

		const auto box = new QDialogButtonBox { this };
		switch (buttons)
		{
		case Buttons::Close:
			box->setStandardButtons (QDialogButtonBox::Close);
			break;
		case Buttons::AcceptReject:
			box->setStandardButtons (QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
			break;
		}
		connect (box,
				&QDialogButtonBox::accepted,
				this,
				&QDialog::accept);
		connect (box,
				&QDialogButtonBox::rejected,
				this,
				&QDialog::reject);

		const auto layout = new QVBoxLayout { this };
		layout->addWidget (&Selector_);
		layout->addWidget (box);
	}

	CategorySelector& CategorySelectorWindow::Selector ()
	{
		return Selector_;
	}

	void CategorySelectorWindow::moveEvent (QMoveEvent *e)
	{
		QDialog::moveEvent (e);

		const auto& pos = e->pos ();
		const auto& avail = screen ()->availableGeometry ();
		int dx = 0;
		int dy = 0;
		if (pos.x () + width () > avail.width ())
			dx = width () + pos.x () - avail.width ();
		if (pos.y () + height () > avail.height () &&
				height () < avail.height ())
			dy = height () + pos.y () - avail.height ();

		if (dx || dy)
			move (pos - QPoint { dx, dy });
	}
}
