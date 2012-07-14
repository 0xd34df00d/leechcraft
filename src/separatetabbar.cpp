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

#include "separatetabbar.h"
#include <QMouseEvent>
#include <QStyle>
#include <QStylePainter>
#include <QStyleOption>
#include <QtDebug>
#include "coreproxy.h"
#include "separatetabwidget.h"
#include "core.h"
#include "tabmanager.h"

namespace LeechCraft
{
	SeparateTabBar::SeparateTabBar (QWidget *parent)
	: QTabBar (parent)
	, Id_ (0)
	, IsLastTab_ (false)
	, InMove_ (false)
	{
		setObjectName ("org_LeechCraft_MainWindow_CentralTabBar");
		setExpanding (false);
		setIconSize (QSize (15, 15));
		setContextMenuPolicy (Qt::CustomContextMenu);
		setElideMode (Qt::ElideRight);

		addTab (QString ());

		IsLastTab_ = true;
	}

	void SeparateTabBar::SetTabData (int index)
	{
		if (index < 0 || index >= count () - 1)
		{
			qWarning () << Q_FUNC_INFO
					<< "invalid index "
					<< index;
			return;
		}

		setTabData (index, ++Id_);
	}

	void SeparateTabBar::SetTabClosable (int index, bool closable, QWidget *closeButton)
	{
		if (index < 0 ||
				index >= count ())
		{
			qWarning () << Q_FUNC_INFO
					<< "invalid index "
					<< index;
			return;
		}

		setTabButton (index, GetCloseButtonPosition (), closeButton);
	}

	void SeparateTabBar::SetLastTab (bool isLast)
	{
		IsLastTab_ = isLast;
	}

	void SeparateTabBar::SetTabWidget (SeparateTabWidget *widget)
	{
		TabWidget_ = widget;
	}

	QTabBar::ButtonPosition SeparateTabBar::GetCloseButtonPosition ()
	{
		return static_cast<QTabBar::ButtonPosition> (style ()->
				styleHint (QStyle::SH_TabBar_CloseButtonPosition));
	}

	void SeparateTabBar::SetInMove (bool inMove)
	{
		InMove_ = inMove;
	}

	QSize SeparateTabBar::tabSizeHint (int index) const
	{
		QSize size = QTabBar::tabSizeHint (index);
		const int tc = count ();
		if (index == tc - 1 && IsLastTab_)
			size.setWidth (30);
		else
		{
			const int target = std::max (100,
					this->size ().width () / tc);
			if (size.width () > target)
				size.setWidth (target);
		}

		return size;
	}

	void SeparateTabBar::mouseReleaseEvent (QMouseEvent *event)
	{
		int index = tabAt (event->pos ());
		if (index == count () - 1 &&
				event->button () == Qt::LeftButton &&
				IsLastTab_)
		{
			emit addDefaultTab ();
			return;
		}

		if (InMove_)
		{
			emit releasedMouseAfterMove (currentIndex ());
			InMove_ = false;
			emit currentChanged (currentIndex ());
		}
		else if (index != -1 &&
				event->button () == Qt::MidButton &&
				index != count () - 1)
			Core::Instance ().GetTabManager ()->remove (index);

		QTabBar::mouseReleaseEvent (event);
	}

	void SeparateTabBar::mousePressEvent (QMouseEvent *event)
	{
		if (IsLastTab_ &&
				event->button () == Qt::LeftButton &&
				tabAt (event->pos ()) == count () - 1)
			return;

		QTabBar::mousePressEvent (event);
	}

	void SeparateTabBar::mouseDoubleClickEvent (QMouseEvent *event)
	{
		QWidget::mouseDoubleClickEvent (event);
		if (tabAt (event->pos ()) == -1)
			emit addDefaultTab ();
	}

	void SeparateTabBar::tabInserted (int index)
	{
		QTabBar::tabInserted (index);

		int length = 0;
		for (int i = 0; i < count (); ++i)
			length += tabRect (i).width ();

		if (length + 30 > width () && IsLastTab_)
		{
			IsLastTab_ = false;
			removeTab (count () - 1);
			emit showAddTabButton (true);
		}
		
		if (index != count () - 1 && (IsLastTab_))
			emit tabWasInserted (index);
	}

	void SeparateTabBar::tabRemoved (int index)
	{
		QTabBar::tabRemoved (index);

		int length = 0;
		for (int i = 0; i < count (); ++i)
			length += tabRect (i).width ();

		if (length + 60 < width () && !IsLastTab_)
		{
			IsLastTab_ = true;

			addTab (QString ());
			SetTabClosable (count () - 1, false);

			emit showAddTabButton (false);
		}

		if (index != count () - 1 && !IsLastTab_)
			emit tabWasRemoved (index);
	}

	void SeparateTabBar::paintEvent (QPaintEvent *event)
	{
		QTabBar::paintEvent (event);
		QStylePainter painter (this);

		if (count () > 0 && IsLastTab_)
		{
			CoreProxy proxy;
			QStyleOptionTabV2 option;
			initStyleOption (&option, count () - 1);
			QIcon icon = proxy.GetIcon ("list-add");
			painter.drawItemPixmap (option.rect, Qt::AlignCenter, 
					icon.pixmap (QSize (15, 15)));
		}
	}
}
