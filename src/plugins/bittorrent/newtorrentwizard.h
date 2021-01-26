/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QWizard>
#include <QCoreApplication>

namespace LC::BitTorrent
{
	struct NewTorrentParams;

	class NewTorrentWizard : public QWizard
	{
		Q_DECLARE_TR_FUNCTIONS (LC::BitTorrent::NewTorrentWizard)
	public:
		explicit NewTorrentWizard (QWidget *parent = nullptr);

		NewTorrentParams GetParams () const;
	};
}
