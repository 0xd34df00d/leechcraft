/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <memory>
#include <deque>
#include <QMainWindow>
#include <QToolBar>
#include <interfaces/iinfo.h>
#include <interfaces/idownload.h>
#include <interfaces/ientityhandler.h>
#include <interfaces/ijobholder.h>
#include <interfaces/iimportexport.h>
#include <interfaces/itaggablejobs.h>
#include <interfaces/ihavesettings.h>
#include <interfaces/ihaveshortcuts.h>
#include <interfaces/ihavetabs.h>
#include <interfaces/istartupwizard.h>
#include <interfaces/iactionsexporter.h>
#include <interfaces/ihavediaginfo.h>
#include <util/tags/tagscompleter.h>
#include <xmlsettingsdialog/xmlsettingsdialog.h>
#include "tabwidget.h"
#include "torrentinfo.h"

class QTimer;
class QToolBar;
class QComboBox;
class QTabWidget;
class QTranslator;
class QSortFilterProxyModel;

namespace LC
{
namespace BitTorrent
{
	class AddTorrent;
	class RepresentationModel;
	class SpeedSelectorAction;
	class TorrentTab;

	class TorrentPlugin : public QObject
						, public IInfo
						, public IDownload
						, public IJobHolder
						, public IImportExport
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
				IImportExport
				ITaggableJobs
				IHaveSettings
				IHaveShortcuts
				IHaveTabs
				IStartupWizard
				IActionsExporter
				IHaveDiagInfo)

		LC_PLUGIN_METADATA ("org.LeechCraft.BitTorrent")

		ICoreProxy_ptr Proxy_;

		std::shared_ptr<LC::Util::XmlSettingsDialog> XmlSettingsDialog_;
		std::unique_ptr<AddTorrent> AddTorrentDialog_;
		std::unique_ptr<LC::Util::TagsCompleter> TagsAddDiaCompleter_;
		std::unique_ptr<TabWidget> TabWidget_;
		std::unique_ptr<QToolBar> Toolbar_;
		std::unique_ptr<QAction> OpenTorrent_,
			RemoveTorrent_,
			Resume_,
			Stop_,
			CreateTorrent_,
			MoveUp_,
			MoveDown_,
			MoveToTop_,
			MoveToBottom_,
			ForceReannounce_,
			ForceRecheck_,
			OpenMultipleTorrents_,
			IPFilter_,
			MoveFiles_,
			ChangeTrackers_,
			MakeMagnetLink_,
			OpenInTorrentTab_;

		SpeedSelectorAction *DownSelectorAction_,
				*UpSelectorAction_;

		enum
		{
			EAOpenTorrent_,
			EAChangeTrackers_,
			EACreateTorrent_,
			EAOpenMultipleTorrents_,
			EARemoveTorrent_,
			EAResume_,
			EAStop_,
			EAMoveUp_,
			EAMoveDown_,
			EAMoveToTop_,
			EAMoveToBottom_,
			EAForceReannounce_,
			EAForceRecheck_,
			EAMoveFiles_,
			EAImport_,
			EAExport_,
			EAMakeMagnetLink_,
			EAIPFilter_
		};

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
		void StartAll () override;
		void StopAll () override;
		EntityTestHandleResult CouldDownload (const LC::Entity&) const override;
		QFuture<Result> AddJob (LC::Entity) override;

		// IJobHolder
		QAbstractItemModel* GetRepresentation () const override;
		IJobHolderRepresentationHandler_ptr CreateRepresentationHandler () override;

		// IImportExport
		void ImportSettings (const QByteArray&) override;
		void ImportData (const QByteArray&) override;
		QByteArray ExportSettings () const override;
		QByteArray ExportData () const override;

		// ITaggableJobs
		void SetTags (int, const QStringList&) override;

		// IHaveSettings
		Util::XmlSettingsDialog_ptr GetSettingsDialog () const override;

		// IHaveShortcuts
		void SetShortcut (const QString&, const QKeySequences_t&) override;
		QMap<QString, ActionInfo> GetActionInfo () const override;

		// IHaveTabs
		TabClasses_t GetTabClasses () const override;
		void TabOpenRequested (const QByteArray&) override;

		// IStartupWizard
		QList<QWizardPage*> GetWizardPages () const override;

		// IToolBarEmbedder
		QList<QAction*> GetActions (ActionsEmbedPlace) const override;

		// IHaveDiagInfo
		QString GetDiagInfoString () const override;
	private slots:
		void on_OpenTorrent__triggered ();
		void on_OpenMultipleTorrents__triggered ();
		void on_IPFilter__triggered ();
		void on_CreateTorrent__triggered ();
		void on_RemoveTorrent__triggered ();
		void on_Resume__triggered ();
		void on_Stop__triggered ();
		void on_MoveUp__triggered ();
		void on_MoveDown__triggered ();
		void on_MoveToTop__triggered ();
		void on_MoveToBottom__triggered ();
		void on_ForceReannounce__triggered ();
		void on_ForceRecheck__triggered ();
		void on_ChangeTrackers__triggered ();
		void on_MoveFiles__triggered ();
		void on_MakeMagnetLink__triggered ();
		void on_OpenInTorrentTab__triggered ();
		void handleFastSpeedComboboxes ();
		void setActionsEnabled ();
	private:
		void SetupCore ();
		void SetupStuff ();
		void SetupActions ();
	signals:
		void addNewTab (const QString&, QWidget*) override;
		void changeTabIcon (QWidget*, const QIcon&) override;
		void changeTabName (QWidget*, const QString&) override;
		void raiseTab (QWidget*) override;
		void removeTab (QWidget*) override;
		void statusBarChanged (QWidget*, const QString&) override;

		void gotActions (QList<QAction*>, LC::ActionsEmbedPlace) override;
	};
}
}
