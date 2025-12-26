/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <interfaces/devices/iremovabledevmanager.h>

namespace LC::Vrooby
{
	class DevBackend : public QObject
					 , public IRemovableDevManager
	{
		Q_OBJECT
		Q_INTERFACES (IRemovableDevManager)
	public:
		virtual void Start () = 0;
	public slots:
		virtual void toggleMount (const QString&) = 0;
	};

	template<typename T>
	concept DevBackendType = std::is_base_of_v<DevBackend, T> && requires
	{
		{ T::IsAvailable () } -> std::same_as<bool>;
		{ T::GetBackendName () } -> std::same_as<QString>;
	};
}
