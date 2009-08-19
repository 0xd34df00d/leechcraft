/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2009  Georg Rudoy
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

#include "morphfile.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace CSTP
		{
			MorphFile::MorphFile (const QString& name)
			: QFile (name)
			, Gunzip_ (false)
			, Counter_ (0)
			{
			}
			
			MorphFile::MorphFile (QObject *parent)
			: QFile (parent)
			, Gunzip_ (false)
			, Counter_ (0)
			{
			}
			
			MorphFile::MorphFile (const QString& name, QObject *parent)
			: QFile (name, parent)
			, Gunzip_ (false)
			, Counter_ (0)
			{
			}
			
			MorphFile::~MorphFile ()
			{
			}
			
			void MorphFile::AddRef ()
			{
				++Counter_;
			}
			
			void MorphFile::Release ()
			{
				--Counter_;
				if (!Counter_)
					deleteLater ();
			}
			
			void MorphFile::Gunzip (bool state)
			{
				Gunzip_ = state;
			}
			
			void intrusive_ptr_add_ref (MorphFile *file)
			{
				file->AddRef ();
			}
			
			void intrusive_ptr_release (MorphFile *file)
			{
				file->Release ();
			}
		};
	};
};

