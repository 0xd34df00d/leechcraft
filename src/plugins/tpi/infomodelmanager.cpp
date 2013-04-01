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

#include "infomodelmanager.h"
#include <QStandardItemModel>
#include <interfaces/core/ipluginsmanager.h>
#include <interfaces/ijobholder.h>

Q_DECLARE_METATYPE (QPersistentModelIndex);

namespace LeechCraft
{
namespace TPI
{
	namespace
	{
		class InfoModel : public QStandardItemModel
		{
		public:
			enum Roles
			{
				Done = Qt::UserRole + 1,
				Total,
				Name
			};

			InfoModel (QObject *parent)
			: QStandardItemModel (parent)
			{
				QHash<int, QByteArray> roleNames;
				roleNames [Roles::Done] = "jobDone";
				roleNames [Roles::Total] = "jobTotal";
				roleNames [Roles::Name] = "jobName";
				setRoleNames (roleNames);
			}
		};
	}

	InfoModelManager::InfoModelManager (ICoreProxy_ptr proxy, QObject *parent)
	: QObject (parent)
	, Proxy_ (proxy)
	, Model_ (new InfoModel (this))
	{
	}

	QAbstractItemModel* InfoModelManager::GetModel () const
	{
		return Model_;
	}

	void InfoModelManager::SecondInit ()
	{
		auto pm = Proxy_->GetPluginsManager ();
		for (auto ijh : pm->GetAllCastableTo<IJobHolder*> ())
			ManageModel (ijh->GetRepresentation ());
	}

	void InfoModelManager::ManageModel (QAbstractItemModel *model)
	{
		connect (model,
				SIGNAL (rowsInserted (QModelIndex, int, int)),
				this,
				SLOT (handleRowsInserted (QModelIndex, int, int)));
		connect (model,
				SIGNAL (rowsAboutToBeRemoved (QModelIndex, int, int)),
				this,
				SLOT (handleRowsRemoved (QModelIndex, int, int)));
		connect (model,
				SIGNAL (dataChanged (QModelIndex, QModelIndex)),
				this,
				SLOT (handleDataChanged (QModelIndex, QModelIndex)));

		if (auto numRows = model->rowCount ())
			HandleRows (model, 0, numRows - 1);
	}

	void InfoModelManager::HandleRows (QAbstractItemModel *model, int from, int to)
	{
		for (int i = from; i <= to; ++i)
		{
			const auto& idx = model->index (i, JobHolderColumn::JobProgress);
			const auto row = idx.data (CustomDataRoles::RoleJobHolderRow).value<JobHolderRow> ();
			if (row != JobHolderRow::DownloadProgress &&
					row != JobHolderRow::ProcessProgress)
				continue;

			if (idx.data (ProcessState::Done) == idx.data (ProcessState::Total))
				continue;

			auto ourItem = new QStandardItem;

			const auto& name = model->index (i, JobHolderColumn::JobName).data ().toString ();
			ourItem->setData (name, InfoModel::Name);

			PIdx2Item_ [idx] = ourItem;
			HandleData (model, i, i);

			Model_->appendRow (ourItem);
		}
	}

	void InfoModelManager::HandleData (QAbstractItemModel *model, int from, int to)
	{
		for (int i = from; i <= to; ++i)
		{
			const auto& idx = model->index (i, JobHolderColumn::JobProgress);

			auto done = idx.data (ProcessState::Done).toLongLong ();
			auto total = idx.data (ProcessState::Total).toLongLong ();

			auto item = PIdx2Item_.value (idx);
			if (!item)
			{
				if (done != total)
					HandleRows (model, i, i);
				continue;
			}

			if (done == total)
			{
				PIdx2Item_.remove (idx);
				Model_->removeRow (item->row ());
				continue;
			}

			while (done > 1000 && total > 1000)
			{
				done /= 10;
				total /= 10;
			}

			item->setData (static_cast<double> (done), InfoModel::Roles::Done);
			item->setData (static_cast<double> (total), InfoModel::Roles::Total);
		}
	}

	void InfoModelManager::handleRowsInserted (const QModelIndex& parent, int from, int to)
	{
		if (parent.isValid ())
			return;

		auto model = qobject_cast<QAbstractItemModel*> (sender ());
		HandleRows (model, from, to);
	}

	void InfoModelManager::handleRowsRemoved (const QModelIndex& parent, int from, int to)
	{
		if (parent.isValid ())
			return;

		auto model = qobject_cast<QAbstractItemModel*> (sender ());

		for (int i = from; i <= to; ++i)
		{
			const auto& idx = model->index (i, JobHolderColumn::JobProgress);
			auto item = PIdx2Item_.take (idx);
			if (!item)
				continue;

			Model_->removeRow (item->row ());
		}
	}

	void InfoModelManager::handleDataChanged (const QModelIndex& topLeft, const QModelIndex& bottomRight)
	{
		if (bottomRight.column () < JobHolderColumn::JobProgress)
			return;

		auto model = qobject_cast<QAbstractItemModel*> (sender ());
		HandleData (model, topLeft.row (), bottomRight.row ());
	}
}
}
