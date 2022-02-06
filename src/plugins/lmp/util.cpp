/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "util.h"
#include <algorithm>
#include <atomic>
#include <QDirIterator>
#include <QPixmap>
#include <QApplication>
#include <QMessageBox>
#include <QFileInfo>
#include <util/util.h>
#include <util/gui/util.h>
#include <util/sll/prelude.h>
#include <util/sll/slotclosure.h>
#include <util/xpc/util.h>
#include <util/lmp/util.h>
#include <interfaces/core/ientitymanager.h>
#include "core.h"
#include "localcollection.h"
#include "xmlsettingsmanager.h"
#include "radiotracksgrabdialog.h"

namespace LC::LMP
{
	QList<QFileInfo> RecIterateInfo (const QString& dirPath, bool followSymlinks, std::atomic<bool> *stopFlag)
	{
		static const QStringList nameFilters
		{
			"*.aiff",
			"*.ape",
			"*.asf",
			"*.flac",
			"*.m4a",
			"*.mp3",
			"*.mp4",
			"*.mpc",
			"*.mpeg",
			"*.mpg",
			"*.ogg",
			"*.tta",
			"*.wav",
			"*.wma",
			"*.wv",
			"*.wvp"
		};

		const QFileInfo dirInfo (dirPath);
		if (dirInfo.isFile ())
		{
			for (const auto& filter : nameFilters)
				if (dirPath.endsWith (filter.mid (1), Qt::CaseInsensitive))
					return { dirInfo };

			return {};
		}

		auto filters = QDir::AllDirs | QDir::Files | QDir::NoDotAndDotDot;
		if (!followSymlinks)
			filters |= QDir::NoSymLinks;

		QList<QFileInfo> result;
		const auto& list = QDir (dirPath).entryInfoList (nameFilters, filters);
		for (const auto& entryInfo : list)
		{
			if (stopFlag && stopFlag->load (std::memory_order_relaxed))
				return result;

			const auto& path = entryInfo.absoluteFilePath ();
			if (entryInfo.isSymLink () &&
					entryInfo.symLinkTarget () == path)
				continue;

			if (entryInfo.isDir ())
				result += RecIterateInfo (path, followSymlinks, stopFlag);
			else if (entryInfo.isFile ())
				result += entryInfo;
		}

		return result;
	}

	QStringList RecIterate (const QString& dirPath, bool followSymlinks)
	{
		const auto& infos = RecIterateInfo (dirPath, followSymlinks);
		QStringList result;
		result.reserve (infos.size ());
		for (const auto& info : infos)
			result << info.absoluteFilePath ();
		return result;
	}

	QString FindAlbumArtPath (const QString& near, bool ignoreCollection)
	{
		if (near.isEmpty ())
			return QString ();

		if (!ignoreCollection)
		{
			auto collection = Core::Instance ().GetLocalCollection ();
			const int trackId = collection->FindTrack (near);
			if (trackId >= 0)
			{
				auto album = collection->GetTrackAlbum (trackId);
				if (!album->CoverPath_.isEmpty ())
					return album->CoverPath_;
			}
		}

		static const QStringList possibleBases { "cover", "folder", "front" };

		const auto& dir = QFileInfo (near).absoluteDir ();
		const auto& entryList = dir.entryList ({ "*.jpg", "*.png", "*.bmp" });
		if (entryList.size () > 1)
		{
			auto pos = std::find_if (entryList.begin (), entryList.end (),
					[] (const QString& name)
					{
						return std::any_of (possibleBases.begin (), possibleBases.end (),
								[&name] (const auto& pBase) { return name.startsWith (pBase, Qt::CaseInsensitive); });
					});
			return pos == entryList.end () ? QString () : dir.filePath (*pos);
		}
		else if (entryList.size () == 1)
			return dir.filePath (entryList.first ());
		else
			return {};
	}

	void ShowAlbumArt (const QString& near, const QPoint& pos)
	{
		auto px = FindAlbumArt (near);
		if (px.isNull ())
			return;

		auto label = Util::ShowPixmapLabel (px, pos);
		label->setWindowTitle (QObject::tr ("Album art"));
	}

	QString PerformSubstitutionsPlaylist (const MediaInfo& info)
	{
		auto text = XmlSettingsManager::Instance ()
				.property ("SingleTrackDisplayMask").toString ();

		text = PerformSubstitutions (text, info).simplified ();
		text.replace ("- -", "-");
		if (text.startsWith ("- "))
			text = text.mid (2);
		if (text.endsWith (" -"))
			text.chop (2);
		return text;
	}

