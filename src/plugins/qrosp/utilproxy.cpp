/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "utilproxy.h"
#include <util/util.h>
#include <util/xpc/util.h>
#include <util/sys/paths.h>
#include <interfaces/structures.h>

namespace LC
{
namespace Qrosp
{
	UtilProxy::UtilProxy (QObject *parent)
	: QObject (parent)
	{
	}

	QString UtilProxy::GetUserText (const LC::Entity& entity) const
	{
		return Util::GetUserText (entity);
	}

	QString UtilProxy::MakePrettySize (qint64 size) const
	{
		return Util::MakePrettySize (size);
	}

	QString UtilProxy::MakeTimeFromLong (ulong time) const
	{
		return Util::MakePrettySize (time);
	}

	QTranslator* UtilProxy::InstallTranslator (const QString& base,
			const QString& prefix, const QString& appname) const
	{
		return Util::InstallTranslator (base, prefix, appname);
	}

	QString UtilProxy::GetLocaleName () const
	{
		return Util::GetLocaleName ();
	}

	QString UtilProxy::GetLanguage () const
	{
		return Util::GetLanguage ();
	}

	QDir UtilProxy::CreateIfNotExists (const QString& path) const
	{
		return Util::CreateIfNotExists (path);
	}

	QString UtilProxy::GetTemporaryName (const QString& pattern) const
	{
		return Util::GetTemporaryName (pattern);
	}

	QObject* UtilProxy::MakeEntity (const QVariant& entity,
			const QString& location, LC::TaskParameters tp, const QString& mime) const
	{
		return new EntityWrapper (Util::MakeEntity (entity, location, tp,  mime));
	}

	namespace
	{
		Priority Str2Priority (QString str)
		{
			str = str.toLower ();
			if (str == "info")
				return Priority::Info;
			else if (str == "warning")
				return Priority::Warning;
			else if (str == "critical")
				return Priority::Critical;
			else
			{
				qWarning () << Q_FUNC_INFO
						<< "unknown priority"
						<< str;
				return Priority::Info;
			}
		}
	}

	QObject* UtilProxy::MakeNotification (const QString& header,
			const QString& text, QString priorityStr) const
	{
		return new EntityWrapper (Util::MakeNotification (header, text, Str2Priority (priorityStr)));
	}
}
}
