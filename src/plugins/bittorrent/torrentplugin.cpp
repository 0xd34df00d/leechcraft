/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2015  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "torrentplugin.h"
#include <QMessageBox>
#include <QUrl>
#include <QTemporaryFile>
#include <QtDebug>
#include <QDir>
#include <QMenu>
#include <QAction>
#include <QTabWidget>
#include <QTranslator>
#include <QTextCodec>
#include <QTimer>
#include <QToolBar>
#include <QHeaderView>
#include <QInputDialog>
#include <QSortFilterProxyModel>
#include <QUrlQuery>
#include <boost/preprocessor/seq/size.hpp>
#include <boost/preprocessor/seq/elem.hpp>
#include <boost/preprocessor/repetition/repeat.hpp>
#include <libtorrent/session.hpp>
#include <libtorrent/announce_entry.hpp>
#include <interfaces/entitytesthandleresult.h>
#include <interfaces/ijobholderrepresentationhandler.h>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/itagsmanager.h>
#include <interfaces/core/irootwindowsmanager.h>
#include <interfaces/core/ientitymanager.h>
#include <interfaces/core/iiconthememanager.h>
#include <util/tags/tagscompletionmodel.h>
#include <util/tags/tagscompleter.h>
#include <util/util.h>
#include <util/xpc/util.h>
#include <util/shortcuts/shortcutmanager.h>
#include <util/threads/futures.h>
#include "core.h"
#include "addtorrent.h"
#include "addmultipletorrents.h"
#include "newtorrentwizard.h"
#include "xmlsettingsmanager.h"
#include "movetorrentfiles.h"
#include "trackerschanger.h"
#include "wizardgenerator.h"
#include "fastspeedcontrolwidget.h"
#include "ipfilterdialog.h"
#include "speedselectoraction.h"
#include "torrenttab.h"
#include "sessionsettingsmanager.h"
#include "sessionstats.h"
#include "types.h"
#include "newtorrentparams.h"

using LC::ActionInfo;
using namespace LC::Util;

namespace LC
{
namespace BitTorrent
{
	namespace
	{
		class ReprProxy final : public QSortFilterProxyModel
		{
		public:
			explicit ReprProxy (QAbstractItemModel *model)
			: QSortFilterProxyModel (model)
			{
				setDynamicSortFilter (true);
				setSourceModel (model);
			}

			QVariant data (const QModelIndex& unmapped, int role) const override
			{
				const auto& index = mapToSource (unmapped);
				const int normCol = index.column ();

				if (normCol == Core::ColumnProgress && role == Qt::DisplayRole)
					return sourceModel ()->data (index, Core::Roles::FullLengthText);
				else
					return QSortFilterProxyModel::data (unmapped, role);
			}
		protected:
			bool filterAcceptsColumn (int sourceColumn, const QModelIndex&) const override
			{
				return sourceColumn >= Core::Columns::ColumnName &&
						sourceColumn <= Core::Columns::ColumnProgress;
			}
		};
	}

	void TorrentPlugin::Init (ICoreProxy_ptr proxy)
	{
		Proxy_ = proxy;

		InstallTranslator ("bittorrent");
		Core::Instance ()->SetProxy (proxy);

		TabTC_ =
		{
			GetUniqueID () + "_TorrentTab",
			tr ("BitTorrent tab"),
			tr ("Full BitTorrent downloads tab."),
			GetIcon (),
			10,
			TFSingle | TFOpenableByRequest | TFSuggestOpening
		};

		SetupCore ();
		SetupStuff ();

		setActionsEnabled ();

		TorrentTab_ = new TorrentTab (TabTC_, this);
		connect (TorrentTab_,
				SIGNAL (removeTab (QWidget*)),
				this,
				SIGNAL (removeTab (QWidget*)));

		ReprProxy_ = new ReprProxy (Core::Instance ());
	}

	void TorrentPlugin::SecondInit ()
	{
	}

	QByteArray TorrentPlugin::GetUniqueID () const
	{
		return "org.LeechCraft.BitTorrent";
	}

	QString TorrentPlugin::GetName () const
	{
		return "BitTorrent";
	}

	QString TorrentPlugin::GetInfo () const
	{
		return tr ("Full-featured BitTorrent client.");
	}

	QStringList TorrentPlugin::Provides () const
	{
		return QStringList ("bittorrent") << "resume" << "remoteable";
	}

	void TorrentPlugin::Release ()
	{
		delete TorrentTab_;
		Core::Instance ()->Release ();
		XmlSettingsManager::Instance ()->Release ();
		XmlSettingsDialog_.reset ();
	}

	QIcon TorrentPlugin::GetIcon () const
	{
		return Proxy_->GetIconThemeManager ()->GetPluginIcon ();
	}

	qint64 TorrentPlugin::GetDownloadSpeed () const
	{
		return Core::Instance ()->GetSessionStats ().Rate_.Down_;
	}

