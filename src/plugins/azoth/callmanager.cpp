/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "callmanager.h"
#include <functional>

#ifdef ENABLE_MEDIACALLS
#include <QAudioDeviceInfo>
#include <QAudioInput>
#include <QAudioOutput>
#endif

#include <QtDebug>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/ientitymanager.h>
#include <util/xpc/util.h>
#include <util/xpc/notificationactionhandler.h>
#include "interfaces/azoth/iclentry.h"
#include "interfaces/azoth/iaccount.h"
#include "xmlsettingsmanager.h"
#include "core.h"

namespace LC
{
namespace Azoth
{
	struct CallState
	{
		QIODevice::OpenMode LastMode_;
		std::shared_ptr<QAudioInput> InDevice_;
		std::shared_ptr<QAudioOutput> OutDevice_;
	};

	CallManager::CallManager (QObject *parent)
	: QObject (parent)
	{
	}

	CallManager::~CallManager ()
	{
	}

	void CallManager::AddAccount (QObject *account)
	{
		if (!qobject_cast<ISupportMediaCalls*> (account))
			return;

		connect (account,
				SIGNAL (called (QObject*)),
				this,
				SLOT (handleCall (QObject*)));
	}

	QObject* CallManager::Call (ICLEntry *entry, const QString& variant)
	{
#ifdef ENABLE_MEDIACALLS
		const auto ismc = qobject_cast<ISupportMediaCalls*> (entry->GetParentAccount ()->GetQObject ());
		if (!ismc)
		{
			qWarning () << Q_FUNC_INFO
					<< entry->GetQObject ()
					<< "parent account doesn't support media calls";
			return nullptr;
		}

		const auto callObj = ismc->Call (entry->GetEntryID (), variant);
		if (!callObj)
		{
			qWarning () << Q_FUNC_INFO
					<< "got null call obj for"
					<< entry->GetEntryID ()
					<< variant;
			return nullptr;
		}

		return callObj;
#else
		Q_UNUSED (entry)
		Q_UNUSED (variant)
		return nullptr;
#endif
	}

	QObjectList CallManager::GetCallsForEntry (const QString& id) const
	{
		return Entry2Calls_ [id];
	}

#ifdef ENABLE_MEDIACALLS
	namespace
	{
		QAudioDeviceInfo FindDevice (const QByteArray& property, QAudio::Mode mode)
		{
			const auto& name = XmlSettingsManager::Instance ()
					.property (property).toString ();

			for (const auto& info : QAudioDeviceInfo::availableDevices (mode))
				if (info.deviceName () == name)
					return info;

			return mode == QAudio::AudioInput ?
					QAudioDeviceInfo::defaultInputDevice () :
					QAudioDeviceInfo::defaultOutputDevice ();
		}
	}
#endif

	void CallManager::HandleIncomingCall (IMediaCall *mediaCall)
	{
		const auto entry = qobject_cast<ICLEntry*> (Core::Instance ().GetEntry (mediaCall->GetSourceID ()));
		const auto& name = entry ?
				entry->GetEntryName () :
				mediaCall->GetSourceID ();

		auto e = Util::MakeNotification ("Azoth",
				tr ("Incoming call from %1").arg (name),
				Priority::Info);
		const auto nh = new Util::NotificationActionHandler (e, this);
		nh->AddFunction (tr ("Accept"), [mediaCall] { mediaCall->Accept (); });
		nh->AddFunction (tr ("Hangup"), [mediaCall] { mediaCall->Hangup (); });

		GetProxyHolder ()->GetEntityManager ()->HandleEntity (e);
	}

	void CallManager::handleCall (QObject *obj)
	{
		const auto mediaCall = qobject_cast<IMediaCall*> (obj);
		if (!mediaCall)
		{
			qWarning () << Q_FUNC_INFO
					<< obj
					<< "is not a IMediaCall, got from"
					<< sender ();
			return;
		}

		const auto& sourceId = mediaCall->GetSourceID ();
		if (Entry2Calls_.value (sourceId).contains (obj))
		{
			qWarning () << Q_FUNC_INFO
					<< "attempt to double-add the call"
					<< obj
					<< "from"
					<< sourceId
					<< sender ();
			return;
		}

		Entry2Calls_ [sourceId] << obj;

		connect (obj,
				SIGNAL (stateChanged (LC::Azoth::IMediaCall::State)),
				this,
				SLOT (handleStateChanged (LC::Azoth::IMediaCall::State)));
		connect (obj,
				SIGNAL (audioModeChanged (QIODevice::OpenMode)),
				this,
				SLOT (handleAudioModeChanged (QIODevice::OpenMode)));
		connect (obj,
				SIGNAL (readFormatChanged ()),
				this,
				SLOT (handleReadFormatChanged ()));
		connect (obj,
				SIGNAL (writeFormatChanged ()),
				this,
				SLOT (handleWriteFormatChanged ()));

		if (mediaCall->GetDirection () == IMediaCall::DIn)
			HandleIncomingCall (mediaCall);
	}

