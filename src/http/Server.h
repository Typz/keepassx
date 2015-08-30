/*
 *  Copyright (C) 2013 Francois Ferrand <thetypz@gmail.com>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 2 or (at your option)
 *  version 3 of the License.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef SERVER_H
#define SERVER_H

#include <QtCore/QObject>
#include <QtCore/QList>

class QHttpServer;
class QHttpRequest;
class QHttpResponse;

namespace KeepassHttpProtocol {

class Request;
class Response;
class Entry;

class RequestHandler: public QObject {
    Q_OBJECT

public:
    RequestHandler(QHttpRequest *request, QHttpResponse *response);
    ~RequestHandler();

private Q_SLOTS:
    void onEnd();

Q_SIGNALS:
    void requestComplete(QHttpRequest *request, QHttpResponse *response);

private:
    QHttpRequest * m_request;
    QHttpResponse *m_response;
};

class Server : public QObject
{
    Q_OBJECT
public:
    explicit Server(QObject *parent = 0);

    //TODO: use QByteArray?
    virtual bool isDatabaseOpened() const = 0;
    virtual bool openDatabase(bool userRequest) = 0;
    virtual QString getDatabaseRootUuid() = 0;
    virtual QString getDatabaseRecycleBinUuid() = 0;
    virtual QString getKey(const QString &id) = 0;
    virtual QString storeKey(const QString &key) = 0;
    virtual QList<Entry> findMatchingEntries(const QString &id, const QString &url, const QString & submitUrl, const QString & realm, bool sortSelection) = 0;
    virtual int countMatchingEntries(const QString &id, const QString &url, const QString & submitUrl, const QString & realm) = 0;
    virtual QList<Entry> searchAllEntries(const QString &id) = 0;
    virtual void addEntry(const QString &id, const QString &login, const QString &password, const QString &url, const QString &submitUrl, const QString &realm) = 0;
    virtual void updateEntry(const QString &id, const QString &uuid, const QString &login, const QString &password, const QString &url) = 0;
    virtual QString generatePassword(int* qualityBits) = 0;

public Q_SLOTS:
    void start(int port);
    void stop();

private Q_SLOTS:
    void handleRequest(QHttpRequest * request, QHttpResponse* response);
    void handleRequestComplete(QHttpRequest * request, QHttpResponse* response);

private:
    void testAssociate(const KeepassHttpProtocol::Request &r, KeepassHttpProtocol::Response *protocolResp);
    void associate(const KeepassHttpProtocol::Request &r, KeepassHttpProtocol::Response *protocolResp);
    void getLogins(const KeepassHttpProtocol::Request &r, KeepassHttpProtocol::Response *protocolResp);
    void getLoginsCount(const KeepassHttpProtocol::Request &r, KeepassHttpProtocol::Response *protocolResp);
    void getAllLogins(const KeepassHttpProtocol::Request &r, KeepassHttpProtocol::Response *protocolResp);
    void setLogin(const KeepassHttpProtocol::Request &r, KeepassHttpProtocol::Response *protocolResp);
    void generatePassword(const KeepassHttpProtocol::Request &r, KeepassHttpProtocol::Response *protocolResp);

    QHttpServer * const m_httpServer;
    bool m_started;
    int m_port;
};

}   /*namespace KeepassHttpProtocol*/

#endif // SERVER_H
