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

#include "entitywrapper.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Qrosp
		{
			EntityWrapper::EntityWrapper ()
			{
			}

			EntityWrapper::EntityWrapper (const EntityWrapper& ew)
			: E_ (ew.E_)
			{
			}

			EntityWrapper::EntityWrapper (const Entity& entity)
			: E_ (entity)
			{
			}

			Entity EntityWrapper::ToEntity () const
			{
				return E_;
			}

			void* EntityWrapper::wrappedObject () const
			{
				return const_cast<Entity*> (&E_);
			}

			const QVariant& EntityWrapper::GetEntity () const
			{
				return E_.Entity_;
			}

			void EntityWrapper::SetEntity (const QVariant& entity)
			{
				E_.Entity_ = entity;
			}

			const QString& EntityWrapper::GetLocation () const
			{
				return E_.Location_;
			}

			void EntityWrapper::SetLocation (const QString& location)
			{
				E_.Location_ = location;
			}

			const QString& EntityWrapper::GetMime () const
			{
				return E_.Mime_;
			}

			void EntityWrapper::SetMime (const QString& mime)
			{
				E_.Mime_ = mime;
			}

			const TaskParameters& EntityWrapper::GetParameters () const
			{
				return E_.Parameters_;
			}

			void EntityWrapper::SetParameters (const TaskParameters& tp)
			{
				E_.Parameters_ = tp;
			}

			const QVariantMap& EntityWrapper::GetAdditional () const
			{
				return E_.Additional_;
			}

			void EntityWrapper::SetAdditional (const QVariantMap& additional)
			{
				E_.Additional_ = additional;
			}

			QVariant EntityHandler (void *ptr)
			{
				Entity e = *static_cast<Entity*> (ptr);
				EntityWrapper *w = new EntityWrapper (e);
				return QVariant::fromValue<QObject*> (w);
			}
		};
	};
};
