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

#ifndef PLUGINS_POSHUKU_INTERFACES_POSHUKUTYPES_H
#define PLUGINS_POSHUKU_INTERFACES_POSHUKUTYPES_H

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Poshuku
		{
			/** Enumeration describing the part of menu that's being
			 * constructed inside QWebView's subclass'
			 * contextMenuEvent.
			 */
			enum WebViewCtxMenuStage
			{
				/// Just the beginning of menu construction.
				WVSStart,
				/// Stage related to clicking on a hyperlink finished.
				WVSAfterLink,
				/// Stage related to clicking on an image finished.
				WVSAfterImage,
				/// Stage related to clicking with having some
				/// selected text finished.
				WVSAfterSelectedText,
				/// The standard set of actions was embedded, This
				/// stage is just before executing the menu.
				WVSAfterFinish
			};
		};
	};
};

#endif
