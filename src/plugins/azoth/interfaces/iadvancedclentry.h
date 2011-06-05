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

#ifndef PLUGINS_AZOTH_INTERFACES_IADVANCEDCLENTRY_H
#define PLUGINS_AZOTH_INTERFACES_IADVANCEDCLENTRY_H
#include "imucbookmarkeditorwidget.h"

namespace LeechCraft
{
namespace Azoth
{
	/** This interface is to be implemented by CL entries that can more
	 * that basic ICLEntry interface exposes.
	 * 
	 * @sa ICLEntry
	 */
	class IAdvancedCLEntry
	{
	public:		
		virtual ~IAdvancedCLEntry () {}
		
		/** This enum represents some advanced features that may or may
		 * be not supported by advanced CL entries.
		 */
		enum AdvancedFeature
		{
			/** This entry supports drawing attention.
			 */
			AFSupportsAttention = 0x0001
		};
		
		Q_DECLARE_FLAGS (AdvancedFeatures, AdvancedFeature);
		
		virtual AdvancedFeatures GetAdvancedFeatures () const = 0;
		
		virtual void DrawAttention (const QString& text, const QString& variant) = 0;

		virtual void attentionDrawn (const QString& text, const QString& variant) = 0;
	};
}
}

Q_DECLARE_INTERFACE (LeechCraft::Azoth::IAdvancedCLEntry,
		"org.Deviant.LeechCraft.Azoth.IAdvancedCLEntry/1.0");

#endif
