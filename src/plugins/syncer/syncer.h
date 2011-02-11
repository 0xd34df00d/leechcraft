/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010  Georg Rudoy
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

#ifndef PLUGINS_SYNCER_SYNCER_H
#define PLUGINS_SYNCER_SYNCER_H
#include <memory>
#include <QObject>
#include <QTranslator>
#include <interfaces/iinfo.h>
#include <interfaces/ihavesettings.h>

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Syncer
		{
			class Plugin : public QObject
						 , public IInfo
						 , public IHaveSettings
			{
				Q_OBJECT
				Q_INTERFACES (IInfo IHaveSettings)

				std::auto_ptr<QTranslator> Translator_;
				Util::XmlSettingsDialog_ptr XmlSettingsDialog_;
			public:
				void Init (ICoreProxy_ptr);
				void SecondInit ();
				void Release ();
				QByteArray GetUniqueID () const;
				QString GetName () const;
				QString GetInfo () const;
				QIcon GetIcon () const;
				QStringList Provides () const;

				Util::XmlSettingsDialog_ptr GetSettingsDialog () const;
			signals:
				void gotEntity (const LeechCraft::Entity&);
			};
		};
	};
};

#endif

