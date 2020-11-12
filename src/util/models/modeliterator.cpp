/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "modeliterator.h"
#include <QAbstractItemModel>
#include <QtDebug>

namespace LC::Util
{
	ModelIterator::ModelIterator (QAbstractItemModel *model,
			int row, int col, ModelIterator::Direction dir, const QModelIndex& parent)
	: Model_ (model)
	, Parent_ (parent)
	, Row_ (row)
	, Col_ (col)
	, Dir_ (dir)
	{
	}

	ModelIterator& ModelIterator::operator++ ()
	{
		++GetIncrementable ();
		return *this;
	}

	ModelIterator ModelIterator::operator++ (int)
	{
		ModelIterator oldThis (*this);
		++GetIncrementable ();
		return oldThis;
	}

	ModelIterator& ModelIterator::operator-- ()
	{
		--GetIncrementable ();
		return *this;
	}

	ModelIterator ModelIterator::operator-- (int)
	{
		ModelIterator oldThis (*this);
		--GetIncrementable ();
		return oldThis;
	}

	ModelIterator& ModelIterator::operator+= (int diff)
	{
		GetIncrementable () += diff;
		return *this;
	}

	ModelIterator& ModelIterator::operator-= (int diff)
	{
		GetIncrementable () -= diff;
		return *this;
	}

	int ModelIterator::operator- (const ModelIterator& other) const
	{
		return GetIncrementable () - other.GetIncrementable ();
	}

	bool operator== (const ModelIterator& left, const ModelIterator& right)
	{
		return left.Row_ == right.Row_ &&
				left.Col_ == right.Col_ &&
				left.Model_ == right.Model_ &&
				left.Parent_ == right.Parent_;
	}

	bool operator!= (const ModelIterator& left, const ModelIterator& right)
	{
		return !(left == right);
	}

	QModelIndex ModelIterator::operator*() const
	{
		return Model_->index (Row_, Col_, Parent_);
	}

	int ModelIterator::GetRow () const
	{
		return Row_;
	}

	int ModelIterator::GetCol () const
	{
		return Col_;
	}

	int& ModelIterator::GetIncrementable ()
	{
		switch (Dir_)
		{
		case Direction::Rows:
			return Row_;
		case Direction::Cols:
			return Col_;
		}

		qWarning () << Q_FUNC_INFO
				<< "unknown direction";
		return Row_;
	}

	int ModelIterator::GetIncrementable () const
	{
		switch (Dir_)
		{
		case Direction::Rows:
			return Row_;
		case Direction::Cols:
			return Col_;
		}

		qWarning () << Q_FUNC_INFO
				<< "unknown direction";
		return Row_;
	}
}
