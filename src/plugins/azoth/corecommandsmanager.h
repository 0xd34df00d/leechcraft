/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <interfaces/azoth/iprovidecommands.h>

namespace LC
{
namespace Azoth
{
	class CoreCommandsManager : public QObject
							  , public IProvideCommands
	{
		Q_OBJECT
		Q_INTERFACES (LC::Azoth::IProvideCommands)

		const StaticCommand Help_;
		const StaticCommand Clear_;
	public:
		CoreCommandsManager (QObject *parent = nullptr);

		StaticCommands_t GetStaticCommands (ICLEntry*);
	};
}
}
