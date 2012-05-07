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
#include <interfaces/iinfo.h>
#include <interfaces/ihavetabs.h>
#include <interfaces/ihavesettings.h>
#include <interfaces/iplugin2.h>
#include <xmlsettingsdialog/xmlsettingsdialog.h>
#include <interfaces/blogique/ibloggingplatformplugin.h>

namespace LeechCraft
{
namespace Blogique
{
namespace Metida
{
	class Plugin : public QObject
				, public IInfo
				, public IHaveSettings
				, public IPlugin2
				, public IBloggingPlatformPlugin
	{
		Q_OBJECT
		Q_INTERFACES (IInfo IHaveSettings IPlugin2
				LeechCraft::Blogique::IBloggingPlatformPlugin)

		Util::XmlSettingsDialog_ptr XmlSettingsDialog_;

	public:
		void Init (ICoreProxy_ptr proxy);
		void SecondInit ();
		QByteArray GetUniqueID () const;
		void Release ();
		QString GetName () const;
		QString GetInfo () const;
		QIcon GetIcon () const;

		Util::XmlSettingsDialog_ptr GetSettingsDialog () const;

		QSet<QByteArray> GetPluginClasses () const;

		QObject* GetObject ();
		QList<QObject*> GetBloggingPlatforms () const;

	public slots:
		void initPlugin (QObject *proxy);

	signals:
		void gotEntity (LeechCraft::Entity e);
		void delegateEntity (LeechCraft::Entity e, int *id, QObject **obj);
	};
}
}
}
