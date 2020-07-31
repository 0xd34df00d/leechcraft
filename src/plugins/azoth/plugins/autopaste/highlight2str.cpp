/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "highlight2str.h"
#include <QtDebug>

namespace LC::Azoth::Autopaste::HlConverters
{
	QByteArray SpacePaste (Highlight high)
	{
		switch (high)
		{
		case Highlight::CPP:
			return "cpp";
		case Highlight::C:
			return "c";
		case Highlight::XML:
			return "xml";
		case Highlight::Haskell:
			return "haskell";
		case Highlight::Java:
			return "java";
		case Highlight::Python:
			return "python";
		case Highlight::Shell:
			return "shell";
		case Highlight::None:
			return "text";
		}

		qWarning () << Q_FUNC_INFO
				<< "unknown highlighting"
				<< static_cast<int> (high);
		return "text";
	}
}
