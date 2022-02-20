/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "graffititab.h"
#include <functional>
#include <QFileSystemModel>
#include <QToolBar>
#include <QMessageBox>
#include <QInputDialog>
#include <QProgressDialog>
#include <QtConcurrentRun>
#include <QtDebug>
#include <QSettings>
#include <taglib/fileref.h>
#include <taglib/tag.h>
#include <util/tags/tagscompletionmodel.h>
#include <util/tags/tagscompleter.h>
#include <util/xpc/util.h>
#include <util/sll/either.h>
#include <util/sll/prelude.h>
#include <util/threads/futures.h>
#include <interfaces/core/ipluginsmanager.h>
#include <interfaces/core/ientitymanager.h>
#include <interfaces/media/itagsfetcher.h>
#include <interfaces/lmp/ilmpproxy.h>
#include <interfaces/lmp/itagresolver.h>
#include <interfaces/lmp/mediainfo.h>
#include "filesmodel.h"
#include "renamedialog.h"
#include "fileswatcher.h"
#include "cuesplitter.h"
#include "tagsfetchmanager.h"
#include "reciterator.h"
#include "literals.h"

namespace LC::LMP::Graffiti
{
	const int MaxHistoryCount = 25;

	GraffitiTab::GraffitiTab (ILMPProxy_ptr proxy, TabClassInfo tc, QObject *plugin)
	: LMPProxy_ (proxy)
	, TC_ (std::move (tc))
	, Plugin_ (plugin)
	, FSModel_ (new QFileSystemModel (this))
	, FilesModel_ (new FilesModel (this))
	, FilesWatcher_ (new FilesWatcher (this))
	, Toolbar_ (new QToolBar (Lits::LMPGraffiti))
	{
		Ui_.setupUi (this);

		SetupEdits ();
		SetupViews ();
		SetupToolbar ();

		connect (FilesWatcher_,
				&FilesWatcher::rereadFiles,
				this,
				&GraffitiTab::RereadFiles);

		RestorePathHistory ();
	}

	TabClassInfo GraffitiTab::GetTabClassInfo () const
	{
		return TC_;
	}

	QObject* GraffitiTab::ParentMultiTabs ()
	{
		return Plugin_;
	}

	void GraffitiTab::Remove ()
	{
		emit removeTab ();
		deleteLater ();
	}

	QToolBar* GraffitiTab::GetToolBar () const
	{
		return Toolbar_.get ();
	}

	void GraffitiTab::SetPath (const QString& path, const QString& filename)
	{
		if (path.isEmpty ())
		{
			qWarning () << "empty path for file"
					<< filename;
			return;
		}

		AddToPathHistory (path);

		setEnabled (false);
		FilesModel_->Clear ();
		FilesWatcher_->Clear ();

		auto recIterator = new RecIterator (LMPProxy_, this);
		connect (recIterator,
				&RecIterator::finished,
				this,
				[=]
				{
					recIterator->deleteLater ();
					HandleDirIterateResults (recIterator->GetResult (), filename);
				});
		connect (recIterator,
				&RecIterator::canceled,
				this,
				[=]
				{
					recIterator->deleteLater ();
					setEnabled (true);
				});

		auto progDialog = new QProgressDialog (this);
		progDialog->setLabelText (tr ("Scanning path %1...")
					.arg ("<em>" + path + "</em>"));
		progDialog->setAttribute (Qt::WA_DeleteOnClose);
		connect (recIterator,
				&RecIterator::finished,
				progDialog,
				&QDialog::close);
		connect (progDialog,
				&QProgressDialog::canceled,
				recIterator,
				&RecIterator::Cancel);
		progDialog->show ();

		recIterator->Start (path);

		SplitCue_->setEnabled (!QDir (path).entryList ({ QStringLiteral ("*.cue") }).isEmpty ());
	}