	qint64 TorrentPlugin::GetUploadSpeed () const
	{
		return Core::Instance ()->GetSessionStats ().Rate_.Up_;
	}

	void TorrentPlugin::StartAll ()
	{
		int numTorrents = Core::Instance ()->columnCount (QModelIndex ());
		for (int i = 0; i < numTorrents; ++i)
			Core::Instance ()->ResumeTorrent (i);
		setActionsEnabled ();
	}

	void TorrentPlugin::StopAll ()
	{
		int numTorrents = Core::Instance ()->columnCount (QModelIndex ());
		for (int i = 0; i < numTorrents; ++i)
			Core::Instance ()->PauseTorrent (i);
	}

	EntityTestHandleResult TorrentPlugin::CouldDownload (const Entity& e) const
	{
		return Core::Instance ()->CouldDownload (e);
	}

	namespace
	{
		void ExecDialog (QDialog& dia)
		{
			dia.show ();
			QEventLoop dialogGuard;
			QObject::connect (&dia,
					&QDialog::finished,
					&dialogGuard,
					&QEventLoop::quit);
			dialogGuard.exec ();
		}
	}

	QFuture<IDownload::Result> TorrentPlugin::AddJob (Entity e)
	{
		QString suggestedFname;

		const auto tagsMgr = GetProxyHolder ()->GetTagsManager ();

		const auto& suggestedTags = e.Additional_ [" Tags"].toStringList ();
		const auto& autoTags = XmlSettingsManager::Instance ()->property ("AutomaticTags").toString ();
		auto tagsIds = tagsMgr->SplitToIDs (autoTags) + tagsMgr->GetIDs (suggestedTags);

		if (e.Entity_.canConvert<QUrl> ())
		{
			QUrl resource = e.Entity_.toUrl ();
			if (resource.scheme () == "magnet")
			{
				for (const auto& [key, value] : QUrlQuery { resource }.queryItems ())
					if (key == QStringLiteral ("kt"))
						tagsIds += tagsMgr->GetIDs (value.split ('+', Qt::SkipEmptyParts));

				return Core::Instance ()->AddMagnet (resource.toString (),
						e.Location_,
						tagsIds,
						e.Parameters_);
			}
			else if (resource.scheme () == "file")
				suggestedFname = resource.toLocalFile ();
		}
		else if (Core::Instance ()->IsValidTorrent (e.Entity_.toByteArray ()))
		{
			QTemporaryFile tmpFile ("lctemporarybittorrentfile.XXXXXX");
			tmpFile.write (e.Entity_.toByteArray ());
			suggestedFname = tmpFile.fileName ().toUtf8 ();
			tmpFile.setAutoRemove (false);
		}

		QFile file { suggestedFname };
		if (!file.open (QIODevice::ReadOnly))
		{
			qWarning () << Q_FUNC_INFO
					<< "unable to open file"
					<< suggestedFname
					<< file.errorString ();
			return Util::MakeReadyFuture (Result::Left ({ Error::Type::LocalError, "Unable to open file" }));
		}

		QString path;
		QVector<bool> files;
		QString fname;
		bool tryLive = e.Additional_ ["TryToStreamLive"].toBool ();
		if (e.Parameters_ & FromUserInitiated)
		{
			AddTorrent dia;
			dia.SetFilename (suggestedFname);
			if (!e.Location_.isEmpty ())
				dia.SetSavePath (e.Location_);
			else if (e.Parameters_ & IsDownloaded && !suggestedFname.isEmpty ())
				dia.SetSavePath (QFileInfo (suggestedFname).absolutePath ());

			if (!suggestedTags.isEmpty ())
				dia.SetTags (suggestedTags);

			ExecDialog (dia);

			if (dia.result () == QDialog::Rejected)
				return Util::MakeReadyFuture (Result::Left ({ Error::Type::UserCanceled, {} }));

			fname = dia.GetFilename (),
			path = dia.GetSavePath ();
			tryLive = dia.GetTryLive ();
			files = dia.GetSelectedFiles ();
			tagsIds = dia.GetTags ();
			if (dia.GetAddType () == AddState::Started)
				e.Parameters_ &= ~NoAutostart;
			else
				e.Parameters_ |= NoAutostart;
		}
		else
		{
			fname = suggestedFname;
			path = e.Location_;
		}
		auto result = Core::Instance ()->AddFile (fname,
				path,
				tagsIds,
				tryLive,
				files,
				e.Parameters_);
		setActionsEnabled ();
		file.remove ();
		return result;
	}

	QAbstractItemModel* TorrentPlugin::GetRepresentation () const
	{
		return ReprProxy_;
	}

