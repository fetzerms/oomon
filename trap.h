#ifndef __TRAP_H__
#define __TRAP_H__
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
#include <fstream>
#include <string>
#include <map>
#include <ctime>

// Boost C++ Headers
#include <boost/shared_ptr.hpp>

// OOMon Headers
#include "strtype"
#include "botsock.h"
#include "filter.h"
#include "userentry.h"


enum TrapAction
{
  TRAP_ECHO, TRAP_KILL, TRAP_KLINE, TRAP_KLINE_HOST, TRAP_KLINE_DOMAIN,
  TRAP_KLINE_IP, TRAP_KLINE_USERNET, TRAP_KLINE_NET, TRAP_DLINE_IP,
  TRAP_DLINE_NET
};


typedef unsigned int TrapKey;


class Trap
{
public:
  Trap(const TrapAction action, const long timeout, const std::string & line);
  Trap(const Trap & copy);
  virtual ~Trap(void);

  void update(const TrapAction action, const long timeout,
    const std::string & reason);

  bool matches(const UserEntryPtr user, const std::string & version,
    const std::string & privmsg, const std::string & notice) const;
  bool operator==(const Trap & other) const;
  bool operator==(const std::string & pattern) const;
  void doAction(const TrapKey key, const UserEntryPtr user) const;

  void updateStats(void);

  TrapAction getAction(void) const { return this->action_; }
  long getTimeout(void) const { return this->timeout_; }
  std::string getFilter(void) const { return this->filter_.get(); }
  std::string getReason(void) const { return this->reason_; }
  std::string getString(bool showCount = false, bool showTime = false) const;
  std::time_t getLastMatch(void) const { return this->lastMatch_; };
  unsigned long getMatchCount(void) const { return this->matchCount_; };

  bool loaded(void) const { return this->loaded_; }
  void loaded(const bool value) { this->loaded_ = value; }

private:
  typedef boost::shared_ptr<Pattern> PatternPtr;
  TrapAction	action_;
  long		timeout_;	// For K-Lines only
  Filter        filter_;
  std::string	reason_;	// For Kills, K-Lines, and D-Lines only
  std::time_t	lastMatch_;
  unsigned long	matchCount_;
  bool          loaded_;
};


class TrapList
{
public:
  static void cmd(class BotClient * client, std::string line);
  static bool remove(const TrapKey key);
  static bool remove(const std::string & pattern);
  static void clear(void) { TrapList::traps.clear(); };

  static void match(const UserEntryPtr user, const std::string & version,
    const std::string & privmsg, const std::string & notice);
  static void match(const UserEntryPtr user);
  static void matchCtcpVersion(const UserEntryPtr user,
    const std::string & version);
  static void matchPrivmsg(const UserEntryPtr user,
    const std::string & privmsg);
  static void matchNotice(const UserEntryPtr user, const std::string & notice);

  static void list(class BotClient * client, bool showCounts, bool showTimes);

  static void save(std::ofstream & file);

  static void preLoad(void);
  static void postLoad(void);

  static TrapAction actionType(const std::string & text);
  static std::string actionString(const TrapAction & action);

private:
  typedef std::multimap<TrapKey, Trap> TrapMap;
  static TrapMap traps;

  static Trap add(const TrapKey key, const TrapAction action,
    const long timeout, const std::string & line);
  static TrapKey getMaxKey(void);
};

#endif /* __TRAP_H__ */