	template<typename T, typename F>
	void GraffitiTab::UpdateData (const T& newData, F getter)
	{
		static_assert (std::is_lvalue_reference<typename std::result_of<F (MediaInfo&)>::type>::value,
				"functor doesn't return an lvalue reference");

		bool changed = false;

		const auto& selected = Ui_.FilesList_->selectionModel ()->selectedRows ();
		for (const auto& index : selected)
		{
			const auto& infoData = index.data (FilesModel::Roles::MediaInfoRole);
			auto info = infoData.template value<MediaInfo> ();
			if (getter (info) == newData)
				continue;

			getter (info) = newData;
			FilesModel_->UpdateInfo (index, info);
			changed = true;
		}

		if (changed)
		{
			Save_->setEnabled (true);
			Revert_->setEnabled (true);
		}
	}

	void GraffitiTab::SetupEdits ()
	{
		Ui_.Genre_->SetSeparator (QStringLiteral (" / "));

		auto model = new Util::TagsCompletionModel (this);
		model->UpdateTags (Util::Map (Lits::Genres, [] (const QByteArray& ba) { return QString::fromUtf8 (ba); }));
		auto completer = new Util::TagsCompleter (Ui_.Genre_);
		completer->OverrideModel (model);

		Ui_.Genre_->AddSelector ();

		auto initField = [this]<typename T> (T *edit, QAbstractButton *button, auto handler)
		{
			connect (edit,
					&T::textChanged,
					this,
					handler);
			connect (button,
					&QAbstractButton::released,
					this,
					handler);
		};

		initField (Ui_.Artist_, Ui_.ArtistSetAll_,
				[this]
				{
					const auto& artist = Ui_.Artist_->text ();
					UpdateData (artist, [] (MediaInfo& info) -> QString& { return info.Artist_; });
				});
		initField (Ui_.Album_, Ui_.AlbumSetAll_,
				[this]
				{
					const auto& album = Ui_.Album_->text ();
					UpdateData (album, [] (MediaInfo& info) -> QString& { return info.Album_; });
				});
		initField (Ui_.Title_, Ui_.TitleSetAll_,
				[this]
				{
					const auto& title = Ui_.Title_->text ();
					UpdateData (title, [] (MediaInfo& info) -> QString& { return info.Title_; });
				});
		initField (Ui_.Genre_, Ui_.GenreSetAll_,
				[this]
				{
					const auto& genreString = Ui_.Genre_->text ();
					auto genres = genreString.split ('/', Qt::SkipEmptyParts);
					for (auto& genre : genres)
						genre = genre.trimmed ();

					UpdateData (genres, [] (MediaInfo& info) -> QStringList& { return info.Genres_; });
				});
		initField (Ui_.Year_, Ui_.YearSetAll_,
				[this]
				{
					const auto year = Ui_.Year_->value ();
					UpdateData (year, [] (MediaInfo& info) -> int& { return info.Year_; });
				});

		connect (Ui_.TrackNumber_,
				&QSpinBox::valueChanged,
				this,
				[this] (int number)
				{
					const auto& index = Ui_.FilesList_->currentIndex ();
					if (!index.isValid ())
						return;

					const auto& infoData = index.data (FilesModel::Roles::MediaInfoRole);
					auto info = infoData.value<MediaInfo> ();
					if (info.TrackNumber_ == number)
						return;

					info.TrackNumber_ = number;
					FilesModel_->UpdateInfo (index, info);

					Save_->setEnabled (true);
					Revert_->setEnabled (true);
				});
		connect (Ui_.TrackNumberAutoFill_,
				&QAbstractButton::released,
				this,
				[this]
				{
					QMap<QString, int> album2counter;

					const auto& selected = Ui_.FilesList_->selectionModel ()->selectedRows ();
					for (const auto& index : selected)
					{
						const auto& infoData = index.data (FilesModel::Roles::MediaInfoRole);
						auto info = infoData.value<MediaInfo> ();
						info.TrackNumber_ = ++album2counter [info.Album_];
						FilesModel_->UpdateInfo (index, info);
					}

					if (!selected.isEmpty ())
					{
						Save_->setEnabled (true);
						Revert_->setEnabled (true);
					}
				});
	}

