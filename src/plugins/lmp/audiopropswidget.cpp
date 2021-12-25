/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "audiopropswidget.h"
#include <functional>
#include <QDialog>
#include <QDialogButtonBox>
#include <QStandardItemModel>
#include <QMessageBox>
#include <QClipboard>
#include <QMutexLocker>
#include <QFileInfo>
#include <QAction>
#include <QtDebug>
#include <taglib/taglib_config.h>
#include <taglib/taglib.h>
#include <taglib/fileref.h>
#include <taglib/tag.h>
#include <taglib/apeproperties.h>
#include <taglib/asfproperties.h>
#include <taglib/flacproperties.h>
#include <taglib/mp4properties.h>
#include <taglib/mpcproperties.h>
#include <taglib/mpegproperties.h>
#include <taglib/speexproperties.h>
#include <taglib/vorbisproperties.h>
#include <taglib/aiffproperties.h>
#include <taglib/wavproperties.h>
#include <taglib/trueaudioproperties.h>
#include <taglib/wavpackproperties.h>
#include <util/sll/either.h>
#include <util/sll/visitor.h>
#include <util/sll/qtutil.h>
#include <interfaces/core/iiconthememanager.h>
#include "localfileresolver.h"
#include "core.h"

namespace LC
{
namespace LMP
{
	AudioPropsWidget::AudioPropsWidget (QWidget *parent)
	: QWidget (parent)
	, PropsModel_ (new QStandardItemModel (this))
	{
		Ui_.setupUi (this);
		Ui_.PropsView_->setModel (PropsModel_);

		auto copy = new QAction (tr ("Copy"), this);
		copy->setIcon (QIcon::fromTheme ("edit-copy"));
		connect (copy,
				SIGNAL (triggered ()),
				this,
				SLOT (handleCopy ()));
		Ui_.PropsView_->addAction (copy);
	}

	AudioPropsWidget* AudioPropsWidget::MakeDialog ()
	{
		auto dia = new QDialog ();
		dia->setWindowTitle (tr ("Track properties"));
		dia->resize (800, 600);
		dia->setLayout (new QVBoxLayout);

		auto props = new AudioPropsWidget;

		auto box = new QDialogButtonBox (QDialogButtonBox::Close);
		connect (box,
				SIGNAL (rejected ()),
				dia,
				SLOT (close ()));

		dia->layout ()->addWidget (props);
		dia->layout ()->addWidget (box);

		dia->setAttribute (Qt::WA_DeleteOnClose);
		dia->show ();

		return props;
	}

	void AudioPropsWidget::SetProps (const QString& path)
	{
		Util::Visit (Core::Instance ().GetLocalFileResolver ()->ResolveInfo (path),
				[this] (const MediaInfo& info) { SetProps (info); },
				[this] (const ResolveError& err)
				{
					qWarning () << Q_FUNC_INFO
							<< err.FilePath_;

					QMessageBox::critical (this,
							"LeechCraft",
							tr ("Error showing properties for %1: %2.")
								.arg (QFileInfo (err.FilePath_).fileName ())
								.arg (err.ReasonString_));
				});
	}

	namespace
	{
		QMap<QString, QString> GetGenericProps (TagLib::AudioProperties *props)
		{
			QMap<QString, QString> result;
			result [AudioPropsWidget::tr ("Bitrate")] = QString::number (props->bitrate ()) + " kbps";
			result [AudioPropsWidget::tr ("Channels")] = QString::number (props->channels ());
			result [AudioPropsWidget::tr ("Sample rate")] = QString::number (props->sampleRate ()) + " Hz";
			return result;
		}

		template<typename Appender>
		struct PropsGetter
		{
			Appender F_;

			PropsGetter (Appender app)
			: F_ (app)
			{
			}

			void Append (const QString& name, const QString& val)
			{
				F_ (name, val);
			}

			void Append (const QString& name, int val)
			{
				F_ (name, QString::number (val));
			}

			void Append (const QString& name, bool val)
			{
				F_ (name, val ? AudioPropsWidget::tr ("yes") : AudioPropsWidget::tr ("no"));
			}

			QString Parse (TagLib::APE::Properties *props)
			{
				Append ("APE version", props->version ());
				Append ("Bits per sample", props->bitsPerSample ());
				return "APE";
			}

			QString Parse (TagLib::ASF::Properties*)
			{
				return "ASF";
			}

			QString Parse (TagLib::FLAC::Properties *props)
			{
				Append ("Sample width", props->bitsPerSample ());
				return "FLAC";
			}

			QString Parse (TagLib::MP4::Properties *props)
			{
				Append ("Bits per sample", props->bitsPerSample ());
				return "MP4";
			}

			QString Parse (TagLib::MPC::Properties *props)
			{
				Append ("MPC version", props->mpcVersion ());
				return "MPC";
			}

