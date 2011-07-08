/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2011  Alexander Konovalov
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

#include "ciphertextformat.h"

namespace LeechCraft
{
namespace Plugins
{
namespace SecMan
{
namespace StoragePlugins
{
namespace SecureStorage
{
	CipherTextFormat::CipherTextFormat (void *buffer, int dataLength)
	: Buffer_ (reinterpret_cast<unsigned char*> (buffer))
	, DataLength_ (dataLength)
	{
	}

	unsigned char* CipherTextFormat::Iv () const
	{
		return Buffer_;
	}

	unsigned char* CipherTextFormat::Data () const
	{
		return Buffer_ + IVLength;
	}

	unsigned char* CipherTextFormat::Rnd () const
	{
		return Buffer_ + IVLength + DataLength_;
	}

	unsigned char* CipherTextFormat::Hmac () const
	{
		return Buffer_ + IVLength + DataLength_ + RndLength;
	}

	unsigned char* CipherTextFormat::BufferBegin () const
	{
		return Buffer_;
	}

	unsigned char* CipherTextFormat::BufferEnd () const
	{
		return Buffer_ + IVLength + DataLength_ + RndLength + HMACLength;
	}

	int CipherTextFormat::GetDataLength () const
	{
		return DataLength_;
	}

	namespace CipherTextFormatUtils
	{
		int BufferLengthFor (int dataLength)
		{
			return dataLength + (IVLength + RndLength + HMACLength);
		}

		int DataLengthFor (int bufferLength)
		{
			return bufferLength - (IVLength + RndLength + HMACLength);
		}

		int DecryptBufferLengthFor (int bufferLength)
		{
			return bufferLength - (IVLength + HMACLength);
		}
	}
}
}
}
}
}
