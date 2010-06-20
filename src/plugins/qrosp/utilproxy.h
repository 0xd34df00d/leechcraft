/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2010  Georg Rudoy
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

#ifndef PLUGINS_QROSP_WRAPPERS_UTILPROXY_H
#define PLUGINS_QROSP_WRAPPERS_UTILPROXY_H
#include <QObject>
#include <QDir>
#include <QUrl>
#include <interfaces/structures.h>
#include "wrappers/entitywrapper.h"

class QTranslator;

namespace LeechCraft
{
	struct Entity;

	namespace Plugins
	{
		namespace Qrosp
		{
			class UtilProxy : public QObject
			{
				Q_OBJECT
			public:
				UtilProxy (QObject* = 0);
			public slots:
				QString GetUserText (const LeechCraft::Entity& entity) const;
				QString MakePrettySize (qint64 size) const;
				QString MakeTimeFromLong (ulong time) const;
				QTranslator* InstallTranslator (const QString& base,
						const QString& prefix, const QString& appname) const;
				QString GetLocaleName () const;
				QString GetLanguage () const;
				QDir CreateIfNotExists (const QString& path) const;
				QString GetTemporaryName (const QString& pattern) const;
				QObject* MakeEntity (const QVariant& entity,
						const QString& location,
						LeechCraft::TaskParameters tp,
						const QString& mime) const;
				QObject* MakeNotification (const QString& header,
						const QString& text,
						Priority priority) const;
				QUrl MakeAbsoluteUrl (QUrl baseUrl,
						const QString& hrefUrl) const;
			};
		};
	};
};

#endif
