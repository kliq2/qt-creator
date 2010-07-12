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

#include "sftpoperation_p.h"

#include "sftpoutgoingpacket_p.h"

#include <QtCore/QTime>
#include <QtCore/QFile>

namespace Core {
namespace Internal {

AbstractSftpOperation::AbstractSftpOperation(SftpJobId jobId) : jobId(jobId)
{
}

AbstractSftpOperation::~AbstractSftpOperation() { }


SftpMakeDir::SftpMakeDir(SftpJobId jobId, const QString &path)
    : AbstractSftpOperation(jobId), remoteDir(path)
{
}

SftpOutgoingPacket &SftpMakeDir::initialPacket(SftpOutgoingPacket &packet)
{
    return packet.generateMkDir(remoteDir, jobId);
}


SftpRmDir::SftpRmDir(SftpJobId, const QString &path)
    : AbstractSftpOperation(jobId), remoteDir(path)
{
}

SftpOutgoingPacket &SftpRmDir::initialPacket(SftpOutgoingPacket &packet)
{
    return packet.generateRmDir(remoteDir, jobId);
}


SftpRm::SftpRm(SftpJobId jobId, const QString &path)
    : AbstractSftpOperation(jobId), remoteFile(path) {}

SftpOutgoingPacket &SftpRm::initialPacket(SftpOutgoingPacket &packet)
{
    return packet.generateRm(remoteFile, jobId);
}


SftpRename::SftpRename(SftpJobId jobId, const QString &oldPath,
    const QString &newPath)
    : AbstractSftpOperation(jobId), oldPath(oldPath), newPath(newPath)
{
}

SftpOutgoingPacket &SftpRename::initialPacket(SftpOutgoingPacket &packet)
{
    return packet.generateRename(oldPath, newPath, jobId);
}


AbstractSftpOperationWithHandle::AbstractSftpOperationWithHandle(SftpJobId jobId,
    const QString &remotePath)
    : AbstractSftpOperation(jobId),
      remotePath(remotePath), state(Inactive), hasError(false)
{
}

AbstractSftpOperationWithHandle::~AbstractSftpOperationWithHandle() { }


SftpListDir::SftpListDir(SftpJobId jobId, const QString &path)
    : AbstractSftpOperationWithHandle(jobId, path)
{
}

SftpOutgoingPacket &SftpListDir::initialPacket(SftpOutgoingPacket &packet)
{
    state = OpenRequested;
    return packet.generateOpenDir(remotePath, jobId);
}


SftpCreateFile::SftpCreateFile(SftpJobId jobId, const QString &path,
    SftpOverwriteMode mode)
    : AbstractSftpOperationWithHandle(jobId, path), mode(mode)
{
}

SftpOutgoingPacket & SftpCreateFile::initialPacket(SftpOutgoingPacket &packet)
{
    state = OpenRequested;
    return packet.generateOpenFileForWriting(remotePath, mode, jobId);
}


const int AbstractSftpTransfer::MaxInFlightCount = 10; // Experimentally found to be enough.

AbstractSftpTransfer::AbstractSftpTransfer(SftpJobId jobId, const QString &remotePath,
    const QSharedPointer<QFile> &localFile)
    : AbstractSftpOperationWithHandle(jobId, remotePath),
      localFile(localFile), fileSize(0), offset(0), inFlightCount(0),
      statRequested(false)
{
}

void AbstractSftpTransfer::calculateInFlightCount(quint32 chunkSize)
{
    if (fileSize == 0) {
        inFlightCount = 1;
    } else {
        inFlightCount = fileSize / chunkSize;
        if (fileSize % chunkSize)
            ++inFlightCount;
        if (inFlightCount > MaxInFlightCount)
            inFlightCount = MaxInFlightCount;
    }
}


SftpDownload::SftpDownload(SftpJobId jobId, const QString &remotePath,
    const QSharedPointer<QFile> &localFile)
    : AbstractSftpTransfer(jobId, remotePath, localFile), eofId(SftpInvalidJob)
{
}

SftpOutgoingPacket &SftpDownload::initialPacket(SftpOutgoingPacket &packet)
{
    state = OpenRequested;
    return packet.generateOpenFileForReading(remotePath, jobId);
}


SftpUpload::SftpUpload(SftpJobId jobId, const QString &remotePath,
    const QSharedPointer<QFile> &localFile, SftpOverwriteMode mode)
    : AbstractSftpTransfer(jobId, remotePath, localFile), mode(mode)
{
    fileSize = localFile->size();
}

SftpOutgoingPacket &SftpUpload::initialPacket(SftpOutgoingPacket &packet)
{
    state = OpenRequested;
    return packet.generateOpenFileForWriting(remotePath, mode, jobId);
}

} // namespace Internal
} // namespace Core
