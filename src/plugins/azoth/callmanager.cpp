/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2012  Georg Rudoy
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
#include "interfaces/iclentry.h"
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
#ifdef ENABLE_MEDIACALLS
		IMediaCall *mediaCall = qobject_cast<IMediaCall*> (sender ());
		if (state == IMediaCall::SActive)
		{
			QIODevice *callAudioDev = mediaCall->GetAudioDevice ();
			QAudioDeviceInfo inInfo = FindDevice ("InputAudioDevice", QAudio::AudioInput);
			QAudioDeviceInfo outInfo = FindDevice ("OutputAudioDevice", QAudio::AudioOutput);

			const QAudioFormat& callFormat = mediaCall->GetAudioFormat ();
			const QAudioFormat& inFormat = callFormat;
			const QAudioFormat& outFormat = callFormat;

#if QT_VERSION >= 0x040700
			const int bufSize = callFormat.sampleRate () * callFormat.channelCount () * callFormat.sampleSize () / 8 * 160 / 1000;
#else
			const int bufSize = 8000 * 2 * callFormat.sampleSize () / 8 * 160 / 1000;
#endif

			QAudioOutput *output = new QAudioOutput (outInfo, outFormat, sender ());
			output->setBufferSize (bufSize);
			output->start (callAudioDev);

			QAudioInput *input = new QAudioInput (inInfo, inFormat, sender ());
			input->setBufferSize (bufSize);
			input->start (callAudioDev);

			qDebug () << input->state () << input->error () << inInfo.isFormatSupported (callFormat) << inInfo.supportedCodecs ();
			qDebug () << output->state () << output->error ();
		}
#endif
	}
}
}
