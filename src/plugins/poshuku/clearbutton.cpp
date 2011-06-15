/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2011 Minh Ngo
 * Copyright (C) 2006-2011  Georg Rudoy
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

#include "clearbutton.h"
#include <QPainter>
#include <QVariant>
#include "core.h"

namespace LeechCraft
{
namespace Poshuku
{
	ClearButton::ClearButton (QWidget *parent)
	: QToolButton (parent)
	{
		setCursor (Qt::ArrowCursor);
		setToolTip (tr ("Clear"));
		setToolButtonStyle (Qt::ToolButtonIconOnly);
		setVisible (false);
		setFocusPolicy (Qt::NoFocus);
		setBackgroundRole (QPalette::Light);
		setIcon (Core::Instance ().GetProxy ()->GetIcon ("clearall"));
	}

	void ClearButton::textChanged (const QString& text)
	{
		setVisible (!text.isEmpty ());
	}
}
}
