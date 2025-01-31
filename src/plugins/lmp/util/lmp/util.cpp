/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "util.h"
#include <util/util.h>
#include <util/sll/qtutil.h>
#include <util/lmp/mediainfo.h>

namespace LC
{
namespace LMP
{
	QMap<QString, std::function<QString (MediaInfo)>> GetSubstGetters ()
	{
		static const QMap<QString, std::function<QString (MediaInfo)>> map
		{
			{ "$artist", [] (const MediaInfo& info) { return info.Artist_; } },
			{ "$album", [] (const MediaInfo& info) { return info.Album_; } },
			{ "$title", [] (const MediaInfo& info) { return info.Title_; } },
			{ "$year", [] (const MediaInfo& info) { return QString::number (info.Year_); } },
			{ "$trackNumber", [] (const MediaInfo& info) -> QString
				{
					auto trackNumStr = QString::number (info.TrackNumber_);
					if (info.TrackNumber_ < 10)
						trackNumStr.prepend ('0');
					return trackNumStr;
				} }
		};
		return map;
	}

	QMap<QString, std::function<void (MediaInfo&, QString)>> GetSubstSetters ()
	{
		static const QMap<QString, std::function<void (MediaInfo&, QString)>> map
		{
			{ "$artist", [] (MediaInfo& info, const QString& val) { info.Artist_ = val; } },
			{ "$album", [] (MediaInfo& info, const QString& val) { info.Album_= val; } },
			{ "$title", [] (MediaInfo& info, const QString& val) { info.Title_ = val; } },
			{ "$year", [] (MediaInfo& info, const QString& val) { info.Year_ = val.toInt (); } },
			{ "$trackNumber", [] (MediaInfo& info, QString val)
				{
					if (val.size () == 2 && val.at (0) == '0')
						val = val.mid (1);
					info.TrackNumber_ = val.toInt ();
				} }
		};
		return map;
	}

	QStringList GetSubstGettersKeys ()
	{
		static const QStringList keys = GetSubstGetters ().keys ();
		return keys;
	}

	QString PerformSubstitutions (QString mask, const MediaInfo& info, SubstitutionFlags flags)
	{
		for (const auto& pair : Util::Stlize (GetSubstGetters ()))
		{
			auto value = pair.second (info);
			if (flags & SubstitutionFlag::SFSafeFilesystem)
				value.replace ('/', '_');
			mask.replace (pair.first, value);
		}

		if (flags & SubstitutionFlag::SFSafeFilesystem)
		{
			mask.replace ('?', '_');
			mask.replace ('*', '_');
		}

		return mask;
	}

	QStringList PerformSubstitutions (const QString& pattern,
			const QList<MediaInfo>& infos,
			const std::function<void (int, QString)>& setter,
			SubstitutionFlags flags)
	{
		QStringList names;

		const bool hasExtension = pattern.contains ('.');

		int row = 0;
		for (const auto& info : infos)
		{
			auto name = PerformSubstitutions (pattern, info, flags);
			if (!hasExtension)
				name += '.' + info.LocalPath_.section ('.', -1);

			names << name;

			setter (row++, name);
		}

		return names;
	}
}
}
