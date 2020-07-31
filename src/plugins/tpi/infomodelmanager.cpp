/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "infomodelmanager.h"
#include <QStandardItemModel>
#include <QUrl>
#include <util/models/rolenamesmixin.h>
#include <interfaces/core/ipluginsmanager.h>
#include <interfaces/ijobholder.h>

namespace LC
{
namespace TPI
{
	namespace
	{
		class InfoModel : public Util::RoleNamesMixin<QStandardItemModel>
		{
		public:
			enum Roles
			{
				Done = Qt::UserRole + 1,
				Total,
				Name,
				StateIcon
			};

			InfoModel (QObject *parent)
			: RoleNamesMixin<QStandardItemModel> (parent)
			{
				QHash<int, QByteArray> roleNames;
				roleNames [Roles::Done] = "jobDone";
				roleNames [Roles::Total] = "jobTotal";
				roleNames [Roles::Name] = "jobName";
				roleNames [Roles::StateIcon] = "stateIcon";
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

	namespace
	{
		bool IsInternal (const QModelIndex& idx)
		{
			const auto flags = idx.data (JobHolderRole::ProcessState).value<ProcessStateInfo> ().Params_;
			return flags & TaskParameter::Internal;
		}
	}

	void InfoModelManager::HandleRows (QAbstractItemModel *model, int from, int to)
	{
		for (int i = from; i <= to; ++i)
		{
			const auto& idx = model->index (i, JobHolderColumn::JobProgress);
			if (IsInternal (idx))
				continue;

			const auto row = idx.data (CustomDataRoles::RoleJobHolderRow).value<JobHolderRow> ();
			if (row != JobHolderRow::DownloadProgress &&
					row != JobHolderRow::ProcessProgress)
				continue;

			const auto& state = idx.data (JobHolderRole::ProcessState).value<ProcessStateInfo> ();

			if (state.Done_ == state.Total_)
				continue;

			auto ourItem = new QStandardItem;

			const auto& name = model->index (i, JobHolderColumn::JobName).data ().toString ();
			ourItem->setData (name, InfoModel::Name);

			PIdx2Item_ [idx] = ourItem;
			HandleData (model, i, i);

			Model_->appendRow (ourItem);
		}
	}

	namespace
	{
		QUrl GetIconUrl (ProcessStateInfo::State state)
		{
			switch (state)
			{
			case ProcessStateInfo::State::Unknown:
				return QUrl { "image://ThemeIcons/dialog-information" };
			case ProcessStateInfo::State::Running:
				return QUrl { "image://ThemeIcons/media-playback-start" };
			case ProcessStateInfo::State::Paused:
				return QUrl { "image://ThemeIcons/media-playback-pause" };
			case ProcessStateInfo::State::Error:
				return QUrl { "image://ThemeIcons/dialog-error" };
			}

			qWarning () << Q_FUNC_INFO
					<< "unknown process info state"
					<< static_cast<int> (state);

			return QUrl { "image://ThemeIcons/dialog-information" };
		}
	}

	void InfoModelManager::HandleData (QAbstractItemModel *model, int from, int to)
	{
		for (int i = from; i <= to; ++i)
		{
			const auto& idx = model->index (i, JobHolderColumn::JobProgress);
			if (IsInternal (idx))
				continue;

			const auto& state = idx.data (JobHolderRole::ProcessState).value<ProcessStateInfo> ();
			auto done = state.Done_;
			auto total = state.Total_;

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
			item->setData (model->index (i, JobHolderColumn::JobName).data ().toString (),
					InfoModel::Roles::Name);
			item->setData (GetIconUrl (state.State_), InfoModel::Roles::StateIcon);
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
