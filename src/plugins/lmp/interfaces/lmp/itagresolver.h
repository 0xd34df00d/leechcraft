/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QtPlugin>

class QMutex;

namespace TagLib
{
	class FileRef;
}

namespace LC
{
namespace Util
{
	template<typename, typename>
	class Either;
}

namespace LMP
{
	struct MediaInfo;

	struct ResolveError
	{
		QString FilePath_;

		QString ReasonString_;
	};

	class ITagResolver
	{
	public:
		virtual ~ITagResolver () {}

		using ResolveResult_t = Util::Either<ResolveError, MediaInfo>;

		virtual TagLib::FileRef GetFileRef (const QString&) const = 0;
		virtual ResolveResult_t ResolveInfo (const QString&) = 0;
		virtual QMutex& GetMutex () = 0;
	};
}
}

Q_DECLARE_INTERFACE (LC::LMP::ITagResolver, "org.LeechCraft.LMP.ITagResolver/1.0")
