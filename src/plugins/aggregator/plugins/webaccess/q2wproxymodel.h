/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <QMap>
#include <Wt/WAbstractItemModel.h>
#include <util/models/modelitem.h>
#include "serverupdater.h"

class QAbstractItemModel;
class QModelIndex;

namespace LC
{
namespace Aggregator
{
namespace WebAccess
{
	class Q2WProxyModel : public QObject
						, public Wt::WAbstractItemModel
	{
		Q_OBJECT

		QAbstractItemModel& Src_;
		Util::ModelItem_ptr Root_;

		QMap<int, int> Mapping_;

		Wt::WApplication * const App_;
		ServerUpdater Update_;

		int LastModelResetRC_ = 0;
	public:
		using Morphism_t = std::function<Wt::cpp17::any (QModelIndex, Wt::ItemDataRole)>;
	private:
		QList<Morphism_t> Morphisms_;
	public:
		Q2WProxyModel (QAbstractItemModel&, Wt::WApplication*);

		void SetRoleMappings (const QMap<int, int>&);
		void AddDataMorphism (const Morphism_t&);

		QModelIndex MapToSource (const Wt::WModelIndex&) const;

		int columnCount (const Wt::WModelIndex& parent) const override;
		int rowCount (const Wt::WModelIndex& parent) const override;
		Wt::WModelIndex parent (const Wt::WModelIndex& index) const override;
		Wt::cpp17::any data (const Wt::WModelIndex& index, Wt::ItemDataRole role) const override;
		Wt::WModelIndex index (int row, int column, const Wt::WModelIndex& parent) const override;
		Wt::cpp17::any headerData (int section, Wt::Orientation orientation, Wt::ItemDataRole role) const override;

		void* toRawIndex (const Wt::WModelIndex& index) const override;
		Wt::WModelIndex fromRawIndex (void* rawIndex) const override;
	private:
		int WtRole2Qt (Wt::ItemDataRole) const;
		QModelIndex W2QIdx (const Wt::WModelIndex&) const;
		Wt::WModelIndex Q2WIdx (const QModelIndex&) const;
	private Q_SLOTS:
		void handleDataChanged (const QModelIndex&, const QModelIndex&);

		void handleRowsAboutToBeInserted (const QModelIndex&, int, int);
		void handleRowsInserted (const QModelIndex&, int, int);

		void handleRowsAboutToBeRemoved (const QModelIndex&, int, int);
		void handleRowsRemoved (const QModelIndex&, int, int);

		void handleModelAboutToBeReset ();
		void handleModelReset ();
	};
}
}
}