	IJobHolderRepresentationHandler_ptr TorrentPlugin::CreateRepresentationHandler ()
	{
		class Handler : public IJobHolderRepresentationHandler
		{
			TorrentPlugin * const Plugin_;
		public:
			explicit Handler (TorrentPlugin *plugin)
			: Plugin_ { plugin }
			{
			}

			void HandleCurrentRowChanged (const QModelIndex& index) override
			{
				Core::Instance ()->SetCurrentTorrent (index.row ());
				if (index.isValid ())
					Plugin_->TabWidget_->InvalidateSelection ();

				Plugin_->setActionsEnabled ();
			}
		};

		return std::make_shared<Handler> (this);
	}

	void TorrentPlugin::ImportSettings (const QByteArray& settings)
	{
		XmlSettingsDialog_->MergeXml (settings);
	}

	void TorrentPlugin::ImportData (const QByteArray&)
	{
	}

	QByteArray TorrentPlugin::ExportSettings () const
	{
		return XmlSettingsDialog_->GetXml ().toUtf8 ();
	}

	QByteArray TorrentPlugin::ExportData () const
	{
		return QByteArray ();
	}

	void TorrentPlugin::SetTags (int torrent, const QStringList& tags)
	{
		Core::Instance ()->UpdateTags (tags, torrent);
	}

	XmlSettingsDialog_ptr TorrentPlugin::GetSettingsDialog () const
	{
		return XmlSettingsDialog_;
	}

	void TorrentPlugin::SetShortcut (const QString& name,
			const QKeySequences_t& shortcuts)
	{
		Core::Instance ()->GetShortcutManager ()->SetShortcut (name, shortcuts);
	}

	QMap<QString, ActionInfo> TorrentPlugin::GetActionInfo () const
	{
		return Core::Instance ()->GetShortcutManager ()->GetActionInfo ();
	}

	TabClasses_t TorrentPlugin::GetTabClasses () const
	{
		return { TabTC_ };
	}

	void TorrentPlugin::TabOpenRequested (const QByteArray& tc)
	{
		if (tc == TabTC_.TabClass_)
		{
			emit addNewTab ("BitTorrent", TorrentTab_);
			emit raiseTab (TorrentTab_);
		}
		else
			qWarning () << Q_FUNC_INFO
					<< "unknown tab class"
					<< tc;
	}

	QList<QWizardPage*> TorrentPlugin::GetWizardPages () const
	{
		return WizardGenerator {}.GetPages ();
	}

	QList<QAction*> TorrentPlugin::GetActions (ActionsEmbedPlace place) const
	{
		QList<QAction*> result;

		switch (place)
		{
		case ActionsEmbedPlace::CommonContextMenu:
			result += CreateTorrent_.get ();
			break;
		case ActionsEmbedPlace::ToolsMenu:
			result += OpenMultipleTorrents_.get ();
			result += IPFilter_.get ();
			break;
		default:
			break;
		}

		return result;
	}

	QString TorrentPlugin::GetDiagInfoString () const
	{
		return QStringLiteral ("Built with rb_libtorrent %1 (%2).")
				.arg (LIBTORRENT_VERSION, LIBTORRENT_REVISION);
	}

	void TorrentPlugin::on_OpenTorrent__triggered ()
	{
		AddTorrent dia;
		ExecDialog (dia);

		if (dia.result () == QDialog::Rejected)
			return;

		const auto& filename = dia.GetFilename ();
		const auto& path = dia.GetSavePath ();
		bool tryLive = dia.GetTryLive ();
		QVector<bool> files = dia.GetSelectedFiles ();
		QStringList tags = dia.GetTags ();
		TaskParameters tp = FromUserInitiated;
		if (dia.GetAddType () != AddState::Started)
			tp |= NoAutostart;
		Core::Instance ()->AddFile (filename,
				path,
				tags,
				tryLive,
				files,
				tp);
		setActionsEnabled ();
	}

	void TorrentPlugin::on_OpenMultipleTorrents__triggered ()
	{
		auto rootWM = Core::Instance ()->GetProxy ()->GetRootWindowsManager ();
		AddMultipleTorrents dialog (rootWM->GetPreferredWindow ());
		if (dialog.exec () == QDialog::Rejected)
			return;

		TaskParameters tp = FromUserInitiated;
		if (!dialog.ShouldAddAsStarted ())
			tp |= NoAutostart;

		QString savePath = dialog.GetSaveDirectory (),
				openPath = dialog.GetOpenDirectory ();
		QDir dir (openPath);
		QStringList names = dir.entryList (QStringList ("*.torrent"));
		QStringList tags = dialog.GetTags ();
		for (int i = 0; i < names.size (); ++i)
		{
			QString name = openPath;
			if (!name.endsWith ('/'))
				name += '/';
			name += names.at (i);
			Core::Instance ()->AddFile (name, savePath, tags, false);
		}
		setActionsEnabled ();
	}

