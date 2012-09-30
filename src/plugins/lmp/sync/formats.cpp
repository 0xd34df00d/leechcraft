/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2012  Georg Rudoy
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

#include "formats.h"
#include <QtDebug>
#include "transcodingparams.h"

namespace LeechCraft
{
namespace LMP
{
	class OggFormat : public Format
	{
	public:
		QString GetFormatID () const
		{
			return "ogg";
		}

		QString GetFormatName () const
		{
			return "OGG Vorbis";
		}

		QList<BitrateType> GetSupportedBitrates() const
		{
			return { BitrateType::VBR, BitrateType::CBR };
		}

		QList<int> GetBitrateLabels (BitrateType type) const
		{
			switch (type)
			{
			case BitrateType::CBR:
				return { 64, 80, 96, 112, 128, 160, 192, 224, 256, 320, 500 };
			case BitrateType::VBR:
				return { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
			}

			qWarning () << Q_FUNC_INFO
					<< "unknown bitrate type";
			return QList<int> ();
		}

		QStringList ToFFmpeg (const TranscodingParams& params) const
		{
			QStringList result;
			result << "-acodec" << "libvorbis";
			switch (params.BitrateType_)
			{
			case BitrateType::CBR:
				result << "-ab"
						<< (QString::number (params.Quality_) + "k");
				break;
			case BitrateType::VBR:
				result << "-aq"
						<< QString::number (params.Quality_);
				break;
			}

			return result;
		}
	};

	Formats::Formats ()
	{
		Formats_ << Format_ptr (new OggFormat);
	}

	QList<Format_ptr> Formats::GetFormats () const
	{
		return Formats_;
	}

	Format_ptr Formats::GetFormat (const QString& id) const
	{
		const auto pos = std::find_if (Formats_.begin (), Formats_.end (),
				[&id] (const Format_ptr format) { return format->GetFormatID () == id; });
		return pos == Formats_.end () ?
				Format_ptr () :
				*pos;
	}
}
}
