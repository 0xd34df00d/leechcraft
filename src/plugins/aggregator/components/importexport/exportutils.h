/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

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

	struct LastPathParams
	{
		QLineEdit& Edit_;
		const char *SettingName_;
		QString DefaultPath_;
		QDialog& Parent_;
	};

	void ManageLastPath (const LastPathParams& params);
}
