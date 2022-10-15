/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "userslistwidget.h"
#include <QSortFilterProxyModel>
#include <QStandardItemModel>
#include <util/gui/clearlineeditaddon.h>
#include "components/roster/keyboardrosterfixer.h"
#include "core.h"
#include "resourcesmanager.h"

namespace LC
{
namespace Azoth
{
	namespace
	{
		enum PartsListRoles
		{
			PLRObject = Qt::UserRole + 1
		};
	}

	UsersListWidget::UsersListWidget (const QList<QObject*>& parts,
			std::function<QString (ICLEntry*)> nameGetter, QWidget *parent)
	: QDialog (parent, Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint)
	, Filter_ (new QSortFilterProxyModel (this))
	, PartsModel_ (new QStandardItemModel (this))
	{
		Ui_.setupUi (this);

		for (auto part : parts)
		{
			auto entry = qobject_cast<ICLEntry*> (part);

			auto item = new QStandardItem (nameGetter (entry));
			item->setIcon (ResourcesManager::Instance ().GetIconForState (entry->GetStatus ().State_));
			item->setData (QVariant::fromValue (part), PLRObject);
			item->setEditable (false);

			PartsModel_->appendRow (item);
		}

		Filter_->setSourceModel (PartsModel_);
		Filter_->setSortCaseSensitivity (Qt::CaseInsensitive);
		Filter_->setFilterCaseSensitivity (Qt::CaseInsensitive);
		Filter_->setSortRole (Qt::DisplayRole);
		connect (Ui_.FilterLine_,
				SIGNAL (textChanged (QString)),
				Filter_,
				SLOT (setFilterFixedString (QString)));
		Ui_.ListView_->setModel (Filter_);
		Ui_.ListView_->sortByColumn (0, Qt::AscendingOrder);

		auto clear = new Util::ClearLineEditAddon (Core::Instance ().GetProxy (), Ui_.FilterLine_);
		clear->SetEscClearsEdit (false);

		connect (Ui_.ListView_,
				SIGNAL (activated (QModelIndex)),
				this,
				SLOT (accept ()));
		connect (Ui_.FilterLine_,
				SIGNAL (returnPressed ()),
				this,
				SLOT (accept ()));
		Ui_.ListView_->setCurrentIndex (Filter_->index (0, 0));

		Ui_.ListView_->setFocusProxy (Ui_.FilterLine_);
		Ui_.ListView_->setFocus ();

		auto fixer = new KeyboardRosterFixer (Ui_.FilterLine_, Ui_.ListView_, this);
		fixer->SetInterceptEnter (false);
	}

	QObject* UsersListWidget::GetActivatedParticipant () const
	{
		const auto& current = Ui_.ListView_->currentIndex ();
		if (current.isValid ())
			return current.data (PLRObject).value<QObject*> ();

		if (Filter_->rowCount ())
			return Filter_->index (0, 0).data (PLRObject).value<QObject*> ();

		return 0;
	}
}
}
