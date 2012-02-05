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

#include "msgviewfactory.h"
#include <QWidget>
#include <QModelIndex>
#include <QLabel>
#include <QtDebug>
#include "proto/Imap/Model/ItemRoles.h"
#include "proto/Imap/Network/MsgPartNetAccessManager.h"
#include "msgviewwidgets.h"
#include "attachmentview.h"
#include "inlinepartwidget.h"

namespace LeechCraft
{
namespace Snails
{
	MsgViewFactory::MsgViewFactory (QObject *parent)
	: QObject (parent)
	, MPNAM_ (new Imap::Network::MsgPartNetAccessManager)
	{
	}

	QWidget* MsgViewFactory::Create (const QModelIndex& idx, int depth)
	{
		if (!idx.isValid ())
		{
			qWarning () << Q_FUNC_INFO
					<< "invalid index";
			return 0;
		}

		if (depth > 100)
		{
			qWarning () << Q_FUNC_INFO
					<< idx
					<< depth
					<< "~ depth level exceeded";
			return new QLabel (tr ("MIME message part depth level exceeded."));
		}

		auto creator = [this, depth] (const QModelIndex& i) { return Create (i, depth + 1); };

		const auto& type = idx.data (Imap::Mailbox::RolePartMimeType).toString ();
		if (type == "multipart/alternative")
			return new MultipartAlternativeWidget (idx, creator);
		else if (type == "multipart/signed")
			return new MultipartSignedWidget (idx, creator);
		else if (type.startsWith ("multipart/"))
			return new GenericMultipartWidget (idx, creator);
		else if (type == "message/rfc822")
			return new Message822Widget (idx, creator);
		else
		{
			QStringList allowedMimes;
			allowedMimes << "text/html"
					<< "text/plain"
					<< "image/jpeg"
					<< "image/jpg"
					<< "image/pjpeg"
					<< "image/png"
					<< "image/gif";
			if (!allowedMimes.contains (type) ||
					idx.data (Imap::Mailbox::RolePartBodyDisposition).toByteArray ().toLower () == "attachment")
				return new AttachmentView (idx, MPNAM_);
			else
			{
				MPNAM_->setModelMessage (idx);
				return new InlinePartWidget (idx, MPNAM_);
			}
		}

		return new QLabel (type);
	}
}
}
