/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <functional>
#include <QString>

class QAbstractItemModel;
class QDialog;
class QLineEdit;
class QWidget;

namespace LC::Aggregator
{
	class ChannelsModel;

	void RunExportItems (ChannelsModel&, QWidget* = nullptr);
	void RunExportChannels (QAbstractItemModel&, QWidget* = nullptr);

	struct PathEdit
	{
		std::function<QString ()> GetPath_;
		std::function<void (QString)> SetPath_;

		template<typename T>
		PathEdit (T *context, QString (T::*get) () const, void (T::*set) (const QString&))
		: PathEdit { std::bind_front (get, context), std::bind_front (set, context) }
		{
		}

		PathEdit (std::function<QString ()> get, std::function<void (QString)> set);

		explicit (false) PathEdit (QLineEdit& edit);
	};

	struct LastPathParams
	{
		PathEdit Edit_;
		const char *SettingName_;
		QString DefaultPath_;
		QDialog& Parent_;
	};

	void ManageLastPath (const LastPathParams& params);
}
