/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2012  Georg Rudoy
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

#include "msgviewwidgets.h"
#include <QModelIndex>
#include <QVBoxLayout>
#include "proto/Imap/Model/ItemRoles.h"
#include "proto/Imap/Model/MailboxTree.h"

namespace LeechCraft
{
namespace Snails
{
	MultipartAlternativeWidget::MultipartAlternativeWidget (const QModelIndex& idx,
			WidgetCreator_f creator, QWidget *parent)
	: QTabWidget (parent)
	{
		for (int i = 0, size = idx.model ()->rowCount (idx); i < size; ++i)
		{
			const auto& part = idx.child (i, 0);
			auto itemWidget = creator (part);
			if (itemWidget)
				addTab (itemWidget, part.data (Imap::Mailbox::RolePartMimeType).toString ());
		}
	}

	MultipartSignedWidget::MultipartSignedWidget (const QModelIndex& idx,
			WidgetCreator_f creator, QWidget *parent)
	: QGroupBox (parent)
	{
		QVBoxLayout *lay = new QVBoxLayout (this);

		const int childrenCount = idx.model ()->rowCount (idx);
		if (childrenCount != 2)
			setTitle (tr ("Malformed multipart/signed message: %n parts.", 0, childrenCount));

		if (childrenCount)
		{
			auto w = creator (idx.child (0, 0));
			if (w)
				lay->addWidget (w);
		}
	}

	GenericMultipartWidget::GenericMultipartWidget (const QModelIndex& idx,
			WidgetCreator_f creator, QWidget *parent)
	: QGroupBox (parent)
	{
		QVBoxLayout *lay = new QVBoxLayout (this);

		for (int i = 0, size = idx.model ()->rowCount (idx); i < size; ++i)
		{
			auto w = creator (idx.child (i, 0));
			if (w)
				lay->addWidget (w);
		}
	}

	Message822Widget::Message822Widget (const QModelIndex& idx,
			WidgetCreator_f creator, QWidget *parent)
	: QGroupBox (parent)
	{
		QVBoxLayout *lay = new QVBoxLayout (this);

		const auto& hdrIdx = idx.child (0, Imap::Mailbox::TreeItem::OFFSET_HEADER);
		//TODO
		//lay->addWidget (new RFC822HeaderView (hdrIdx));
		for (int i = 0, size = idx.model ()->rowCount (idx); i < size; ++i)
		{
			auto w = creator (idx.child (i, 0));
			if (w)
				lay->addWidget (w);
		}
	}
}
}
