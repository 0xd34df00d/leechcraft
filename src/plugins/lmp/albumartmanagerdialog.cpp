/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "albumartmanagerdialog.h"
#include <QStandardItemModel>
#include <QKeyEvent>
#include <QFileDialog>
#include <QTimer>
#include <QtConcurrentMap>
#include <util/gui/clearlineeditaddon.h>
#include "core.h"
#include "albumartmanager.h"

namespace LC
{
namespace LMP
{
	namespace
	{
		class ReturnPressSwallower : public QObject
		{
		public:
			ReturnPressSwallower (QObject *parent)
			: QObject (parent)
			{
			}

			bool eventFilter (QObject *obj, QEvent *e)
			{
				if (e->type () != QEvent::KeyPress)
					return false;

				auto keyEv = static_cast<QKeyEvent*> (e);
				if (keyEv->key () == Qt::Key_Enter || keyEv->key () == Qt::Key_Return)
				{
					QMetaObject::invokeMethod (obj, "returnPressed");
					return true;
				}

				return false;
			}
		};
	}

	AlbumArtManagerDialog::AlbumArtManagerDialog (const QString& artist,
			const QString& album, AlbumArtManager *aamgr, QWidget *parent)
	: QDialog (parent)
	, AAMgr_ (aamgr)
	, Model_ (new QStandardItemModel (this))
	, SourceInfo_ ({ artist, album })
	{
		Ui_.setupUi (this);
		Ui_.ArtistLine_->setText (artist);
		Ui_.ArtistLine_->installEventFilter (new ReturnPressSwallower (this));
		Ui_.AlbumLine_->setText (album);
		Ui_.AlbumLine_->installEventFilter (new ReturnPressSwallower (this));

		new Util::ClearLineEditAddon (Core::Instance ().GetProxy (), Ui_.ArtistLine_);
		new Util::ClearLineEditAddon (Core::Instance ().GetProxy (), Ui_.AlbumLine_);

		Ui_.ArtView_->setModel (Model_);

		connect (Ui_.ArtistLine_,
				SIGNAL (textChanged (QString)),
				this,
				SLOT (scheduleRequest ()));
		connect (Ui_.AlbumLine_,
				SIGNAL (textChanged (QString)),
				this,
				SLOT (scheduleRequest ()));
		connect (Ui_.ArtistLine_,
				SIGNAL (returnPressed ()),
				this,
				SLOT (request ()));
		connect (Ui_.AlbumLine_,
				SIGNAL (returnPressed ()),
				this,
				SLOT (request ()));

		connect (aamgr,
				SIGNAL (gotImages (Media::AlbumInfo, QList<QImage>)),
				this,
				SLOT (handleImages (Media::AlbumInfo, QList<QImage>)));

		connect (Ui_.ArtView_,
				SIGNAL (activated (QModelIndex)),
				this,
				SLOT (accept ()));

		scheduleRequest ();
	}

	QString AlbumArtManagerDialog::GetArtist () const
	{
		return Ui_.ArtistLine_->text ();
	}

	QString AlbumArtManagerDialog::GetAlbum () const
	{
		return Ui_.AlbumLine_->text ();
	}

	void AlbumArtManagerDialog::accept ()
	{
		std::shared_ptr<void> guard (nullptr, [this] (void*) { QDialog::accept (); });

		const auto& idx = Ui_.ArtView_->currentIndex ();
		if (!idx.isValid ())
			return;

		const auto& img = FullImages_ [idx.row ()];
		AAMgr_->HandleGotAlbumArt (SourceInfo_, { img });
	}

	void AlbumArtManagerDialog::on_BrowseButton__released ()
	{
		const auto& filename = QFileDialog::getOpenFileName (this,
				tr ("Choose album art"),
				QDir::homePath (),
				tr ("Images (*.png *.jpg *.jpeg);;All files (*.*)"));
		if (filename.isEmpty ())
			return;

		QImage image (filename);
		if (image.isNull ())
			return;

		handleImages ({ GetArtist (), GetAlbum () }, { image });
	}

	namespace
	{
		struct ScaleResult
		{
			QImage Image_;
			QImage SourceImage_;
			Media::AlbumInfo Info_;
		};
	}

	void AlbumArtManagerDialog::handleImages (const Media::AlbumInfo& info, const QList<QImage>& images)
	{
		qDebug () << Q_FUNC_INFO << images.size ();
		if (info.Album_ != GetAlbum () || info.Artist_ != GetArtist ())
			return;

		auto watcher = new QFutureWatcher<ScaleResult> ();
		connect (watcher,
				SIGNAL (finished ()),
				this,
				SLOT (handleResized ()));
		auto worker = [info] (const QImage& image) -> ScaleResult
		{
			const auto& scaled = image.scaled (200, 200, Qt::KeepAspectRatio, Qt::SmoothTransformation);
			return { scaled, image, info };
		};
		watcher->setFuture (QtConcurrent::mapped (images, std::function<ScaleResult (QImage)> (worker)));
	}

	void AlbumArtManagerDialog::handleResized ()
	{
		auto watcher = dynamic_cast<QFutureWatcher<ScaleResult>*> (sender ());
		watcher->deleteLater ();

		for (const auto& result : watcher->future ())
		{
			if (result.Info_.Album_ != GetAlbum () ||
					result.Info_.Artist_ != GetArtist ())
				continue;

			auto item = new QStandardItem ();
			item->setIcon (QIcon (QPixmap::fromImage (result.Image_)));
			item->setText (QString::fromUtf8 ("%1×%2")
						.arg (result.SourceImage_.width ())
						.arg (result.SourceImage_.height ()));
			item->setEditable (false);
			Model_->appendRow (item);

			FullImages_ << result.SourceImage_;
		}
	}

	void AlbumArtManagerDialog::request ()
	{
		Model_->clear ();

		const auto& artist = GetArtist ();
		const auto& album = GetAlbum ();
		if (artist.isEmpty () || album.isEmpty ())
			return;

		AAMgr_->CheckAlbumArt (artist, album, true);
		RequestScheduled_ = false;
	}

	void AlbumArtManagerDialog::requestScheduled ()
	{
		if (!RequestScheduled_)
			return;

		request ();
	}

	void AlbumArtManagerDialog::scheduleRequest ()
	{
		if (RequestScheduled_)
			return;

		QTimer::singleShot (1000,
				this,
				SLOT (requestScheduled ()));
		RequestScheduled_ = true;
	}
}
}
