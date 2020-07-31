/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QString>
#include <QMetaType>
#include "formats.h"

namespace LC
{
namespace LMP
{
	struct TranscodingParams
	{
		QString FilePattern_;

		/** Possible format IDs are:
		 *
		 * - ogg
		 * - aac-nonfree
		 * - aac-free
		 * - mp3
		 * - wma
		 */
		QString FormatID_;
		Format::BitrateType BitrateType_;
		int Quality_;
		int NumThreads_;

		bool OnlyLossless_;
	};

	QDataStream& operator<< (QDataStream&, const TranscodingParams&);
	QDataStream& operator>> (QDataStream&, TranscodingParams&);
}
}

Q_DECLARE_METATYPE (LC::LMP::TranscodingParams)
