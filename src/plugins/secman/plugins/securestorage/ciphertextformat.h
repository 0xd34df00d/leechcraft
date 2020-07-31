/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2011  Alexander Konovalov
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

namespace LC
{
namespace SecMan
{
namespace SecureStorage
{
	const int HMACLength = 256 / 8;
	const int HashLength = 256 / 8;
	const int KeyLength = 256 / 8;
	const int IVLength = 128 / 8;
	const int RndLength = 4;

	namespace CipherTextFormatUtils
	{
		/**
		 * Compute buffer length for known data length.
		 * @param dataLength length of data.
		 * @return length of buffer.
		 */
		int BufferLengthFor (int dataLength);

		/**
		 * Compute data length from known buffer length.
		 * @param bufferlength of buffer.
		 * @return length of data.
		 */
		int DataLengthFor (int bufferLength);

		/**
		 * Compute decryption buffer length from known buffer length.
		 * Decryption buffer contains data and random block,
		 * and has length (Datalength + RND_LENGTH)
		 * @param bufferlength of buffer.
		 * @return length of buffer for decryption.
		 */
		int DecryptBufferLengthFor (int bufferLength);
	}

	/**
	 * Block of ciphertext in specific format.
	 *
	 * Format:
	 * IV + CIPHER(TEXT+RND) + HMAC(TEXT+RND)
	 */
	class CipherTextFormat
	{
		/** Pointer to buffer containing ciphertext. */
		unsigned char *Buffer_;
		/** Length of unencrypted data. */
		int DataLength_;

	public:
		CipherTextFormat (void *buffer, int dataLength);
		/** Pointer to initialization vector. */
		unsigned char* Iv () const;
		/** Pointer to encrypted data block. */
		unsigned char* Data () const;
		/** Pointer to encrypted random block. */
		unsigned char* Rnd () const;
		/** Pointer to HMAC. */
		unsigned char* Hmac () const;
		/** Pointer to beginning of buffer. */
		unsigned char* BufferBegin () const;
		/** Pointer to end of buffer. */
		unsigned char* BufferEnd () const;
		/** Length of data.*/
		int GetDataLength () const;
	};
}
}
}
