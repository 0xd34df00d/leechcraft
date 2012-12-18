/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2012  Georg Rudoy
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

#include "tooltipview.h"
#include <QAbstractItemModel>
#include <QDeclarativeContext>
#include <util/sys/paths.h>
#include <util/gui/unhoverdeletemixin.h>
#include <util/qml/colorthemeproxy.h>

namespace LeechCraft
{
namespace TPI
{
	TooltipView::TooltipView (QAbstractItemModel *model,
			IColorThemeManager *manager, QWidget *parent)
	: QDeclarativeView (parent)
	, UnhoverDeleter_ (new Util::UnhoverDeleteMixin (this, SLOT (hide ())))
	{
		setStyleSheet ("background: transparent");
		setWindowFlags (Qt::ToolTip);
		setAttribute (Qt::WA_TranslucentBackground);

		rootContext ()->setContextProperty ("infoModel", model);
		rootContext ()->setContextProperty ("colorProxy", new Util::ColorThemeProxy (manager, this));
		setSource (QUrl::fromLocalFile (Util::GetSysPath (Util::SysPath::QML, "tpi", "Tooltip.qml")));
	}

	void TooltipView::Hovered ()
	{
		UnhoverDeleter_->Stop ();
	}

	void TooltipView::Unhovered ()
	{
		UnhoverDeleter_->Start ();
	}
}
}
