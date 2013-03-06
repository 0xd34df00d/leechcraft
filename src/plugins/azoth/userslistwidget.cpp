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

#include "userslistwidget.h"
#include <QSortFilterProxyModel>
#include <QStandardItemModel>
#include <util/gui/clearlineeditaddon.h>
#include "core.h"
#include "keyboardrosterfixer.h"

namespace LeechCraft
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

	UsersListWidget::UsersListWidget (const QList<QObject*>& parts, QWidget *parent)
	: QDialog (parent,
			static_cast<Qt::WindowFlags> (Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint))
	, Filter_ (new QSortFilterProxyModel (this))
	, PartsModel_ (new QStandardItemModel (this))
	{
		Ui_.setupUi (this);

		for (auto part : parts)
		{
			auto entry = qobject_cast<ICLEntry*> (part);

			auto item = new QStandardItem (entry->GetEntryName ());
			item->setData (QVariant::fromValue (part), PLRObject);
			item->setEditable (false);

			PartsModel_->appendRow (item);
		}

		Filter_->setSourceModel (PartsModel_);
		Filter_->setSortCaseSensitivity (Qt::CaseInsensitive);
		Filter_->setSortRole (Qt::DisplayRole);
		connect (Ui_.FilterLine_,
				SIGNAL (textChanged (QString)),
				Filter_,
				SLOT (setFilterFixedString (QString)));
		Ui_.ListView_->setModel (Filter_);
		Ui_.ListView_->sortByColumn (0, Qt::AscendingOrder);

		new Util::ClearLineEditAddon (Core::Instance ().GetProxy (), Ui_.FilterLine_);

		connect (Ui_.ListView_,
				SIGNAL (activated (QModelIndex)),
				this,
				SLOT (accept ()));

		Ui_.ListView_->setFocusProxy (Ui_.FilterLine_);
		Ui_.ListView_->setFocus ();
		Ui_.FilterLine_->installEventFilter (new KeyboardRosterFixer (Ui_.ListView_, this));
	}

	QObject* UsersListWidget::GetActivatedParticipant () const
	{
		const auto& current = Ui_.ListView_->currentIndex ();
		if (!current.isValid ())
			return 0;

		return current.data (PLRObject).value<QObject*> ();
	}
}
}
