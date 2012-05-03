/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2012  Oleg Linkin
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

#pragma once

#include <QObject>
#include <QSet>
#include <interfaces/core/icoreproxy.h>

namespace LeechCraft
{
namespace Blogique
{
	class IAccount;
	class IBloggingPlatform;

	class Core : public QObject
	{
		Q_OBJECT

		ICoreProxy_ptr Proxy_;
		QObjectList BlogPlatformPlugins_;

		Core ();
		Q_DISABLE_COPY (Core)

	public:
		static Core& Instance ();

		void SetCoreProxy (ICoreProxy_ptr proxy);
		ICoreProxy_ptr GetCoreProxy ();

		QSet<QByteArray> GetExpectedPluginClasses () const;
		void AddPlugin (QObject *plugin);

		QList<IBloggingPlatform*> GetBloggingPlatforms () const;
		QList<IAccount*> GetAccounts () const;

	private:
		void AddBlogPlatformPlugin (QObject *plugin);

	signals:
		void accountAdded (IAccount *account);
		void accountRemoved (IAccount *account);
	};
}
}
