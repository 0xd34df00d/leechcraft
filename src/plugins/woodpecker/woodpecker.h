
/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2011  Georg Rudoy
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

#ifndef PLUGINS_WOODPECKER_H
#define PLUGINS_WOODPECKER_H
#include <QObject>
#include <interfaces/iinfo.h>
#include <interfaces/ihavetabs.h>
#include <interfaces/ientityhandler.h>
#include <interfaces/ihavesettings.h>
#include <interfaces/structures.h>

class QTranslator;

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Woodpecker
		{
			class Plugin : public QObject
						 , public IInfo
						 , public IHaveTabs
						 , public IHaveSettings
			{
				Q_OBJECT
				Q_INTERFACES (IInfo IHaveTabs IHaveSettings)

				boost::shared_ptr<QTranslator> Translator_;
				boost::shared_ptr<Util::XmlSettingsDialog> XmlSettingsDialog_;
			public:
				void Init (ICoreProxy_ptr);
				void SecondInit ();
				void Release ();
				QByteArray GetUniqueID () const;
				QString GetName () const;
				QString GetInfo () const;
				QIcon GetIcon () const;

				TabClasses_t GetTabClasses () const;
				void TabOpenRequested (const QByteArray&);

				boost::shared_ptr<Util::XmlSettingsDialog> GetSettingsDialog () const;
			signals:
				void addNewTab (const QString&, QWidget*);
				void removeTab (QWidget*);
				void changeTabName (QWidget*, const QString&);
				void changeTabIcon (QWidget*, const QIcon&);
				void changeTooltip (QWidget*, QWidget*);
				void statusBarChanged (QWidget*, const QString&);
				void raiseTab (QWidget*);
				void delegateEntity (const LeechCraft::Entity&,
						int*, QObject**);
				void gotEntity (const LeechCraft::Entity&);

				void couldHandle (const LeechCraft::Entity&, bool*);
			};
		};
	};
};

#endif

