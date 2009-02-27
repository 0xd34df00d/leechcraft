#include "requestnormalizer.h"
#include <stdexcept>
#include <QtDebug>
#include <interfaces/ifinder.h>

using namespace LeechCraft;

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

RequestNormalizer::RequestNormalizer (QObject *parent)
: QObject (parent)
{
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
		Current_ = Parse (req);
	}
	catch (const std::runtime_error& e)
	{
		qWarning () << Q_FUNC_INFO << e.what ();
		emit error (tr ("Request parsing error: %1").arg (e.what ()));
	}

	SetMerger (Current_);
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
		node->Right_ = Parse (req.mid (pos + sizeof (" OR ")));
	}
	else if ((pos = FindWB (" AND ", req)) != -1)
	{
		node->Op_ = OperationalModel::OpAnd;
		node->Left_ = Parse (req.left (pos));
		node->Right_ = Parse (req.mid (pos + sizeof (" AND ")));
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
		CategoryMerger *merger = new CategoryMerger;
		merger->SetRequest (*holder->Req_);
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

