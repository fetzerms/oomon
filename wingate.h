#ifndef __WINGATE_H__
#define __WINGATE_H__
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

// ===========================================================================
// File Description:
//
//  A simple open WinGate proxy detector.
// ===========================================================================

#include <string>

#include "oomon.h"
#include "proxy.h"
#include "proxylist.h"


class WinGate : public Proxy
{
public:
  WinGate(const std::string & hostname, const std::string & nick,
    const std::string & userhost);
  virtual ~WinGate(void)
  {
    if (!this->detectedProxy_)
    {
      proxies.addToCache(this->address(), this->port(), Proxy::WINGATE);
    }
  }

  bool onConnect(void);

protected:
  bool onRead(std::string);
  virtual std::string typeName(void) const { return "WinGate"; };
  virtual Proxy::Protocol type(void) const { return Proxy::WINGATE; };
};

#endif /* __WINGATE_H__ */