			QString Parse (TagLib::MPEG::Properties *props)
			{
				switch (props->version ())
				{
				case TagLib::MPEG::Header::Version1:
					Append ("MPEG version", 1);
					break;
				case TagLib::MPEG::Header::Version2:
					Append ("MPEG version", 2);
					break;
				case TagLib::MPEG::Header::Version2_5:
					Append ("MPEG version", "2.5");
					break;
				}

				Append ("MPEG layer", props->layer ());
				Append ("Protected", props->protectionEnabled ());
				Append ("Copyrighted", props->isCopyrighted ());
				Append ("Original", props->isOriginal ());

				switch (props->channelMode ())
				{
				case TagLib::MPEG::Header::Stereo:
					Append ("Channel mode", "Stereo");
					break;
				case TagLib::MPEG::Header::JointStereo:
					Append ("Channel mode", "Joint Stereo");
					break;
				case TagLib::MPEG::Header::DualChannel:
					Append ("Channel mode", "Dual Mono");
					break;
				case TagLib::MPEG::Header::SingleChannel:
					Append ("Channel mode", "Mono");
					break;
				}

				return "MPEG";
			}

			QString Parse (TagLib::Ogg::Speex::Properties *props)
			{
				Append ("Speex version", props->speexVersion ());
				return "Speex";
			}

			QString Parse (TagLib::Vorbis::Properties *props)
			{
				Append ("Vorbis version", props->vorbisVersion ());
				Append ("Minimum bitrate", props->bitrateMinimum ());
				Append ("Maximum bitrate", props->bitrateMaximum ());
				Append ("Nominal bitrate", props->bitrateNominal ());
				return "OGG Vorbis";
			}

			QString Parse (TagLib::RIFF::AIFF::Properties *props)
			{
				Append ("Sample width", props->bitsPerSample ());
				return "AIFF";
			}

			QString Parse (TagLib::RIFF::WAV::Properties *props)
			{
				Append ("Sample width", props->bitsPerSample ());
				return "WAV";
			}

			QString Parse (TagLib::TrueAudio::Properties *props)
			{
				Append ("Bits per sample", props->bitsPerSample ());
				Append ("TTA version", props->ttaVersion ());
				return "TTA (TrueAudio)";
			}

			QString Parse (TagLib::WavPack::Properties *props)
			{
				Append ("Bits per sample", props->bitsPerSample ());
				Append ("WavPack version", props->version ());
				return "WavPack";
			}
		};

		template<typename... Props>
		void AppendProps (auto&& getter, TagLib::AudioProperties *props)
		{
			(getter.Parse (dynamic_cast<Props> (props)), ...);
		}
	}

	void AudioPropsWidget::SetProps (const MediaInfo& info)
	{
		PropsModel_->clear ();

		auto append = [this] (const QString& name, const QString& val)
		{
			auto nameItem = new QStandardItem (name);
			nameItem->setEditable (false);
			auto valItem = new QStandardItem (val);
			valItem->setEditable (false);
			QList<QStandardItem*> items;
			items << nameItem << valItem;
			PropsModel_->appendRow (items);
		};

		append (tr ("Artist"), info.Artist_);
		append (tr ("Title"), info.Title_);
		append (tr ("Album"), info.Album_);
		append (tr ("Track number"), QString::number (info.TrackNumber_));
		append (tr ("Year"), QString::number (info.Year_));
		append (tr ("Genres"), info.Genres_.join ("; "));
		append (tr ("Length"), QString::number (info.Length_));
		append (tr ("Local path"), info.LocalPath_.isEmpty () ? tr ("unknown") : info.LocalPath_);

		for (auto i = info.Additional_.begin (), end = info.Additional_.end (); i != end; ++i)
			append (i.key (), i.value ().toString ());

		if (info.LocalPath_.isEmpty ())
			return;

		QMutexLocker tlLocker (&Core::Instance ().GetLocalFileResolver ()->GetMutex ());

		auto r = Core::Instance ().GetLocalFileResolver ()->GetFileRef (info.LocalPath_);
		auto tag = r.tag ();
		if (!tag)
			return;

		append (tr ("Comment"), QString::fromUtf8 (tag->comment ().toCString (true)));

		auto props = r.audioProperties ();
		if (!props)
			return;

		auto addMap = [append] (const QMap<QString, QString>& map)
		{
			for (const auto& pair : Util::Stlize (map))
				append (pair.first, pair.second);
		};

		addMap (GetGenericProps (props));

		using namespace TagLib;

		AppendProps<
		        APE::Properties*,
				ASF::Properties*,
				FLAC::Properties*,
				MP4::Properties*,
				MPC::Properties*,
				MPEG::Properties*,
				Ogg::Speex::Properties*,
				Vorbis::Properties*,
				RIFF::AIFF::Properties*,
				RIFF::WAV::Properties*,
				TrueAudio::Properties*,
				WavPack::Properties*
			> (PropsGetter { append }, props);
	}

	void AudioPropsWidget::handleCopy ()
	{
		const auto& idx = Ui_.PropsView_->currentIndex ();
		if (!idx.isValid ())
			return;

		QString text = idx.sibling (idx.row (), 1).data ().toString ();
		if (!idx.column ())
		{
			text.prepend (": ");
			text.prepend (idx.data ().toString ());
		}

		qApp->clipboard ()->setText (text);
	}
}
}