	void TorrentPlugin::on_IPFilter__triggered ()
	{
		IPFilterDialog dia;
		if (dia.exec () != QDialog::Accepted)
			return;

		Core::Instance ()->ClearFilter ();

		const auto& filter = dia.GetFilter ();
		for (const auto& pair : filter)
			Core::Instance ()->BanPeers (pair.first, pair.second);
	}

	void TorrentPlugin::on_CreateTorrent__triggered ()
	{
		auto rootWM = Core::Instance ()->GetProxy ()->GetRootWindowsManager ();
		NewTorrentWizard wizard (rootWM->GetPreferredWindow ());
		if (wizard.exec () == QDialog::Accepted)
			Core::Instance ()->MakeTorrent (wizard.GetParams ());
		setActionsEnabled ();
	}

	void TorrentPlugin::on_RemoveTorrent__triggered ()
	{
		QModelIndexList sis;
		try
		{
			sis = Util::GetSummarySelectedRows (sender ());
		}
		catch (const std::runtime_error& e)
		{
			qWarning () << Q_FUNC_INFO
					<< e.what ();
			return;
		}

		QList<int> rows;
		for (const auto& si : sis)
		{
			auto mapped = Core::Instance ()->GetProxy ()->MapToSource (si);
			if (mapped.isValid ())
				rows << mapped.row ();
		}

		auto rootWM = Core::Instance ()->GetProxy ()->GetRootWindowsManager ();
		QMessageBox confirm (QMessageBox::Question,
				"LeechCraft BitTorrent",
				tr ("Do you really want to delete %n torrent(s)?", 0, rows.size ()),
				QMessageBox::Cancel,
				rootWM->GetPreferredWindow ());
		auto deleteTorrentsButton = confirm.addButton (tr ("&Delete"),
				QMessageBox::ActionRole);
		auto deleteTorrentsAndFilesButton = confirm.addButton (tr ("Delete with &files"),
				QMessageBox::ActionRole);
		confirm.setDefaultButton (QMessageBox::Cancel);

		confirm.exec ();

		bool withFiles = false;
		if (confirm.clickedButton () == deleteTorrentsAndFilesButton)
			withFiles = true;
		else if (confirm.clickedButton () == deleteTorrentsButton)
			;// do nothing
		else return;

		std::sort (rows.begin (), rows.end (), std::greater<> ());

		for (int row : rows)
			Core::Instance ()->RemoveTorrent (row, withFiles);
		TabWidget_->InvalidateSelection ();
		setActionsEnabled ();
	}

	void TorrentPlugin::on_Resume__triggered ()
	{
		QModelIndexList sis;
		try
		{
			sis = Util::GetSummarySelectedRows (sender ());
		}
		catch (const std::runtime_error& e)
		{
			qWarning () << Q_FUNC_INFO
					<< e.what ();
			return;
		}

		for (const auto& si : sis)
			Core::Instance ()->ResumeTorrent (Core::Instance ()->GetProxy ()->MapToSource (si).row ());
		setActionsEnabled ();
	}

	void TorrentPlugin::on_Stop__triggered ()
	{
		QModelIndexList sis;
		try
		{
			sis = Util::GetSummarySelectedRows (sender ());
		}
		catch (const std::runtime_error& e)
		{
			qWarning () << Q_FUNC_INFO
					<< e.what ();
			return;
		}

		for (const auto& si : sis)
			Core::Instance ()->PauseTorrent (Core::Instance ()->GetProxy ()->MapToSource (si).row ());
		setActionsEnabled ();
	}

	namespace
	{
		std::vector<int> GetSelections (QAbstractItemModel *model,
				QObject *sender)
		{
			QModelIndexList sis;
			try
			{
				sis = Util::GetSummarySelectedRows (sender);
			}
			catch (const std::runtime_error& e)
			{
				qWarning () << Q_FUNC_INFO
						<< e.what ();
				throw;
			}

			std::vector<int> selections;
			for (const auto& si : sis)
			{
				auto mapped = Core::Instance ()->GetProxy ()->MapToSource (si);
				if (mapped.model () != model)
					continue;
				selections.push_back (mapped.row ());
			}

			return selections;
		}
	};

	void TorrentPlugin::on_MoveUp__triggered ()
	{
		QModelIndexList sis;
		try
		{
			sis = Util::GetSummarySelectedRows (sender ());
		}
		catch (const std::runtime_error& e)
		{
			qWarning () << Q_FUNC_INFO
					<< e.what ();
			return;
		}

		std::vector<int> selections;
		try
		{
			selections = GetSelections (GetRepresentation (), sender ());
		}
		catch (const std::exception& e)
		{
			qWarning () << Q_FUNC_INFO
				<< e.what ();
			return;
		}

		Core::Instance ()->MoveUp (selections);

		QItemSelectionModel *sel = qobject_cast<QItemSelectionModel*> (sender ()->
				property ("ItemSelectionModel").value<QObject*> ());

		if (sel)
			sel->clearSelection ();

		QItemSelection selection;
		for (const auto& si : sis)
		{
			QModelIndex sibling = si.sibling (si.row () - 1, si.column ());
			if (Core::Instance ()->GetProxy ()->MapToSource (sibling).model () != GetRepresentation ())
				continue;

			selection.select (sibling, sibling);
		}

		if (sel)
			sel->select (selection, QItemSelectionModel::Rows |
					QItemSelectionModel::SelectCurrent);
	}

