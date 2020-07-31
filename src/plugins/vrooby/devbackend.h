/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <interfaces/core/icoreproxyfwd.h>
#include <interfaces/devices/iremovabledevmanager.h>

namespace LC
{
struct Entity;
struct DeviceInfo;

namespace Vrooby
{
	class DevBackend : public QObject
					 , public IRemovableDevManager
	{
		Q_OBJECT
		Q_INTERFACES (IRemovableDevManager)

		const ICoreProxy_ptr Proxy_;
	public:
		DevBackend (const ICoreProxy_ptr&);

		virtual QString GetBackendName () const = 0;
		virtual bool IsAvailable () = 0;
		virtual void Start () = 0;
	protected:
		void HandleEntity (const Entity&);
	public slots:
		virtual void toggleMount (const QString&) = 0;
	};
}
}
