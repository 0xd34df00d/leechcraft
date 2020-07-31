/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "rootpathsettingsmanager.h"
#include <QStandardItemModel>
#include <QFile>
#include <QMessageBox>
#include <util/sll/prelude.h>
#include <xmlsettingsdialog/datasourceroles.h>
#include "xmlsettingsmanager.h"
#include "core.h"
#include "localcollection.h"

namespace LC
{
namespace LMP
{
	RootPathSettingsManager::RootPathSettingsManager (QObject *parent)
	: QObject (parent)
	, Model_ (new QStandardItemModel (this))
	{
		QStandardItem *item = new QStandardItem (tr ("Path"));
		item->setData (DataSources::DataFieldType::LocalPath,
				DataSources::DataSourceRole::FieldType);
		Model_->setHorizontalHeaderItem (0, item);

		auto collection = Core::Instance ().GetLocalCollection ();
		connect (collection,
				SIGNAL (rootPathsChanged (QStringList)),
				this,
				SLOT (handleRootPathsChanged ()));
		handleRootPathsChanged ();
	}

	QAbstractItemModel* RootPathSettingsManager::GetModel () const
	{
		return Model_;
	}

	void RootPathSettingsManager::addRequested (const QString&, const QVariantList& list)
	{
		if (!XmlSettingsManager::Instance ().Property ("HasAskedAboutAAFetch", false).toBool ())
		{
			XmlSettingsManager::Instance ().setProperty ("HasAskedAboutAAFetch", true);
			const auto fetch = QMessageBox::question (nullptr,
					"LeechCraft",
					tr ("Do you want LMP to automatically fetch missing album art? It is done in "
						"the background and won't disturb you, but can consume quite some traffic "
						"and local storage space, especially if you have a lot of albums in your "
						"collection.<br/><br/>You can always toggle this option later in LMP "
						"settings."),
					QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes;

			XmlSettingsManager::Instance ().setProperty ("AutoFetchAlbumArt", fetch);
		}

		const QString& str = list.value (0).toString ();
		if (QFile::exists (str))
			Core::Instance ().GetLocalCollection ()->Scan (str);
	}

	void RootPathSettingsManager::removeRequested (const QString&, const QModelIndexList& indexes)
	{
		auto paths = Util::Map (indexes, [] (const auto& idx) { return idx.data ().toString (); });

		auto coll = Core::Instance ().GetLocalCollection ();
		for (const auto& path : paths)
			coll->Unscan (path);
	}

	void RootPathSettingsManager::handleRootPathsChanged ()
	{
		if (const auto rc = Model_->rowCount ())
			Model_->removeRows (0, rc);

		const auto& dirs = Core::Instance ().GetLocalCollection ()->GetDirs ();
		for (const auto& dir : dirs)
			Model_->appendRow (new QStandardItem (dir));
	}
}
}
