#ifndef __WINGATE_H__
#define __WINGATE_H__
// ===========================================================================
// OOMon - Objected Oriented Monitor Bot
// Copyright (C) 2003  Timothy L. Jensen
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

// ===========================================================================
// File Description:
//
//  A simple open WinGate proxy detector.
// ===========================================================================

#include <string>

#include "oomon.h"
#include "proxy.h"


class WinGate : public Proxy
{
public:
  WinGate(const std::string & hostname, const std::string & nick,
    const std::string & userhost) : Proxy(hostname, nick, userhost) { }
  virtual ~WinGate(void)
  {
    if (!this->_detectedProxy)
    {
      Proxy::addToCache(this->address(), this->port(), Proxy::WINGATE);
    }
  }

  virtual bool onConnect();

protected:
  virtual bool onRead(std::string);
  virtual std::string typeName(void) const { return "WinGate"; };
  virtual Proxy::ProxyType type(void) const { Proxy::WINGATE; };
};

#endif /* __WINGATE_H__ */

