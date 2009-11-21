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

#ifndef PLUGINS_SUMMARY_REQUESTNORMALIZER_H
#define PLUGINS_SUMMARY_REQUESTNORMALIZER_H
#include <QObject>
#include <boost/shared_ptr.hpp>
#include "requestparser.h"
#include "categorymerger.h"
#include "operationalmodel.h"

namespace LeechCraft
{
	struct Request;

	namespace Plugins
	{
		namespace Summary
		{
			class RequestNormalizer : public QObject
			{
				Q_OBJECT

				boost::shared_ptr<Util::MergeModel> MergeModel_;
				/** Forms a binary tree, where each node is an operation holding
				 * a corresponding operational model and each leaf is a category
				 * merger which parses the requests and provides the
				 * QAbstractItemModel with the result.
				 *
				 * If the holder is a leaf, Req_ is filled, Left_ and Right_ are
				 * filled otherwise. Merger_ is filled in each case.
				 */
				struct RequestHolder
				{
					RequestHolder ();
					~RequestHolder ();

					OperationalModel::Operation Op_;
					boost::shared_ptr<RequestHolder> Left_;
					boost::shared_ptr<RequestHolder> Right_;

					boost::shared_ptr<Request> Req_;
					boost::shared_ptr<Util::MergeModel> Merger_;
				};
				typedef boost::shared_ptr<RequestHolder> RequestHolder_ptr;
				RequestHolder_ptr Current_;
				boost::shared_ptr<Util::MergeModel> Root_;
				boost::shared_ptr<RequestParser> Parser_;
			public:
				RequestNormalizer (const boost::shared_ptr<Util::MergeModel>&,
						QObject* = 0);

				void SetRequest (const QString&);
				QAbstractItemModel* GetModel () const;
			private:
				void Validate (const QString&) const;
				RequestHolder_ptr Parse (QString) const;
				void SetMerger (RequestHolder_ptr);
			signals:
				void error (const QString&);
			};
		};
	};
};

#endif

