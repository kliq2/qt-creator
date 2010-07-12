#include <coreplugin/ssh/sftpchannel.h>
#include <coreplugin/ssh/sshconnection.h>
#include <coreplugin/ssh/sshremoteprocess.h>

#include <QtCore/QCoreApplication>
#include <QtCore/QList>
#include <QtCore/QObject>
#include <QtCore/QPair>
#include <QtCore/QTimer>

using namespace Core;

class Test : public QObject {
    Q_OBJECT
public:
    Test()
    {
        m_timeoutTimer.setSingleShot(true);
        m_connection = SshConnection::create();
        if (m_connection->state() != SshConnection::Unconnected) {
            qDebug("Error: Newly created SSH connection has state %d.",
                m_connection->state());
        }

        if (m_connection->createRemoteProcess(""))
            qDebug("Error: Unconnected SSH connection creates remote process.");
        if (m_connection->createSftpChannel())
            qDebug("Error: Unconnected SSH connection creates SFTP channel.");

        SshConnectionParameters noHost;
        noHost.host = QLatin1String("hgdfxgfhgxfhxgfchxgcf");
        noHost.port = 12345;
        noHost.timeout = 10;

        SshConnectionParameters noUser;
        noUser.host = QLatin1String("localhost");
        noUser.port = 22;
        noUser.timeout = 30;
        noUser.authType = SshConnectionParameters::AuthByPwd;
        noUser.uname = QLatin1String("dumdidumpuffpuff");
        noUser.uname = QLatin1String("whatever");

        SshConnectionParameters wrongPwd;
        wrongPwd.host = QLatin1String("localhost");
        wrongPwd.port = 22;
        wrongPwd.timeout = 30;
        wrongPwd.authType = SshConnectionParameters::AuthByPwd;
        wrongPwd.uname = QLatin1String("root");
        noUser.uname = QLatin1String("thiscantpossiblybeapasswordcanit");

        SshConnectionParameters invalidKeyFile;
        invalidKeyFile.host = QLatin1String("localhost");
        invalidKeyFile.port = 22;
        invalidKeyFile.timeout = 30;
        invalidKeyFile.authType = SshConnectionParameters::AuthByKey;
        invalidKeyFile.uname = QLatin1String("root");
        invalidKeyFile.privateKeyFile
            = QLatin1String("somefilenamethatwedontexpecttocontainavalidkey");

        // TODO: Create a valid key file and check for authentication error.

        m_testSet << TestItem("Behavior with non-existing host",
            noHost, ErrorList() << SshSocketError);
        m_testSet << TestItem("Behavior with non-existing user", noUser,
            ErrorList() << SshSocketError << SshTimeoutError
                << SshAuthenticationError);
        m_testSet << TestItem("Behavior with wrong password", wrongPwd,
            ErrorList() << SshSocketError << SshTimeoutError
                << SshAuthenticationError);
        m_testSet << TestItem("Behavior with invalid key file", invalidKeyFile,
            ErrorList() << SshSocketError << SshTimeoutError
                << SshKeyFileError);

        runNextTest();
    }

    ~Test();

private slots:
    void handleConnected()
    {
        qDebug("Error: Received unexpected connected() signal.");
        qApp->quit();
    }

    void handleDisconnected()
    {
        qDebug("Error: Received unexpected disconnected() signal.");
        qApp->quit();
    }

    void handleDataAvailable(const QString &msg)
    {
        qDebug("Error: Received unexpected dataAvailable() signal. "
            "Message was: '%s'.", qPrintable(msg));
        qApp->quit();
    }

    void handleError(SshError error)
    {
        if (m_testSet.isEmpty()) {
            qDebug("Error: Received error %d, but no test was running.", error);
            qApp->quit();
        }

        const TestItem testItem = m_testSet.takeFirst();
        if (testItem.allowedErrors.contains(error)) {
            qDebug("Received error %d, as expected.", error);
            if (m_testSet.isEmpty()) {
                qDebug("All tests finished successfully.");
                qApp->quit();
            } else {
                runNextTest();
            }
        } else {
            qDebug("Received unexpected error %d.", error);
            qApp->quit();
        }
    }

    void handleTimeout()
    {
        if (m_testSet.isEmpty()) {
            qDebug("Error: timeout, but no test was running.");
            qApp->quit();
        }
        const TestItem testItem = m_testSet.takeFirst();
        qDebug("Error: The following test timed out: %s", testItem.description);
    }

private:
    void runNextTest()
    {
        if (m_connection)
            disconnect(m_connection.data(), 0, this, 0);
        m_connection = SshConnection::create();
        connect(m_connection.data(), SIGNAL(connected()), this,
            SLOT(handleConnected()));
        connect(m_connection.data(), SIGNAL(disconnected()), this,
            SLOT(handleDisconnected()));
        connect(m_connection.data(), SIGNAL(dataAvailable(QString)), this,
            SLOT(handleDataAvailable(QString)));
        connect(m_connection.data(), SIGNAL(error(SshError)), this,
            SLOT(handleError(SshError)));
        const TestItem &nextItem = m_testSet.first();
        m_timeoutTimer.stop();
        m_timeoutTimer.setInterval(qMax(10000, nextItem.params.timeout * 1000));
        qDebug("Testing: %s", nextItem.description);
        m_connection->connectToHost(m_testSet.first().params);
    }

    SshConnection::Ptr m_connection;
    typedef QList<SshError> ErrorList;
    struct TestItem {
        TestItem(const char *d, const SshConnectionParameters &p,
            const ErrorList &e) : description(d), params(p), allowedErrors(e) {}

        const char *description;
        SshConnectionParameters params;
        ErrorList allowedErrors;
    };
    QList<TestItem> m_testSet;
    QTimer m_timeoutTimer;
};

Test::~Test() {}

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    Test t;

    return a.exec();
}


#include "main.moc"
