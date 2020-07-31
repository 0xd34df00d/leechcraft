/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <memory>
#include <QObject>
#include <QHash>

#ifdef ENABLE_MEDIACALLS
#include <QAudio>
#endif

#include "interfaces/azoth/imediacall.h"
#include "interfaces/azoth/isupportmediacalls.h"

class QAudioInput;
class QAudioOutput;

namespace LC
{
namespace Azoth
{
	class ICLEntry;
	class IMediaCall;

	struct CallState;

	class CallManager : public QObject
	{
		Q_OBJECT

		QHash<QString, QObjectList> Entry2Calls_;

		QHash<QObject*, CallState> CallStates_;
	public:
		CallManager (QObject* = 0);
		~CallManager ();

		void AddAccount (QObject*);
		QObject* Call (ICLEntry*, const QString&);
		QObjectList GetCallsForEntry (const QString&) const;
	private:
		void HandleIncomingCall (IMediaCall*);
	private slots:
		void handleCall (QObject*);
		void handleStateChanged (LC::Azoth::IMediaCall::State);
		void handleAudioModeChanged (QIODevice::OpenMode);

		void handleReadFormatChanged ();
		void handleWriteFormatChanged ();

#ifdef ENABLE_MEDIACALLS
		void handleDevStateChanged (QAudio::State);
#endif
	};
}
}
