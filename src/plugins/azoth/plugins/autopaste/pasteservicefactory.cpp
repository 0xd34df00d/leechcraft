/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "pasteservicefactory.h"
#include <QIcon>
#include "codepadservice.h"
#include "bpasteservice.h"
#include "hastebinservice.h"
#include "pasteorgruservice.h"

namespace LC::Azoth::Autopaste
{
	namespace
	{
		template<typename T>
		T* Creator (QObject *entry, const ICoreProxy_ptr& proxy)
		{
			return new T { entry, proxy };
		}
	}

	PasteServiceFactory::PasteServiceFactory ()
	{
		Infos_.push_back ({ "bpaste.net", QIcon (), &Creator<BPasteService> });
		Infos_.push_back ({ "codepad.org", QIcon (), &Creator<CodepadService> });
		Infos_.push_back ({ "paste.org.ru", QIcon (), &Creator<PasteOrgRuService> });
		Infos_.push_back ({ "hastebin.com", QIcon (), &Creator<HastebinService> });
	}

	QList<PasteServiceFactory::PasteInfo> PasteServiceFactory::GetInfos () const
	{
		return Infos_;
	}
}
