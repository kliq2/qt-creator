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

#ifndef SSHABSTRACTCRYPTOFACILITY_P_H
#define SSHABSTRACTCRYPTOFACILITY_P_H

#include <botan/auto_rng.h>
#include <botan/symkey.h>

#include <QtCore/QByteArray>
#include <QtCore/QScopedPointer>

namespace Botan {
    class BigInt;
    class BlockCipher;
    class BlockCipherMode;
    class BlockCipherModePaddingMethod;
    class HashFunction;
    class HMAC;
    class Pipe;
    class PK_Signing_Key;
}

namespace Core {
namespace Internal {

class SshKeyExchange;

class SshAbstractCryptoFacility
{
public:
    virtual ~SshAbstractCryptoFacility();

    void clearKeys();
    void recreateKeys(const SshKeyExchange &kex);
    QByteArray generateMac(const QByteArray &data, quint32 dataSize) const;
    quint32 cipherBlockSize() const { return m_cipherBlockSize; }
    quint32 macLength() const { return m_macLength; }

protected:
    SshAbstractCryptoFacility();
    void convert(QByteArray &data, quint32 offset, quint32 dataSize) const;
    QByteArray sessionId() const { return m_sessionId; }

private:
    SshAbstractCryptoFacility(const SshAbstractCryptoFacility &);
    SshAbstractCryptoFacility &operator=(const SshAbstractCryptoFacility &);

    virtual QByteArray cryptAlgoName(const SshKeyExchange &kex) const=0;
    virtual QByteArray hMacAlgoName(const SshKeyExchange &kex) const=0;
    virtual Botan::BlockCipherMode *makeCipherMode(Botan::BlockCipher *cipher,
        Botan::BlockCipherModePaddingMethod *paddingMethod,
        const Botan::InitializationVector &iv,
        const Botan::SymmetricKey &key)=0;
    virtual char ivChar() const=0;
    virtual char keyChar() const=0;
    virtual char macChar() const=0;

    QByteArray generateHash(const SshKeyExchange &kex, char c, quint32 length);
    void checkInvariant() const;

    QByteArray m_sessionId;
    QScopedPointer<Botan::Pipe> m_pipe;
    QScopedPointer<Botan::HMAC> m_hMac;
    quint32 m_cipherBlockSize;
    quint32 m_macLength;
};

class SshEncryptionFacility : public SshAbstractCryptoFacility
{
public:
    void encrypt(QByteArray &data) const;

    void createAuthenticationKey(const QByteArray &privKeyFileContents);
    QByteArray authenticationAlgorithmName() const;
    QByteArray authenticationPublicKey() const { return m_authPubKeyBlob; }
    QByteArray authenticationKeySignature(const QByteArray &data) const;
    QByteArray getRandomNumbers(int count) const;

    ~SshEncryptionFacility();

private:
    virtual QByteArray cryptAlgoName(const SshKeyExchange &kex) const;
    virtual QByteArray hMacAlgoName(const SshKeyExchange &kex) const;
    virtual Botan::BlockCipherMode *makeCipherMode(Botan::BlockCipher *cipher,
        Botan::BlockCipherModePaddingMethod *paddingMethod,
        const Botan::InitializationVector &iv, const Botan::SymmetricKey &key);
    virtual char ivChar() const { return 'A'; }
    virtual char keyChar() const { return 'C'; }
    virtual char macChar() const { return 'E'; }

    void createAuthenticationKeyFromPKCS8(const QByteArray &privKeyFileContents,
        QList<Botan::BigInt> &pubKeyParams, QList<Botan::BigInt> &allKeyParams);
    void createAuthenticationKeyFromOpenSSL(const QByteArray &privKeyFileContents,
        QList<Botan::BigInt> &pubKeyParams, QList<Botan::BigInt> &allKeyParams);

    static const QByteArray PrivKeyFileStartLineRsa;
    static const QByteArray PrivKeyFileStartLineDsa;
    static const QByteArray PrivKeyFileEndLineRsa;
    static const QByteArray PrivKeyFileEndLineDsa;

    QByteArray m_authKeyAlgoName;
    QByteArray m_authPubKeyBlob;
    QByteArray m_cachedPrivKeyContents;
    QScopedPointer<Botan::PK_Signing_Key> m_authKey;
    mutable Botan::AutoSeeded_RNG m_rng;
};

class SshDecryptionFacility : public SshAbstractCryptoFacility
{
public:
    void decrypt(QByteArray &data, quint32 offset, quint32 dataSize) const;

private:
    virtual QByteArray cryptAlgoName(const SshKeyExchange &kex) const;
    virtual QByteArray hMacAlgoName(const SshKeyExchange &kex) const;
    virtual Botan::BlockCipherMode *makeCipherMode(Botan::BlockCipher *cipher,
        Botan::BlockCipherModePaddingMethod *paddingMethod,
        const Botan::InitializationVector &iv, const Botan::SymmetricKey &key);
    virtual char ivChar() const { return 'B'; }
    virtual char keyChar() const { return 'D'; }
    virtual char macChar() const { return 'F'; }
};

} // namespace Internal
} // namespace Core

#endif // SSHABSTRACTCRYPTOFACILITY_P_H
