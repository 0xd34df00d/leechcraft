/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <memory>
#include <QStringList>

namespace LC
{
namespace LMP
{
	struct TranscodingParams;

	class Format
	{
	public:
		virtual QString GetFormatID () const = 0;
		virtual QString GetFileExtension () const;
		virtual QString GetFormatName () const = 0;
		virtual QString GetCodecName () const = 0;
		virtual QString GetCodecID () const;

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
