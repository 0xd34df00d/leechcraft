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

#ifndef CATEGORYMERGER_H
#define CATEGORYMERGER_H
#include <vector>
#include <boost/shared_ptr.hpp>
#include <plugininterface/mergemodel.h>
#include <interfaces/ifinder.h>
#include "filtermodel.h"

namespace LeechCraft
{
	struct Request;

	class CategoryMerger : public Util::MergeModel
	{
		Q_OBJECT

		typedef std::vector<boost::shared_ptr<IFindProxy> > proxies_t;
		proxies_t Proxies_;
		boost::shared_ptr<Util::MergeModel> MergeModel_;
		std::auto_ptr<FilterModel> FilterModel_;
	public:
		CategoryMerger (const Request&,
				const boost::shared_ptr<Util::MergeModel>&,
				QObject* = 0);
	};
};

#endif

