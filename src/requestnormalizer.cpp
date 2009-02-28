#include "requestnormalizer.h"
#include <stdexcept>
#include <QtDebug>
#include <interfaces/ifinder.h>

using namespace LeechCraft;
using namespace LeechCraft::Util;

RequestNormalizer::RequestHolder::RequestHolder ()
: Op_ (OperationalModel::OpNull)
{
}

RequestNormalizer::RequestHolder::~RequestHolder ()
{
	if (Op_ != OperationalModel::OpNull)
	{
		Merger_->RemoveModel (Left_->Merger_.get ());
		Merger_->RemoveModel (Right_->Merger_.get ());
	}
}

RequestNormalizer::RequestNormalizer (const boost::shared_ptr<MergeModel>& merge,
		const boost::shared_ptr<MergeModel>& history,
		QObject *parent)
: QObject (parent)
, MergeModel_ (merge)
, HistoryMergeModel_ (history)
, Root_ (new MergeModel (QStringList (tr ("Name"))
			<< tr ("State")
			<< tr ("Progress")))
, Parser_ (new RequestParser)
{
	Root_->setProperty ("__LeechCraft_own_core_model", true);
}

void RequestNormalizer::SetRequest (const QString& req)
{
	try
	{
		Validate (req);
	}
	catch (const std::runtime_error& e)
	{
		qWarning () << Q_FUNC_INFO << e.what ();
		emit error (tr ("Request validation error: %1").arg (e.what ()));
	}

	try
	{
		RequestHolder_ptr c = Parse (req);
		SetMerger (c);

		Root_->AddModel (c->Merger_.get ());
		if (Current_)
			Root_->RemoveModel (Current_->Merger_.get ());
		Current_ = c;
	}
	catch (const std::runtime_error& e)
	{
		qWarning () << Q_FUNC_INFO << e.what ();
		emit error (tr ("Request parsing error: %1").arg (e.what ()));
	}
}

QAbstractItemModel* RequestNormalizer::GetModel () const
{
	return Root_.get ();
}

void RequestNormalizer::Validate (const QString& req) const
{
	if (req.size () < 2)
		return;

	// Check for braces
	int openedBraces = 0;
	for (int i = 0; i < req.size () - 1; ++i)
	{
		if (req.mid (i, 2) == " (")
			++openedBraces;
		else if (req.mid (i, 2) == ") ")
			--openedBraces;
	}
	if (openedBraces)
		throw std::runtime_error (qPrintable (tr ("Open/close braces mismatch")));
}

namespace
{
	int FindWB (const QString& text, const QString& string)
	{
		if (string.size () < 2)
			return -1;

		int openedBraces = 0;
		int i = 0;
		for ( ; i <= string.size (); ++i)
		{
			if (i == string.size ())
			{
				i = -1;
				break;
			}

			if (string.mid (i, 2) == " (")
				++openedBraces;
			else if (string.mid (i, 2) == ") ")
				--openedBraces;

			if (!openedBraces &&
					string.mid (i, text.size ()) == text)
				break;
		}
		return i;
	}
};

RequestNormalizer::RequestHolder_ptr RequestNormalizer::Parse (QString req) const
{
	req = req.trimmed ();
	if (req.size () > 1 &&
			req.at (0) == '(' &&
			req.at (req.size () - 1) == '2')
		req = req.mid (1, req.size () - 2);

	RequestHolder_ptr node (new RequestHolder ());

	int pos = 0;
	if ((pos = FindWB (" OR ", req)) != -1)
	{
		node->Op_ = OperationalModel::OpOr;
		node->Left_ = Parse (req.left (pos));
		node->Right_ = Parse (req.mid (pos + sizeof (" OR ") - 1));
	}
	else if ((pos = FindWB (" AND ", req)) != -1)
	{
		node->Op_ = OperationalModel::OpAnd;
		node->Left_ = Parse (req.left (pos));
		node->Right_ = Parse (req.mid (pos + sizeof (" AND ") - 1));
	}
	else
	{
		Parser_->Parse (req);
		node->Req_.reset (new Request (Parser_->GetRequest ()));
	}

	return node;
}

void RequestNormalizer::SetMerger (RequestHolder_ptr holder)
{
	if (holder->Req_)
	{
		CategoryMerger *merger = new CategoryMerger (*holder->Req_,
				MergeModel_, HistoryMergeModel_);
		holder->Merger_.reset (merger);
	}
	else
	{
		SetMerger (holder->Left_);
		SetMerger (holder->Right_);

		OperationalModel *oper = new OperationalModel;
		oper->SetOperation (holder->Op_);
		oper->AddModel (holder->Left_->Merger_.get ());
		oper->AddModel (holder->Right_->Merger_.get ());
		holder->Merger_.reset (oper);
	}
}