	void GraffitiTab::SetupViews ()
	{
		FSModel_->setRootPath (QDir::rootPath ());
		FSModel_->setFilter (QDir::Dirs | QDir::NoDotAndDotDot);
		FSModel_->setReadOnly (true);
		Ui_.DirectoryTree_->setModel (FSModel_);
		Ui_.DirectoryTree_->sortByColumn (0, Qt::AscendingOrder);

		auto idx = FSModel_->index (QDir::homePath ());
		while (idx.isValid ())
		{
			Ui_.DirectoryTree_->expand (idx);
			idx = idx.parent ();
		}

		Ui_.FilesList_->setModel (FilesModel_);

		connect (Ui_.FilesList_->selectionModel (),
				&QItemSelectionModel::currentRowChanged,
				this,
				&GraffitiTab::PopulateFields);

		connect (Ui_.PathLine_,
				&QComboBox::textActivated,
				[this]
				{
					auto path = Ui_.PathLine_->currentText ();
					if (path.startsWith ('~'))
					{
						path.replace (0, 1, QDir::homePath ());
						Ui_.PathLine_->blockSignals (true);
						Ui_.PathLine_->setEditText (path);
						Ui_.PathLine_->blockSignals (false);
					}

					Ui_.DirectoryTree_->setCurrentIndex (FSModel_->index (path));
					SetPath (path);
				});

		connect (Ui_.DirectoryTree_,
				&QTreeView::activated,
				[this] (const QModelIndex& index)
				{
					const auto& path = FSModel_->filePath (index);
					Ui_.PathLine_->blockSignals (true);
					Ui_.PathLine_->setEditText (path);
					Ui_.PathLine_->blockSignals (false);

					SetPath (path);
				});
	}

	void GraffitiTab::SetupToolbar ()
	{
		Save_ = Toolbar_->addAction (tr ("Save"),
				this, &GraffitiTab::Save);
		Save_->setProperty ("ActionIcon", "document-save");
		Save_->setShortcut (QKeySequence::Save);

		Revert_ = Toolbar_->addAction (tr ("Revert"),
				this, &GraffitiTab::Revert);
		Revert_->setProperty ("ActionIcon", "document-revert");

		Toolbar_->addSeparator ();

		RenameFiles_ = Toolbar_->addAction (tr ("Rename files"),
				this, &GraffitiTab::RenameFiles);
		RenameFiles_->setProperty ("ActionIcon", "edit-rename");

		Toolbar_->addSeparator ();

		GetTags_ = Toolbar_->addAction (tr ("Fetch tags"),
				this, &GraffitiTab::FetchTags);
		GetTags_->setProperty ("ActionIcon", "download");

		SplitCue_ = Toolbar_->addAction (tr ("Split CUE..."),
				this, &GraffitiTab::SplitCue);
		SplitCue_->setProperty ("ActionIcon", "split");
		SplitCue_->setEnabled (false);
	}

	namespace
	{
		const QString PathHistoryGroup = QStringLiteral ("PathHistory");
		const QString HistListKey = QStringLiteral ("HistList");
	}

	void GraffitiTab::RestorePathHistory ()
	{
		QSettings settings (QCoreApplication::organizationName (),
				QCoreApplication::applicationName () + "_LMP_Graffiti");
		settings.beginGroup (PathHistoryGroup);
		const auto& paths = settings.value (HistListKey).toStringList ();
		settings.endGroup ();

		Ui_.PathLine_->blockSignals (true);
		for (const auto& item : paths)
			if (QFile::exists (item))
				Ui_.PathLine_->addItem (item);

		Ui_.PathLine_->setCurrentIndex (-1);
		Ui_.PathLine_->blockSignals (false);
	}

	void GraffitiTab::AddToPathHistory (const QString& path)
	{
		const auto sameIdx = Ui_.PathLine_->findText (path);
		if (!sameIdx)
			return;

		Ui_.PathLine_->blockSignals (true);
		if (sameIdx > 0)
			Ui_.PathLine_->removeItem (sameIdx);

		Ui_.PathLine_->insertItem (0, path);

		while (Ui_.PathLine_->count () > MaxHistoryCount)
			Ui_.PathLine_->removeItem (Ui_.PathLine_->count () - 1);

		Ui_.PathLine_->setCurrentIndex (0);
		Ui_.PathLine_->blockSignals (false);

		QStringList paths;
		for (int i = 0; i < Ui_.PathLine_->count (); ++i)
			paths << Ui_.PathLine_->itemText (i);

		QSettings settings (QCoreApplication::organizationName (),
				QCoreApplication::applicationName () + "_LMP_Graffiti");
		settings.beginGroup (PathHistoryGroup);
		settings.setValue (HistListKey, paths);
		settings.endGroup ();
	}

