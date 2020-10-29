/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "quarkorderview.h"
#include <QQmlContext>
#include <QQuickItem>
#include <util/gui/unhoverdeletemixin.h>
#include <util/sys/paths.h>
#include <util/qml/colorthemeproxy.h>
#include <util/qml/themeimageprovider.h>
#include <util/qml/unhidelistmodel.h>
#include <util/qml/util.h>
#include <util/util.h>
#include "viewmanager.h"
#include "quarkmanager.h"

namespace LC::SB2
{
	QuarkOrderView::QuarkOrderView (ViewManager *manager, ICoreProxy_ptr proxy, QWidget *parent)
	: QQuickWidget (parent)
	, Manager_ (manager)
	, Proxy_ (proxy)
	, Model_ (new Util::UnhideListModel (this))
	{
		new Util::UnhoverDeleteMixin (this);

		const auto& file = Util::GetSysPath (Util::SysPath::QML, QStringLiteral ("sb2"), QStringLiteral ("QuarkOrderView.qml"));
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

			const auto& manifest = quarkMgr->GetManifest ();

			item->setData (manifest.GetName (), Util::UnhideListModel::Roles::ItemName);
			item->setData (manifest.GetDescription (), Util::UnhideListModel::Roles::ItemDescription);
			item->setData (manifest.GetID (), Util::UnhideListModel::Roles::ItemClass);
			item->setData (Util::GetAsBase64Src (manifest.GetIcon ().pixmap (32, 32).toImage ()),
					Util::UnhideListModel::Roles::ItemIcon);
			Model_->appendRow (item);
		}

		setWindowFlags (Qt::ToolTip);
		Util::EnableTransparency (this);

		for (const auto& cand : Util::GetPathCandidates (Util::SysPath::QML, {}))
			engine ()->addImportPath (cand);

		rootContext ()->setContextProperty (QStringLiteral ("quarkListModel"), Model_);
		rootContext ()->setContextProperty (QStringLiteral ("colorProxy"),
				new Util::ColorThemeProxy (proxy->GetColorThemeManager (), this));
		engine ()->addImageProvider (QStringLiteral ("ThemeIcons"), new Util::ThemeImageProvider (proxy));
		setSource (QUrl::fromLocalFile (file));

		connect (rootObject (),
				SIGNAL (closeRequested ()),
				this,
				SLOT (deleteLater ()));
		connect (rootObject (),
				SIGNAL (moveRequested (QString, QString, int)),
				this,
				SLOT (moveQuark (QString, QString, int)));
		connect (rootObject (),
				SIGNAL (quarkClassHovered (QString)),
				this,
				SIGNAL (quarkClassHovered (QString)));
		connect (rootObject (),
				SIGNAL (quarkRemoveRequested (QString)),
				this,
				SLOT (handleQuarkCloseRequested (QString)),
				Qt::QueuedConnection);
	}

	void QuarkOrderView::handleQuarkCloseRequested (const QString& qClass)
	{
		Manager_->RemoveQuark (qClass);

		for (int i = 0, rc = Model_->rowCount (); i < rc; ++i)
		{
			auto item = Model_->item (i);
			if (item->data (Util::UnhideListModel::Roles::ItemClass) == qClass)
			{
				Model_->removeRow (i);
				break;
			}
		}
	}

	namespace
	{
		int FindClassRow (QStandardItemModel *model, const QString& itemClass)
		{
			for (int i = 0, rc = model->rowCount (); i < rc; ++i)
			{
				auto item = model->item (i);
				if (item->data (Util::UnhideListModel::Roles::ItemClass).toString () == itemClass)
					return i;
			}
			return -1;
		}
	}

	void QuarkOrderView::moveQuark (const QString& from, const QString& to, int shift)
	{
		const auto fromPos = FindClassRow (Model_, from);
		auto toPos = FindClassRow (Model_, to);
		if (fromPos < 0 || toPos < 0)
		{
			qWarning () << Q_FUNC_INFO
					<< "incorrect classes"
					<< from
					<< to
					<< fromPos
					<< toPos;
			return;
		}

		toPos += shift;

		Manager_->MoveQuark (fromPos, toPos);

		if (fromPos < toPos)
			--toPos;
		Model_->insertRow (toPos, Model_->takeRow (fromPos));
	}
}
