/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <variant>
#include <functional>
#include <stdexcept>
#include <QStringList>
#include <QtPlugin>

namespace LC
{
namespace Azoth
{
	class ICLEntry;

	struct StringCommandResult
	{
		bool StopProcessing_;
		QString Message_;
	};
	struct TextMorphResult
	{
		QString NewText_;
	};
	typedef std::variant<bool, StringCommandResult, TextMorphResult> CommandResult_t;

	typedef std::function<CommandResult_t (ICLEntry*, QString&)> Command_f;

	class CommandException : public std::runtime_error
	{
		const QString Error_;
		const bool TryOtherCommands_;
	public:
		CommandException (const QString& error, bool canTryOthers = false)
		: std::runtime_error { error.toUtf8 ().constData () }
		, Error_ { error }
		, TryOtherCommands_ { canTryOthers }
		{
		}

		const QString& GetError () const
		{
			return Error_;
		}

		bool CanTryOtherCommands () const
		{
			return TryOtherCommands_;
		}
	};

	struct StaticCommand
	{
		QStringList Names_;
		Command_f Command_;

		QString Description_;
		QString Help_;

		StaticCommand () = default;
		StaticCommand (const StaticCommand&) = default;

		StaticCommand (const QStringList& names, const Command_f& command)
		: Names_ { names }
		, Command_ { command }
		{
		}

		StaticCommand (const QStringList& names, const Command_f& command,
				const QString& descr, const QString& help)
		: Names_ { names }
		, Command_ { command }
		, Description_ { descr }
		, Help_ { help }
		{
		}
	};

	typedef QList<StaticCommand> StaticCommands_t;

	class IProvideCommands
	{
	public:
		virtual ~IProvideCommands () {}

		virtual StaticCommands_t GetStaticCommands (ICLEntry*) = 0;
	};
}
}

Q_DECLARE_INTERFACE (LC::Azoth::IProvideCommands, "org.LeechCraft.Azoth.IProvideCommands/1.0")
