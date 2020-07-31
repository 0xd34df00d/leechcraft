/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "mucjoinwidget.h"
#include <QStandardItemModel>
#include <QSortFilterProxyModel>
#include "vkaccount.h"
#include "vkentry.h"
#include <util/gui/clearlineeditaddon.h>

namespace LC
{
namespace Azoth
{
namespace Murm
{
	namespace
	{
		enum Role
		{
			EntryObj = Qt::UserRole + 1
		};

		class UsersSortFilterProxy : public QSortFilterProxyModel
		{
		public:
			UsersSortFilterProxy (QObject *parent)
			: QSortFilterProxyModel (parent)
			{
				setDynamicSortFilter (true);
				setSortCaseSensitivity (Qt::CaseInsensitive);
			}
		};
	}

	MucJoinWidget::MucJoinWidget (ICoreProxy_ptr proxy)
	: UsersModel_ (new QStandardItemModel (this))
	, UsersFilter_ (new UsersSortFilterProxy (this))
	{
		UsersFilter_->setSourceModel (UsersModel_);

		Ui_.setupUi (this);
		Ui_.UsersTree_->setModel (UsersFilter_);

		new Util::ClearLineEditAddon (proxy, Ui_.UsersFilter_);
		connect (Ui_.UsersFilter_,
				SIGNAL (textChanged (QString)),
				UsersFilter_,
				SLOT (setFilterFixedString (QString)));
	}

	void MucJoinWidget::AccountSelected (QObject *accObj)
	{
		UsersModel_->clear ();

		auto acc = qobject_cast<VkAccount*> (accObj);
		for (auto entryObj : acc->GetCLEntries ())
		{
			auto entry = qobject_cast<VkEntry*> (entryObj);
			if (!entry)
				continue;

			auto item = new QStandardItem (entry->GetEntryName ());
			item->setEditable (false);
			item->setData (QVariant::fromValue (entryObj), Role::EntryObj);
			item->setCheckable (true);
			item->setCheckState (Qt::Unchecked);
			UsersModel_->appendRow (item);
		}
	}

	void MucJoinWidget::Join (QObject *accObj)
	{
		auto acc = qobject_cast<VkAccount*> (accObj);

		acc->CreateChat (Ui_.ChatName_->text (), GetSelectedEntries ());
	}

	void MucJoinWidget::Cancel ()
	{
	}

	QVariantMap MucJoinWidget::GetIdentifyingData () const
	{
		return {};
	}

	void MucJoinWidget::SetIdentifyingData (const QVariantMap&)
	{
	}

	QList<VkEntry*> MucJoinWidget::GetSelectedEntries () const
	{
		QList<VkEntry*> result;
		for (int i = 0; i < UsersModel_->rowCount (); ++i)
		{
			const auto item = UsersModel_->item (i);
			if (item->checkState () != Qt::Checked)
				continue;

			result << qobject_cast<VkEntry*> (item->data (Role::EntryObj).value<QObject*> ());
		}
		return result;
	}
}
}
}
