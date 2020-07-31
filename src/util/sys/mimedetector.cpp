/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "mimedetector.h"

#include <QString>

#ifdef HAVE_MAGIC
#include <magic.h>
#endif

namespace LC
{
namespace Util
{
#ifdef HAVE_MAGIC
	class MimeDetectorImpl
	{
		std::shared_ptr<magic_set> Magic_;
	public:
		MimeDetectorImpl ()
		: Magic_ (magic_open (MAGIC_MIME_TYPE), magic_close)
		{
			magic_load (Magic_.get (), nullptr);
		}

		QByteArray Detect (const QString& path)
		{
			return magic_file (Magic_.get (), path.toUtf8 ().constData ());
		}
	};
#else
	class MimeDetectorImpl
	{
	public:
		MimeDetectorImpl ()
		{
		}

		QByteArray Detect (const QString&)
		{
			return "application/octet-stream";
		}
	};
#endif

	MimeDetector::MimeDetector ()
	: Impl_ { std::make_shared<MimeDetectorImpl> () }
	{
	}

	QByteArray MimeDetector::Detect (const QString& path)
	{
		return Impl_->Detect (path);
	}

	QByteArray MimeDetector::operator() (const QString& path)
	{
		return Detect (path);
	}

	QByteArray DetectFileMime (const QString& path)
	{
		return MimeDetector {} (path);
	}
}
}
