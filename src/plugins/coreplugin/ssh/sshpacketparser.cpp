/**************************************************************************
**
** This file is part of Qt Creator
**
** Copyright (c) 2010 Nokia Corporation and/or its subsidiary(-ies).
**
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** Commercial Usage
**
** Licensees holding valid Qt Commercial licenses may use this file in
** accordance with the Qt Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Nokia.
**
** GNU Lesser General Public License Usage
**
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** If you are unsure which license is appropriate for your use, please
** contact the sales department at http://qt.nokia.com/contact.
**
**************************************************************************/

#include "sshpacketparser_p.h"

#include <cctype>

namespace Core {
namespace Internal {

namespace { quint32 size(const QByteArray &data) { return data.size(); } }

QString SshPacketParser::asUserString(const QByteArray &rawString)
{
    QByteArray filteredString;
    filteredString.resize(rawString.size());
    for (int i = 0; i < rawString.size(); ++i) {
        const char c = rawString.at(i);
        filteredString[i]
            = std::isprint(c) || c == '\n' || c == '\r' || c == '\t' ? c : '?';
    }
    return QString::fromUtf8(filteredString);
}

bool SshPacketParser::asBool(const QByteArray &data, quint32 offset)
{
    if (size(data) <= offset)
        throw SshPacketParseException();
    return data.at(offset);
}

bool SshPacketParser::asBool(const QByteArray &data, quint32 *offset)
{
    bool b = asBool(data, *offset);
    ++(*offset);
    return b;
}


quint32 SshPacketParser::asUint32(const QByteArray &data, quint32 offset)
{
    if (size(data) < offset + 4)
        throw SshPacketParseException();
    const quint32 value = ((data.at(offset) & 0xff) << 24)
        + ((data.at(offset + 1) & 0xff) << 16)
        + ((data.at(offset + 2) & 0xff) << 8) + (data.at(offset + 3) & 0xff);
    return value;
}

quint32 SshPacketParser::asUint32(const QByteArray &data, quint32 *offset)
{
    const quint32 v = asUint32(data, *offset);
    *offset += 4;
    return v;
}

quint64 SshPacketParser::asUint64(const QByteArray &data, quint32 offset)
{
    if (size(data) < offset + 8)
        throw SshPacketParseException();
    const quint64 value = (static_cast<quint64>(data.at(offset) & 0xff) << 56)
        + (static_cast<quint64>(data.at(offset + 1) & 0xff) << 48)
        + (static_cast<quint64>(data.at(offset + 2) & 0xff) << 40)
        + (static_cast<quint64>(data.at(offset + 3) & 0xff) << 32)
        + ((data.at(offset + 4) & 0xff) << 24)
        + ((data.at(offset + 5) & 0xff) << 16)
        + ((data.at(offset + 6) & 0xff) << 8)
        + (data.at(offset + 7) & 0xff);
    return value;
}

quint64 SshPacketParser::asUint64(const QByteArray &data, quint32 *offset)
{
    const quint64 val = asUint64(data, *offset);
    *offset += 8;
    return val;
}

QByteArray SshPacketParser::asString(const QByteArray &data, quint32 *offset)
{
    const quint32 length = asUint32(data, offset);
    if (size(data) < *offset + length)
        throw SshPacketParseException();
    const QByteArray &string = data.mid(*offset, length);
    *offset += length;
    return string;
}

QString SshPacketParser::asUserString(const QByteArray &data, quint32 *offset)
{
    return asUserString(asString(data, offset));
}

SshNameList SshPacketParser::asNameList(const QByteArray &data, quint32 *offset)
{
    const quint32 length = asUint32(data, offset);
    const int listEndPos = *offset + length;
    if (data.size() < listEndPos)
        throw SshPacketParseException();
    SshNameList names(length + 4);
    int nextNameOffset = *offset;
    int nextCommaOffset = data.indexOf(',', nextNameOffset);
    while (nextNameOffset > 0 && nextNameOffset < listEndPos) {
        const int stringEndPos = nextCommaOffset == -1
            || nextCommaOffset > listEndPos ? listEndPos : nextCommaOffset;
        names.names << QByteArray(data.constData() + nextNameOffset,
            stringEndPos - nextNameOffset);
        nextNameOffset = nextCommaOffset + 1;
        nextCommaOffset = data.indexOf(',', nextNameOffset);
    }
    *offset += length;
    return names;
}

Botan::BigInt SshPacketParser::asBigInt(const QByteArray &data, quint32 *offset)
{
    const quint32 length = asUint32(data, offset);
    if (length == 0)
        return Botan::BigInt();
    const Botan::byte *numberStart
        = reinterpret_cast<const Botan::byte *>(data.constData() + *offset);
    *offset += length;
    return Botan::BigInt::decode(numberStart, length);
}

} // namespace Internal
} // namespace Core
