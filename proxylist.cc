// ===========================================================================
// OOMon - Objected Oriented Monitor Bot
// Copyright (C) 2004  Timothy L. Jensen
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
// ===========================================================================

// $Id$

// Std C++ headers
#include <iostream>
#include <fstream>
#include <string>
#include <list>
#include <algorithm>
#include <ctime>

// Boost C++ Headers
#include <boost/bind.hpp>
#include <boost/lexical_cast.hpp>

// Std C headers
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <netdb.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <pwd.h>

// OOMon headers
#include "oomon.h"
#include "proxylist.h"
#include "botsock.h"
#include "socks4.h"
#include "socks5.h"
#include "wingate.h"
#include "botexcept.h"
#include "botclient.h"
#include "http.h"
#include "httppost.h"
#include "config.h"
#include "log.h"
#include "util.h"
#include "main.h"
#include "engine.h"
#include "irc.h"
#include "vars.h"


#ifdef DEBUG
# define PROXYLIST_DEBUG
#endif

ProxyList proxies;

const std::time_t ProxyList::CACHE_EXPIRE = (60 * 60); /* 1 hour */
const ProxyList::SockList::size_type ProxyList::CACHE_SIZE = 5000;


ProxyList::ProxyList(void) : safeHosts(ProxyList::CACHE_SIZE), cacheHits(0),
  cacheMisses(0)
{
}


// connect(address, port)
//
// Attempts to connect to a proxy
//
// Returns true if no errors occurred
//
bool
ProxyList::connect(ProxyPtr newProxy, const BotSock::Address & address,
    const BotSock::Port port)
{
#ifdef PROXYLIST_DEBUG
  std::cout << "Creating proxy connection to address " << address << ":" <<
    port << std::endl;
#endif

  newProxy->bindTo(config.proxyVhost());
  newProxy->setTimeout(PROXY_IDLE_MAX);

  if (!newProxy->connect(address, port))
  {
    std::cerr << "Connect to proxy failed (other)" << std::endl;
    return false;
  }

  this->scanners.push_back(newProxy);

  return true;
}


bool
ProxyList::initiateCheck(const UserEntryPtr user, const BotSock::Port port,
    Proxy::Protocol protocol)
{
  bool result = false;

  try
  {
    ProxyPtr newProxy;

    switch (protocol)
    {
      case Proxy::HTTP_CONNECT:
        {
          ProxyPtr tmp(new Http(user));
          newProxy.swap(tmp);
        }
        break;
      case Proxy::HTTP_POST:
        {
          ProxyPtr tmp(new HttpPost(user));
          newProxy.swap(tmp);
        }
        break;
      case Proxy::WINGATE:
        {
          ProxyPtr tmp(new WinGate(user));
          newProxy.swap(tmp);
        }
        break;
      case Proxy::SOCKS4:
        {
          ProxyPtr tmp(new Socks4(user));
          newProxy.swap(tmp);
        }
        break;
      case Proxy::SOCKS5:
        {
          ProxyPtr tmp(new Socks5(user));
          newProxy.swap(tmp);
        }
        break;
      default:
        std::cerr << "Unknown proxy type?" << std::endl;
        break;
    }

    if (newProxy)
    {
      result = this->connect(newProxy, user->getIP(), port);
    }
  }
  catch (OOMon::errno_error & e)
  {
    std::cerr << "Connect to proxy failed (" << e.why() << ")" << std::endl;
  }

  return result;
}


void
ProxyList::enqueueScan(const UserEntryPtr user, const std::string & portStr,
    const Proxy::Protocol protocol)
{
  try
  {
    BotSock::Port port = boost::lexical_cast<BotSock::Port>(portStr);

    if (!this->skipCheck(user->getIP(), port, protocol))
    {
      ProxyList::QueueNode enqueue(user, port, protocol);

      this->queuedScans.push_back(enqueue);
    }
  }
  catch (boost::bad_lexical_cast)
  {
    std::cerr << "Bad port number: " << portStr << std::endl;
  }
}


