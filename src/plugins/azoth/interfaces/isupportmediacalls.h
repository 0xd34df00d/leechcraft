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

#ifndef PLUGINS_AZOTH_INTERFACES_ISUPPORTMEDIACALLS_H
#define PLUGINS_AZOTH_INTERFACES_ISUPPORTMEDIACALLS_H
#include <QtPlugin>

namespace LeechCraft
{
namespace Azoth
{
	class ISupportMediaCalls
	{
	public:
		virtual ~ISupportMediaCalls () {}
		
		enum MediaCallFeature
		{
			MCFNoFeatures,
			MCFSupportsAudioCalls = 0x01,
			MCFSupportsVideoCalls = 0x02
		};
		
		Q_DECLARE_FLAGS (MediaCallFeatures, MediaCallFeature);
		
		virtual MediaCallFeatures GetMediaCallFeatures () const = 0;
		
		virtual QObject* Call (const QString& id, const QString& variant) = 0;
		
		virtual void called (QObject*) = 0;
	};
}
}

Q_DECLARE_INTERFACE (LeechCraft::Azoth::ISupportMediaCalls,
		"org.Deviant.LeechCraft.Azoth.ISupportMediaCalls/1.0");

#endif