	void GraffitiTab::Save ()
	{
		const auto& modified = FilesModel_->GetModified ();
		if (modified.isEmpty ())
			return;

		if (QMessageBox::question (this,
				Lits::LMPGraffiti,
				tr ("Do you really want to accept changes to %n file(s)?", nullptr, modified.size ()),
				QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes)
			return;

		const auto resolver = LMPProxy_->GetTagResolver ();

		auto toTLStr = [] (const QString& str)
		{
			return TagLib::String (str.toUtf8 ().constData (), TagLib::String::UTF8);
		};

		for (const auto& pair : modified)
		{
			const auto& newInfo = pair.first;

			QMutexLocker locker (&resolver->GetMutex ());
			auto file = resolver->GetFileRef (newInfo.LocalPath_);
			auto tag = file.tag ();

			tag->setArtist (toTLStr (newInfo.Artist_));
			tag->setAlbum (toTLStr (newInfo.Album_));
			tag->setTitle (toTLStr (newInfo.Title_));
			tag->setYear (newInfo.Year_);
			tag->setGenre (toTLStr (newInfo.Genres_.join (u" / ")));
			tag->setTrack (newInfo.TrackNumber_);

			if (!file.save ())
				qWarning () << Q_FUNC_INFO
						<< "unable to save file"
						<< newInfo.LocalPath_;
		}

		RereadFiles ();
	}

	void GraffitiTab::Revert ()
	{
		const auto& modified = FilesModel_->GetModified ();
		if (modified.isEmpty ())
			return;

		if (QMessageBox::question (this,
				Lits::LMPGraffiti,
				tr ("Do you really want to revert changes to %n file(s)?", nullptr, modified.size ()),
				QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes)
			return;

		QList<MediaInfo> origs;
		for (const auto& pair : modified)
			origs << pair.second;
		FilesModel_->SetInfos (origs);

		Save_->setEnabled (false);
		Revert_->setEnabled (false);

		PopulateFields (Ui_.FilesList_->currentIndex ());
	}

	void GraffitiTab::RenameFiles ()
	{
		if (!FilesModel_->GetModified ().isEmpty ())
		{
			auto res = QMessageBox::question (this,
					Lits::LMPGraffiti,
					tr ("You have unsaved files with changed tags. Do you want to save or discard those changes?"),
					QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
			if (res == QMessageBox::Save)
				Save ();
			else if (res == QMessageBox::Discard)
				Revert ();
			else
				return;
		}

		QList<MediaInfo> infos;
		for (const auto& index : Ui_.FilesList_->selectionModel ()->selectedRows ())
			infos << index.data (FilesModel::Roles::MediaInfoRole).value<MediaInfo> ();
		if (infos.isEmpty ())
			return;

		auto dia = new RenameDialog (LMPProxy_, this);
		dia->SetInfos (infos);

		dia->setAttribute (Qt::WA_DeleteOnClose);
		dia->show ();
	}

	void GraffitiTab::FetchTags ()
	{
		auto provs = GetProxyHolder ()->GetPluginsManager ()->GetAllCastableTo<Media::ITagsFetcher*> ();
		if (provs.isEmpty ())
			return;

		const auto& rows = Ui_.FilesList_->selectionModel ()->selectedRows ();
		QStringList paths;
		for (const auto& index : rows)
		{
			const auto& info = index.data (FilesModel::Roles::MediaInfoRole).value<MediaInfo> ();
			paths << info.LocalPath_;
		}

		if (paths.isEmpty ())
			return;

		GetTags_->setEnabled (false);

		auto fetcher = new TagsFetchManager (paths, provs.first (), FilesModel_, this);
		connect (fetcher,
				&TagsFetchManager::tagsFetchProgress,
				this,
				&GraffitiTab::tagsFetchProgress);
		connect (fetcher,
				&TagsFetchManager::tagsFetched,
				[this] (const QString& filename)
				{
					const auto& curIdx = Ui_.FilesList_->selectionModel ()->currentIndex ();
					const auto& curInfo = curIdx.data (FilesModel::Roles::MediaInfoRole).value<MediaInfo> ();
					if (curInfo.LocalPath_ == filename)
						PopulateFields (curIdx);
				});
		connect (fetcher,
				&TagsFetchManager::finished,
				[this] { GetTags_->setEnabled (true); });
	}

	void GraffitiTab::SplitCue ()
	{
		const auto& curDirIdx = Ui_.DirectoryTree_->currentIndex ();
		if (!curDirIdx.isValid ())
			return;

		const auto& path = FSModel_->filePath (curDirIdx);
		const QDir dir (path);

		const auto& cues = dir.entryList ({ "*.cue" });
		if (cues.isEmpty ())
		{
			QMessageBox::critical (this,
					Lits::LMPGraffiti,
					tr ("No cue sheets are available in this directory."));
			return;
		}

		QString cue;
		if (cues.size () >= 2)
		{
			cue = QInputDialog::getItem (this,
					Lits::LMPGraffiti,
					tr ("Select cue sheet to use for splitting:"),
					cues,
					0,
					false);
			if (cue.isEmpty ())
				return;
		}
		else
			cue = cues.first ();

		auto splitter = new CueSplitter (cue, path);
		connect (splitter,
				&CueSplitter::error,
				[] (const QString& error)
				{
					const auto& e = Util::MakeNotification (Lits::LMPGraffiti, error, Priority::Critical);
					GetProxyHolder ()->GetEntityManager ()->HandleEntity (e);
				});
		connect (splitter,
				&CueSplitter::finished,
				[]
				{
					const auto& e = Util::MakeNotification (Lits::LMPGraffiti,
							tr ("Finished splitting CUE file"),
							Priority::Info);
					GetProxyHolder ()->GetEntityManager ()->HandleEntity (e);
				});
		emit cueSplitStarted (splitter);
	}

	void GraffitiTab::PopulateFields (const QModelIndex& index)
	{
		const auto& infoData = FilesModel_->data (index, FilesModel::Roles::MediaInfoRole);
		const auto& info = infoData.value<MediaInfo> ();

		QWidget* const widgets [] { Ui_.Album_, Ui_.Artist_, Ui_.Title_, Ui_.Genre_, Ui_.Year_, Ui_.TrackNumber_ };

		for (const auto w : widgets)
			w->blockSignals (true);

		Ui_.Album_->setText (info.Album_);
		Ui_.Artist_->setText (info.Artist_);
		Ui_.Title_->setText (info.Title_);
		Ui_.Genre_->setText (info.Genres_.join (u" / "));
		Ui_.Year_->setValue (info.Year_);
		Ui_.TrackNumber_->setValue (info.TrackNumber_);

		for (const auto w : widgets)
			w->blockSignals (false);
	}

	void GraffitiTab::RereadFiles ()
	{
		SetPath (FSModel_->filePath (Ui_.DirectoryTree_->currentIndex ()));
	}

	void GraffitiTab::HandleDirIterateResults (const QList<QFileInfo>& files, const QString& origFilename)
	{
		FilesWatcher_->AddFiles (files);
		FilesModel_->AddFiles (files);

		auto resolver = LMPProxy_->GetTagResolver ();
		auto worker = [resolver, files]
		{
			const auto& eithers = Util::Map (files,
					[resolver] (const QFileInfo& info)
						{ return resolver->ResolveInfo (info.absoluteFilePath ()); });
			const auto& parts = Util::PartitionEithers (eithers);

			for (const auto& resolveError : parts.first)
				qWarning () << resolveError.FilePath_
						<< resolveError.ReasonString_;

			return parts.second;
		};

		Util::Sequence (this, QtConcurrent::run (worker)) >>
				[=] (const QList<MediaInfo>& infos)
				{
					FilesModel_->SetInfos (infos);
					setEnabled (true);

					if (const auto& index = FilesModel_->FindIndexByFileName (origFilename);
						index.isValid ())
						Ui_.FilesList_->setCurrentIndex (index);
				};
	}
}
