/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2009  Georg Rudoy
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

#include "deadlyrics.h"
#include <QIcon>
#include <xmlsettingsdialog/xmlsettingsdialog.h>
#include "core.h"
#include "findproxy.h"
#include "xmlsettingsmanager.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace DeadLyrics
		{
			void DeadLyRicS::Init (ICoreProxy_ptr proxy)
			{
				Core::Instance ().SetProxy (proxy);

				SettingsDialog_.reset (new Util::XmlSettingsDialog ());
				SettingsDialog_->RegisterObject (XmlSettingsManager::Instance (),
						"deadlyricssettings.xml");
			}

			void DeadLyRicS::SecondInit ()
			{
			}
			
			void DeadLyRicS::Release ()
			{
			}
			
			QString DeadLyRicS::GetName () const
			{
				return "DeadLyRicS";
			}
			
			QString DeadLyRicS::GetInfo () const
			{
				return tr ("Lyrics Searcher");
			}
			
			QIcon DeadLyRicS::GetIcon () const
			{
				return QIcon (":/resources/images/deadlyrics.svg");
			}
			
			QStringList DeadLyRicS::Provides () const
			{
				return QStringList ("search::lyrics");
			}
			
			QStringList DeadLyRicS::Needs () const
			{
				return QStringList ();
			}
			
			QStringList DeadLyRicS::Uses () const
			{
				return QStringList ();
			}
			
			void DeadLyRicS::SetProvider (QObject*, const QString&)
			{
			}
			
			QStringList DeadLyRicS::GetCategories () const
			{
				return Core::Instance ().GetCategories ();
			}
			
			QList<IFindProxy_ptr> DeadLyRicS::GetProxy (const LeechCraft::Request& req)
			{
				QList<IFindProxy_ptr> result;
				result << IFindProxy_ptr (new FindProxy (req));
				return result;
			}

			boost::shared_ptr<Util::XmlSettingsDialog> DeadLyRicS::GetSettingsDialog () const
			{
				return SettingsDialog_;
			}
			
			Q_EXPORT_PLUGIN2 (leechcraft_deadlyrics, DeadLyRicS);
		};
	};
};

