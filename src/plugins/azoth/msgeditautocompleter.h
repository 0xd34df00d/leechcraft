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

namespace LC
{
namespace Azoth
{
	struct NickReplacement;

	class MsgEditAutocompleter : public QObject
	{
		Q_OBJECT

		const QString EntryId_;
		QTextEdit * const MsgEdit_;

		QStringList AvailableNickList_;
		int CurrentNickIndex_ = 0;
		int LastSpacePosition_ = -1;
		QString NickFirstPart_;
	public:
		MsgEditAutocompleter (const QString& entryId, QTextEdit *msgEdit, QObject* = nullptr);
	private:
		QStringList GetPossibleCompletions (const QString& firstPart, int position) const;
		QStringList GetCommandCompletions (int position) const;
		QStringList GetNickCompletions (int position) const;
		NickReplacement GetNickFromState (const QStringList& participants);
	public slots:
		void complete ();
		void resetState ();
	};
}
}
