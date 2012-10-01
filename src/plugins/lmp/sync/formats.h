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

#pragma once

#include <memory>
#include <QStringList>

namespace LeechCraft
{
namespace LMP
{
	class TranscodingParams;

	class Format
	{
	public:
		virtual QString GetFormatID () const = 0;
		virtual QString GetFileExtension () const;
		virtual QString GetFormatName () const = 0;
		virtual QString GetCodecName () const = 0;

		enum class BitrateType
		{
			VBR,
			CBR
		};
		virtual QList<BitrateType> GetSupportedBitrates () const = 0;
		virtual QList<int> GetBitrateLabels (BitrateType) const = 0;

		virtual QStringList ToFFmpeg (const TranscodingParams&) const;
	protected:
		void StandardQualityAppend (QStringList&, const TranscodingParams&) const;
	};
	typedef std::shared_ptr<Format> Format_ptr;

	class Formats
	{
		QList<Format_ptr> Formats_;
		QList<Format_ptr> EnabledFormats_;

		static QString S_FFmpegCodecs_;
	public:
		Formats ();

		QList<Format_ptr> GetFormats () const;
		Format_ptr GetFormat (const QString&) const;
	};
}
}
