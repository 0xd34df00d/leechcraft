/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2009  Georg Rudoy
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

#ifndef PLUGINS_BITTORRENT_ADDTORRENT_H
#define PLUGINS_BITTORRENT_ADDTORRENT_H
#include <QDialog>
#include <QVector>
#include "ui_addtorrent.h"
#include "core.h"

namespace LeechCraft
{
	namespace Util
	{
		class TagsLineEdit;
	};

	namespace Plugins
	{
		namespace BitTorrent
		{
			class TorrentFilesModel;

			class AddTorrent : public QDialog, private Ui::AddTorrent
			{
				Q_OBJECT

				TorrentFilesModel *FilesModel_;
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
				/** Sets tags for the newly added torrent.
				 *
				 * @param[in] tags List of IDs of tags.
				 */
				void SetTags (const QStringList& tags);
				/** Returns the list of tags after the dialog is executed.
				 *
				 * @return List if IDs of tags.
				 */
				QStringList GetTags () const;
				Util::TagsLineEdit* GetEdit ();
			private slots:
				void on_TorrentBrowse__released ();
				void on_DestinationBrowse__released ();
				void on_MarkAll__released ();
				void on_UnmarkAll__released ();
				void on_MarkSelected__released ();
				void on_UnmarkSelected__released ();
				void setOkEnabled ();
			private:
				void ParseBrowsed ();
			signals:
				void on_TorrentFile__textChanged ();
				void on_Destination__textChanged ();
			};
		};
	};
};

#endif

