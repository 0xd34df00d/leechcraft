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

#include "utilproxy.h"
#include <plugininterface/util.h>
#include <interfaces/structures.h>

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Qrosp
		{
			UtilProxy::UtilProxy (QObject *parent)
			: QObject (parent)
			{
			}

			QString UtilProxy::GetUserText (const LeechCraft::Entity& entity) const
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
					const QString& location, LeechCraft::TaskParameters tp, const QString& mime) const
			{
				return new EntityWrapper (Util::MakeEntity (entity, location, tp,  mime));
			}

			QObject* UtilProxy::MakeNotification (const QString& header,
					const QString& text, Priority priority) const
			{
				return new EntityWrapper (Util::MakeNotification (header, text, priority));
			}
		};
	};
};
