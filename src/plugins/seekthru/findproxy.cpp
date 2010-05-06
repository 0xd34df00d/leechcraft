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

#include "findproxy.h"
#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <plugininterface/mergemodel.h>
#include "core.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace SeekThru
		{
			using LeechCraft::Util::MergeModel;
			
			FindProxy::FindProxy (const LeechCraft::Request& r)
			: R_ (r)
			, MergeModel_ (new MergeModel (QStringList ("1") << "2" << "3"))
			{
			}
			
			FindProxy::~FindProxy ()
			{
				Q_FOREACH (SearchHandler_ptr sh, Handlers_)
					MergeModel_->RemoveModel (sh.get ());
			}
			
			QAbstractItemModel* FindProxy::GetModel ()
			{
				return MergeModel_.get ();
			}
			
			QByteArray FindProxy::GetUniqueSearchID () const
			{
				return QString ("org.LeechCraft.SeekThru.%1.%2")
						.arg (R_.Category_)
						.arg (R_.String_)
						.toUtf8 ();
			}

			QStringList FindProxy::GetCategories () const
			{
				return QStringList (R_.Category_);
			}

			void FindProxy::SetHandlers (const QList<SearchHandler_ptr>& handlers)
			{
				Handlers_ = handlers;
				Q_FOREACH (SearchHandler_ptr sh, Handlers_)
				{
					MergeModel_->AddModel (sh.get ());
					sh->Start (R_);
				}
			}
			
		};
	};
};

