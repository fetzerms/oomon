#ifndef __DEFAULTS_H__
#define __DEFAULTS_H__
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


// NOTE:  Please do not modify this file.  Instead, use the ".set" command
//        to customize your OOMon settings.


#define DEFAULT_AUTO_KLINE_HOST		true
#define DEFAULT_AUTO_KLINE_HOST_TIME	1440
#define DEFAULT_AUTO_KLINE_NOIDENT	true
#define DEFAULT_AUTO_KLINE_NOIDENT_TIME	30
#define DEFAULT_AUTO_KLINE_USERHOST	true
#define DEFAULT_AUTO_KLINE_USERHOST_TIME	0
#define DEFAULT_AUTO_PILOT		true
#define DEFAULT_AUTO_SAVE		true
#define DEFAULT_BROKEN_HOSTNAME_MUNGING	false
#define DEFAULT_CHECK_FOR_SPOOFS	false
#define DEFAULT_DEFAULT_DLINE_TIMEOUT	0
#define DEFAULT_DEFAULT_KLINE_TIMEOUT	0
#define DEFAULT_DNSBL_PROXY_ACTION	ACTION_KLINE_HOST
#define DEFAULT_DNSBL_PROXY_REASON	"Open Proxy (DNSBL)"
#define DEFAULT_DNSBL_PROXY_ZONE	"opm.blitzed.org."
#define DEFAULT_EXTRA_KLINE_INFO	true
#define DEFAULT_FAKE_IP_SPOOF_ACTION	ACTION_KILL
#define DEFAULT_FAKE_IP_SPOOF_REASON	"Fake IP spoofer"
#define DEFAULT_FLOODER_ACTION		ACTION_KILL
#define DEFAULT_FLOODER_MAX_COUNT	2
#define DEFAULT_FLOODER_MAX_TIME	300
#define DEFAULT_FLOODER_REASON		"Flooding is prohibited"
#define DEFAULT_GLIST_FORMAT		"%_%_%n%_(%@)%-%i%-%g"
#define DEFAULT_IGNORE_UNKNOWN_COMMAND	true
#define DEFAULT_ILLEGAL_CHAR_SPOOF_ACTION	ACTION_KILL
#define DEFAULT_ILLEGAL_CHAR_SPOOF_REASON	"Illegal character spoofer"
#define DEFAULT_ILLEGAL_TLD_SPOOF_ACTION	ACTION_KILL
#define DEFAULT_ILLEGAL_TLD_SPOOF_REASON	"Illegal TLD spoofer"
#define DEFAULT_INFO_FLOOD_ACTION	ACTION_KILL
#define DEFAULT_INFO_FLOOD_MAX_COUNT	2
#define DEFAULT_INFO_FLOOD_MAX_TIME	600
#define DEFAULT_INFO_FLOOD_REASON	"INFO flooding"
#define DEFAULT_INVALID_USERNAME_ACTION	ACTION_KLINE_HOST
#define DEFAULT_INVALID_USERNAME_REASON	"Invalid username"
#define DEFAULT_JUPE_JOIN_ACTION	ACTION_KLINE
#define DEFAULT_JUPE_JOIN_ACTION_TIME	60
#define DEFAULT_JUPE_JOIN_IGNORE_CHANNEL	false
#define DEFAULT_JUPE_JOIN_MAX_COUNT	2
#define DEFAULT_JUPE_JOIN_MAX_TIME	300
#define DEFAULT_JUPE_JOIN_REASON	"Forbidden channel joiner"
#define DEFAULT_KILLLIST_REASON		"Too many connections, read MOTD"
#define DEFAULT_KILLNFIND_REASON	"Too many connections, read MOTD"
#define DEFAULT_LINKS_FLOOD_ACTION	ACTION_KILL
#define DEFAULT_LINKS_FLOOD_MAX_COUNT	2
#define DEFAULT_LINKS_FLOOD_MAX_TIME	600
#define DEFAULT_LINKS_FLOOD_REASON	"LINKS flooding"
#define DEFAULT_LIST_FORMAT		"%_%_%n%_(%@)%-%i"
#define DEFAULT_MOTD_FLOOD_ACTION	ACTION_KILL
#define DEFAULT_MOTD_FLOOD_MAX_COUNT	2
#define DEFAULT_MOTD_FLOOD_MAX_TIME	600
#define DEFAULT_MOTD_FLOOD_REASON	"MOTD flooding"
#define DEFAULT_MULTI_MIN		3
#define DEFAULT_NFIND_FORMAT		"%_%_%n%_(%@)%-%i"
#define DEFAULT_NICK_CHANGE_MAX_COUNT	5
#define DEFAULT_NICK_CHANGE_T1_TIME	10
#define DEFAULT_NICK_CHANGE_T2_TIME	300
#define DEFAULT_NICK_FLOOD_ACTION	ACTION_KILL
#define DEFAULT_NICK_FLOOD_REASON	"NICK flooding"
#define DEFAULT_OPER_IN_MULTI		false
#define DEFAULT_OPER_NICK_IN_REASON	false
#define DEFAULT_OPER_ONLY_DCC		true
#define DEFAULT_RELAY_MSGS_TO_LOCOPS	false
#define DEFAULT_SCAN_FOR_PROXIES	true
#define DEFAULT_SCAN_PROXY_ACTION	ACTION_KLINE_HOST
#define DEFAULT_SCAN_PROXY_REASON	"Open Proxy (Scanned)"
#define DEFAULT_SEEDRAND_ACTION		ACTION_NOTHING
#define DEFAULT_SEEDRAND_COMMAND_MIN	2500
#define DEFAULT_SEEDRAND_FORMAT		"%_%_%s%_--%_%n%_(%@)%-%i"
#define DEFAULT_SEEDRAND_REASON		"Possible drone (random nick)"
#define DEFAULT_SEEDRAND_REPORT_MIN	4500
#define DEFAULT_SERVER_TIMEOUT		300
#define DEFAULT_SERVICES_CHECK_INTERVAL	1
#define DEFAULT_SERVICES_CLONE_LIMIT	4
#define DEFAULT_SPAMBOT_ACTION		ACTION_KILL
#define DEFAULT_SPAMBOT_MAX_COUNT	2
#define DEFAULT_SPAMBOT_MAX_TIME	300
#define DEFAULT_SPAMBOT_REASON		"Spamming is prohibited"
#define DEFAULT_SPAMTRAP_ACTION		ACTION_KLINE_HOST
#define DEFAULT_SPAMTRAP_DEFAULT_REASON	"Caught by Spam Trap"
#define DEFAULT_SPAMTRAP_ENABLE		true
#define DEFAULT_SPAMTRAP_MIN_SCORE	10
#define DEFAULT_SPAMTRAP_NICK		"spamtrap"
#define DEFAULT_SPAMTRAP_USERHOST	"spamtrap@spam.trap"
#define DEFAULT_STATS_FLOOD_ACTION	ACTION_KILL
#define DEFAULT_STATS_FLOOD_MAX_COUNT	2
#define DEFAULT_STATS_FLOOD_MAX_TIME	600
#define DEFAULT_STATS_FLOOD_REASON	"STATS flooding"
#define DEFAULT_STATSP_CASE_INSENSITIVE	false
#define DEFAULT_STATSP_MESSAGE		""
#define DEFAULT_STATSP_REPLY		true
#define DEFAULT_STATSP_SHOW_IDLE	true
#define DEFAULT_STATSP_SHOW_USERHOST	true
#define DEFAULT_TOOMANY_ACTION		ACTION_DLINE_IP
#define DEFAULT_TOOMANY_ACTION_TIME	60
#define DEFAULT_TOOMANY_IGNORE_USERNAME	true
#define DEFAULT_TOOMANY_MAX_COUNT	2
#define DEFAULT_TOOMANY_MAX_TIME	300
#define DEFAULT_TOOMANY_REASON		"Clones are prohibited"
#define DEFAULT_TRACE_FLOOD_ACTION	ACTION_KILL
#define DEFAULT_TRACE_FLOOD_MAX_COUNT	2
#define DEFAULT_TRACE_FLOOD_MAX_TIME	600
#define DEFAULT_TRACE_FLOOD_REASON	"TRACE flooding"
#define DEFAULT_TRACK_TEMP_DLINES	false
#define DEFAULT_TRACK_TEMP_KLINES	false
#define DEFAULT_TRAP_CONNECTS		true
#define DEFAULT_TRAP_NICK_CHANGES	false
#define DEFAULT_UMODE			"+bcfknrswyz"
#define DEFAULT_UNAUTHED_MAY_CHAT	false
#define DEFAULT_USER_COUNT_DELTA_MAX	10
#define DEFAULT_WATCH_FLOODER_NOTICES	true
#define DEFAULT_WATCH_INFO_NOTICES	true
#define DEFAULT_WATCH_JUPE_NOTICES	false
#define DEFAULT_WATCH_LINKS_NOTICES	true
#define DEFAULT_WATCH_MOTD_NOTICES	true
#define DEFAULT_WATCH_SPAMBOT_NOTICES	true
#define DEFAULT_WATCH_STATS_NOTICES	false
#define DEFAULT_WATCH_TOOMANY_NOTICES	false
#define DEFAULT_WATCH_TRACE_NOTICES	false
#define DEFAULT_XO_SERVICES_ENABLE	true
#define DEFAULT_XO_SERVICES_REQUEST	"x@services.xo"
#define DEFAULT_XO_SERVICES_RESPONSE	"services.xo"


#endif /* __DEFAULTS_H__ */

