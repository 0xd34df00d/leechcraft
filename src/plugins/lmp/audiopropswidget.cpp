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
#include "literals.h"
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
							Lits::LMP,
							tr ("Error showing properties for %1: %2.")
								.arg (QFileInfo (err.FilePath_).fileName ())
								.arg (err.ReasonString_));
				});
	}

	namespace
	{
		using PropsVec_t = QVector<QPair<QString, QString>>;

		using namespace TagLib;

		struct PropsGetter
		{
			PropsVec_t& Vec_;

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
				Append (name, val ? tr ("yes") : tr ("no"));
			}

			QString VersionField (QLatin1String codec) const
			{
				return tr ("%1 version").arg (codec);
			}

			QString Parse (APE::Properties *props)
			{
				Append (VersionField ("APE"_ql), props->version ());
				Append (tr ("Bits per sample"), props->bitsPerSample ());
				return QStringLiteral ("APE");
			}

			QString Parse (ASF::Properties*)
			{
				return QStringLiteral ("ASF");
			}

			QString Parse (FLAC::Properties *props)
			{
				Append (tr ("Bits per sample"), props->bitsPerSample ());
				return QStringLiteral ("FLAC");
			}

			QString Parse (MP4::Properties *props)
			{
				Append (tr ("Bits per sample"), props->bitsPerSample ());
				return QStringLiteral ("MP4");
			}

			QString Parse (MPC::Properties *props)
			{
				Append (VersionField ("MPC"_ql), props->mpcVersion ());
				return QStringLiteral ("MPC");
			}

			QString Parse (MPEG::Properties *props)
			{
				QString mpegVersion;
				switch (props->version ())
				{
				case MPEG::Header::Version1:
					mpegVersion = "1";
					break;
				case MPEG::Header::Version2:
					mpegVersion = "2";
					break;
				case MPEG::Header::Version2_5:
					mpegVersion = "2.5";
					break;
				}
				Append (VersionField ("MPEG"_ql), mpegVersion);

				Append (tr ("MPEG layer"), props->layer ());
				Append (tr ("Protected"), props->protectionEnabled ());
				Append (tr ("Copyrighted"), props->isCopyrighted ());
				Append (tr ("Original"), props->isOriginal ());

				QString channelMode;
				switch (props->channelMode ())
				{
				case MPEG::Header::Stereo:
					channelMode = tr ("Stereo");
					break;
				case MPEG::Header::JointStereo:
					channelMode = tr ("Joint Stereo");
					break;
				case MPEG::Header::DualChannel:
					channelMode = tr ("Dual Mono");
					break;
				case MPEG::Header::SingleChannel:
					channelMode = tr ("Mono");
					break;
				}
				Append ("Channel mode", channelMode);

				return QStringLiteral ("MPEG");
			}

			QString Parse (Ogg::Speex::Properties *props)
			{
				Append (VersionField ("Speex"_ql), props->speexVersion ());
				return QStringLiteral ("Speex");
			}

			QString Parse (Vorbis::Properties *props)
			{
				Append (VersionField ("Vorbis"_ql), props->vorbisVersion ());
				Append (tr ("Minimum bitrate"), props->bitrateMinimum ());
				Append (tr ("Maximum bitrate"), props->bitrateMaximum ());
				Append (tr ("Nominal bitrate"), props->bitrateNominal ());
				return QStringLiteral ("OGG Vorbis");
			}

			QString Parse (RIFF::AIFF::Properties *props)
			{
				Append (tr ("Bits per sample"), props->bitsPerSample ());
				return QStringLiteral ("AIFF");
			}

			QString Parse (RIFF::WAV::Properties *props)
			{
				Append (tr ("Bits per sample"), props->bitsPerSample ());
				return QStringLiteral ("WAV");
			}

			QString Parse (TrueAudio::Properties *props)
			{
				Append (VersionField ("TTA"_ql), props->ttaVersion ());
				Append (tr ("Bits per sample"), props->bitsPerSample ());
				return QStringLiteral ("TTA (TrueAudio)");
			}

			QString Parse (WavPack::Properties *props)
			{
				Append (VersionField ("WavPack"_ql), props->version ());
				Append (tr ("Bits per sample"), props->bitsPerSample ());
				return QStringLiteral ("WavPack");
			}

			template<typename T>
			void HandleProp (AudioProperties *props)
			{
				if (const auto val = dynamic_cast<T> (props))
				{
					Append (tr ("File type"), Parse (val));
					return;
				}
			}

			Q_DECLARE_TR_FUNCTIONS (LC::LMP::AudioPropsWidget)
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

			vec.push_back ({ AudioPropsWidget::tr ("Bitrate"), AudioPropsWidget::tr ("%1 kbps").arg (props->bitrate ()) });
			vec.push_back ({ AudioPropsWidget::tr ("Channels"), QString::number (props->channels ()) });
			vec.push_back ({ AudioPropsWidget::tr ("Sample rate"), AudioPropsWidget::tr ("%1 Hz").arg (props->sampleRate ()) });

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
			{ tr ("Genres"), info.Genres_.join (QStringLiteral ("; ")) },
			{ tr ("Length"), QString::number (info.Length_) },
		};

		for (const auto& [key, value] : Util::Stlize (info.Additional_))
			props.push_back ({ key, value.toString () });

		if (!info.LocalPath_.isEmpty ())
		{
			props.push_back ({ tr ("Local path"), info.LocalPath_ }),
			CollectLocalProps (props, info.LocalPath_);
		}

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
