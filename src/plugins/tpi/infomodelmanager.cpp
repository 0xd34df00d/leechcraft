/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "infomodelmanager.h"
#include <QConcatenateTablesProxyModel>
#include <QSortFilterProxyModel>
#include <QIdentityProxyModel>
#include <QUrl>
#include <util/models/rolenamesmixin.h>
#include <util/sll/visitor.h>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/ipluginsmanager.h>
#include <interfaces/ijobholder.h>

namespace LC
{
namespace TPI
{
	namespace
	{
		bool IsInternal (const ProcessInfo& info)
		{
			return info.Parameters_ & TaskParameter::Internal;
		}

		class FilterModel : public QSortFilterProxyModel
		{
		public:
			using QSortFilterProxyModel::QSortFilterProxyModel;
		protected:
			bool filterAcceptsRow (int row, const QModelIndex& parent) const override
			{
				if (parent.isValid ())
					return false;

				const auto& rowInfo = sourceModel ()->index (row, 0).data (+JobHolderRole::RowInfo).value<RowInfo> ();
				return Util::Visit (rowInfo.Specific_,
						[] (const ProcessInfo& info) { return !IsInternal (info); },
						[] (const NewsInfo&) { return false; });
			}
		};

		QUrl GetIconUrl (ProcessState state)
		{
			switch (state)
			{
			case ProcessState::Unknown:
				return QUrl { "image://ThemeIcons/dialog-information" };
			case ProcessState::Running:
				return QUrl { "image://ThemeIcons/media-playback-start" };
			case ProcessState::Paused:
				return QUrl { "image://ThemeIcons/media-playback-pause" };
			case ProcessState::Finished:
				return QUrl { "image://ThemeIcons/task-complete" };
			case ProcessState::Error:
				return QUrl { "image://ThemeIcons/dialog-error" };
			}

			qWarning () << "unknown process info state" << static_cast<int> (state);

			return QUrl { "image://ThemeIcons/dialog-information" };
		}

		class InfoModel : public Util::RoleNamesMixin<QIdentityProxyModel>
		{
		public:
			enum Roles
			{
				Done = Qt::UserRole + 1,
				Total,
				Name,
				StateIcon
			};

			explicit InfoModel (QObject *parent)
			: RoleNamesMixin (parent)
			{
				QHash<int, QByteArray> roleNames;
				roleNames [Roles::Done] = "jobDone";
				roleNames [Roles::Total] = "jobTotal";
				roleNames [Roles::Name] = "jobName";
				roleNames [Roles::StateIcon] = "stateIcon";
				setRoleNames (roleNames);
			}

			QVariant data (const QModelIndex& index, int role) const override
			{
				const auto& srcIdx = mapToSource (index);
				switch (role)
				{
				case Done:
					return srcIdx.data (+JobHolderProcessRole::Done);
				case Total:
					return srcIdx.data (+JobHolderProcessRole::Total);
				case Name:
					return srcIdx.data (Qt::DisplayRole);
				case StateIcon:
					return GetIconUrl (srcIdx.data (+JobHolderProcessRole::State).value<ProcessState> ());
				}

				return srcIdx.data (role);
			}
		};
	}

	InfoModelManager::InfoModelManager (QObject *parent)
	: QObject { parent }
	, Concat_ { *new QConcatenateTablesProxyModel { this } }
	, Filter_ { *new FilterModel { this } }
	, Structurize_ { *new InfoModel { this } }
	{
		Filter_.setSourceModel (&Concat_);
		Structurize_.setSourceModel (&Filter_);
	}

	QAbstractItemModel* InfoModelManager::GetModel () const
	{
		return &Structurize_;
	}

	void InfoModelManager::SecondInit ()
	{
		for (const auto ijh : GetProxyHolder ()->GetPluginsManager ()->GetAllCastableTo<IJobHolder*> ())
			Concat_.addSourceModel (ijh->GetRepresentation ());
	}
}
}
