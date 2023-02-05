/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "declarativewindow.h"
#include <QQmlContext>
#include <QQuickItem>
#include <util/sll/qtutil.h>
#include <util/sys/paths.h>
#include <util/qml/colorthemeproxy.h>
#include <util/qml/themeimageprovider.h>
#include <util/qml/util.h>
#include <util/gui/unhoverdeletemixin.h>
#include <util/gui/autoresizemixin.h>
#include <util/gui/util.h>
#include "viewmanager.h"

namespace LC::SB2
{
	DeclarativeWindow::DeclarativeWindow (const QUrl& url, QVariantMap params,
			const QPoint& orig, ViewManager *viewMgr, QWidget *parent)
	: QQuickWidget (parent)
	{
		new Util::AutoResizeMixin (orig, [viewMgr] { return viewMgr->GetFreeCoords (); }, this);

		if (!params.take (QStringLiteral ("keepOnFocusLeave")).toBool ())
			new Util::UnhoverDeleteMixin (this, SLOT (beforeDelete ()));

		setWindowFlags (Qt::Tool | Qt::FramelessWindowHint);
		Util::EnableTransparency (*this);

		for (const auto& cand : Util::GetPathCandidates (Util::SysPath::QML, {}))
			engine ()->addImportPath (cand);

		QVector<QQmlContext::PropertyPair> propsVec
		{
			{ QStringLiteral ("colorProxy"),
					QVariant::fromValue (new Util::ColorThemeProxy (GetProxyHolder ()->GetColorThemeManager (), this)) }
		};
		for (const auto& [key, value] : Util::Stlize (params))
			propsVec.push_back ({ key, value });
		rootContext ()->setContextProperties (propsVec);

		engine ()->addImageProvider (QStringLiteral ("ThemeIcons"), new Util::ThemeImageProvider (GetProxyHolder ()));
		setSource (url);

		connect (rootObject (),
				SIGNAL (closeRequested ()),
				this,
				SLOT (deleteLater ()));
	}

	void DeclarativeWindow::beforeDelete ()
	{
		QMetaObject::invokeMethod (rootObject (), "beforeDelete");
		deleteLater ();
	}
}
