/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QWidget>
#include "ui_devicesbrowserwidget.h"

class IRemovableDevManager;

namespace LC
{
namespace Util
{
	class MergeModel;
}

namespace LMP
{
	class ISyncPlugin;
	class UploadModel;
	class UnmountableDevManager;

	class DevicesBrowserWidget : public QWidget
	{
		Q_OBJECT

		Ui::DevicesBrowserWidget Ui_;
		UploadModel *DevUploadModel_;

		Util::MergeModel *Merger_;
		UnmountableDevManager *UnmountableMgr_;
		QMap<QAbstractItemModel*, IRemovableDevManager*> Flattener2DevMgr_;

		ISyncPlugin *CurrentSyncer_ = nullptr;

		QString LastDevice_;
		QMap<QString, TranscodingParams> Device2Params_;
	public:
		DevicesBrowserWidget (QWidget* = nullptr);

		void InitializeDevices ();
	private:
		void LoadLastParams ();
		void SaveLastParams () const;

		void UploadMountable (int);
		void UploadUnmountable (int);
		void HandleMountableSelected (int);
		void HandleUnmountableSelected (int);
	private slots:
		void handleDevDataChanged (const QModelIndex&, const QModelIndex&);
		void handleRowsInserted (const QModelIndex&, int, int);
		void on_UploadButton__released ();
		void on_RefreshButton__released ();
		void on_DevicesSelector__activated (int);
		void on_MountButton__released ();

		void appendUpLog (QString);

		void handleTranscodingProgress (int, int);
		void handleUploadProgress (int, int);
		void handleSingleUploadProgress (int, int);
	};
}
}
