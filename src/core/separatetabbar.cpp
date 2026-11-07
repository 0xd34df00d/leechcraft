/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2011  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "separatetabbar.h"
#include <QMouseEvent>
#include <QStyle>
#include <QStylePainter>
#include <QStyleOption>
#include <QApplication>
#include <QMimeData>
#include <QDrag>
#include <QtDebug>
#include <util/sll/prelude.h>
#include <interfaces/ihavetabs.h>
#include <interfaces/idndtab.h>
#include "coreproxy.h"
#include "entitymanager.h"
#include "separatetabwidget.h"

namespace LC
{
	SeparateTabBar::SeparateTabBar (QWidget *parent)
	: QTabBar (parent)
	{
		setObjectName ("org_LeechCraft_MainWindow_CentralTabBar");
		setExpanding (false);
		setContextMenuPolicy (Qt::CustomContextMenu);
		setElideMode (Qt::ElideRight);
		setDocumentMode (true);

		setAcceptDrops (true);
		setMovable (true);
		setUsesScrollButtons (true);

		addTab (QString ());

		connect (this,
				SIGNAL (currentChanged (int)),
				this,
				SLOT (toggleCloseButtons ()));
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

		setTabButton (index, GetCloseButtonPosition (), closable ? closeButton : nullptr);
	}

	void SeparateTabBar::SetTabWidget (SeparateTabWidget *widget)
	{
		TabWidget_ = widget;
	}

	void SeparateTabBar::SetAddTabButton (QWidget *w)
	{
		AddTabButton_ = w;
		setTabButton (count () - 1, GetAntiCloseButtonPosition (), w);
	}

	QTabBar::ButtonPosition SeparateTabBar::GetCloseButtonPosition () const
	{
		return static_cast<QTabBar::ButtonPosition> (style ()->
				styleHint (QStyle::SH_TabBar_CloseButtonPosition));
	}

	void SeparateTabBar::SetInMove (bool inMove)
	{
		InMove_ = inMove;
	}

	QTabBar::ButtonPosition SeparateTabBar::GetAntiCloseButtonPosition () const
	{
		return GetCloseButtonPosition () == QTabBar::LeftSide ?
				QTabBar::RightSide :
				QTabBar::LeftSide;
	}

	struct TabInfo
	{
		int Idx_;
		int WidthHint_;
	};

	QVector<TabInfo> SeparateTabBar::GetTabInfos () const
	{
		const auto maxTabWidth = width () / 4;
		const auto cnt = count ();

		QVector<TabInfo> infos;
		for (int i = 0; i < cnt - 1; ++i)
		{
			auto hintedWidth = QTabBar::tabSizeHint (i).width ();

			if (const auto button = tabButton (i, GetCloseButtonPosition ()))
				if (!button->isVisible ())
					hintedWidth -= button->width ();

			infos.push_back ({
					i,
					std::min (hintedWidth, maxTabWidth)
				});
		}
		std::sort (infos.begin (), infos.end (), Util::ComparingBy (&TabInfo::WidthHint_));

		return infos;
	}

	void SeparateTabBar::UpdateComputedWidths () const
	{
		auto infos = GetTabInfos ();

		ComputedWidths_.resize (infos.size ());

		const auto btnWidth = QTabBar::tabSizeHint (count () - 1).width ();

		auto remainder = width () - btnWidth;

		while (!infos.isEmpty ())
		{
			auto currentMax = remainder / infos.size ();
			if (infos.front ().WidthHint_ > currentMax)
				break;

			const auto& info = infos.front ();
			remainder -= info.WidthHint_;
			ComputedWidths_ [info.Idx_] = info.WidthHint_;
			infos.pop_front ();
		}

		if (infos.isEmpty ())
			return;

		const auto uniform = remainder / infos.size ();
		for (const auto& info : infos)
			ComputedWidths_ [info.Idx_] = uniform;
	}

	void SeparateTabBar::toggleCloseButtons () const
	{
		const bool widthsEmpty = ComputedWidths_.isEmpty ();
		if (widthsEmpty)
			UpdateComputedWidths ();

		const auto current = currentIndex ();
		for (int i = 0, cnt = count () - 1; i < cnt; ++i)
		{
			const auto button = tabButton (i, GetCloseButtonPosition ());
			if (!button)
				continue;

			const auto visible = i == current ||
					button->width () * 2.5 < ComputedWidths_.value (i);
			button->setVisible (visible);
		}

		if (widthsEmpty)
			ComputedWidths_.clear ();
	}

	void SeparateTabBar::tabLayoutChange ()
	{
		ComputedWidths_.clear ();
		QTabBar::tabLayoutChange ();
	}

	QSize SeparateTabBar::tabSizeHint (int index) const
	{
		auto result = QTabBar::tabSizeHint (index);
		if (index == count () - 1)
			return result;

		if (ComputedWidths_.isEmpty ())
		{
			UpdateComputedWidths ();
			toggleCloseButtons ();
		}

		result.setWidth (ComputedWidths_.value (index));
		return result;
	}

