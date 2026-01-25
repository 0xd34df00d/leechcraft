/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <QIODevice>
#include "interfaces/azoth/imediacall.h"

namespace LC::Azoth::Emitters
{
	class MediaCall : public QObject
	{
		Q_OBJECT
	public:
		using QObject::QObject;

		void stateChanged (IMediaCall::State);
		void audioModeChanged (QIODevice::OpenMode);
		void readFormatChanged ();
		void writeFormatChanged ();
	};
}
