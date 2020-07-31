/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#ifndef PLUGINS_QROSP_WRAPPERS_ENTITYWRAPPER_H
#define PLUGINS_QROSP_WRAPPERS_ENTITYWRAPPER_H
#include <QObject>
#include <qross/core/wrapperinterface.h>
#include <interfaces/structures.h>

namespace LC
{
namespace Qrosp
{
	class EntityWrapper : public QObject, public Qross::WrapperInterface
	{
		Q_OBJECT

		mutable Entity E_;

		Q_PROPERTY (QVariant Entity READ GetEntity WRITE SetEntity)
		Q_PROPERTY (QString Location READ GetLocation WRITE SetLocation)
		Q_PROPERTY (QString Mime READ GetMime WRITE SetMime)
		Q_PROPERTY (TaskParameters Parameters READ GetParameters WRITE SetParameters)
		Q_PROPERTY (QVariantMap Additional READ GetAdditional WRITE SetAdditional)
	public:
		EntityWrapper ();
		EntityWrapper (const EntityWrapper&);
		EntityWrapper (const Entity&);

		void* wrappedObject () const;
	public slots:
		LC::Entity Native () const;
		const QVariant& GetEntity () const;
		void SetEntity (const QVariant&);
		const QString& GetLocation () const;
		void SetLocation (const QString&);
		const QString& GetMime () const;
		void SetMime (const QString&);
		const TaskParameters& GetParameters () const;
		void SetParameters (const TaskParameters&);
		const QVariantMap& GetAdditional () const;
		void SetAdditional (const QVariantMap&);
	};

	QVariant EntityHandler (void*);
}
}

Q_DECLARE_METATYPE (LC::Qrosp::EntityWrapper)

#endif
