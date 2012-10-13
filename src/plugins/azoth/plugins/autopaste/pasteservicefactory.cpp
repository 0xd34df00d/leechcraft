/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2012  Georg Rudoy
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

#include "pasteservicefactory.h"
#include <QIcon>
#include "codepadservice.h"
#include "bpasteservice.h"

namespace LeechCraft
{
namespace Azoth
{
namespace Autopaste
{
	PasteServiceFactory::PasteServiceFactory ()
	{
		Infos_.push_back ({ "bpaste.net", QIcon (), [] (QObject *entry) { return new BPasteService (entry); } });
		Infos_.push_back ({ "codepad.org", QIcon (), [] (QObject *entry) { return new CodepadService (entry); } });
	}

	QList<PasteServiceFactory::PasteInfo> PasteServiceFactory::GetInfos () const
	{
		return Infos_;
	}
}
}
}
