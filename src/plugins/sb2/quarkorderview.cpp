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

#include "quarkorderview.h"
#include <QDeclarativeContext>
#include <QDeclarativeEngine>
#include <QGraphicsObject>
#include <QtDebug>
#include <util/gui/unhoverdeletemixin.h>
#include <util/sys/paths.h>
#include <util/qml/colorthemeproxy.h>
#include <util/util.h>
#include "viewmanager.h"
#include "themeimageprovider.h"
#include "unhidelistmodel.h"
#include "quarkmanager.h"

namespace LeechCraft
{
namespace SB2
{
	QuarkOrderView::QuarkOrderView (ViewManager *manager, ICoreProxy_ptr proxy, QWidget *parent)
	: QDeclarativeView (parent)
	, Manager_ (manager)
	, Proxy_ (proxy)
	, Model_ (new UnhideListModel (this))
	{
		new Util::UnhoverDeleteMixin (this);

		const auto& file = Util::GetSysPath (Util::SysPath::QML, "sb2", "QuarkOrderView.qml");
		if (file.isEmpty ())
		{
			qWarning () << Q_FUNC_INFO
					<< "file not found";
			deleteLater ();
			return;
		}

		const auto& quarks = manager->GetAddedQuarks ();
		if (quarks.isEmpty ())
		{
			deleteLater ();
			return;
		}

		for (const auto& quark : quarks)
		{
			auto quarkMgr = manager->GetAddedQuarkManager (quark);
			auto item = new QStandardItem;
			item->setData (quarkMgr->GetName (), UnhideListModel::Roles::ItemName);
			item->setData (quarkMgr->GetDescription (), UnhideListModel::Roles::ItemDescription);
			item->setData (quarkMgr->GetID (), UnhideListModel::Roles::ItemClass);
			item->setData (Util::GetAsBase64Src (quarkMgr->GetIcon ().pixmap (32, 32).toImage ()),
					UnhideListModel::Roles::ItemIcon);
			Model_->appendRow (item);
		}

		setStyleSheet ("background: transparent");
		setWindowFlags (Qt::ToolTip);
		setAttribute (Qt::WA_TranslucentBackground);

		rootContext ()->setContextProperty ("quarkListModel", Model_);
		rootContext ()->setContextProperty ("colorProxy",
				new Util::ColorThemeProxy (proxy->GetColorThemeManager (), this));
		engine ()->addImageProvider ("ThemeIcons", new ThemeImageProvider (proxy));
		setSource (QUrl::fromLocalFile (file));

		connect (rootObject (),
				SIGNAL (closeRequested ()),
				this,
				SLOT (deleteLater ()));
		connect (rootObject (),
				SIGNAL (moveRequested (int, int)),
				this,
				SLOT (moveQuark (int, int)));
	}

	void QuarkOrderView::moveQuark (int from, int to)
	{
		qDebug () << Q_FUNC_INFO << from << to;
	}
}
}
