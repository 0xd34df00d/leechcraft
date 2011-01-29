/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2010  Georg Rudoy
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

#include "defaulthookproxy.h"

namespace LeechCraft
{
	namespace Util
	{
		DefaultHookProxy::DefaultHookProxy ()
		: Cancelled_ (false)
		{
		}

		void DefaultHookProxy::CancelDefault ()
		{
			Cancelled_ = true;
		}

		bool DefaultHookProxy::IsCancelled () const
		{
			return Cancelled_;
		}

		const QVariant& DefaultHookProxy::GetReturnValue () const
		{
			return ReturnValue_;
		}

		void DefaultHookProxy::SetReturnValue (const QVariant& val)
		{
			ReturnValue_ = val;
		}

		void DefaultHookProxy::SetValue (const QByteArray& name, const QVariant& val)
		{
			Name2NewVal_ [name] = val;
		}
	};
};
