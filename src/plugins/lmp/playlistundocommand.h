/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QUndoCommand>
#include "engine/audiosource.h"

namespace LC
{
namespace LMP
{
	class Player;

	class PlaylistUndoCommand : public QUndoCommand
	{
		Player *Player_;
		QList<AudioSource> Sources_;
	public:
		PlaylistUndoCommand (const QString&, const QList<AudioSource>&, Player*);

		void undo ();
		void redo ();
	};
}
}
