/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QWidget>
#include "ui_cloudwidget.h"

namespace LC
{
namespace LMP
{
	class UploadModel;

	class CloudWidget : public QWidget
	{
		Q_OBJECT

		Ui::CloudWidget Ui_;
		UploadModel *DevUploadModel_;
		QObjectList Clouds_;
	public:
		CloudWidget (QWidget* = 0);
	private slots:
		void on_CloudSelector__activated (int);
		void handleCloudStoragePlugins ();
		void handleAccountsChanged ();

		void on_UploadButton__released ();

		void appendUpLog (QString);
		void handleTranscodingProgress (int, int);
		void handleUploadProgress (int, int);
	};
}
}
