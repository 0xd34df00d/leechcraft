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

#include "contextwrapper.h"
#include <QSortFilterProxyModel>
#include <QSettings>
#include <QStringList>
#include <QCoreApplication>
#include <QtDebug>
#include <util/qml/unhidelistviewbase.h>
#include <util/qml/unhidelistmodel.h>
#include "plotmanager.h"
#include "sensorsgraphmodel.h"

namespace LeechCraft
{
namespace HotSensors
{
	class SensorsFilterModel : public QSortFilterProxyModel
	{
		QStringList Hidden_;
	public:
		SensorsFilterModel (QObject *parent)
		: QSortFilterProxyModel (parent)
		{
		}

		void SetSourceWithRoles (QAbstractItemModel *model)
		{
			setSourceModel (model);
			setRoleNames (model->roleNames ());
		}

		void SetHidden (const QStringList& hidden)
		{
			Hidden_ = hidden;
			invalidateFilter ();
		}

	protected:
		bool filterAcceptsRow (int row, const QModelIndex&) const
		{
			const auto& idx = sourceModel ()->index (row, 0);
			return !Hidden_.contains (idx.data (SensorsGraphModel::Role::SensorName).toString ());
		}
	};

	ContextWrapper::ContextWrapper (PlotManager *manager, ICoreProxy_ptr proxy)
	: QObject (manager)
	, Proxy_ (proxy)
	, Filter_ (new SensorsFilterModel (this))
	{
		Filter_->setDynamicSortFilter (true);
		Filter_->SetSourceWithRoles (manager->GetModel ());
	}

	QStringList ContextWrapper::LoadHiddenNames () const
	{
		QSettings settings (qApp->organizationName (), qApp->applicationName () + "_HotSensors");
		settings.beginGroup (Context_);
		const auto& list = settings.value ("Hidden").toStringList ();
		settings.endGroup ();
		return list;
	}

	void ContextWrapper::SaveHiddenNames (const QStringList& names) const
	{
		QSettings settings (qApp->organizationName (), qApp->applicationName () + "_HotSensors");
		settings.beginGroup (Context_);
		settings.setValue ("Hidden", names);
		settings.endGroup ();
	}

	void ContextWrapper::setContext (const QString& context)
	{
		Context_ = context;

		const auto& list = LoadHiddenNames ();
		Filter_->SetHidden (list);
	}

	QObject* ContextWrapper::getModel () const
	{
		return Filter_;
	}

	void ContextWrapper::sensorUnhideListRequested (int x, int y)
	{
		QList<QStandardItem*> items;
		for (const auto& name : LoadHiddenNames ())
		{
			auto item = new QStandardItem;
			item->setData (name, Util::UnhideListModel::ItemClass);
			item->setData (name, Util::UnhideListModel::ItemName);
			item->setData (QIcon (), Util::UnhideListModel::ItemIcon);
			item->setData (QString (), Util::UnhideListModel::ItemDescription);
			items << item;
		}
		if (items.isEmpty ())
			return;

		auto list = new Util::UnhideListViewBase (Proxy_);
		list->SetItems (items);
		connect (list,
				SIGNAL (itemUnhideRequested (QString)),
				this,
				SLOT (unhideSensor (QString)));
		list->show ();
	}

	void ContextWrapper::unhideSensor (const QString& name)
	{
		auto list = LoadHiddenNames ();
		if (!list.removeAll (name))
			return;

		SaveHiddenNames (list);
		Filter_->SetHidden (list);
	}

	void ContextWrapper::hideSensor (const QString& name)
	{
		auto list = LoadHiddenNames ();
		list << name;
		list.removeDuplicates ();
		SaveHiddenNames (list);

		Filter_->SetHidden (list);
	}
}
}
