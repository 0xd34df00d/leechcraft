/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "entitywrapper.h"

namespace LC
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

	void* EntityWrapper::wrappedObject () const
	{
		return &E_;
	}

	Entity EntityWrapper::Native () const
	{
		return E_;
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
}
}
