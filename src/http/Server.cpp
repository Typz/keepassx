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

#include "Server.h"
#include "qhttpserver/qhttpserver.h"
#include "qhttpserver/qhttprequest.h"
#include "qhttpserver/qhttpresponse.h"
#include "Protocol.h"
#include "crypto/Crypto.h"
#include <QtCore/QHash>
#include <QtCore/QCryptographicHash>
#include <QtGui/QMessageBox>

using namespace KeepassHttpProtocol;

////////////////////////////////////////////////////////////////////////////////////////////////////
/// Request
////////////////////////////////////////////////////////////////////////////////////////////////////

RequestHandler::RequestHandler(QHttpRequest *request, QHttpResponse *response): m_request(request), m_response(response)
{
    m_request->storeBody();
    connect(m_request, SIGNAL(end()), this, SLOT(onEnd()));
    connect(m_response, SIGNAL(done()), this, SLOT(deleteLater()));
}

RequestHandler::~RequestHandler()
{
    delete m_request;
}

void RequestHandler::onEnd()
{
    Q_EMIT requestComplete(m_request, m_response);
}


////////////////////////////////////////////////////////////////////////////////////////////////////
/// Request
////////////////////////////////////////////////////////////////////////////////////////////////////

Server::Server(QObject *parent) :
    QObject(parent),
    m_httpServer(new QHttpServer(this)),
    m_started(false),
    m_port(19455)
{
    connect(m_httpServer, SIGNAL(newRequest(QHttpRequest*,QHttpResponse*)), this, SLOT(handleRequest(QHttpRequest*,QHttpResponse*)));
}

void Server::start(int port)
{
    if (m_port != port)
        stop(); //restart to listen on the new port

    if (m_started)
        return;

    m_port = port;
    m_started = m_httpServer->listen(QHostAddress::LocalHost, port)
             || m_httpServer->listen(QHostAddress::LocalHostIPv6, port);
}

void Server::stop()
{
    if (!m_started)
        return;
    m_httpServer->close();
    m_started = false;
}

void Server::handleRequest(QHttpRequest *request, QHttpResponse *response)
{
    RequestHandler * h = new RequestHandler(request, response);
    connect(h, SIGNAL(requestComplete(QHttpRequest*,QHttpResponse*)), this, SLOT(handleRequestComplete(QHttpRequest*,QHttpResponse*)));
}

void Server::handleRequestComplete(QHttpRequest *request, QHttpResponse *response)
{
    Request r;
    if (request->header("content-type").compare("application/json", Qt::CaseInsensitive) != 0 ||
        !r.fromJson(request->body())) {
        response->writeHead(QHttpResponse::STATUS_BAD_REQUEST);
        response->write(QString("Failed to parse content"));
        response->end();
    }
    else if (!isDatabaseOpened() && !openDatabase(r.triggerUnlock())) {
        response->writeHead(QHttpResponse::STATUS_SERVICE_UNAVAILABLE);
        response->end();
    } else {
        QByteArray hash = QCryptographicHash::hash((getDatabaseRootUuid() + getDatabaseRecycleBinUuid()).toUtf8(),
                                                   QCryptographicHash::Sha1).toHex();

        Response protocolResp(r, QString::fromAscii(hash));
        switch(r.requestType()) {
        case INVALID:           break;
        case TEST_ASSOCIATE:    testAssociate(r, &protocolResp); break;
        case ASSOCIATE:         associate(r, &protocolResp); break;
        case GET_LOGINS:        getLogins(r, &protocolResp); break;
        case GET_LOGINS_COUNT:  getLoginsCount(r, &protocolResp); break;
        case GET_ALL_LOGINS:    getAllLogins(r, &protocolResp); break;
        case SET_LOGIN:         setLogin(r, &protocolResp); break;
        case GENERATE_PASSWORD: generatePassword(r, &protocolResp); break;
        }

        QByteArray s = protocolResp.toJson().toUtf8();
        response->setHeader("Content-Type", "application/json");
        response->setHeader("Content-Length", QString::number(s.size()));
        response->writeHead(QHttpResponse::STATUS_OK);
        response->write(s);
        response->end();
    }
}

void Server::testAssociate(const Request& r, Response * protocolResp)
{
    if (r.id().isEmpty())
        return;   //ping

    QString key = getKey(r.id());
    if (key.isEmpty() || !r.CheckVerifier(key))
        return;

    protocolResp->setSuccess();
    protocolResp->setId(r.id());
    protocolResp->setVerifier(key);
}

void Server::associate(const Request& r, Response * protocolResp)
{
    if (!r.CheckVerifier(r.key()))
        return;

    QString id = storeKey(r.key());
    if (id.isEmpty())
        return;

    protocolResp->setSuccess();
    protocolResp->setId(id);
    protocolResp->setVerifier(r.key());
}

void Server::getLogins(const Request &r, Response *protocolResp)
{
    QString key = getKey(r.id());
    if (!r.CheckVerifier(key))
        return;

    protocolResp->setSuccess();
    protocolResp->setId(r.id());
    protocolResp->setVerifier(key);
    QList<Entry> entries = findMatchingEntries(r.id(), r.url(), r.submitUrl(), r.realm(), r.sortSelection());
    protocolResp->setEntries(entries);
}

void Server::getLoginsCount(const Request &r, Response *protocolResp)
{
    QString key = getKey(r.id());
    if (!r.CheckVerifier(key))
        return;

    protocolResp->setSuccess();
    protocolResp->setId(r.id());
    protocolResp->setVerifier(key);
    protocolResp->setCount(countMatchingEntries(r.id(), r.url(), r.submitUrl(), r.realm()));
}

void Server::getAllLogins(const Request &r, Response *protocolResp)
{
    QString key = getKey(r.id());
    if (!r.CheckVerifier(key))
        return;

    protocolResp->setSuccess();
    protocolResp->setId(r.id());
    protocolResp->setVerifier(key);
    protocolResp->setEntries(searchAllEntries(r.id()));   //TODO: ensure there is no password --> change API?
}

void Server::setLogin(const Request &r, Response *protocolResp)
{
    QString key = getKey(r.id());
    if (!r.CheckVerifier(key))
        return;

    QString uuid = r.uuid();
    if (uuid.isEmpty())
        addEntry(r.id(), r.login(), r.password(), r.url(), r.submitUrl(), r.realm());
    else
        updateEntry(r.id(), r.uuid(), r.login(), r.password(), r.url());

    protocolResp->setSuccess();
    protocolResp->setId(r.id());
    protocolResp->setVerifier(key);
}

void Server::generatePassword(const Request &r, Response *protocolResp)
{
    QString key = getKey(r.id());
    if (!r.CheckVerifier(key))
        return;

    int qualityBits;
    QString password = generatePassword(&qualityBits);

    protocolResp->setSuccess();
    protocolResp->setId(r.id());
    protocolResp->setVerifier(key);
    protocolResp->setEntries(QList<Entry>() << Entry(r.requestTypeStr(), QString::number(qualityBits), password, r.requestTypeStr()));

    memset(password.data(), 0, password.length());
}
