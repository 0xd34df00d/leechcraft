#ifndef REQUESTNORMALIZER_H
#define REQUESTNORMALIZER_H
#include <QObject>
#include <boost/shared_ptr.hpp>
#include "requestparser.h"
#include "categorymerger.h"
#include "operationalmodel.h"

namespace LeechCraft
{
	struct Request;

	class RequestNormalizer : public QObject
	{
		Q_OBJECT

		boost::shared_ptr<Util::MergeModel> MergeModel_;
		boost::shared_ptr<Util::MergeModel> HistoryMergeModel_;
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
				const boost::shared_ptr<Util::MergeModel>&,
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

#endif

