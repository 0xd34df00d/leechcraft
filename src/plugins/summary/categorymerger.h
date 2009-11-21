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

#ifndef PLUGINS_SUMMARY_CATEGORYMERGER_H
#define PLUGINS_SUMMARY_CATEGORYMERGER_H
#include <vector>
#include <boost/shared_ptr.hpp>
#include <plugininterface/mergemodel.h>
#include <interfaces/ifinder.h>
#include "filtermodel.h"

namespace LeechCraft
{
	struct Request;

	namespace Plugins
	{
		namespace Summary
		{
			/** Extracts categories from the request, finds the corresponding
			 * plugins that can handle it and merges models received from them
			 * into one.
			 *
			 * If the category is "embedded", like "downloaders", it also filters
			 * the results according to the request paremeters.
			 */
			class CategoryMerger : public Util::MergeModel
			{
				Q_OBJECT

				typedef std::vector<boost::shared_ptr<IFindProxy> > proxies_t;
				proxies_t Proxies_;
				boost::shared_ptr<Util::MergeModel> MergeModel_;
				std::auto_ptr<FilterModel> FilterModel_;
			public:
				/** Constructs the merger according to request. Uses the merge
				 * as a MergeModel if categories is the one of the built-ins.
				 */
				CategoryMerger (const Request& req,
						const boost::shared_ptr<Util::MergeModel>& merge,
						QObject* = 0);
			};
		};
	};
};

#endif

