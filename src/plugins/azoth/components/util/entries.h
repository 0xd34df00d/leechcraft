/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QStringList>

class QObject;

namespace LC::Azoth
{
	class IAccount;
	class ICLEntry;
	class ISupportPGP;

	void AuthorizeEntry (ICLEntry*);
	void DenyAuthForEntry (ICLEntry*);

	QObject* FindByHRId (IAccount*, const QString&);

	bool ChoosePGPKey (ISupportPGP*, ICLEntry*);

	QStringList GetMucParticipants (const QString& mucEntryId);
}
