/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <cstdint>
#include <optional>
#include <variant>
#include <QMap>
#include <QString>

namespace LC::Azoth
{
	class ICLEntry;

	using MUCPerms_t = QMap<QByteArray, QList<QByteArray>>;
}

namespace LC::Azoth::MucEvents
{
	enum class Liveness : std::uint8_t
	{
		Historical,
		Live,
	};

	struct ParticipantLeft { QString Message_; };

	struct ParticipantForcedOut
	{
		ICLEntry *Actor_ = nullptr;
		QString Reason_;

		enum class Action : std::uint8_t
		{
			Kicked,
			Banned,
		} Action_;
	};

	using ParticipantLeaveInfo = std::variant<ParticipantLeft, ParticipantForcedOut>;

	struct SubjectChange
	{
		QString Subject_;
		std::optional<QString> ActorNick_ {};
		std::optional<Liveness> Liveness_ {};
	};

	struct ParticipantPermsChange
	{
		ICLEntry& Participant_;
		QString Reason_;
		ICLEntry *Actor_ = nullptr;

		std::optional<MUCPerms_t> PrevPerms_;
	};
}
