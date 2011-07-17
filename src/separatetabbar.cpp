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
#include <QtDebug>
#include "coreproxy.h"
#include "separatetabwidget.h"
#include <QStylePainter>
#include <QStyleOption>

namespace LeechCraft
{
	SeparateTabBar::SeparateTabBar (QWidget *parent)
	: QTabBar (parent)
	, Id_ (0)
	, IsLastTab_ (false)
	{
		setObjectName ("org_LeechCraft_MainWindow_CentralTabBar");
		setExpanding (false);
		setIconSize (QSize (15, 15));
		setContextMenuPolicy (Qt::CustomContextMenu);

		CloseSide_ = (QTabBar::ButtonPosition)this->style ()->
				styleHint (QStyle::SH_TabBar_CloseButtonPosition);

		addTab (QString ());

		IsLastTab_ = true;
	}

	bool SeparateTabBar::IsPinTab (int index) const
	{
		return PinTabsIndex2Name_.contains (tabData (index).toInt ());
	}

	int SeparateTabBar::PinTabsCount () const
	{
		return PinTabsIndex2Name_.count ();
	}

	QList<int> SeparateTabBar::GetPinTabs () const
	{
		QList<int> list;
		for (int i = 0; i < count (); ++i)
			if (IsPinTab (tabData (i).toInt ()))
				list << i;
		return list;
	}

	void SeparateTabBar::SetTabData (int index)
	{
		if ((index < 0) || (index >= count () - 1))
		{
			qWarning () << Q_FUNC_INFO
					<< "invalid index "
					<< index;
			return;
		}

		setTabData (index, ++Id_);
	}

	void SeparateTabBar::SetTabNoClosable (int index)
	{
		if ((index < 0) || (index >= count ()))
		{
			qWarning () << Q_FUNC_INFO
					<< "invalid index "
					<< index;
			return;
		}

		setTabButton (index, CloseSide_, 0);
	}

	void SeparateTabBar::SetLastTab (bool isLast)
	{
		IsLastTab_ = isLast;
	}

	void SeparateTabBar::SetTabWidget (SeparateTabWidget *widget)
	{
		TabWidget_ = widget;
	}

	QString SeparateTabBar::GetPinTabText (int index) const
	{
		return PinTabsIndex2Name_ [tabData (index).toInt ()];
	}

	QSize SeparateTabBar::tabSizeHint (int index) const
	{
		QSize size = QTabBar::tabSizeHint (index);
		if ((index == count () - 1) && IsLastTab_)
			size.setWidth (30);

		return size;
	}

	void SeparateTabBar::mouseReleaseEvent (QMouseEvent *event)
	{
		int index = tabAt (event->pos ());
		if ((index == count () - 1) &&
				(event->button () == Qt::LeftButton) &&
				IsLastTab_)
			emit addDefaultTab (true);

		if (TabWidget_->IsInMoveProcess ())
		{
			int index = currentIndex ();
			if (IsPinTab (index + 1) && !IsPinTab (index))
			{
				SetTabData (index);
				setPinTab (index);
			}
			else if (IsPinTab (index) && index && !IsPinTab (index - 1))
				setUnPinTab (index);

			TabWidget_->SetInMoveProcess (false);
		}
		QTabBar::mouseReleaseEvent (event);
	}

	void SeparateTabBar::mousePressEvent (QMouseEvent *event)
	{
		int index = tabAt (event->pos ());
		if ((index == count () - 1) &&
				(event->button () == Qt::LeftButton) &&
				IsLastTab_)
		{
			mouseReleaseEvent (event);
			return;
		}

		QTabBar::mousePressEvent (event);
	}

	void SeparateTabBar::mouseDoubleClickEvent (QMouseEvent *event)
	{
		QWidget::mouseDoubleClickEvent (event);
		if (tabAt (event->pos ()) == -1)
			emit addDefaultTab (true);
	}

	void SeparateTabBar::tabInserted (int index)
	{
		QTabBar::tabInserted (index);

		int length = 0;
		for (int i = 0; i < count (); ++i)
			length += tabRect (i).width ();

		if ((length + 30 > width ()) && IsLastTab_)
		{
			IsLastTab_ = false;
			removeTab (count () - 1);
			emit showAddTabButton (true);
		}
		
		if ((index != count () - 1 ) && (IsLastTab_))
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
			SetTabNoClosable (count () - 1);

			emit showAddTabButton (false);
		}

		if ((index != count () - 1 ) && (!IsLastTab_))
			emit tabWasRemoved (index);
	}

	void SeparateTabBar::paintEvent (QPaintEvent *event)
	{
		QTabBar::paintEvent (event);
		QStylePainter painter (this);

		for(int i = 0; i < count (); ++i)
		{
			QStyleOptionTabV2 option;
			initStyleOption(&option, i);
			if ((i == count () - 1) && (IsLastTab_))
			{
				CoreProxy proxy;
				QIcon icon = proxy.GetIcon ("addjob");
				painter.drawItemPixmap (option.rect, Qt::AlignCenter, 
						icon.pixmap (QSize (15, 15)));
			}
		}
	}
	
	void SeparateTabBar::setPinTab (int index)
	{
		if ((index < 0) || (index >= count () - 1))
		{
			qWarning () << Q_FUNC_INFO
					<< "invalid index "
					<< index;
			return;
		}

		PinTabsIndex2Name_ [tabData (index).toInt ()] = tabText (index);
		PinTabsIndex2CloseWidget_ [tabData (index).toInt ()] = tabButton (index, CloseSide_);

		setTabText (index, QString ());
		SetTabNoClosable (index);
	}

	void SeparateTabBar::setUnPinTab (int index)
	{
		if ((index < 0) || (index >= count () - 1))
		{
			qWarning () << Q_FUNC_INFO
					<< "invalid index "
					<< index;
			return;
		}

		setTabText (index, PinTabsIndex2Name_ [tabData (index).toInt ()]);
		setTabButton (index, CloseSide_, PinTabsIndex2CloseWidget_ [tabData (index).toInt ()]);
		PinTabsIndex2Name_.remove (tabData (index).toInt ());
		PinTabsIndex2CloseWidget_.remove (tabData (index).toInt ());
		setTabData (index, -1);
	}
}