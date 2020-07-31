/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "torrentmaker.h"
#include <deque>
#include <QFile>
#include <QFileInfo>
#include <QProgressDialog>
#include <QMessageBox>
#include <QDir>
#include <QtDebug>
#include <QMainWindow>
#include <libtorrent/create_torrent.hpp>
#include <util/xpc/util.h>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/irootwindowsmanager.h>
#include <interfaces/core/ientitymanager.h>
#include "core.h"

namespace LC
{
namespace BitTorrent
{
	namespace
	{
		bool FileFilter (const std::string& pathStr)
		{
			QFileInfo fi { QString::fromStdString (pathStr) };
			return fi.isReadable () && !fi.isHidden () &&
					(fi.isDir () || fi.isFile () || fi.isSymLink ());
		}
	}

	TorrentMaker::TorrentMaker (const ICoreProxy_ptr& proxy, QObject *parent)
	: QObject { parent }
	, Proxy_ { proxy }
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
			ReportError (tr ("Could not open file %1 for write!").arg (filename));
			return;
		}

		libtorrent::file_storage fs;
		const auto& fullPath = std::string (params.Path_.toUtf8 ().constData ());
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

		QProgressDialog pd;
		pd.setWindowTitle (tr ("Hashing torrent..."));
		pd.setMaximum (ct.num_pieces ());

		libtorrent::error_code hashesError;
		libtorrent::set_piece_hashes (ct,
				fullPath,
				[&pd] (int i) { pd.setValue (i); },
				hashesError);
		if (hashesError)
		{
			QString message = QString::fromUtf8 (hashesError.message ().c_str ());
			qWarning () << Q_FUNC_INFO
					<< "while in libtorrent::set_piece_hashes():"
					<< message
					<< hashesError.category ().name ();
			ReportError (tr ("Torrent creation failed: %1")
					.arg (message));
			return;
		}

		libtorrent::entry e = ct.generate ();
		std::deque<char> outbuf;
		libtorrent::bencode (std::back_inserter (outbuf), e);

		for (size_t i = 0; i < outbuf.size (); ++i)
			file.write (&outbuf.at (i), 1);
		file.close ();

		auto rootWM = Core::Instance ()->GetProxy ()->GetRootWindowsManager ();
		if (QMessageBox::question (rootWM->GetPreferredWindow (),
					"LeechCraft",
					tr ("Torrent file generated: %1.<br />Do you want to start seeding now?")
						.arg (QDir::toNativeSeparators (filename)),
					QMessageBox::Yes | QMessageBox::No) ==
				QMessageBox::Yes)
			Core::Instance ()->AddFile (filename,
					QString::fromUtf8 (fullPath.c_str ()),
					QStringList (),
					false);
	}

	void TorrentMaker::ReportError (const QString& error)
	{
		const auto& entity = Util::MakeNotification ("BitTorrent", error, Priority::Critical);
		Proxy_->GetEntityManager ()->HandleEntity (entity);
	}
}
}
