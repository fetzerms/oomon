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
ProxyList::connect(ProxyPtr newProxy, const std::string & address,
  const BotSock::Port port)
{
#ifdef PROXYLIST_DEBUG
  std::cout << "Creating proxy connection to address " << address << ":" <<
    port << std::endl;
#endif

  newProxy->bindTo(Config::getProxyVhost());
  newProxy->setTimeout(PROXY_IDLE_MAX);

  if (!newProxy->connect(address, port))
  {
    std::cerr << "Connect to proxy failed (other)" << std::endl;
    return false;
  }

  this->scanners.push_back(newProxy);

#ifdef PROXYLIST_DEBUG
  std::cout << this->scanners.size() << " proxies queued up for processing." <<
    std::endl;
#endif

  return true;
}


// check(address, hostname, nick, userhost)
//
// Checks a client for proxies
//
// Returns true if no errors occurred
//
void
ProxyList::check(const std::string & address, const std::string & hostname,
  const std::string & nick, const std::string & userhost)
{
  typedef struct { Proxy::Protocol type; BotSock::Port port; } ScanDef;
  ScanDef defs[] =
  {
    { Proxy::WINGATE, 23 }, { Proxy::SOCKS4, 1080 },
    { Proxy::SOCKS5, 1080 }, { Proxy::HTTP, 80 },
    { Proxy::HTTP, 1080 }, { Proxy::HTTP, 3128 },
    { Proxy::HTTP, 8080 }
  };

  for (unsigned long i = 0; i < (sizeof(defs) / sizeof(ScanDef)); ++i)
  {
    try
    {
      if (!this->skipCheck(address, defs[i].port, defs[i].type))
      {
	ProxyPtr newProxy;

	switch (defs[i].type)
	{
	  case Proxy::HTTP:
	    newProxy = ProxyPtr(new Http(hostname, nick, userhost));
	    break;
	  case Proxy::WINGATE:
	    newProxy = ProxyPtr(new WinGate(hostname, nick, userhost));
	    break;
	  case Proxy::SOCKS4:
	    newProxy = ProxyPtr(new Socks4(hostname, nick, userhost));
	    break;
	  case Proxy::SOCKS5:
	    newProxy = ProxyPtr(new Socks5(hostname, nick, userhost));
	    break;
	  default:
	    std::cerr << "Unknown proxy type?" << std::endl;
	    break;
	}

        if (0 != newProxy)
	{
          this->connect(newProxy, address, defs[i].port);
	}
      }
    }
    catch (OOMon::errno_error & e)
    {
      std::cerr << "Connect to proxy failed (" << e.why() << ")" << std::endl;
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
    remove = !proxy->process(this->_readset, this->_writeset);
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

  ProxyList::ProxyProcessor p(readset, writeset);

  this->scanners.remove_if(p);
}


bool
ProxyList::ProxyIsChecking::operator()(ProxyPtr proxy)
{
  return ((proxy->address() == this->_address) &&
    (proxy->port() == this->_port) && (proxy->type() == this->_type));
}


bool
ProxyList::isChecking(const std::string & address, const BotSock::Port port,
  const Proxy::Protocol type) const
{
  return (this->scanners.end() != std::find_if(this->scanners.begin(),
    this->scanners.end(), ProxyList::ProxyIsChecking(address, port, type)));
}


void
ProxyList::addToCache(const std::string & address, const BotSock::Port port,
  const Proxy::Protocol type)
{
  ProxyList::CacheEntry newEntry(BotSock::inet_addr(address), port, type);
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
ProxyList::isVerifiedClean(const std::string & address,
  const BotSock::Port port, const Proxy::Protocol type)
{
  const ProxyList::CacheEntry tmp(BotSock::inet_addr(address), port, type);

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
  std::string scanners("Proxy scanners: ");
  scanners += IntToStr(this->scanners.size());
  client->send(scanners);

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
  cache += IntToStr(cacheCount);
  cache += '/';
  cache += IntToStr(this->safeHosts.size());
  cache += " (";
  cache += ULongToStr(this->cacheHits);
  cache += " hits, ";
  cache += ULongToStr(this->cacheMisses);
  cache += " misses)";
  client->send(cache);
}

