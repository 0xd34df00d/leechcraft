/****************************************************************************
**
** Copyright (C) 2009 Nokia Corporation and/or its subsidiary(-ies).
** Contact: Qt Software Information (qt-info@nokia.com)
**
** This file is part of the Qt Messaging Framework.
**
** $QT_BEGIN_LICENSE:LGPL$
** No Commercial Usage
** This file contains pre-release code and may not be distributed.
** You may use this file in accordance with the terms and conditions
** contained in the either Technology Preview License Agreement or the
** Beta Release License Agreement.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain
** additional rights. These rights are described in the Nokia Qt LGPL
** Exception version 1.0, included in the file LGPL_EXCEPTION.txt in this
** package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
** If you are unsure which license is appropriate for your use, please
** contact the sales department at qt-sales@nokia.com.
** $QT_END_LICENSE$
**
****************************************************************************/
#ifndef QMAILCODEC_H
#define QMAILCODEC_H

#define QTOPIAMAIL_EXPORT

#include <QByteArray>
#include <QDataStream>
#include <QString>
#include <QTextStream>

class QTOPIAMAIL_EXPORT QMailCodec
{
public:
    enum { ChunkCharacters = 8192 };

    virtual ~QMailCodec();

    virtual QString name() const = 0;

    // Stream conversion interface including character translation
    virtual void encode(QDataStream& out, QTextStream& in, const QString& charset = "UTF-8");
    virtual void decode(QTextStream& out, QDataStream& in, const QString& charset);

    // Stream conversion interface
    virtual void encode(QDataStream& out, QDataStream& in);
    virtual void decode(QDataStream& out, QDataStream& in);

    // Convenience functions to encapsulate stream processing
    QByteArray encode(const QString& in, const QString& charset = "UTF-8");
    QString decode(const QByteArray& in, const QString& charset);

    QByteArray encode(const QByteArray& in);
    QByteArray decode(const QByteArray& in);

    // Static helper functions
    static void copy(QDataStream& out, QDataStream& in);
    static void copy(QTextStream& out, QTextStream& in);

protected:
    // Helper functions to convert stream chunks
    virtual void encodeChunk(QDataStream& out, const unsigned char* in, int length, bool finalChunk) = 0;
    virtual void decodeChunk(QDataStream& out, const char* in, int length, bool finalChunk) = 0;
};

class QTOPIAMAIL_EXPORT QMailBase64Codec : public QMailCodec
{
public:
    enum ContentType { Text, Binary };

    QMailBase64Codec(ContentType content, int maximumLineLength = -1);

    virtual QString name() const;

protected:
    virtual void encodeChunk(QDataStream& out, const unsigned char* in, int length, bool finalChunk);
    virtual void decodeChunk(QDataStream& out, const char* in, int length, bool finalChunk);

private:
    ContentType _content;
    int _maximumLineLength;

    unsigned char _encodeBuffer[3];
    unsigned char* _encodeBufferOut;
    int _encodeLineCharsRemaining;

    unsigned char _decodeBuffer[4];
    unsigned char* _decodeBufferOut;
    int _decodePaddingCount;

    unsigned char _lastChar;
};

class QTOPIAMAIL_EXPORT QMailQuotedPrintableCodec : public QMailCodec
{
public:
    enum ContentType { Text, Binary };
    enum ConformanceType { Rfc2045, Rfc2047 };

    QMailQuotedPrintableCodec(ContentType content, ConformanceType conformance, int maximumLineLength = -1);

    virtual QString name() const;

protected:
    virtual void encodeChunk(QDataStream& out, const unsigned char* in, int length, bool finalChunk);
    virtual void decodeChunk(QDataStream& out, const char* in, int length, bool finalChunk);

private:
    ContentType _content;
    ConformanceType _conformance;
    int _maximumLineLength;

    int _encodeLineCharsRemaining;
    unsigned char _encodeLastChar;

    char _decodePrecedingInput;
    unsigned char _decodeLastChar;
};

class QTOPIAMAIL_EXPORT QMailPassThroughCodec : public QMailCodec
{
    virtual QString name() const;

protected:
    virtual void encodeChunk(QDataStream& out, const unsigned char* in, int length, bool finalChunk);
    virtual void decodeChunk(QDataStream& out, const char* in, int length, bool finalChunk);
};

class QTOPIAMAIL_EXPORT QMailLineEndingCodec : public QMailCodec
{
    virtual QString name() const;

public:
    QMailLineEndingCodec();

protected:
    virtual void encodeChunk(QDataStream& out, const unsigned char* in, int length, bool finalChunk);
    virtual void decodeChunk(QDataStream& out, const char* in, int length, bool finalChunk);

private:
    unsigned char _lastChar;
};

#endif
