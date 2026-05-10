/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <optional>
#include <QMap>
#include <QObject>
#include "../azothutilconfig.h"

namespace LC::Azoth
{
	class ICLEntry;

	using MUCPerms_t = QMap<QByteArray, QList<QByteArray>>;
}

namespace LC::Azoth::MucEvents
{
	struct ParticipantPermsChange
	{
		ICLEntry& Participant_;
		QString Reason_;
		ICLEntry *Actor_ = nullptr;

		std::optional<MUCPerms_t> PrevPerms_;
	};
}

namespace LC::Azoth::Emitters
{
	class AZOTH_UTIL_API MUCPerms : public QObject
	{
		Q_OBJECT
	public:
		using QObject::QObject;
	signals:
		void permsChanged (const MucEvents::ParticipantPermsChange& change);
	};
}
