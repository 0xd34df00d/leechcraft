/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2011  Georg Rudoy
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

#ifndef PLUGINS_AZOTH_PLUGINS_XOOX_MEDIACALL_H
#define PLUGINS_AZOTH_PLUGINS_XOOX_MEDIACALL_H
#include <QObject>
#include <QXmppCallManager.h>
#include <interfaces/imediacall.h>

namespace LeechCraft
{
namespace Azoth
{
namespace Xoox
{
	class GlooxAccount;

	class MediaCall : public QObject
					, public IMediaCall
	{
		Q_OBJECT
		Q_INTERFACES (LeechCraft::Azoth::IMediaCall);

		QXmppCall *Call_;
		GlooxAccount *Account_;
	public:
		MediaCall (GlooxAccount*, QXmppCall*);

		Direction GetDirection () const;
		QString GetSourceID () const;
		void Accept ();
		void Hangup ();
		QIODevice* GetAudioDevice ();
		AudioParams GetAudioParams ();
		QIODevice* GetVideoDevice ();
	private slots:
		void handleStateChanged (QXmppCall::State);
	signals:
		void stateChanged (LeechCraft::Azoth::IMediaCall::State);
	};
}
}
}

#endif
