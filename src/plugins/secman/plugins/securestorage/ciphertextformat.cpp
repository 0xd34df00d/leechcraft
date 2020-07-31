/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2011  Alexander Konovalov
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "ciphertextformat.h"

namespace LC
{
namespace SecMan
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
