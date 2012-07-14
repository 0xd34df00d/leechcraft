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

#ifndef PLUGINS_AZOTH_INTERFACES_IMEDIACALL_H
#define PLUGINS_AZOTH_INTERFACES_IMEDIACALL_H
#include <QtPlugin>

class QIODevice;
class QAudioFormat;

namespace LeechCraft
{
namespace Azoth
{
	class IMediaCall
	{
	public:
		virtual ~IMediaCall () {}
		
		enum Direction
		{
			DIn,
			DOut
		};
		
		enum State
		{
			SConnecting,
			SActive,
			SDisconnecting,
			SFinished
		};
		
		virtual Direction GetDirection () const = 0;
		
		virtual QString GetSourceID () const = 0;
		
		virtual void Accept () = 0;

		virtual void Hangup () = 0;
		
		virtual QIODevice* GetAudioDevice () = 0;
		
		virtual QAudioFormat GetAudioFormat () = 0;
		
		virtual QIODevice* GetVideoDevice () = 0;
		
		virtual void stateChanged (State) = 0;
	};
}
}

Q_DECLARE_INTERFACE (LeechCraft::Azoth::IMediaCall,
		"org.Deviant.LeechCraft.Azoth.IMediaCall/1.0");

#endif
