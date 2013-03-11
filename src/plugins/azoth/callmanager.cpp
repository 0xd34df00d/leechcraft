/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2013  Georg Rudoy
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

#include "callmanager.h"

#ifdef ENABLE_MEDIACALLS
#include <QAudioDeviceInfo>
#include <QAudioInput>
#include <QAudioOutput>
#endif

#include <QtDebug>
#include <util/util.h>
#include <util/notificationactionhandler.h>
#include "interfaces/azoth/iclentry.h"
#include "xmlsettingsmanager.h"
#include "core.h"

namespace LeechCraft
{
namespace Azoth
{
	CallManager::CallManager (QObject *parent)
	: QObject (parent)
	{
	}

	void CallManager::AddAccount (QObject *account)
	{
		if (!qobject_cast<ISupportMediaCalls*> (account))
			return;

		connect (account,
				SIGNAL (called (QObject*)),
				this,
				SLOT (handleIncomingCall (QObject*)));
	}

	QObject* CallManager::Call (ICLEntry *entry, const QString& variant)
	{
#ifdef ENABLE_MEDIACALLS
		ISupportMediaCalls *ismc = qobject_cast<ISupportMediaCalls*> (entry->GetParentAccount ());
		if (!ismc)
		{
			qWarning () << Q_FUNC_INFO
					<< entry->GetObject ()
					<< "parent account doesn't support media calls";
			return 0;
		}

		QObject *callObj = ismc->Call (entry->GetEntryID (), variant);
		if (!callObj)
		{
			qWarning () << Q_FUNC_INFO
					<< "got null call obj for"
					<< entry->GetEntryID ()
					<< variant;
			return 0;
		}

		HandleCall (callObj);
		return callObj;
#else
		return 0;
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
			const QString& name = XmlSettingsManager::Instance ()
					.property (property).toString ();

			QAudioDeviceInfo result = mode == QAudio::AudioInput ?
					QAudioDeviceInfo::defaultInputDevice () :
					QAudioDeviceInfo::defaultOutputDevice ();
			Q_FOREACH (const QAudioDeviceInfo& info,
					QAudioDeviceInfo::availableDevices (mode))
				if (info.deviceName () == name)
				{
					result = info;
					break;
				}

			return result;
		}
	}
#endif

	void CallManager::HandleCall (QObject *obj)
	{
		IMediaCall *mediaCall = qobject_cast<IMediaCall*> (obj);
		if (!mediaCall)
		{
			qWarning () << Q_FUNC_INFO
					<< obj
					<< "is not a IMediaCall, got from"
					<< sender ();
			return;
		}

		Entry2Calls_ [mediaCall->GetSourceID ()] << obj;

		connect (obj,
				SIGNAL (stateChanged (LeechCraft::Azoth::IMediaCall::State)),
				this,
				SLOT (handleStateChanged (LeechCraft::Azoth::IMediaCall::State)));
		connect (obj,
				SIGNAL (audioModeChanged (QIODevice::OpenMode)),
				this,
				SLOT (handleAudioModeChanged (QIODevice::OpenMode)));
	}

	void CallManager::handleIncomingCall (QObject *obj)
	{
		HandleCall (obj);

		IMediaCall *call = qobject_cast<IMediaCall*> (obj);

		ICLEntry *entry = qobject_cast<ICLEntry*> (Core::Instance ().GetEntry (call->GetSourceID ()));
		const QString& name = entry ?
				entry->GetEntryName () :
				call->GetSourceID ();

		Entity e = Util::MakeNotification ("Azoth",
				tr ("Incoming call from %1").arg (name),
				PInfo_);
		Util::NotificationActionHandler *nh =
				new Util::NotificationActionHandler (e, this);
		nh->AddFunction (tr ("Accept"), [call] () { call->Accept (); });
		nh->AddFunction (tr ("Hangup"), [call] () { call->Hangup (); });
		Core::Instance ().SendEntity (e);

		emit gotCall (obj);
	}

	void CallManager::handleStateChanged (IMediaCall::State state)
	{
		qDebug () << Q_FUNC_INFO << state << (state == IMediaCall::SActive);
	}

	void CallManager::handleAudioModeChanged (QIODevice::OpenMode mode)
	{
		qDebug () << Q_FUNC_INFO;
#ifdef ENABLE_MEDIACALLS
		IMediaCall *mediaCall = qobject_cast<IMediaCall*> (sender ());
		QIODevice *callAudioDev = mediaCall->GetAudioDevice ();

		const QAudioFormat& format = mediaCall->GetAudioFormat ();
		const int bufSize = (format.frequency () * format.channels () * (format.sampleSize () / 8) * 160) / 1000;

		if (mode & QIODevice::WriteOnly)
		{
			qDebug () << "opening output...";
			QAudioDeviceInfo info (QAudioDeviceInfo::defaultOutputDevice ());
			if (!info.isFormatSupported (format))
				qWarning () << "raw audio format not supported by backend, cannot play audio"
						<< info.supportedByteOrders () << info.supportedChannelCounts ()
						<< info.supportedCodecs () << info.supportedFrequencies ()
						<< info.supportedSampleTypes ();

			QAudioDeviceInfo outInfo = FindDevice ("OutputAudioDevice", QAudio::AudioOutput);
			QAudioOutput *output = new QAudioOutput (/*outInfo, */format, sender ());
			connect (output,
					SIGNAL (stateChanged (QAudio::State)),
					this,
					SLOT (handleDevStateChanged (QAudio::State)));
			output->setBufferSize (bufSize);
			output->start (callAudioDev);
		}

		if (mode & QIODevice::ReadOnly)
		{
			qDebug () << "opening input...";
			QAudioDeviceInfo info (QAudioDeviceInfo::defaultInputDevice ());
			if (!info.isFormatSupported (format))
				qWarning () << "raw audio format not supported by backend, cannot record audio"
						<< info.supportedByteOrders () << info.supportedChannelCounts ()
						<< info.supportedCodecs () << info.supportedFrequencies ()
						<< info.supportedSampleTypes ();

			QAudioDeviceInfo inInfo = FindDevice ("InputAudioDevice", QAudio::AudioInput);
			QAudioInput *input = new QAudioInput (/*inInfo, */format, sender ());
			connect (input,
					SIGNAL (stateChanged (QAudio::State)),
					this,
					SLOT (handleDevStateChanged (QAudio::State)));
			input->setBufferSize (bufSize);
			input->start (callAudioDev);
		}
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
