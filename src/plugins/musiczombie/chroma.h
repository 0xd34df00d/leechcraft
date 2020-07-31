/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <memory>
#include <QByteArray>
#include <QMutex>

class QString;

using ChromaprintContext = struct ChromaprintContextPrivate;

namespace LC
{
namespace MusicZombie
{
	class Chroma
	{
		std::shared_ptr<ChromaprintContext> Ctx_;

		static QMutex CodecMutex_;
	public:
		struct Result
		{
			QByteArray FP_;
			int Duration_;
		};

		Chroma ();

		Chroma (const Chroma&) = delete;
		Chroma (Chroma&&) = delete;

		Result operator() (const QString&);
	};
}
}
