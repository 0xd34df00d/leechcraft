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

#ifndef PLUGINS_SECMAN_PLUGINS_SECURESTORAGE_CIPHERTEXTFORMAT_H
#define PLUGINS_SECMAN_PLUGINS_SECURESTORAGE_CIPHERTEXTFORMAT_H

#include "cryptosystem.h"
#include <QByteArray>

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
					const int HMAC_LENGTH = 256 / 8;
					const int HASH_LENGTH = 256 / 8;
					const int KEY_LENGTH = 256 / 8;
					const int IV_LENGTH = 128 / 8;
					const int RND_LENGTH = sizeof (int);

					/**
					 * Block of ciphertext in specific format.
					 * 
					 * Format:
					 * IV + CIPHER(TEXT+RND) + HMAC(TEXT+RND)
					 */
					struct CipherTextFormat
					{
						/** Pointer to buffer containing ciphertext. */
						unsigned char* Buffer_;
						/** Length of unencrypted data. */
						int DataLength_;

						CipherTextFormat (void* buffer, int dataLength)
							: Buffer_ (reinterpret_cast<unsigned char*> (buffer))
							, DataLength_ (dataLength) { }

						/**
						 * Compute buffer length for known data length.
						 * @param dataLength length of data.
						 * @return length of buffer.
						 */
						static int BufferLengthFor (int dataLength)
						{
							return dataLength + (IV_LENGTH + RND_LENGTH + HMAC_LENGTH);
						}

						/**
						 * Compute data length from known buffer length.
						 * @param bufferlength of buffer.
						 * @return length of data.
						 */
						static int DataLengthFor (int bufferLength)
						{
							return bufferLength - (IV_LENGTH + RND_LENGTH + HMAC_LENGTH);
						}

						/**
						 * Compute decryption buffer length from known buffer length.
						 * Decryption buffer contains data and random block,
						 * and has length (Datalength + RND_LENGTH)
						 * @param bufferlength of buffer.
						 * @return length of buffer for decryption.
						 */
						static int DecryptBufferLengthFor (int bufferLength)
						{
							return bufferLength - (IV_LENGTH + HMAC_LENGTH);
						}

						/** Pointer to initialization vector. */
						unsigned char* Iv () const
						{
							return Buffer_;
						}

						/** Pointer to encrypted data block. */
						unsigned char* Data () const
						{
							return Buffer_ + IV_LENGTH;
						}

						/** Pointer to encrypted random block. */
						unsigned char* Rnd () const
						{
							return Buffer_ + IV_LENGTH + DataLength_;
						}

						/** Pointer to HMAC. */
						unsigned char* Hmac () const
						{
							return Buffer_ + IV_LENGTH + DataLength_ + RND_LENGTH;
						}

						/** Pointer to begin of buffer. */
						unsigned char* BufferBegin () const
						{
							return Buffer_;
						}

						/** Pointer to end of buffer. */
						unsigned char* BufferEnd () const
						{
							return Buffer_ + IV_LENGTH + DataLength_ + RND_LENGTH + HMAC_LENGTH;
						}
					};
				}
			}
		}
	}
}

#endif