	void TorrentPlugin::on_MoveDown__triggered ()
	{
		QModelIndexList sis;
		try
		{
			sis = Util::GetSummarySelectedRows (sender ());
		}
		catch (const std::runtime_error& e)
		{
			qWarning () << Q_FUNC_INFO
					<< e.what ();
			return;
		}

		std::vector<int> selections;
		try
		{
			selections = GetSelections (GetRepresentation (), sender ());
		}
		catch (const std::exception& e)
		{
			qWarning () << Q_FUNC_INFO
				<< e.what ();
			return;
		}

		Core::Instance ()->MoveDown (selections);

		QItemSelectionModel *sel = qobject_cast<QItemSelectionModel*> (sender ()->
				property ("ItemSelectionModel").value<QObject*> ());

		if (sel)
			sel->clearSelection ();

		QItemSelection selection;
		for (const auto& si : sis)
		{
			QModelIndex sibling = si.sibling (si.row () + 1, si.column ());
			if (Core::Instance ()->GetProxy ()->MapToSource (sibling).model () != GetRepresentation ())
				continue;

			selection.select (sibling, sibling);
		}

		if (sel)
			sel->select (selection, QItemSelectionModel::Rows |
					QItemSelectionModel::SelectCurrent);
	}

	void TorrentPlugin::on_MoveToTop__triggered ()
	{
		try
		{
			Core::Instance ()->MoveToTop (GetSelections (GetRepresentation (),
					sender ()));
		}
		catch (const std::exception& e)
		{
			qWarning () << Q_FUNC_INFO
				<< e.what ();
			return;
		}
	}

	void TorrentPlugin::on_MoveToBottom__triggered ()
	{
		try
		{
			Core::Instance ()->MoveToBottom (GetSelections (GetRepresentation (),
					sender ()));
		}
		catch (const std::exception& e)
		{
			qWarning () << Q_FUNC_INFO
				<< e.what ();
			return;
		}
	}

	void TorrentPlugin::on_ForceReannounce__triggered ()
	{
		try
		{
			for (int torrent : GetSelections (GetRepresentation (), sender ()))
				Core::Instance ()->ForceReannounce (torrent);
		}
		catch (const std::exception& e)
		{
			qWarning () << Q_FUNC_INFO
				<< e.what ();
			return;
		}
	}

	void TorrentPlugin::on_ForceRecheck__triggered ()
	{
		try
		{
			for (int torrent : GetSelections (GetRepresentation (), sender ()))
				Core::Instance ()->ForceRecheck (torrent);
		}
		catch (const std::exception& e)
		{
			qWarning () << Q_FUNC_INFO
				<< e.what ();
			return;
		}
	}

	void TorrentPlugin::on_ChangeTrackers__triggered ()
	{
		QModelIndexList sis;
		try
		{
			sis = Util::GetSummarySelectedRows (sender ());
		}
		catch (const std::runtime_error& e)
		{
			qWarning () << Q_FUNC_INFO
					<< e.what ();
			return;
		}

		std::vector<libtorrent::announce_entry> allTrackers;
		for (const auto& si : sis)
		{
			auto those = Core::Instance ()->GetTrackers (Core::Instance ()->
					GetProxy ()->MapToSource (si).row ());
			std::copy (those.begin (), those.end (), std::back_inserter (allTrackers));
		}

		if (allTrackers.empty ())
			allTrackers = Core::Instance ()->
					GetTrackers (Core::Instance ()->
							GetCurrentTorrent ());

		std::stable_sort (allTrackers.begin (), allTrackers.end (),
				[] (const libtorrent::announce_entry& l, const libtorrent::announce_entry& r)
					{ return l.url < r.url; });

		auto newLast = std::unique (allTrackers.begin (), allTrackers.end (),
				[] (const libtorrent::announce_entry& l, const libtorrent::announce_entry& r)
					{ return l.url == r.url; });

		allTrackers.erase (newLast, allTrackers.end ());

		if (allTrackers.empty ())
			return;

		auto rootWM = Core::Instance ()->GetProxy ()->GetRootWindowsManager ();
		TrackersChanger changer (rootWM->GetPreferredWindow ());
		changer.SetTrackers (allTrackers);

		if (changer.exec () != QDialog::Accepted)
			return;

		const auto& trackers = changer.GetTrackers ();
		for (const auto& si : sis)
			Core::Instance ()->SetTrackers (trackers, Proxy_->MapToSource (si).row ());
	}

