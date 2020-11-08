/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "unhidelistviewbase.h"
#include <QQmlContext>
#include <QQmlEngine>
#include <QQuickItem>
#include <QtDebug>
#include <util/qml/colorthemeproxy.h>
#include <util/qml/themeimageprovider.h>
#include <util/gui/unhoverdeletemixin.h>
#include <util/sys/paths.h>
#include <util/qml/unhidelistmodel.h>
#include <util/qml/util.h>

namespace LC::Util
{
	UnhideListViewBase::UnhideListViewBase (const ICoreProxy_ptr& proxy,
			const std::function<void (UnhideListModel*)>& filler, QWidget *parent)
	: QQuickWidget (parent)
	, Model_ (new UnhideListModel (this))
	{
		new UnhoverDeleteMixin (this);

		if (filler)
			filler (Model_);

		const auto& file = GetSysPath (SysPath::QML, "common", "UnhideListView.qml");
		if (file.isEmpty ())
		{
			qWarning () << Q_FUNC_INFO
					<< "file not found";
			deleteLater ();
			return;
		}

		setWindowFlags (Qt::ToolTip);
		Util::EnableTransparency (this);

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
