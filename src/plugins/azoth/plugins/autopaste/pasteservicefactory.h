/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <functional>
#include <QList>
#include <QIcon>
#include <QString>
#include <interfaces/core/icoreproxy.h>

namespace LC::Azoth::Autopaste
{
	class PasteServiceBase;

	class PasteServiceFactory
	{
	public:
		typedef std::function<PasteServiceBase* (QObject*, ICoreProxy_ptr)> Creator_f;
		struct PasteInfo
		{
			QString Name_;
			QIcon Icon_;
			Creator_f Creator_;
		};
	private:
		QList<PasteInfo> Infos_;
	public:
		PasteServiceFactory ();

		QList<PasteInfo> GetInfos () const;
	};
}
