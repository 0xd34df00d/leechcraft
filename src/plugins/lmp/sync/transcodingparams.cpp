/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "transcodingparams.h"
#include <QDataStream>
#include <util/sll/qtutil.h>
#include <util/sys/serialization.h>

namespace LC::LMP
{
	constexpr quint8 CurVersion = 3;

	QDataStream& operator<< (QDataStream& out, const TranscodingParams& params)
	{
		auto fmtStr = "unknown"_qba;
		switch (params.BitrateType_)
		{
		case Format::BitrateType::CBR:
			fmtStr = "cbr"_qba;
			break;
		case Format::BitrateType::VBR:
			fmtStr = "vbr"_qba;
			break;
		}
		out << CurVersion
				<< params.FormatID_
				<< fmtStr
				<< params.Quality_
				<< params.NumThreads_
				<< params.OnlyLossless_;
		return out;
	}

	QDataStream& operator>> (QDataStream& in, TranscodingParams& params)
	{
		if (!Util::EnsureVersion<quint8> (in, 3, CurVersion))
			return in;

		QByteArray fmtStr;
		in >> params.FormatID_
				>> fmtStr
				>> params.Quality_
				>> params.NumThreads_
				>> params.OnlyLossless_;
		if (fmtStr == "cbr"_qba)
			params.BitrateType_ = Format::BitrateType::CBR;
		else if (fmtStr == "vbr"_qba)
			params.BitrateType_ = Format::BitrateType::VBR;
		return in;
	}
}
