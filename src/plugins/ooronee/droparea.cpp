/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "droparea.h"
#include <QCursor>
#include <QMimeData>

namespace LC
{
namespace Ooronee
{
	DropArea::DropArea (QQuickItem *parent)
	: QQuickItem { parent }
	{
		SetAcceptingDrops (true);
	}

	bool DropArea::GetAcceptingDrops () const
	{
		return flags () & ItemAcceptsDrops;
	}

	void DropArea::SetAcceptingDrops (bool accepting)
	{
		if (accepting == GetAcceptingDrops ())
			return;

		setFlag (ItemAcceptsDrops, accepting);
		emit acceptingDropsChanged (accepting);
	}

	void DropArea::dragEnterEvent (QDragEnterEvent *event)
	{
		auto data = event->mimeData ();
		if (!(data->hasImage () || data->hasText ()))
			return;

		event->acceptProposedAction ();
		setCursor (Qt::DragCopyCursor);

		emit dragEntered (data->hasImage () ?
				data->imageData () :
				data->text ());
	}

	void DropArea::dragLeaveEvent (QDragLeaveEvent*)
	{
		unsetCursor ();
		emit dragLeft ();
	}

	void DropArea::dropEvent (QDropEvent *event)
	{
		unsetCursor ();

		const auto data = event->mimeData ();
		emit dataDropped (data->hasImage () ?
				data->imageData () :
				data->text ());
	}
}
}