	void TorrentPlugin::on_MoveFiles__triggered ()
	{
		const auto& oldDir = Core::Instance ()->GetTorrentDirectory (Core::Instance ()->GetCurrentTorrent ());
		MoveTorrentFiles mtf { { oldDir } };
		if (mtf.exec () == QDialog::Rejected)
			return;

		const auto& newDir = mtf.GetNewLocation ();
		if (oldDir == newDir)
			return;

		if (!Core::Instance ()->MoveTorrentFiles (newDir, Core::Instance ()->GetCurrentTorrent ()))
		{
			const auto& msg = tr ("Failed to move torrent's files from %1 to %2")
					.arg (oldDir)
					.arg (newDir);

			const auto& e = Util::MakeNotification ("BitTorrent", msg, Priority::Critical);
			Proxy_->GetEntityManager ()->HandleEntity (e);
		}
	}

	void TorrentPlugin::on_MakeMagnetLink__triggered ()
	{
		QString magnet = Core::Instance ()->GetMagnetLink (Core::Instance ()->GetCurrentTorrent ());
		if (magnet.isEmpty ())
			return;

		QInputDialog *dia = new QInputDialog ();
		dia->setWindowTitle ("LeechCraft");
		dia->setLabelText (tr ("Magnet link:"));
		dia->setAttribute (Qt::WA_DeleteOnClose);
		dia->setInputMode (QInputDialog::TextInput);
		dia->setTextValue (magnet);
		dia->resize (700, dia->height ());
		dia->show ();
	}

	void TorrentPlugin::on_OpenInTorrentTab__triggered ()
	{
		const auto torrent = Core::Instance ()->GetCurrentTorrent ();
		if (torrent == -1)
			return;

		TorrentTab_->SetCurrentTorrent (torrent);
		TabOpenRequested (TabTC_.TabClass_);
	}

	void TorrentPlugin::handleFastSpeedComboboxes ()
	{
		const auto ssm = Core::Instance ()->GetSessionSettingsManager ();
		ssm->SetOverallDownloadRate (DownSelectorAction_->CurrentData ());
		ssm->SetOverallUploadRate (UpSelectorAction_->CurrentData ());
	}

	void TorrentPlugin::setActionsEnabled ()
	{
		int torrent = Core::Instance ()->GetCurrentTorrent ();
		bool isValid = false;
		if (torrent != -1)
			isValid = Core::Instance ()->CheckValidity (torrent);
		RemoveTorrent_->setEnabled (isValid);
		Stop_->setEnabled (isValid);
		Resume_->setEnabled (isValid);
		ForceReannounce_->setEnabled (isValid);
	}

	void TorrentPlugin::SetupCore ()
	{
		XmlSettingsDialog_.reset (new XmlSettingsDialog ());
		XmlSettingsDialog_->RegisterObject (XmlSettingsManager::Instance (),
				"torrentsettings.xml");

		Core::Instance ()->DoDelayedInit ();

		SetupActions ();
		TabWidget_.reset (new TabWidget);

		Core::Instance ()->SetWidgets (Toolbar_.get (), TabWidget_.get ());
	}

	void TorrentPlugin::SetupStuff ()
	{
		auto statsUpdateTimer = new QTimer { this };
		connect (statsUpdateTimer,
				SIGNAL (timeout ()),
				TabWidget_.get (),
				SLOT (updateTorrentStats ()));
		statsUpdateTimer->start (2000);

		FastSpeedControlWidget *fsc = new FastSpeedControlWidget ();
		XmlSettingsDialog_->SetCustomWidget ("FastSpeedControl", fsc);
		connect (fsc,
				SIGNAL (speedsChanged ()),
				DownSelectorAction_,
				SLOT (handleSpeedsChanged ()));
		connect (fsc,
				SIGNAL (speedsChanged ()),
				UpSelectorAction_,
				SLOT (handleSpeedsChanged ()));
		XmlSettingsManager::Instance ()->
			RegisterObject ("EnableFastSpeedControl",
				DownSelectorAction_, "handleSpeedsChanged");
		XmlSettingsManager::Instance ()->
			RegisterObject ("EnableFastSpeedControl",
				UpSelectorAction_, "handleSpeedsChanged");

#define _LC_MERGE(a) "Torrent"#a

#define _LC_SINGLE(a) \
		Core::Instance ()->GetShortcutManager ()->RegisterAction (_LC_MERGE(a), a.get ());

#define _LC_TRAVERSER(z,i,array) \
		_LC_SINGLE (BOOST_PP_SEQ_ELEM(i, array))

#define _LC_EXPANDER(Names) \
		BOOST_PP_REPEAT (BOOST_PP_SEQ_SIZE (Names), _LC_TRAVERSER, Names)

		_LC_EXPANDER ((OpenTorrent_)
				(ChangeTrackers_)
				(CreateTorrent_)
				(OpenMultipleTorrents_)
				(IPFilter_)
				(RemoveTorrent_)
				(Resume_)
				(Stop_)
				(MoveUp_)
				(MoveDown_)
				(MoveToTop_)
				(MoveToBottom_)
				(ForceReannounce_)
				(ForceRecheck_)
				(MoveFiles_)
				(MakeMagnetLink_));
	}

