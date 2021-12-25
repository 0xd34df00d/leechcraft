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
#include <util/models/flatitemsmodel.h>
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
	: QWidget { parent }
	, PropsModel_ { new Util::FlatItemsModel<QPair<QString, QString>> { { tr ("Property"), tr ("Value") }, this } }
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
		using PropsVec_t = QVector<QPair<QString, QString>>;

		struct PropsGetter
		{
			PropsVec_t& Vec_;

			PropsGetter (PropsVec_t& vec)
			: Vec_ { vec }
			{
			}

			void Append (const QString& name, const QString& val)
			{
				Vec_.push_back ({ name, val });
			}

			void Append (const QString& name, int val)
			{
				Append (name, QString::number (val));
			}

			void Append (const QString& name, bool val)
			{
				Append (name, val ? AudioPropsWidget::tr ("yes") : AudioPropsWidget::tr ("no"));
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

			template<typename T>
			void HandleProp (TagLib::AudioProperties *props)
			{
				if (const auto val = dynamic_cast<T> (props))
				{
					Parse (val);
					return;
				}
			}
		};

		template<typename... Props>
		void AppendProps (auto&& getter, TagLib::AudioProperties *props)
		{
			(getter.template HandleProp<Props> (props), ...);
		}

		void CollectLocalProps (PropsVec_t& vec, const QString& path)
		{
			const auto resolver = Core::Instance ().GetLocalFileResolver ();

			QMutexLocker tlLocker { &resolver->GetMutex () };
			const auto r = resolver->GetFileRef (path);
			const auto tag = r.tag ();
			if (!tag)
				return;

			vec.push_back ({ AudioPropsWidget::tr ("Comment"), QString::fromStdString (tag->comment ().to8Bit (true)) });

			const auto props = r.audioProperties ();
			if (!props)
				return;

			vec.push_back ({ AudioPropsWidget::tr ("Bitrate"), QString::number (props->bitrate ()) + " kbps" });
			vec.push_back ({ AudioPropsWidget::tr ("Channels"), QString::number (props->channels ()) });
			vec.push_back ({ AudioPropsWidget::tr ("Sample rate"), QString::number (props->sampleRate ()) + " Hz" });

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
			> (PropsGetter { vec }, props);
		}
	}

	void AudioPropsWidget::SetProps (const MediaInfo& info)
	{
		PropsVec_t props
		{
			{ tr ("Artist"), info.Artist_ },
			{ tr ("Title"), info.Title_ },
			{ tr ("Album"), info.Album_ },
			{ tr ("Track number"), QString::number (info.TrackNumber_) },
			{ tr ("Year"), QString::number (info.Year_) },
			{ tr ("Genres"), info.Genres_.join ("; ") },
			{ tr ("Length"), QString::number (info.Length_) },
			{ tr ("Local path"), info.LocalPath_.isEmpty () ? tr ("unknown") : info.LocalPath_ },
		};

		for (const auto& [key, value] : Util::Stlize (info.Additional_))
			props.push_back ({ key, value.toString () });

		if (!info.LocalPath_.isEmpty ())
			CollectLocalProps (props, info.LocalPath_);

		PropsModel_->SetItems (props);
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
		QGuiApplication::clipboard ()->setText (text);
	}
}
}
