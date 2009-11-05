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

#include "core.h"
#include <algorithm>
#include <boost/bind.hpp>
#include <QCryptographicHash>
#include <QUrl>
#include <QtDebug>
#include <interfaces/iwebbrowser.h>
#include "lyricwikisearcher.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace DeadLyrics
		{
			Core::Core ()
			{
				qRegisterMetaType<Lyrics> ("LeechCraft::Plugins::DeadLyrics::Lyrics");
				qRegisterMetaTypeStreamOperators<Lyrics> ("LeechCraft::Plugins::DeadLyrics::Lyrics");
				Searchers_.push_back (searcher_ptr (new LyricWikiSearcher));
			}
			
			Core& Core::Instance ()
			{
				static Core core;
				return core;
			}
			
			void Core::Release ()
			{
				Searchers_.clear ();
			}
			
			void Core::SetProxy (ICoreProxy_ptr proxy)
			{
				Proxy_ = proxy;
			}
			
			QNetworkAccessManager* Core::GetNetworkAccessManager () const
			{
				return Proxy_->GetNetworkAccessManager ();
			}

			IWebBrowser* Core::GetWebBrowser () const
			{
				IPluginsManager *pm = Proxy_->GetPluginsManager ();
				QObjectList browsers = pm->Filter<IWebBrowser*> (pm->GetAllPlugins ());
				return browsers.size () ?
					qobject_cast<IWebBrowser*> (browsers.at (0)) :
					0;
			}
			
			QStringList Core::GetCategories () const
			{
				return QStringList (tr ("lyrics"));
			}
			
			searchers_t Core::GetSearchers (const QString& category) const
			{
				if (category == tr ("lyrics"))
					return Searchers_;
				else
					return searchers_t ();
			}
		};
	};
};