	void CallManager::handleStateChanged (IMediaCall::State state)
	{
		qDebug () << Q_FUNC_INFO << state << (state == IMediaCall::SActive);

		if (state != IMediaCall::SFinished)
			return;

		CallStates_.remove (sender ());

		if (const auto call = qobject_cast<IMediaCall*> (sender ()))
			Entry2Calls_ [call->GetSourceID ()].removeAll (sender ());
		else
			qWarning () << Q_FUNC_INFO
					<< "sender isn't an IMediaCall";
	}

	namespace
	{
#ifdef ENABLE_MEDIACALLS
		void WarnUnsupported (const QAudioDeviceInfo& info, const QAudioFormat& format, const QByteArray& dbgstr)
		{
			qWarning () << dbgstr
					<< "\n\tbyte orders:"
					<< info.supportedByteOrders ()
					<< "\n\tchannel counts:"
					<< info.supportedChannelCounts ()
					<< "\n\tcodecs:"
					<< info.supportedCodecs ()
					<< "\n\tfrequencies:"
					<< info.supportedSampleRates ()
					<< "\n\tsample types:"
					<< info.supportedSampleTypes ();
			qWarning () << "instead we got:"
					<< "\n\tbyte order:"
					<< format.byteOrder ()
					<< "\n\tchannel count:"
					<< format.channelCount ()
					<< "\n\tcodec:"
					<< format.codec ()
					<< "\n\tfrequency:"
					<< format.sampleRate ()
					<< "\n\tsample type:"
					<< format.sampleType ();
		}

		int GetBufSize (const QAudioFormat& format)
		{
			const auto frequency = format.sampleRate ();
			const auto channels = format.channelCount ();
			return (frequency * channels * (format.sampleSize () / 8) * 160) / 1000;
		}
#endif
	}

	void CallManager::handleAudioModeChanged (QIODevice::OpenMode mode)
	{
		qDebug () << Q_FUNC_INFO;
#ifdef ENABLE_MEDIACALLS
		auto& callState = CallStates_ [sender ()];
		callState.LastMode_ = mode;

		if ((mode & QIODevice::WriteOnly) && !callState.InDevice_)
			handleWriteFormatChanged ();

		if ((mode & QIODevice::ReadOnly) && !callState.OutDevice_)
			handleReadFormatChanged ();
#else
		Q_UNUSED (mode)
#endif
	}

#ifdef ENABLE_MEDIACALLS
	namespace
	{
		template<typename FormatGetter, typename DeviceSetter>
		void HandleDeviceReopening (CallState& callState,
				IMediaCall *mediaCall,
				FormatGetter fmtGetter,
				const QAudioDeviceInfo& info,
				DeviceSetter setter,
				QObject *callMgr)
		{
			const auto callAudioDev = mediaCall->GetAudioDevice ();

			const auto& format = std::invoke (fmtGetter, mediaCall);
			if (!format.isValid ())
			{
				qDebug () << "format is invalid for now, waiting for a better chance";
				return;
			}

			qDebug () << "opening:" << info.deviceName () << format;

			if (!info.isFormatSupported (format))
				WarnUnsupported (info, format, "raw audio format not supported by backend");

			using AudioDevice_t = std::decay_t<decltype (*(callState.*setter))>;

			const auto device = std::make_shared<AudioDevice_t> (info, format);

			callState.*setter = device;

			QObject::connect (device.get (),
					SIGNAL (stateChanged (QAudio::State)),
					callMgr,
					SLOT (handleDevStateChanged (QAudio::State)));
			device->setBufferSize (GetBufSize (format));
			device->start (callAudioDev);
		}
	}
#endif

	void CallManager::handleReadFormatChanged ()
	{
		qDebug () << Q_FUNC_INFO;

#ifdef ENABLE_MEDIACALLS
		HandleDeviceReopening (CallStates_ [sender ()],
				qobject_cast<IMediaCall*> (sender ()),
				&IMediaCall::GetAudioReadFormat,
				FindDevice ("OutputAudioDevice", QAudio::AudioOutput),
				&CallState::OutDevice_,
				this);
#endif
	}

	void CallManager::handleWriteFormatChanged ()
	{
		qDebug () << Q_FUNC_INFO;

#ifdef ENABLE_MEDIACALLS
		HandleDeviceReopening (CallStates_ [sender ()],
				qobject_cast<IMediaCall*> (sender ()),
				&IMediaCall::GetAudioWriteFormat,
				FindDevice ("InputAudioDevice", QAudio::AudioInput),
				&CallState::InDevice_,
				this);
#endif
	}

#ifdef ENABLE_MEDIACALLS
	void CallManager::handleDevStateChanged (QAudio::State state)
	{
		auto input = qobject_cast<QAudioInput*> (sender ());
		auto output = qobject_cast<QAudioOutput*> (sender ());
		if (state == QAudio::StoppedState)
		{
			 if (input && input->error () != QAudio::NoError)
				 qWarning () << Q_FUNC_INFO << input->error ();
			 if (output && output->error () != QAudio::NoError)
				 qWarning () << Q_FUNC_INFO << output->error ();
		}
	}
#endif
}
}
