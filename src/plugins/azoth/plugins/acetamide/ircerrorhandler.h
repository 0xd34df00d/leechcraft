/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2011  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/


#ifndef PLUGINS_AZOTH_PLUGINS_ACETAMIDE_IRCERRORHANDLER_H
#define PLUGINS_AZOTH_PLUGINS_ACETAMIDE_IRCERRORHANDLER_H

#include <QObject>
#include <QVector>
#include "localtypes.h"

namespace LC
{
namespace Azoth
{
namespace Acetamide
{

	class IrcServerHandler;

	class IrcErrorHandler : public QObject
	{
		Q_OBJECT

		QVector<int> ErrorKeys_;
	public:
		IrcErrorHandler (QObject *parent);
		void HandleError (const IrcMessageOptions& opts);
		bool IsError (int code);
	private:
		void InitErrors ();
	};
};
};
};

#endif // PLUGINS_AZOTH_PLUGINS_ACETAMIDE_IRCERRORHANDLER_H
