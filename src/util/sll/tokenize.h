/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QStringView>

namespace LC::Util
{
	struct Tokenize
	{
		QStringView Str_;
		QChar Ch_;

		struct Iterator
		{
			using difference_type = qsizetype;
			using value_type = QStringView;
			using reference = QStringView;
			using pointer = void;
			using iterator_category = std::bidirectional_iterator_tag;

			const Tokenize& Tok_;
			qsizetype CurStart_;
			qsizetype NextStart_;

			QStringView operator* () const
			{
				return NextStart_ >= 0 ?
						Tok_.Str_.mid (CurStart_, NextStart_ - CurStart_ - 1) :
						Tok_.Str_.mid (CurStart_);
			}

			Iterator& operator++ ()
			{
				CurStart_ = NextStart_;
				if (CurStart_ == -1)
					return *this;

				NextStart_ = Tok_.Str_.indexOf (Tok_.Ch_, NextStart_);
				if (NextStart_ >= 0)
					++NextStart_;

				return *this;
			}

			Iterator& operator-- ()
			{
				if (CurStart_ == 0)
					return *this;

				NextStart_ = CurStart_;

				if (CurStart_ == 1)
					CurStart_ = 0;
				else if (CurStart_ >= 0) // actually implies CurStart_ > 1
					CurStart_ = Tok_.Str_.lastIndexOf (Tok_.Ch_, CurStart_ - 2) + 1;
				else
					CurStart_ = Tok_.Str_.lastIndexOf (Tok_.Ch_) + 1;

				return *this;
			}

			bool operator== (const Iterator& other) const
			{
				return CurStart_ == other.CurStart_ && NextStart_ == other.NextStart_;
			}
		};

		Iterator begin () const
		{
			Iterator it { *this, -1, 0 };
			return ++it;
		}

		Iterator end () const
		{
			return { *this, -1, -1 };
		}

		auto rbegin () const
		{
			return std::reverse_iterator { end () };
		}

		auto rend () const
		{
			return std::reverse_iterator { begin () };
		}
	};
}
