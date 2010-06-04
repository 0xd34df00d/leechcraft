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

#ifndef PLUGINS_QROSP_WRAPPERS_ENTITYWRAPPER_H
#define PLUGINS_QROSP_WRAPPERS_ENTITYWRAPPER_H
#include <QObject>
#include <qross/core/wrapperinterface.h>
#include <interfaces/structures.h>

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Qrosp
		{
			class EntityWrapper : public QObject, public Qross::WrapperInterface
			{
				Q_OBJECT

				Entity E_;

				Q_PROPERTY (QVariant Entity READ GetEntity WRITE SetEntity);
				Q_PROPERTY (QString Location READ GetLocation WRITE SetLocation);
				Q_PROPERTY (QString Mime READ GetMime WRITE SetMime);
				Q_PROPERTY (TaskParameters Parameters READ GetParameters WRITE SetParameters);
				Q_PROPERTY (QVariantMap Additional READ GetAdditional WRITE SetAdditional);
			public:
				EntityWrapper ();
				EntityWrapper (const EntityWrapper&);
				EntityWrapper (const Entity&);

				void* wrappedObject () const;
			public slots:
				Entity ToEntity () const;
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
		};
	};
};

Q_DECLARE_METATYPE(LeechCraft::Plugins::Qrosp::EntityWrapper);

#endif