// check(address, hostname, nick, userhost)
//
// Checks a client for proxies
//
// Returns true if no errors occurred
//
void
ProxyList::check(const UserEntryPtr user)
{
  if (vars[VAR_SCAN_FOR_PROXIES]->getBool())
  {
    StrVector httpConnect, httpPost, socks4, socks5, wingate;

    StrSplit(httpConnect, vars[VAR_SCAN_HTTP_CONNECT_PORTS]->getString(), " ,",
        true);
    StrSplit(httpPost, vars[VAR_SCAN_HTTP_POST_PORTS]->getString(), " ,", true);
    StrSplit(socks4, vars[VAR_SCAN_SOCKS4_PORTS]->getString(), " ,", true);
    StrSplit(socks4, vars[VAR_SCAN_SOCKS5_PORTS]->getString(), " ,", true);
    StrSplit(wingate, vars[VAR_SCAN_WINGATE_PORTS]->getString(), " ,", true);

    for (StrVector::iterator pos = httpConnect.begin();
        pos != httpConnect.end(); ++pos)
    {
      this->enqueueScan(user, *pos, Proxy::HTTP_CONNECT);
    }
    for (StrVector::iterator pos = httpPost.begin(); pos != httpPost.end();
        ++pos)
    {
      this->enqueueScan(user, *pos, Proxy::HTTP_POST);
    }
    for (StrVector::iterator pos = socks4.begin(); pos != socks4.end(); ++pos)
    {
      this->enqueueScan(user, *pos, Proxy::SOCKS4);
    }
    for (StrVector::iterator pos = socks5.begin(); pos != socks5.end(); ++pos)
    {
      this->enqueueScan(user, *pos, Proxy::SOCKS5);
    }
    for (StrVector::iterator pos = wingate.begin(); pos != wingate.end(); ++pos)
    {
      this->enqueueScan(user, *pos, Proxy::WINGATE);
    }
  }
}


// SetAllFD(readset, writeset)
//
// Do this if you want to be able to receive data :P
//
void
ProxyList::setAllFD(fd_set & readset, fd_set & writeset)
{
  std::for_each(this->scanners.begin(), this->scanners.end(),
    BotSock::FDSetter(readset, writeset));
}


bool
ProxyList::ProxyProcessor::operator()(ProxyPtr proxy)
{
  bool remove;

  try
  {
    remove = !proxy->process(this->readset_, this->writeset_);
  }
  catch (OOMon::timeout_error)
  {
#ifdef PROXYLIST_DEBUG
    std::cout << "Proxy connection to " << proxy->address() <<
      " timed out." << std::endl;
#endif
    remove = true;
  }

  return remove;
}


// ProcessAll(readset, writeset)
//
// Processes any received data at each proxy connection.
//
void
ProxyList::processAll(const fd_set & readset, const fd_set & writeset)
{
  // Expire any old proxy cache entries
  std::time_t now = std::time(NULL);
  ProxyList::CacheEntry empty;
  std::replace_if(this->safeHosts.begin(), this->safeHosts.end(),
    boost::bind(&ProxyList::CacheEntry::isExpired, _1, now), empty);

  // Process queue
  const ProxyList::SockList::size_type max = vars[VAR_SCAN_MAX_COUNT]->getInt();
  while (!this->queuedScans.empty() && (this->scanners.size() < max))
  {
    ProxyList::QueueNode front(this->queuedScans.front());

    if (this->initiateCheck(front.user, front.port, front.protocol))
    {
      this->queuedScans.pop_front();
    }
    else
    {
      break;
    }
  }

  // Process active scans
  ProxyList::ProxyProcessor p(readset, writeset);
  this->scanners.remove_if(p);
}


