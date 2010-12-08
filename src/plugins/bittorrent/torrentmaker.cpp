/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2010  Georg Rudoy
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

#include "torrentmaker.h"
#include <deque>
#include <boost/filesystem.hpp>
#include <QFile>
#include <QFileInfo>
#include <QProgressDialog>
#include <QMessageBox>
#include <QDir>
#include <QtDebug>
#include <QMainWindow>
#include <libtorrent/create_torrent.hpp>
#include "config.h"
#include "core.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace BitTorrent
		{
			namespace
			{
				bool FileFilter (const boost::filesystem::path& filename)
				{
					if (filename.leaf () [0] == '.')
						return false;
					QFileInfo fi (QString::fromUtf8 (filename.string ().c_str ()));
					if ((fi.isDir () ||
								fi.isFile () ||
								fi.isSymLink ()) &&
							fi.isReadable ())
						return true;
					return false;
				}

				void UpdateProgress (int i, QProgressDialog *pd)
				{
					pd->setValue (i);
				}
			}

			TorrentMaker::TorrentMaker (QObject *parent)
			: QObject (parent)
			{
			}

			void TorrentMaker::Start (NewTorrentParams params)
			{
				QString filename = params.Output_;
				if (!filename.endsWith (".torrent"))
					filename.append (".torrent");
				QFile file (filename);
				if (!file.open (QIODevice::WriteOnly | QIODevice::Truncate))
				{
					emit error (tr ("Could not open file %1 for write!").arg (filename));
					return;
				}

				boost::filesystem::path::default_name_check (boost::filesystem::no_check);

				libtorrent::file_storage fs;
				boost::filesystem::path fullPath =
					boost::filesystem::complete (params.Path_.toUtf8 ().constData ());
				libtorrent::add_files (fs, fullPath, FileFilter);
				libtorrent::create_torrent ct (fs, params.PieceSize_);

				ct.set_creator (qPrintable (QString ("LeechCraft BitTorrent %1")
							.arg (Core::Instance ()->GetProxy ()->GetVersion ())));
				if (!params.Comment_.isEmpty ())
					ct.set_comment (params.Comment_.toUtf8 ());
				for (int i = 0; i < params.URLSeeds_.size (); ++i)
					ct.add_url_seed (params.URLSeeds_.at (0).toStdString ());
				ct.set_priv (!params.DHTEnabled_);

				if (params.DHTEnabled_)
					for (int i = 0; i < params.DHTNodes_.size (); ++i)
					{
						QStringList splitted = params.DHTNodes_.at (i).split (":");
						ct.add_node (std::pair<std::string, int> (splitted [0].trimmed ().toStdString (),
									splitted [1].trimmed ().toInt ()));
					}

				ct.add_tracker (params.AnnounceURL_.toStdString ());

				std::auto_ptr<QProgressDialog> pd (new QProgressDialog ());
				pd->setWindowTitle (tr ("Hashing torrent..."));
				pd->setMaximum (ct.num_pieces ());

				boost::system::error_code hashesError;
				libtorrent::set_piece_hashes (ct, fullPath.branch_path (),
						boost::bind (&UpdateProgress, _1, pd.get ()), hashesError);
				if (hashesError)
				{
					QString message = QString::fromUtf8 (hashesError.message ().c_str ());
					libtorrent::file_entry entry = fs.at (hashesError.value ());
					QString fn = QString::fromUtf8 (entry.path.string ().c_str ());
					qWarning () << Q_FUNC_INFO
						<< "while in libtorrent::set_piece_hashes():"
						<< message
						<< hashesError.category ().name ()
						<< fn;
					emit error (tr ("Torrent creation failed: %1")
							.arg (message));
					return;
				}

				libtorrent::entry e = ct.generate ();
				std::deque<char> outbuf;
				libtorrent::bencode (std::back_inserter (outbuf), e);

				for (size_t i = 0; i < outbuf.size (); ++i)
					file.write (&outbuf.at (i), 1);
				file.close ();

				if (QMessageBox::question (Core::Instance ()->GetProxy ()->GetMainWindow (),
							"LeechCraft",
							tr ("Torrent file generated: %1.<br />Do you want to start seeding now?")
								.arg (QDir::toNativeSeparators (filename)),
							QMessageBox::Yes | QMessageBox::No) ==
						QMessageBox::Yes)
					Core::Instance ()->AddFile (filename,
							QString::fromUtf8 (fullPath.branch_path ().string ().c_str ()),
							QStringList (),
							false);
			}
		};
	};
};

