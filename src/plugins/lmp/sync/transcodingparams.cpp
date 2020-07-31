/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "transcodingparams.h"
#include <QDataStream>
#include <QtDebug>

namespace LC
{
namespace LMP
{
	QDataStream& operator<< (QDataStream& out, const TranscodingParams& params)
	{
		out << static_cast<quint8> (2);
		out << params.FilePattern_
				<< params.FormatID_;

		auto fmtStr = "unknown";
		switch (params.BitrateType_)
		{
		case Format::BitrateType::CBR:
			fmtStr = "cbr";
			break;
		case Format::BitrateType::VBR:
			fmtStr = "vbr";
			break;
		}
		out << fmtStr
				<< params.Quality_
				<< params.NumThreads_
				<< params.OnlyLossless_;

		return out;
	}

	QDataStream& operator>> (QDataStream& in, TranscodingParams& params)
	{
		quint8 version = 0;
		in >> version;
		if (version < 1 || version > 2)
		{
			qWarning () << Q_FUNC_INFO
					<< "unknown version"
					<< version;
			return in;
		}

		QString fmtStr;
		in >> params.FilePattern_
				>> params.FormatID_
				>> fmtStr
				>> params.Quality_
				>> params.NumThreads_;

		if (fmtStr == "cbr")
			params.BitrateType_ = Format::BitrateType::CBR;
		else if (fmtStr == "vbr")
			params.BitrateType_ = Format::BitrateType::VBR;

		if (version >= 2)
			in >> params.OnlyLossless_;
		else
			params.OnlyLossless_ = true;

		return in;
	}
}
}
