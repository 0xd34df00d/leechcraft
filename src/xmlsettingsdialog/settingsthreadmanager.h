/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <memory>
#include <QObject>

namespace LC
{
namespace Util
{
	class BaseSettingsManager;
}

	class SettingsThread;

	class SettingsThreadManager : public QObject
	{
		Q_OBJECT

		QThread * const Thread_;
		const std::shared_ptr<SettingsThread> Worker_;

		SettingsThreadManager ();
	public:
		SettingsThreadManager (const SettingsThreadManager&) = delete;
		SettingsThreadManager& operator= (const SettingsThreadManager&) = delete;

		~SettingsThreadManager ();

		static SettingsThreadManager& Instance ();

		void Add (Util::BaseSettingsManager*,
				const QString& name, const QVariant& value);
	};
}
