/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QSet>
#include <QString>

class QObject;

namespace LC
{
namespace Azoth
{
class ICLEntry;

namespace ChatHistory
{
	class LoggingStateKeeper
	{
		QSet<QString> DisabledIDs_;
	public:
		LoggingStateKeeper ();

		bool IsLoggingEnabled (ICLEntry*) const;
		void SetLoggingEnabled (ICLEntry*, bool);
	private:
		void LoadDisabled ();
		void SaveDisabled ();
	};
}
}
}
