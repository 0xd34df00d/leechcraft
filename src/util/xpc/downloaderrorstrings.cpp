/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "downloaderrorstrings.h"
#include <QObject>

namespace LC::Util
{
	QString GetErrorString (IDownload::Error::Type type)
	{
		switch (type)
		{
		case IDownload::Error::Type::Unknown:
			break;
		case IDownload::Error::Type::NoError:
			return QObject::tr ("no error");
		case IDownload::Error::Type::NotFound:
			return QObject::tr ("not found");
		case IDownload::Error::Type::Gone:
			return QObject::tr ("gone forever");
		case IDownload::Error::Type::AccessDenied:
			return QObject::tr ("access denied");
		case IDownload::Error::Type::AuthRequired:
			return QObject::tr ("authentication required");
		case IDownload::Error::Type::ProtocolError:
			return QObject::tr ("protocol error");
		case IDownload::Error::Type::NetworkError:
			return QObject::tr ("network error");
		case IDownload::Error::Type::ContentError:
			return QObject::tr ("content error");
		case IDownload::Error::Type::ProxyError:
			return QObject::tr ("proxy error");
		case IDownload::Error::Type::ServerError:
			return QObject::tr ("server error");
		case IDownload::Error::Type::LocalError:
			return QObject::tr ("local error");
		case IDownload::Error::Type::UserCanceled:
			return QObject::tr ("user canceled the download");
		}

		return QObject::tr ("unknown error");
	}
}
