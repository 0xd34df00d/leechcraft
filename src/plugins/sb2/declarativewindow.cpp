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

#include "declarativewindow.h"
#include <QResizeEvent>
#include <QDeclarativeContext>
#include <QDeclarativeEngine>
#include <QGraphicsObject>
#include <QtDebug>
#include <QFile>
#include <util/sys/paths.h>
#include <util/qml/colorthemeproxy.h>
#include <util/qml/themeimageprovider.h>
#include <util/gui/unhoverdeletemixin.h>
#include <util/gui/util.h>
#include "autoresizemixin.h"

namespace LeechCraft
{
namespace SB2
{
	DeclarativeWindow::DeclarativeWindow (const QUrl& url, QVariantMap params,
			const QPoint& orig, ViewManager *viewMgr, ICoreProxy_ptr proxy, QWidget *parent)
	: QDeclarativeView (parent)
	{
		new AutoResizeMixin (orig, viewMgr, this);

		if (!params.take ("keepOnFocusLeave").toBool ())
			new Util::UnhoverDeleteMixin (this);

		setStyleSheet ("background: transparent");
		setWindowFlags (Qt::ToolTip);
		setAttribute (Qt::WA_TranslucentBackground);

		for (const auto& cand : Util::GetPathCandidates (Util::SysPath::QML, ""))
			engine ()->addImportPath (cand);

		rootContext ()->setContextProperty ("colorProxy",
				new Util::ColorThemeProxy (proxy->GetColorThemeManager (), this));
		for (const auto& key : params.keys ())
			rootContext ()->setContextProperty (key, params [key]);
		engine ()->addImageProvider ("ThemeIcons", new Util::ThemeImageProvider (proxy));
		setSource (url);

		connect (rootObject (),
				SIGNAL (closeRequested ()),
				this,
				SLOT (deleteLater ()));
	}
}
}