bool
ProxyList::ProxyIsChecking::operator()(ProxyPtr proxy)
{
  return ((proxy->address() == this->address_) &&
    (proxy->port() == this->port_) && (proxy->type() == this->type_));
}


bool
ProxyList::ProxyIsQueued::operator()(const QueueNode & node) const
{
  return ((node.user->getIP() == this->address_) &&
    (node.port == this->port_) && (node.protocol == this->protocol_));
}


bool
ProxyList::isChecking(const BotSock::Address & address,
    const BotSock::Port port, const Proxy::Protocol protocol) const
{
  return ((this->scanners.end() != std::find_if(this->scanners.begin(),
    this->scanners.end(), ProxyList::ProxyIsChecking(address, port,
      protocol))) || (this->queuedScans.end() !=
    std::find_if(this->queuedScans.begin(), this->queuedScans.end(),
      ProxyList::ProxyIsQueued(address, port, protocol))));
}


void
ProxyList::addToCache(const BotSock::Address & address,
    const BotSock::Port port, const Proxy::Protocol type)
{
  ProxyList::CacheEntry newEntry(address, port, type);
  ProxyList::CacheEntry empty;

  ProxyList::Cache::iterator findEmpty = std::find(this->safeHosts.begin(),
    this->safeHosts.end(), empty);
  if (findEmpty != this->safeHosts.end())
  {
    // Found an empty slot!  Now use it!
    *findEmpty = newEntry;
  }
  else if (!this->safeHosts.empty())
  {
    // No empty slot found!  Find the oldest entry and replace it.
    std::sort(this->safeHosts.begin(), this->safeHosts.end());

    this->safeHosts[0] = newEntry;
  }
}


bool
ProxyList::skipCheck(const BotSock::Address & address,
    const BotSock::Port & port, const Proxy::Protocol & type)
{
  return (this->isChecking(address, port, type) ||
      this->isVerifiedClean(address, port, type));
}


bool
ProxyList::isVerifiedClean(const BotSock::Address & address,
    const BotSock::Port port, const Proxy::Protocol type)
{
  const ProxyList::CacheEntry tmp(address, port, type);

  ProxyList::Cache::iterator pos = std::find(this->safeHosts.begin(),
    this->safeHosts.end(), tmp);

  bool result;

  if (this->safeHosts.end() == pos)
  {
    result = false;
    ++this->cacheMisses;
  }
  else
  {
    result = true;
    ++this->cacheHits;
    pos->update();

#ifdef PROXYLIST_DEBUG
    if (result)
    {
      std::cout << "Verified clean: " << address << ":" << port << " (" <<
        type << ")" << std::endl;
    }
#endif
  }

  return result;
}


void
ProxyList::status(BotClient * client) const
{
  ProxyList::SockList::size_type size(this->scanners.size());
  ProxyList::Queue::size_type queueSize(this->queuedScans.size());

  if ((size > 0) || vars[VAR_SCAN_FOR_PROXIES]->getBool())
  {
    std::string scanCount("Proxy scanners: ");
    scanCount += boost::lexical_cast<std::string>(size);
    scanCount += " (";
    scanCount += boost::lexical_cast<std::string>(queueSize);
    scanCount += " queued)";
    client->send(scanCount);

    int cacheCount = 0;
    for (ProxyList::Cache::const_iterator pos = this->safeHosts.begin();
      pos < this->safeHosts.end(); ++pos)
    {
      if (!pos->isEmpty())
      {
        ++cacheCount;
      }
    }
    std::string cache("Proxy cache: ");
    cache += boost::lexical_cast<std::string>(cacheCount);
    cache += '/';
    cache += boost::lexical_cast<std::string>(this->safeHosts.size());
    cache += " (";
    cache += boost::lexical_cast<std::string>(this->cacheHits);
    cache += " hits, ";
    cache += boost::lexical_cast<std::string>(this->cacheMisses);
    cache += " misses)";
    client->send(cache);
  }
}

