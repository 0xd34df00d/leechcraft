/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QDateTime>
#include <QHash>
#include "address.h"
#include "attdescr.h"

namespace LC::Snails
{
	struct MessageInfo
	{
		bool IsRead_;

		QByteArray MessageId_;
		QByteArray FolderId_;
		QStringList Folder_;

		QString Subject_;

		QDateTime Date_;
		quint64 Size_;

		QHash<AddressType, Addresses_t> Addresses_;

		QList<QByteArray> References_;
		QList<QByteArray> InReplyTo_;

		QList<AttDescr> Attachments_;
	};
}

Q_DECLARE_METATYPE (LC::Snails::MessageInfo)
