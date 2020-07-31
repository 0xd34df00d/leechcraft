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
#include "core.h"

class QSortFilterProxyModel;

namespace LC
{
namespace BitTorrent
{
	class AddTorrentFilesModel;

	class AddTorrent : public QDialog
	{
		Q_OBJECT

		Ui::AddTorrent Ui_;

		AddTorrentFilesModel * const FilesModel_;
		QSortFilterProxyModel * const ProxyModel_;
	public:
		AddTorrent (QWidget *parent = 0);
		void Reinit ();
		void SetFilename (const QString&);
		void SetSavePath (const QString&);
		QString GetFilename () const;
		QString GetSavePath () const;
		bool GetTryLive () const;
		QVector<bool> GetSelectedFiles () const;
		Core::AddType GetAddType () const;
		void SetTags (const QStringList& tags);
		QStringList GetTags () const;
		Util::TagsLineEdit* GetEdit ();
	private slots:
		void on_TorrentBrowse__released ();
		void on_DestinationBrowse__released ();

		void on_MarkAll__triggered ();
		void on_UnmarkAll__triggered ();
		void on_MarkSelected__triggered ();
		void on_UnmarkSelected__triggered ();
		void on_MarkExisting__triggered ();
		void on_MarkMissing__triggered ();

		void setOkEnabled ();
		void updateAvailableSpace ();
	private:
		template<typename T>
		void MarkExisting (T);

		void ParseBrowsed ();
		QPair<quint64, quint64> GetAvailableSpaceInDestination ();
	signals:
		void on_TorrentFile__textChanged ();
		void on_Destination__textChanged ();
	};
}
}