	QString MakeTrackListTooltip (const QList<QList<Media::ReleaseTrackInfo>>& infos)
	{
		QString trackTooltip;
		int mediumPos = 0;
		for (const auto& medium : infos)
		{
			if (infos.size () > 1)
			{
				if (mediumPos)
					trackTooltip += "<br />";
				trackTooltip += QObject::tr ("CD %1:").arg (++mediumPos) + "<br />";
			}

			for (const auto& track : medium)
			{
				trackTooltip += QString::number (track.Number_) + ". ";
				trackTooltip += track.Name_;
				if (track.Length_)
				{
					auto lengthStr = Util::MakeTimeFromLong (track.Length_);
					if (lengthStr.startsWith ("00:"))
						lengthStr = lengthStr.mid (3);
					trackTooltip += " (" + lengthStr + ")";
				}
				trackTooltip += "<br/>";
			}
		}
		return trackTooltip;
	}

	bool CompareArtists (QString leftStr, QString rightStr, bool withoutThe)
	{
		if (withoutThe)
		{
			auto chopStr = [] (QString& str)
			{
				if (str.startsWith (u"the ", Qt::CaseInsensitive))
					str = str.mid (4);
				if (str.startsWith (u"a ", Qt::CaseInsensitive))
					str = str.mid (2);
			};

			chopStr (leftStr);
			chopStr (rightStr);
		}

		return QString::localeAwareCompare (leftStr, rightStr) < 0;
	}

	QPair<QString, QColor> GetRuleSymbol (const Entity& rule)
	{
		static const QString flagSym = QString::fromUtf8 ("⚑");
		static const QString disabledFlagSym = QString::fromUtf8 ("⚐");

		const auto& color = rule.Additional_ ["org.LC.AdvNotifications.AssocColor"].value<QColor> ();
		const auto isEnabled = rule.Additional_ ["org.LC.AdvNotifications.IsEnabled"].toBool ();

		return { isEnabled ? flagSym : disabledFlagSym, color };
	}

	QString FormatDateTime (const QDateTime& datetime)
	{
		const auto& current = QDateTime::currentDateTime ();
		const int days = datetime.daysTo (current);

		QLocale defLocale;
		if (days > 30)
			return defLocale.toString (datetime, "MMMM yyyy");
		else if (days >= 7)
			return QObject::tr ("%n day(s) ago", nullptr, days);
		else if (days >= 2)
			return defLocale.toString (datetime, "dddd");
		else if (days == 1)
			return QObject::tr ("yesterday");
		else
			return QObject::tr ("today");
	}

	namespace
	{
		void PerformDownload (QString to,
				const QList<QString>& filenames, const QList<QUrl>& urls, QWidget *parent)
		{
			if (!QFile::exists (to))
				QDir::root ().mkpath (to);

			QFileInfo toInfo { to };
			while (!toInfo.exists () || !toInfo.isDir () || !toInfo.isWritable ())
			{
				qWarning () << Q_FUNC_INFO
						<< "bad directory"
						<< to;
				if (QMessageBox::question (parent,
							QObject::tr ("Invalid directory"),
							QObject::tr ("The audio tracks cannot be downloaded to %1. "
								"Do you wish to choose another directory?")
								.arg ("<em>" + to + "</em>"),
							QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes)
					return;

				to = RadioTracksGrabDialog::SelectDestination (to, parent);
				if (to.isEmpty ())
					return;
			}

			const auto iem = GetProxyHolder ()->GetEntityManager ();
			for (const auto& pair : Util::Zip (urls, filenames))
			{
				const auto& e = Util::MakeEntity (pair.first,
						to + '/' + pair.second,
						OnlyDownload | AutoAccept | FromUserInitiated);
				iem->HandleEntity (e);
			}
		}
	}

	void GrabTracks (const QList<Media::AudioInfo>& infos, QWidget* parent)
	{
		const auto dia = new RadioTracksGrabDialog { infos, parent };
		dia->setAttribute (Qt::WA_DeleteOnClose);
		dia->show ();

		new Util::SlotClosure<Util::DeleteLaterPolicy>
		{
			[parent, dia, infos]
			{
				PerformDownload (dia->GetDestination (),
						dia->GetNames (),
						Util::Map (infos,
								[] (const Media::AudioInfo& info)
									{ return info.Other_ ["URL"].toUrl (); }),
						parent);
			},
			dia,
			SIGNAL (accepted ()),
			dia
		};
	}

	void GrabTracks (const QList<MediaInfo>& infos, QWidget* parent)
	{
		const auto& converted = Util::Map (infos,
				[] (const MediaInfo& info) -> Media::AudioInfo { return info; });
		GrabTracks (converted, parent);
	}
}