	void SeparateTabBar::mouseReleaseEvent (QMouseEvent *event)
	{
		if (tabAt (event->pos ()) == count () - 1)
			switch (event->button ())
			{
			case Qt::LeftButton:
				emit addDefaultTab ();
				return;
			case Qt::MiddleButton:
				return;
			default:
				break;
			}

		if (InMove_)
		{
			emit releasedMouseAfterMove (currentIndex ());
			InMove_ = false;
			emit currentChanged (currentIndex ());
		}

		QTabBar::mouseReleaseEvent (event);
	}

	void SeparateTabBar::mousePressEvent (QMouseEvent *event)
	{
		if (event->button () == Qt::LeftButton &&
				tabAt (event->pos ()) == count () - 1)
			return;

		setMovable (QApplication::keyboardModifiers () == Qt::NoModifier);

		if (event->button () == Qt::LeftButton)
			DragStartPos_ = event->pos ();
		QTabBar::mousePressEvent (event);
	}

	void SeparateTabBar::mouseMoveEvent (QMouseEvent *event)
	{
		if (isMovable ())
		{
			QTabBar::mouseMoveEvent (event);
			return;
		}

		if (!(event->buttons () & Qt::LeftButton))
			return;

		if ((event->pos () - DragStartPos_).manhattanLength () < QApplication::startDragDistance ())
			return;

		const int dragIdx = tabAt (DragStartPos_);
		auto widget = TabWidget_->Widget (dragIdx);
		auto itw = qobject_cast<ITabWidget*> (widget);
		if (!itw)
		{
			qWarning () << Q_FUNC_INFO
					<< "widget at"
					<< dragIdx
					<< "doesn't implement ITabWidget";
			return;
		}

		auto px = widget->grab ();
		px = px.scaledToWidth (px.width () / 2, Qt::SmoothTransformation);

		auto idt = qobject_cast<IDNDTab*> (widget);

		auto data = new QMimeData ();
		if (!idt || QApplication::keyboardModifiers () == Qt::ControlModifier)
		{
			data->setData ("x-leechcraft/tab-drag-action", "reordering");
			data->setData ("x-leechcraft/tab-tabclass", itw->GetTabClassInfo ().TabClass_);
		}
		else
			idt->FillMimeData (data);

		auto drag = new QDrag (this);
		drag->setMimeData (data);
		drag->setPixmap (px);
		drag->exec ();
	}

	namespace
	{
		bool IsTabReordering (const QMimeData& data)
		{
			return data.data ("x-leechcraft/tab-drag-action") == "reordering";
		}

		IDNDTab* GetDndTab (const SeparateTabWidget& tabs, int index)
		{
			return qobject_cast<IDNDTab*> (tabs.Widget (index));
		}

		bool CouldDropOnTab (const IDNDTab *tab, const QMimeData& data)
		{
			return tab && tab->CouldHandleDrop (data);
		}

		bool TryDropOnTab (IDNDTab *tab, const QMimeData& data)
		{
			return CouldDropOnTab (tab, data) && tab->HandleDrop (data);
		}

		// Nothing in LeechCraft takes ownership of dropped data, so a drop is
		// always a copy, and a source offering only a move is refused.
		void AcceptCopy (QDropEvent& event)
		{
			if (!(event.possibleActions () & Qt::CopyAction))
				return;

			event.setDropAction (Qt::CopyAction);
			event.accept ();
		}
	}

	void SeparateTabBar::dragEnterEvent (QDragEnterEvent *event)
	{
		CoreCouldHandleDrop_ = CouldHandleDroppedData (*event->mimeData ());

		// Qt delivers drag moves only after an accepted enter, and the moves
		// are where the hovered tab gets activated and asked about the drop.
		event->acceptProposedAction ();
	}

	void SeparateTabBar::dragMoveEvent (QDragMoveEvent *event)
	{
		event->ignore ();

		const auto tabIdx = tabAt (event->position ().toPoint ());
		const auto isAddTabButton = tabIdx == count () - 1;

		const auto& data = *event->mimeData ();
		if (IsTabReordering (data))
		{
			if (!isAddTabButton)
				event->acceptProposedAction ();
			return;
		}

		if (tabIdx >= 0 && !isAddTabButton)
			TabWidget_->setCurrentIndex (tabIdx);

		if (CouldDropOnTab (GetDndTab (*TabWidget_, tabIdx), data) || CoreCouldHandleDrop_.value ())
			AcceptCopy (*event);
	}

	void SeparateTabBar::dragLeaveEvent (QDragLeaveEvent*)
	{
		CoreCouldHandleDrop_.reset ();
	}

	void SeparateTabBar::dropEvent (QDropEvent *event)
	{
		CoreCouldHandleDrop_.reset ();

		const int to = tabAt (event->position ().toPoint ());
		const auto& data = *event->mimeData ();
		if (IsTabReordering (data))
		{
			const int from = tabAt (DragStartPos_);
			if (from == to || to == count () - 1)
				return;

			moveTab (from, to);
			emit releasedMouseAfterMove (to);
			event->acceptProposedAction ();
			return;
		}

		if (TryDropOnTab (GetDndTab (*TabWidget_, to), data) || HandleDroppedData (data))
			AcceptCopy (*event);
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
		emit tabWasInserted (index);
	}

	void SeparateTabBar::tabRemoved (int index)
	{
		QTabBar::tabRemoved (index);
		emit tabWasRemoved (index);
	}
}
