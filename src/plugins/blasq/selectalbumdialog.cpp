/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "selectalbumdialog.h"
#include <QSortFilterProxyModel>
#include "interfaces/blasq/collection.h"
#include "interfaces/blasq/iaccount.h"
#include "interfaces/blasq/isupportuploads.h"

namespace LC
{
namespace Blasq
{
	class CollectionsFilterModel : public QSortFilterProxyModel
	{
	public:
		CollectionsFilterModel (QObject *parent)
		: QSortFilterProxyModel (parent)
		{
			setDynamicSortFilter (true);
		}
	protected:
		bool filterAcceptsRow (int row, const QModelIndex& sourceParent) const
		{
			const auto& idx = sourceModel ()->index (row, 0, sourceParent);
			return idx.data (CollectionRole::Type).toInt () == ItemType::Collection;
		}
	};

	SelectAlbumDialog::SelectAlbumDialog (IAccount *acc, QWidget *parent)
	: QDialog (parent)
	, Filter_ (new CollectionsFilterModel (this))
	, Acc_ (acc)
	{
		Filter_->setSourceModel (acc->GetCollectionsModel ());

		Ui_.setupUi (this);
		Ui_.View_->setModel (Filter_);
	}

	QModelIndex SelectAlbumDialog::GetSelectedCollection () const
	{
		return Filter_->mapToSource (Ui_.View_->currentIndex ());
	}

	void SelectAlbumDialog::on_AddButton__released ()
	{
		auto isu = qobject_cast<ISupportUploads*> (Acc_->GetQObject ());
		isu->CreateCollection (Ui_.View_->currentIndex ());
	}
}
}
