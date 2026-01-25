/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <QStringList>

class QTextEdit;

namespace LC::Azoth
{
	class MsgEditAutocompleter : public QObject
	{
		const QString EntryId_;
		QTextEdit& Edit_;

		struct Idle {};
		struct Completing
		{
			int StartCursorPos_;
			QStringList Completions_;
			qsizetype Idx_;
			qsizetype CurCompLength_;
		};
		struct NoCompletions {};

		using State = std::variant<Idle, Completing, NoCompletions>;
		State State_;

		bool InsertingCompletion_ = false;
	public:
		explicit MsgEditAutocompleter (const QString& entryId, QTextEdit& msgEdit);
	protected:
		bool eventFilter (QObject*, QEvent*) override;
	private:
		void Complete ();
		Completing OfferCompletion (Completing);
	};
}
