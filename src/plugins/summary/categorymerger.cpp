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

#include "categorymerger.h"
#include <QtDebug>
#include "core.h"
#include "requestparser.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Summary
		{
			CategoryMerger::CategoryMerger (const Request& r,
					const boost::shared_ptr<MergeModel>& merge,
					QObject *parent)
			: MergeModel (QStringList (QString ())
					<< QString ()
					<< QString ())
			, MergeModel_ (merge)
			, FilterModel_ (new FilterModel)
			{
				Q_UNUSED (parent);

				setObjectName (QString ("CategoryMerger ") + r.String_);
				FilterModel_->setObjectName (QString ("CategoryMerger's filter model ") + r.String_);
				setProperty ("__LeechCraft_own_core_model", true);
				FilterModel_->setProperty ("__LeechCraft_own_core_model", true);
				bool builtin = false;

				if (r.Category_.isEmpty () ||
						r.Category_ == "downloads" ||
						r.Category_ == "d")
				{
					builtin = true;
					FilterModel_->setSourceModel (MergeModel_.get ());
				}
				else
				{
					QList<IFinder*> finders = Core::Instance ().GetProxy ()->
						GetPluginsManager ()->GetAllCastableTo<IFinder*> ();

					for (QList<IFinder*>::iterator i = finders.begin (),
							end = finders.end (); i != end; ++i)
					{
						try
						{
							if (!(*i)->GetCategories ().contains (r.Category_))
								continue;
						}
						catch (const std::exception& e)
						{
							qWarning () << Q_FUNC_INFO
								<< "getting categories"
								<< *i
								<< e.what ();
							continue;
						}
						catch (...)
						{
							qWarning () << Q_FUNC_INFO
								<< "getting categories"
								<< *i;
							continue;
						}

						QList<IFindProxy_ptr> proxies;
						try
						{
							proxies = (*i)->GetProxy (r);
						}
						catch (const std::exception& e)
						{
							qWarning () << Q_FUNC_INFO
								<< "getting proxy"
								<< *i
								<< e.what ();
							continue;
						}
						catch (...)
						{
							qWarning () << Q_FUNC_INFO
								<< "getting proxy"
								<< *i;
							continue;
						}

						Q_FOREACH (IFindProxy_ptr proxy, proxies)
						{
							if (!proxy)
							{
								qWarning () << Q_FUNC_INFO
									<< "got null proxy"
									<< *i;
								continue;
							}

							try
							{
								AddModel (proxy->GetModel ());
								Proxies_.push_back (proxy);
							}
							catch (const std::exception& e)
							{
								qWarning () << Q_FUNC_INFO
									<< "getting model"
									<< *i
									<< e.what ();
							}
							catch (...)
							{
								qWarning () << Q_FUNC_INFO
									<< "getting model"
									<< *i;
							}
						}
					}
				}

				if (builtin)
				{
					FilterModel_->setFilterCaseSensitivity (r.CaseSensitive_ ?
							Qt::CaseSensitive : Qt::CaseInsensitive);

					switch (r.Type_)
					{
						case Request::RTFixed:
							FilterModel_->SetTagsMode (false);
							FilterModel_->setFilterFixedString (r.String_);
							break;
						case Request::RTWildcard:
							FilterModel_->SetTagsMode (false);
							FilterModel_->setFilterWildcard (r.String_);
							break;
						case Request::RTRegexp:
							FilterModel_->SetTagsMode (false);
							FilterModel_->setFilterRegExp (r.String_);
							break;
						case Request::RTTag:
							FilterModel_->SetTagsMode (true);
							FilterModel_->setFilterFixedString (r.String_);
							break;
					}
					AddModel (FilterModel_.get ());
				}
			}
		};
	};
};

