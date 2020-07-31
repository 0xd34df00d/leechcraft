/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "extensionsdataimpl.h"
#include <QFile>
#include <QIcon>
#include <QtDebug>

namespace LC
{
namespace Util
{
	namespace
	{
		QHash<QString, QString> ParseMimeTypes ()
		{
			QFile file { "/etc/mime.types" };
			if (!file.open (QIODevice::ReadOnly))
			{
				qWarning () << Q_FUNC_INFO
						<< "cannot open /etc/mime.types:"
						<< file.errorString ();
				return {};
			}

			QRegExp wsRx { "\\s+" };

			QHash<QString, QString> result;
			while (!file.atEnd ())
			{
				const auto& line = file.readLine ().trimmed ();
				const auto& elems = QString::fromLatin1 (line).split (wsRx);

				const auto& mime = elems.at (0);
				for (int i = 1; i < elems.size (); ++i)
					result [elems.at (i)] = mime;
			}
			return result;
		}

		QStringList GetMimeDirs ()
		{
			auto list = qgetenv ("XDG_DATA_HOME").split (':') +
					qgetenv ("XDG_DATA_DIRS").split (':');
			if (list.isEmpty ())
				list << "/usr/share";

			QStringList result;
			for (const auto& item : list)
				if (QFile::exists (item + "/mime"))
					result << item + "/mime/";
			return result;
		}

		void ParseIconsMappings (QHash<QString, QString>& result, const QString& filename)
		{
			QFile file { filename };
			if (!file.open (QIODevice::ReadOnly))
			{
				qWarning () << Q_FUNC_INFO
						<< "cannot open"
						<< filename
						<< file.errorString ();
				return;
			}

			while (!file.atEnd ())
			{
				const auto& line = QString::fromLatin1 (file.readLine ().trimmed ());
				if (!line.indexOf (':'))
					continue;

				result [line.section (':', 0, 0)] = line.section (':', 1, 1);
			}
		}

		QHash<QString, QString> ParseIconsMappings ()
		{
			QHash<QString, QString> result;
			for (const auto& mimeDir : GetMimeDirs ())
			{
				ParseIconsMappings (result, mimeDir + "generic-icons");
				ParseIconsMappings (result, mimeDir + "icons");
			}
			return result;
		}
	}

	struct ExtensionsDataImpl::Details
	{
		QHash<QString, QString> MimeDatabase_;
		QHash<QString, QString> IconsMappings_;

		Details ();
	};

	ExtensionsDataImpl::Details::Details ()
	: MimeDatabase_ { ParseMimeTypes () }
	, IconsMappings_ { ParseIconsMappings () }
	{
	}

	ExtensionsDataImpl::ExtensionsDataImpl ()
	: Details_ { new Details }
	{
	}

	const QHash<QString, QString>& ExtensionsDataImpl::GetMimeDatabase () const
	{
		return Details_->MimeDatabase_;
	}

	QIcon ExtensionsDataImpl::GetExtIcon (const QString& extension) const
	{
		return GetMimeIcon (GetMimeDatabase ().value (extension));
	}

	QIcon ExtensionsDataImpl::GetMimeIcon (const QString& mime) const
	{
		auto iconName = Details_->IconsMappings_.value (mime);
		if (iconName.isEmpty ())
			iconName = mime.section ('/', 0, 0) + "-x-generic";

		auto result = QIcon::fromTheme (iconName);
		if (result.isNull ())
			result = QIcon::fromTheme (mime.section ('/', 0, 0) + "-x-generic");
		if (result.isNull ())
			result = QIcon::fromTheme ("unknown");
		return result;
	}
}
}
