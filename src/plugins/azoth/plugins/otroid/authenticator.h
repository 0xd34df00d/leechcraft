/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <functional>
#include <QObject>

typedef struct context ConnContext;    /* Forward declare */

namespace LC
{
namespace Azoth
{
class ICLEntry;

namespace OTRoid
{
	enum class SmpMethod
	{
		Question,
		SharedSecret
	};

	class Authenticator : public QObject
	{
		Q_OBJECT

		ICLEntry * const Entry_;
		const QString HrId_;
		const QString Name_;
	public:
		Authenticator (ICLEntry*);
		~Authenticator ();

		void AskFor (SmpMethod, const QString&, ConnContext*);
		void Initiate ();

		void Failed ();
		void Cheated ();
		void Success ();
	signals:
		void gotReply (SmpMethod, const QString&, ConnContext*);
		void abortSmp (ConnContext*);
		void initiateRequested (ICLEntry*, SmpMethod, const QString&, const QString&);

		void destroyingAuth (ICLEntry*);
	};
}
}
}
