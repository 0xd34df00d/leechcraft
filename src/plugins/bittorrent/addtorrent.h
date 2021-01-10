/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QDialog>
#include <QVector>
#include "ui_addtorrent.h"
#include "types.h"

class QSortFilterProxyModel;

namespace LC::BitTorrent
{
	class AddTorrentFilesModel;

	class AddTorrent : public QDialog
	{

		Ui::AddTorrent Ui_;

		AddTorrentFilesModel * const FilesModel_;
		QSortFilterProxyModel * const ProxyModel_;
	public:
		explicit AddTorrent (QWidget *parent = nullptr);

		void SetFilename (const QString&);
		void SetSavePath (const QString&);
		QString GetFilename () const;
		QString GetSavePath () const;
		bool GetTryLive () const;
		QVector<bool> GetSelectedFiles () const;
		AddState GetAddType () const;
		void SetTags (const QStringList& tags);
		QStringList GetTags () const;
	private:
		void UpdateSpaceDisplay ();
		void BrowseForTorrent ();
		void BrowseForDestination ();
		void MarkExisting (Qt::CheckState ifExists, Qt::CheckState ifNotExists);
		void ParseBrowsed ();
	};
}
