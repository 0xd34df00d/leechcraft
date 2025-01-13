/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include "sllconfig.h"
#include <memory>
#include <QString>
#include <QMetaType>
#include <QRegularExpression>
#include "visitor.h"

namespace LC::Util
{
	// TODO mark this deprecated once Qt6 port is complete
	class UTIL_SLL_API RegExp
	{
		QRegularExpression Rx_;
	public:
		static bool IsFast ();

		RegExp () = default;
		RegExp (const QString&, Qt::CaseSensitivity);

		bool Matches (const QString&) const;
		bool Matches (const QByteArray&) const;

		QString GetPattern () const;
		Qt::CaseSensitivity GetCaseSensitivity () const;
	};

	struct StopReplace {};

	struct ReplaceAdvance
	{
		qsizetype Shift_;

		explicit ReplaceAdvance (qsizetype shift)
		: Shift_ { std::max<qsizetype> (shift, 1) }
		{
		}
	};

	using ReplacerResult = std::variant<StopReplace, ReplaceAdvance>;

	template<typename R>
	void ReplaceByRegexp (QString& body,
			const QRegularExpression& rx,
			R&& replacer)
		requires requires { { replacer (body, QRegularExpressionMatch {}) } -> std::convertible_to<ReplacerResult>; }
	{
		int pos = 0;
		bool keepGoing = true;
		while (keepGoing)
		{
			const auto& match = rx.match (body, pos);
			if (!match.hasMatch ())
				return;

			Util::Visit (ReplacerResult { replacer (body, match) },
					[&] (StopReplace) { keepGoing = false; },
					[&] (ReplaceAdvance adv) { pos = match.capturedStart (0) + adv.Shift_; });
		}
	}
}

UTIL_SLL_API QDataStream& operator<< (QDataStream&, const LC::Util::RegExp&);
UTIL_SLL_API QDataStream& operator>> (QDataStream&, LC::Util::RegExp&);

Q_DECLARE_METATYPE (LC::Util::RegExp)
