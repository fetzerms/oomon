#ifndef __VARS_H__
#define __VARS_H__
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

// Std C++ Headers
#include <fstream>
#include <string>
#include <vector>

// OOMon Headers
#include "setting.h"


enum VarNum
{
  VAR_AUTO_KLINE_HOST,
  VAR_AUTO_KLINE_HOST_TIME,
  VAR_AUTO_KLINE_NOIDENT,
  VAR_AUTO_KLINE_NOIDENT_TIME,
  VAR_AUTO_KLINE_USERHOST,
  VAR_AUTO_KLINE_USERHOST_TIME,
  VAR_AUTO_PILOT,
  VAR_AUTO_SAVE,
  VAR_BROKEN_HOSTNAME_MUNGING,
  VAR_CHECK_FOR_SPOOFS,
  VAR_DEFAULT_DLINE_TIMEOUT,
  VAR_DEFAULT_KLINE_TIMEOUT,
  VAR_DNSBL_PROXY_ACTION,
  VAR_DNSBL_PROXY_REASON,
  VAR_DNSBL_PROXY_ZONE,
  VAR_EXTRA_KLINE_INFO,
  VAR_FAKE_IP_SPOOF_ACTION,
  VAR_FAKE_IP_SPOOF_REASON,
  VAR_FLOODER_ACTION,
  VAR_FLOODER_MAX_COUNT,
  VAR_FLOODER_MAX_TIME,
  VAR_FLOODER_REASON,
  VAR_GLIST_FORMAT,
  VAR_IGNORE_UNKNOWN_COMMAND,
  VAR_ILLEGAL_CHAR_SPOOF_ACTION,
  VAR_ILLEGAL_CHAR_SPOOF_REASON,
  VAR_ILLEGAL_TLD_SPOOF_ACTION,
  VAR_ILLEGAL_TLD_SPOOF_REASON,
  VAR_INFO_FLOOD_ACTION,
  VAR_INFO_FLOOD_MAX_COUNT,
  VAR_INFO_FLOOD_MAX_TIME,
  VAR_INFO_FLOOD_REASON,
  VAR_INVALID_USERNAME_ACTION,
  VAR_INVALID_USERNAME_REASON,
  VAR_JUPE_JOIN_ACTION,
  VAR_JUPE_JOIN_IGNORE_CHANNEL,
  VAR_JUPE_JOIN_MAX_COUNT,
  VAR_JUPE_JOIN_MAX_TIME,
  VAR_JUPE_JOIN_REASON,
  VAR_KILLLIST_REASON,
  VAR_KILLNFIND_REASON,
  VAR_LINKS_FLOOD_ACTION,
  VAR_LINKS_FLOOD_MAX_COUNT,
  VAR_LINKS_FLOOD_MAX_TIME,
  VAR_LINKS_FLOOD_REASON,
  VAR_LIST_FORMAT,
  VAR_MOTD_FLOOD_ACTION,
  VAR_MOTD_FLOOD_MAX_COUNT,
  VAR_MOTD_FLOOD_MAX_TIME,
  VAR_MOTD_FLOOD_REASON,
  VAR_MULTI_MIN,
  VAR_NFIND_FORMAT,
  VAR_NICK_CHANGE_MAX_COUNT,
  VAR_NICK_CHANGE_T1_TIME,
  VAR_NICK_CHANGE_T2_TIME,
  VAR_NICK_FLOOD_ACTION,
  VAR_NICK_FLOOD_REASON,
  VAR_OPER_IN_MULTI,
  VAR_OPER_NICK_IN_REASON,
  VAR_OPER_ONLY_DCC,
  VAR_RELAY_MSGS_TO_LOCOPS,
  VAR_SCAN_FOR_PROXIES,
  VAR_SCAN_PROXY_ACTION,
  VAR_SCAN_PROXY_REASON,
  VAR_SEEDRAND_ACTION,
  VAR_SEEDRAND_COMMAND_MIN,
  VAR_SEEDRAND_FORMAT,
  VAR_SEEDRAND_REASON,
  VAR_SEEDRAND_REPORT_MIN,
  VAR_SERVER_TIMEOUT,
  VAR_SERVICES_CHECK_INTERVAL,
  VAR_SERVICES_CLONE_LIMIT,
  VAR_SPAMBOT_ACTION,
  VAR_SPAMBOT_MAX_COUNT,
  VAR_SPAMBOT_MAX_TIME,
  VAR_SPAMBOT_REASON,
  VAR_SPAMTRAP_ACTION,
  VAR_SPAMTRAP_DEFAULT_REASON,
  VAR_SPAMTRAP_ENABLE,
  VAR_SPAMTRAP_MIN_SCORE,
  VAR_SPAMTRAP_NICK,
  VAR_SPAMTRAP_USERHOST,
  VAR_STATS_FLOOD_ACTION,
  VAR_STATS_FLOOD_MAX_COUNT,
  VAR_STATS_FLOOD_MAX_TIME,
  VAR_STATS_FLOOD_REASON,
  VAR_STATSP_CASE_INSENSITIVE,
  VAR_STATSP_MESSAGE,
  VAR_STATSP_REPLY,
  VAR_STATSP_SHOW_IDLE,
  VAR_STATSP_SHOW_USERHOST,
  VAR_TOOMANY_ACTION,
  VAR_TOOMANY_IGNORE_USERNAME,
  VAR_TOOMANY_MAX_COUNT,
  VAR_TOOMANY_MAX_TIME,
  VAR_TOOMANY_REASON,
  VAR_TRACE_FLOOD_ACTION,
  VAR_TRACE_FLOOD_MAX_COUNT,
  VAR_TRACE_FLOOD_MAX_TIME,
  VAR_TRACE_FLOOD_REASON,
  VAR_TRACK_TEMP_DLINES,
  VAR_TRACK_TEMP_KLINES,
  VAR_TRAP_CONNECTS,
  VAR_TRAP_NICK_CHANGES,
  VAR_UMODE,
  VAR_UNAUTHED_MAY_CHAT,
  VAR_USER_COUNT_DELTA_MAX,
  VAR_WATCH_FLOODER_NOTICES,
  VAR_WATCH_INFO_NOTICES,
  VAR_WATCH_JUPE_NOTICES,
  VAR_WATCH_LINKS_NOTICES,
  VAR_WATCH_MOTD_NOTICES,
  VAR_WATCH_SPAMBOT_NOTICES,
  VAR_WATCH_STATS_NOTICES,
  VAR_WATCH_TOOMANY_NOTICES,
  VAR_WATCH_TRACE_NOTICES,
  VAR_XO_SERVICES_ENABLE,
  VAR_XO_SERVICES_REQUEST,
  VAR_XO_SERVICES_RESPONSE,
  VAR_COUNT
};


class Vars
{
public:
  typedef std::vector<Setting *> VarVector;
  typedef VarVector::size_type size_type;
  typedef VarVector::const_reference const_reference;

  Vars();
  virtual ~Vars();

  int findVar(const std::string & name) const;
  std::string set(const std::string & name, const std::string & value,
    const std::string & handle = "");
  int get(StrList & output, const std::string & name) const;

  const_reference operator[](size_type __n) const
  {
    return *(vec.begin() + __n);
  };

  void save(std::ofstream & file) const;

private:
  VarVector vec;
};


extern Vars vars;


#endif /* __VARS_H__ */

