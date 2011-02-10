/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2011  Georg Rudoy
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

#include "core.h"
#include <QtDebug>
#include "storage.h"

namespace LeechCraft
{
namespace Azoth
{
namespace ChatHistory
{
	boost::weak_ptr<Core> Core::InstPtr_;

	Core::Core ()
	: Storage_ (new Storage)
	{
	}
	
	boost::shared_ptr<Core> Core::Instance ()
	{
		if (InstPtr_.expired ())
		{
			boost::shared_ptr<Core> ptr (new Core);
			InstPtr_ = ptr;
			return ptr;
		}
		return InstPtr_.lock ();
	}
	
	void Core::Process (IMessage *msg)
	{
		Storage_->AddMessage (msg);
	}
}
}
}
