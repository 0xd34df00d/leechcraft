/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2013  Georg Rudoy
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

#include "unhidelistviewbase.h"
#include <QDeclarativeContext>
#include <QDeclarativeEngine>
#include <QGraphicsObject>
#include <QtDebug>
#include <util/qml/colorthemeproxy.h>
#include <util/qml/themeimageprovider.h>
#include <util/gui/unhoverdeletemixin.h>
#include <util/sys/paths.h>
#include <util/qml/unhidelistmodel.h>

namespace LeechCraft
{
namespace Util
{
	UnhideListViewBase::UnhideListViewBase (ICoreProxy_ptr proxy, QWidget *parent)
	: QDeclarativeView (parent)
	, Model_ (new UnhideListModel (this))
	{
		new UnhoverDeleteMixin (this);

		const auto& file = GetSysPath (SysPath::QML, "common", "UnhideListView.qml");
		if (file.isEmpty ())
		{
			qWarning () << Q_FUNC_INFO
					<< "file not found";
			deleteLater ();
			return;
		}

		setStyleSheet ("background: transparent");
		setWindowFlags (Qt::ToolTip);
		setAttribute (Qt::WA_TranslucentBackground);

		for (const auto& cand : GetPathCandidates (SysPath::QML, ""))
			engine ()->addImportPath (cand);

		rootContext ()->setContextProperty ("unhideListModel", Model_);
		rootContext ()->setContextProperty ("colorProxy",
				new Util::ColorThemeProxy (proxy->GetColorThemeManager (), this));
		engine ()->addImageProvider ("ThemeIcons", new Util::ThemeImageProvider (proxy));
		setSource (QUrl::fromLocalFile (file));

		connect (rootObject (),
				SIGNAL (closeRequested ()),
				this,
				SLOT (deleteLater ()));
		connect (rootObject (),
				SIGNAL (itemUnhideRequested (QString)),
				this,
				SIGNAL (itemUnhideRequested (QString)));
	}

	void UnhideListViewBase::SetItems (const QList<QStandardItem*>& items)
	{
		Model_->invisibleRootItem ()->appendRows (items);
	}
}
}
