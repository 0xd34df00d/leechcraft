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

#include "rootpathsettingsmanager.h"
#include <QStandardItemModel>
#include <QFile>
#include <xmlsettingsdialog/datasourceroles.h>
#include "xmlsettingsmanager.h"
#include "core.h"
#include "localcollection.h"

namespace LeechCraft
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
		const QString& str = list.value (0).toString ();
		if (QFile::exists (str))
			Core::Instance ().GetLocalCollection ()->Scan (str);
	}

	void RootPathSettingsManager::removeRequested (const QString&, const QModelIndexList& indexes)
	{
		QStringList paths;
		Q_FOREACH (const auto& idx, indexes)
			paths << idx.data ().toString ();

		auto coll = Core::Instance ().GetLocalCollection ();
		Q_FOREACH (const auto& path, paths)
			coll->Unscan (path);
	}

	void RootPathSettingsManager::handleRootPathsChanged ()
	{
		while (Model_->rowCount ())
			Model_->removeRow (0);

		const auto& dirs = Core::Instance ().GetLocalCollection ()->GetDirs ();
		Q_FOREACH (const auto& dir, dirs)
			Model_->appendRow (new QStandardItem (dir));
	}
}
}
