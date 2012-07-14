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

#pragma once

#include <QString>
#include <QByteArray>
#include <QTextCodec>

namespace LeechCraft
{
namespace Azoth
{
namespace Vader
{
namespace Proto
{
	class UIDL
	{
		QByteArray ID_;
	public:
		UIDL ();
		UIDL (const UIDL&);
		explicit UIDL (const QByteArray&);

		UIDL& operator= (const QByteArray&);

		operator QByteArray () const;
	};

	QByteArray ToMRIM1251 (const QString&);
	QByteArray ToMRIM16 (const QString&);
	QByteArray ToMRIM (const QString&);
	QByteArray ToMRIM (const QByteArray&);
	QByteArray ToMRIM (const UIDL&);
	QByteArray ToMRIM (quint32);
	QByteArray ToMRIM (int);
	QByteArray ToMRIM ();

	template<typename T, typename... Args>
	QByteArray ToMRIM (T t, Args... args)
	{
		return ToMRIM (t) + ToMRIM (args...);
	}

	struct EncoderProxy
	{
		QString Str_;

		EncoderProxy& operator= (const QByteArray& ba)
		{
			Str_ = QTextCodec::codecForName (GetCodecName ())->toUnicode (ba);
			return *this;
		}

		operator QString () const { return Str_; }
	protected:
		virtual QByteArray GetCodecName () = 0;
	};

	struct Str1251 : EncoderProxy
	{
	protected:
		QByteArray GetCodecName () { return "Windows-1251"; }
	};

	struct Str16 : EncoderProxy
	{
	protected:
		QByteArray GetCodecName () { return "UTF-16LE"; }
	};

	QString FromMRIM1251 (const QByteArray&);
	QString FromMRIM16 (const QByteArray&);
	void FromMRIM (QByteArray&, EncoderProxy&);
	inline void FromMRIM (QByteArray& ba, Str1251& str) { FromMRIM (ba, static_cast<EncoderProxy&> (str)); }
	inline void FromMRIM (QByteArray& ba, Str16& str){ FromMRIM (ba, static_cast<EncoderProxy&> (str)); }
	void FromMRIM (QByteArray&, UIDL&);
	void FromMRIM (QByteArray&, QByteArray&);
	void FromMRIM (QByteArray&, quint32&);
	void FromMRIM (QByteArray&);

	template<typename T, typename... Args>
	void FromMRIM (QByteArray& ba, T& u, Args&... args)
	{
		FromMRIM (ba, u);
		FromMRIM (ba, args...);
	}
}
}
}
}
