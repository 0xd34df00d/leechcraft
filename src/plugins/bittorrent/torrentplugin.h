/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <memory>
#include <interfaces/iinfo.h>
#include <interfaces/idownload.h>
#include <interfaces/ijobholder.h>
#include <interfaces/itaggablejobs.h>
#include <interfaces/ihavesettings.h>
#include <interfaces/ihaveshortcuts.h>
#include <interfaces/ihavetabs.h>
#include <interfaces/istartupwizard.h>
#include <interfaces/iactionsexporter.h>
#include <interfaces/ihavediaginfo.h>
#include <xmlsettingsdialog/xmlsettingsdialog.h>
#include "tabwidget.h"

class QTimer;
class QToolBar;
class QComboBox;
class QTabWidget;
class QTranslator;
class QSortFilterProxyModel;

namespace LC::BitTorrent
{
	class AddTorrent;
	class ListActions;
	class SpeedSelectorAction;
	class TorrentTab;

	class TorrentPlugin : public QObject
						, public IInfo
						, public IDownload
						, public IJobHolder
						, public ITaggableJobs
						, public IHaveSettings
						, public IHaveShortcuts
						, public IHaveTabs
						, public IStartupWizard
						, public IActionsExporter
						, public IHaveDiagInfo
	{
		Q_OBJECT

		Q_INTERFACES (IInfo
				IDownload
				IJobHolder
				ITaggableJobs
				IHaveSettings
				IHaveShortcuts
				IHaveTabs
				IStartupWizard
				IActionsExporter
				IHaveDiagInfo)

		LC_PLUGIN_METADATA ("org.LeechCraft.BitTorrent")

		std::shared_ptr<LC::Util::XmlSettingsDialog> XmlSettingsDialog_;
		std::unique_ptr<TabWidget> TabWidget_;
		ListActions *Actions_;

		SpeedSelectorAction *DownSelectorAction_;
		SpeedSelectorAction *UpSelectorAction_;

		TabClassInfo TabTC_;
		TorrentTab *TorrentTab_;

		QSortFilterProxyModel *ReprProxy_;
	public:
		// IInfo
		void Init (ICoreProxy_ptr) override;
		void SecondInit () override;
		QByteArray GetUniqueID () const override;
		QString GetName () const override;
		QString GetInfo () const override;
		QStringList Provides () const override;
		void Release () override;
		QIcon GetIcon () const override;

		// IDownload
		qint64 GetDownloadSpeed () const override;
		qint64 GetUploadSpeed () const override;
		EntityTestHandleResult CouldDownload (const LC::Entity&) const override;
		QFuture<Result> AddJob (LC::Entity) override;

		// IJobHolder
		QAbstractItemModel* GetRepresentation () const override;
		IJobHolderRepresentationHandler_ptr CreateRepresentationHandler () override;

		// ITaggableJobs
		void SetTags (int, const QStringList&) override;

		// IHaveSettings
		Util::XmlSettingsDialog_ptr GetSettingsDialog () const override;

		// IHaveShortcuts
		void SetShortcut (const QByteArray&, const QKeySequences_t&) override;
		QMap<QByteArray, ActionInfo> GetActionInfo () const override;

		// IHaveTabs
		TabClasses_t GetTabClasses () const override;
		void TabOpenRequested (const QByteArray&) override;

		// IStartupWizard
		QList<QWizardPage*> GetWizardPages () const override;

		// IToolBarEmbedder
		QList<QAction*> GetActions (ActionsEmbedPlace) const override;

		// IHaveDiagInfo
		QString GetDiagInfoString () const override;
	private:
		void SetupCore ();
		void SetupStuff ();
		void SetupActions ();
	signals:
		void gotActions (QList<QAction*>, LC::ActionsEmbedPlace) override;
	};
}