	void TorrentPlugin::SetupActions ()
	{
		Toolbar_.reset (new QToolBar ());
		Toolbar_->setWindowTitle ("BitTorrent");

		OpenTorrent_.reset (new QAction (tr ("Open torrent..."),
					Toolbar_.get ()));
		OpenTorrent_->setShortcut (Qt::Key_Insert);
		OpenTorrent_->setProperty ("ActionIcon", "document-open");
		connect (OpenTorrent_.get (),
				SIGNAL (triggered ()),
				this,
				SLOT (on_OpenTorrent__triggered ()));

		CreateTorrent_.reset (new QAction (tr ("Create torrent..."),
					Toolbar_.get ()));
		CreateTorrent_->setProperty ("ActionIcon", "document-new");
		connect (CreateTorrent_.get (),
				SIGNAL (triggered ()),
				this,
				SLOT (on_CreateTorrent__triggered ()));

		OpenMultipleTorrents_.reset (new QAction (tr ("Open multiple torrents..."),
				Toolbar_.get ()));
		OpenMultipleTorrents_->setProperty ("ActionIcon", "document-open-folder");
		connect (OpenMultipleTorrents_.get (),
				SIGNAL (triggered ()),
				this,
				SLOT (on_OpenMultipleTorrents__triggered ()));

		IPFilter_.reset (new QAction (tr ("IP filter..."),
					Toolbar_.get ()));
		IPFilter_->setProperty ("ActionIcon", "view-filter");
		connect (IPFilter_.get (),
				SIGNAL (triggered ()),
				this,
				SLOT (on_IPFilter__triggered ()));

		RemoveTorrent_.reset (new QAction (tr ("Remove"),
					Toolbar_.get ()));
		RemoveTorrent_->setShortcut (tr ("Del"));
		RemoveTorrent_->setProperty ("ActionIcon", "list-remove");
		connect (RemoveTorrent_.get (),
				SIGNAL (triggered ()),
				this,
				SLOT (on_RemoveTorrent__triggered ()));

		Resume_.reset (new QAction (tr ("Resume"),
					Toolbar_.get ()));
		Resume_->setShortcut (tr ("R"));
		Resume_->setProperty ("ActionIcon", "media-playback-start");
		connect (Resume_.get (),
				SIGNAL (triggered ()),
				this,
				SLOT (on_Resume__triggered ()));

		Stop_.reset (new QAction (tr ("Pause"),
					Toolbar_.get ()));
		Stop_->setShortcut (tr ("S"));
		Stop_->setProperty ("ActionIcon", "media-playback-pause");
		connect (Stop_.get (),
				SIGNAL (triggered ()),
				this,
				SLOT (on_Stop__triggered ()));

		MoveUp_.reset (new QAction (tr ("Move up"),
					Toolbar_.get ()));
		MoveUp_->setShortcut (Qt::CTRL + Qt::Key_Up);
		MoveUp_->setProperty ("ActionIcon", "go-up");
		connect (MoveUp_.get (),
				SIGNAL (triggered ()),
				this,
				SLOT (on_MoveUp__triggered ()));

		MoveDown_.reset (new QAction (tr ("Move down"),
					Toolbar_.get ()));
		MoveDown_->setShortcut (Qt::CTRL + Qt::Key_Down);
		MoveDown_->setProperty ("ActionIcon", "go-down");
		connect (MoveDown_.get (),
				SIGNAL (triggered ()),
				this,
				SLOT (on_MoveDown__triggered ()));

		MoveToTop_.reset (new QAction (tr ("Move to top"),
					Toolbar_.get ()));
		MoveToTop_->setShortcut (Qt::CTRL + Qt::SHIFT + Qt::Key_Up);
		MoveToTop_->setProperty ("ActionIcon", "go-top");
		connect (MoveToTop_.get (),
				SIGNAL (triggered ()),
				this,
				SLOT (on_MoveToTop__triggered ()));

		MoveToBottom_.reset (new QAction (tr ("Move to bottom"),
					Toolbar_.get ()));
		MoveToBottom_->setShortcut (Qt::CTRL + Qt::SHIFT + Qt::Key_Down);
		MoveToBottom_->setProperty ("ActionIcon", "go-bottom");
		connect (MoveToBottom_.get (),
				SIGNAL (triggered ()),
				this,
				SLOT (on_MoveToBottom__triggered ()));

		ForceReannounce_.reset (new QAction (tr ("Reannounce"),
					Toolbar_.get ()));
		ForceReannounce_->setShortcut (tr ("F"));
		ForceReannounce_->setProperty ("ActionIcon", "network-wireless");
		connect (ForceReannounce_.get (),
				SIGNAL (triggered ()),
				this,
				SLOT (on_ForceReannounce__triggered ()));

		ForceRecheck_.reset (new QAction (tr ("Recheck"),
				Toolbar_.get ()));
		ForceRecheck_->setProperty ("ActionIcon", "tools-check-spelling");
		connect (ForceRecheck_.get (),
				SIGNAL (triggered ()),
				this,
				SLOT (on_ForceRecheck__triggered ()));

		MoveFiles_.reset (new QAction (tr ("Move files..."),
					Toolbar_.get ()));
		MoveFiles_->setShortcut (tr ("M"));
		MoveFiles_->setProperty ("ActionIcon", "transform-move");
		connect (MoveFiles_.get (),
				SIGNAL (triggered ()),
				this,
				SLOT (on_MoveFiles__triggered ()));

		ChangeTrackers_.reset (new QAction (tr ("Change trackers..."),
					Toolbar_.get ()));
		ChangeTrackers_->setShortcut (tr ("C"));
		ChangeTrackers_->setProperty ("ActionIcon", "view-media-playlist");
		connect (ChangeTrackers_.get (),
				SIGNAL (triggered ()),
				this,
				SLOT (on_ChangeTrackers__triggered ()));

		MakeMagnetLink_.reset (new QAction (tr ("Make magnet link..."),
					Toolbar_.get ()));
		MakeMagnetLink_->setProperty ("ActionIcon", "insert-link");
		connect (MakeMagnetLink_.get (),
				SIGNAL (triggered ()),
				this,
				SLOT (on_MakeMagnetLink__triggered ()));

		OpenInTorrentTab_.reset (new QAction (tr ("Open in torrent tab"), Toolbar_.get ()));
		OpenInTorrentTab_->setIcon (TabTC_.Icon_);
		connect (OpenInTorrentTab_.get (),
				SIGNAL (triggered ()),
				this,
				SLOT (on_OpenInTorrentTab__triggered ()));

		Toolbar_->addAction (OpenTorrent_.get ());
		Toolbar_->addAction (RemoveTorrent_.get ());
		Toolbar_->addSeparator ();
		Toolbar_->addAction (Resume_.get ());
		Toolbar_->addAction (Stop_.get ());
		Toolbar_->addSeparator ();
		Toolbar_->addAction (MoveUp_.get ());
		Toolbar_->addAction (MoveDown_.get ());
		Toolbar_->addAction (MoveToTop_.get ());
		Toolbar_->addAction (MoveToBottom_.get ());
		Toolbar_->addSeparator ();
		Toolbar_->addAction (ForceReannounce_.get ());
		Toolbar_->addAction (ForceRecheck_.get ());
		Toolbar_->addAction (MoveFiles_.get ());
		Toolbar_->addAction (ChangeTrackers_.get ());
		Toolbar_->addAction (MakeMagnetLink_.get ());
		Toolbar_->addSeparator ();
		Toolbar_->addAction (OpenInTorrentTab_.get ());
		Toolbar_->addSeparator ();
		DownSelectorAction_ = new SpeedSelectorAction ("Down", this);
		DownSelectorAction_->handleSpeedsChanged ();
		Toolbar_->addAction (DownSelectorAction_);
		UpSelectorAction_ = new SpeedSelectorAction ("Up", this);
		UpSelectorAction_->handleSpeedsChanged ();
		Toolbar_->addAction (UpSelectorAction_);

		connect (DownSelectorAction_,
				SIGNAL (currentIndexChanged (int)),
				this,
				SLOT (handleFastSpeedComboboxes ()));
		connect (UpSelectorAction_,
				SIGNAL (currentIndexChanged (int)),
				this,
				SLOT (handleFastSpeedComboboxes ()));

		QMenu *contextMenu = new QMenu (tr ("Torrents actions"));
		contextMenu->addAction (RemoveTorrent_.get ());
		contextMenu->addSeparator ();
		contextMenu->addAction (MoveUp_.get ());
		contextMenu->addAction (MoveDown_.get ());
		contextMenu->addAction (MoveToTop_.get ());
		contextMenu->addAction (MoveToBottom_.get ());
		contextMenu->addSeparator ();
		contextMenu->addAction (ForceReannounce_.get ());
		contextMenu->addAction (ForceRecheck_.get ());
		contextMenu->addAction (MoveFiles_.get ());
		contextMenu->addAction (ChangeTrackers_.get ());
		contextMenu->addAction (MakeMagnetLink_.get ());
		contextMenu->addSeparator ();
		contextMenu->addAction (OpenInTorrentTab_.get ());
		Core::Instance ()->SetMenu (contextMenu);
	}
}
}

LC_EXPORT_PLUGIN (leechcraft_bittorrent, LC::BitTorrent::TorrentPlugin);
