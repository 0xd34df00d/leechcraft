/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "playlistundocommand.h"
#include "player.h"

namespace LC
{
namespace LMP
{
	PlaylistUndoCommand::PlaylistUndoCommand (const QString& title,
			const QList<AudioSource>& sources, Player *player)
	: QUndoCommand (title)
	, Player_ (player)
	, Sources_ (sources)
	{
	}

	void PlaylistUndoCommand::undo ()
	{
		Player_->Enqueue (Sources_);
	}

	void PlaylistUndoCommand::redo ()
	{
		Player_->Dequeue (Sources_);
	}
}
}
