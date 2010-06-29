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

#include "core.h"
#include "repoinfofetcher.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace LackMan
		{
			Core::Core ()
			: RepoInfoFetcher_ (new RepoInfoFetcher (this))
			{
				connect (RepoInfoFetcher_,
						SIGNAL (delegateEntity (const LeechCraft::Entity&,
								int*, QObject**)),
						this,
						SIGNAL (delegateEntity (const LeechCraft::Entity&,
								int*, QObject**)));
				connect (RepoInfoFetcher_,
						SIGNAL (gotEntity (const LeechCraft::Entity&)),
						this,
						SIGNAL (gotEntity (const LeechCraft::Entity&)));
			}

			Core& Core::Instance ()
			{
				static Core c;
				return c;
			}

			void Core::Release ()
			{
				delete RepoInfoFetcher_;
				RepoInfoFetcher_ = 0;
			}

			void Core::SetProxy (ICoreProxy_ptr proxy)
			{
				Proxy_ = proxy;
			}

			ICoreProxy_ptr Core::GetProxy () const
			{
				return Proxy_;
			}

			void Core::AddRepo (const QUrl& url)
			{
				RepoInfoFetcher_->FetchFor (url);
			}

			void Core::infoFetched (const RepoInfo& ri)
			{
				qDebug () << Q_FUNC_INFO << ri.GetUrl ();
			}
		}
	}
}
