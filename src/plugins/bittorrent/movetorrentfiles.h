/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2015  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QDialog>
#include "ui_movetorrentfiles.h"

namespace LC::BitTorrent
{
	class MoveTorrentFiles : public QDialog
	{
		Q_DECLARE_TR_FUNCTIONS (LC::BitTorrent::MoveTorrentFiles)

		Ui::MoveTorrentFiles Ui_;
	public:
		explicit MoveTorrentFiles (QStringList oldDirectories, QWidget *parent = nullptr);

		QString GetNewLocation () const;
	};
}
