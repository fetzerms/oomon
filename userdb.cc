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

// Std C++ Headers
#include <string>
#include <cerrno>

// Boost C++ Headers
#include <boost/shared_ptr.hpp>
#include <boost/lexical_cast.hpp>

// OOMon Headers
#include "userdb.h"
#include "util.h"
#include "botexcept.h"


#ifdef DEBUG
# define USERDB_DEBUG
#endif


boost::shared_ptr<UserDB> userConfig;


bool
UserDB::getEcho(const std::string & handle)
{
  std::string value;

  int result = this->get(handle + ":echo", value);

#ifdef USERDB_DEBUG
  std::cout << "db->get() returned " << result << std::endl;
#endif

  if (1 == result)
  {
    throw OOMon::norecord_error("No ECHO record for " + handle);
  }
  else if (-1 == result)
  {
#ifdef USERDB_DEBUG
    std::cout << "errno " << errno << std::endl;
#endif
    throw OOMon::botdb_error("Failed to get ECHO record for " + handle);
  }

  try
  {
    return (0 != boost::lexical_cast<int>(value));
  }
  catch (boost::bad_lexical_cast)
  {
    throw OOMon::botdb_error("Bad ECHO record for " + handle);
  }
}


void
UserDB::setEcho(const std::string & handle, const bool value)
{
  std::string text = boost::lexical_cast<std::string>(value ? 1 : 0);

  int result = this->put(handle + ":echo", text);

#ifdef USERDB_DEBUG
  std::cout << "db->put() returned " << result << std::endl;
#endif

  if (1 == result)
  {
    throw OOMon::norecord_error("No ECHO record for " + handle);
  }
  else if (-1 == result)
  {
#ifdef USERDB_DEBUG
    std::cout << "errno " << errno << std::endl;
#endif
    throw OOMon::botdb_error("Failed to set ECHO record for " + handle);
  }
}


std::string
UserDB::getWatches(const std::string & handle)
{
  std::string value;

  int result = this->get(handle + ":watches", value);

#ifdef USERDB_DEBUG
  std::cout << "db->get() returned " << result << std::endl;
#endif

  if (1 == result)
  {
    throw OOMon::norecord_error("No WATCHES record for " + handle);
  }
  else if (-1 == result)
  {
#ifdef USERDB_DEBUG
    std::cout << "errno " << errno << std::endl;
#endif
    throw OOMon::botdb_error("Failed to get WATCHES record for " + handle);
  }

  return value;
}


void
UserDB::setWatches(const std::string & handle, const std::string & value)
{
  int result = this->put(handle + ":watches", value);

#ifdef USERDB_DEBUG
  std::cout << "db->put() returned " << result << std::endl;
#endif

  if (1 == result)
  {
    throw OOMon::norecord_error("No WATCHES record for " + handle);
  }
  else if (-1 == result)
  {
#ifdef USERDB_DEBUG
    std::cout << "errno " << errno << std::endl;
#endif
    throw OOMon::botdb_error("Failed to set WATCHES record for " + handle);
  }
}

