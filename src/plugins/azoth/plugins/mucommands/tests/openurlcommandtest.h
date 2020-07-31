/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>

namespace LC
{
namespace Azoth
{
namespace MuCommands
{
	class OpenUrlCommandTest : public QObject
	{
		Q_OBJECT
	private slots:
		void parseSinceLast ();
		void parseLast ();

		void parseAll ();

		void parseByIndex ();

		void parseByRange ();
		void parseByLeftOpenRange ();
		void parseByRightOpenRange ();
		void parseByFullOpenRange ();

		void parseByRx ();
		void parseByRxSpaces ();

		void parseByRxRanged ();
		void parseByRxSpacesRanged ();
		void parseByRxLast ();

		void failLeftover ();
		void failLeftoverLast ();
	};
}
}
}

