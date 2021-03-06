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
#include <iostream>
#include <string>
#include <cerrno>
#include <ctime>

// Boost C++ Headers
#include <boost/bind.hpp>
#include <boost/lexical_cast.hpp>

// Std C Headers
#include <limits.h>

// OOMon Headers
#include "strtype"
#include "oomon.h"
#include "irc.h"
#include "config.h"
#include "log.h"
#include "main.h"
#include "util.h"
#include "engine.h"
#include "services.h"
#include "dcclist.h"
#include "dcc.h"
#include "vars.h"
#include "jupe.h"
#include "userhash.h"
#include "pattern.h"
#include "botclient.h"
#include "defaults.h"


#ifdef DEBUG
# define IRC_DEBUG
#endif


IRC server;


bool IRC::operNickInReason_(DEFAULT_OPER_NICK_IN_REASON);
bool IRC::relayMsgsToLocops_(DEFAULT_RELAY_MSGS_TO_LOCOPS);
int IRC::serverTimeout_(DEFAULT_SERVER_TIMEOUT);
bool IRC::trackPermDlines_(DEFAULT_TRACK_PERM_DLINES);
bool IRC::trackPermKlines_(DEFAULT_TRACK_PERM_KLINES);
bool IRC::trackTempDlines_(DEFAULT_TRACK_TEMP_DLINES);
bool IRC::trackTempKlines_(DEFAULT_TRACK_TEMP_KLINES);
std::string IRC::umode_(DEFAULT_UMODE);
int IRC::userCountDeltaMax_(DEFAULT_USER_COUNT_DELTA_MAX);


IRC::IRC(): sock_(false, true), supportETrace(false), supportKnock(false),
  caseMapping(CASEMAP_RFC1459), klines('K'), dlines('D')
{
  this->amIAnOper = false;
  this->serverName = "";
  this->gettingKlines = false;
  this->gettingTempKlines = false;
  this->gettingDlines = false;
  this->gettingTempDlines = false;
  this->gettingTrace = false;
  this->myNick = "";
  this->lastUserDeltaCheck = 0;
  this->lastCtcpVersionTimeoutCheck = 0;

  this->sock_.registerOnConnectHandler(boost::bind(&IRC::onConnect, this));
  this->sock_.registerOnReadHandler(boost::bind(&IRC::onRead, this, _1));

  addServerNoticeParser("Client connecting: *", ::onClientConnect);
  addServerNoticeParser("Client exiting: *", ::onClientExit);
  addServerNoticeParser("Nick change: *", ::onNickChange);
  addServerNoticeParser("* is now an operator*", ::onOperNotice);
  addServerNoticeParser("Failed OPER attempt - *", ::onOperFailNotice);
  addServerNoticeParser("Received KILL message for *", ::onKillNotice);
  addServerNoticeParser("Invalid username: *", ::onInvalidUsername);
  addServerNoticeParser("LINKS *", ::onLinksNotice);
  addServerNoticeParser("TRACE *", ::onTraceNotice);
  addServerNoticeParser("MOTD *", ::onMotdNotice);
  addServerNoticeParser("INFO *", ::onInfoNotice);
  addServerNoticeParser("STATS *", ::onStatsNotice);
  addServerNoticeParser("Possible flooder*", ::onFlooderNotice);
  addServerNoticeParser("*is a possible spambot*", ::onSpambotNotice);
  addServerNoticeParser("Too many local connections for*",
    ::onTooManyConnNotice);
  addServerNoticeParser("* is attempting to join locally juped channel*",
    ::onJupeJoinNotice);
  addServerNoticeParser("* added K-Line for *",
    boost::bind(&KlineList::parseAndAdd, &this->klines, _1));
  addServerNoticeParser("* added temporary * K-Line for *",
    boost::bind(&KlineList::parseAndAdd, &this->klines, _1));
  addServerNoticeParser("* has removed the K-Line for: *",
    boost::bind(&KlineList::parseAndRemove, &this->klines, _1));
  addServerNoticeParser("* has removed the temporary K-Line for: *",
    boost::bind(&KlineList::parseAndRemove, &this->klines, _1));
  addServerNoticeParser("Temporary K-line for * expired",
    boost::bind(&KlineList::onExpireNotice, &this->klines, _1));
  addServerNoticeParser("* added D-Line for *",
    boost::bind(&KlineList::parseAndAdd, &this->dlines, _1));
  addServerNoticeParser("* added temporary * D-Line for *",
    boost::bind(&KlineList::parseAndAdd, &this->dlines, _1));
  addServerNoticeParser("* has removed the D-Line for: *",
    boost::bind(&KlineList::parseAndRemove, &this->dlines, _1));
  addServerNoticeParser("* has removed the temporary D-Line for: *",
    boost::bind(&KlineList::parseAndRemove, &this->dlines, _1));
  addServerNoticeParser("Temporary D-line for * expired",
    boost::bind(&KlineList::onExpireNotice, &this->dlines, _1));
  addServerNoticeParser("?LINE active for *", ::onLineActive);
  addServerNoticeParser("?LINE over-ruled for *", ::onLineActive);
  addServerNoticeParser("* is requesting gline for *", ::onGlineRequest);
  addServerNoticeParser("* has triggered gline for *", ::onGlineRequest);
  addServerNoticeParser("Rejecting clonebot: *", ::onCsClones);
  addServerNoticeParser("Clonebot killed: *", ::onCsClones);
  addServerNoticeParser("Nick flooding detected by: *", ::onCsNickFlood);
  addServerNoticeParser("Rejecting *", ::onBotReject);
  addServerNoticeParser("* is clearing temp klines", ::onClearTempKlines);
  addServerNoticeParser("* is clearing temp dlines", ::onClearTempDlines);
  addServerNoticeParser("* masking: * as *", ::onMaskHostNotice);
}


bool
IRC::postSelect(const fd_set & readset, const fd_set & writeset)
{
  std::time_t now = std::time(0);

  // If we've been idle for half the timeout period, send a PING to make
  // sure the connection is still good!
  if (this->isConnected() && (this->getIdle() > (this->getTimeout() / 2)) &&
    (this->getWriteIdle() > (this->getTimeout() / 2)))
  {
    this->write("PING :" + this->myNick + '\n');
  }

  if ((now - this->lastCtcpVersionTimeoutCheck) > 10)
  {
    users.checkVersionTimeout();
    this->lastCtcpVersionTimeoutCheck = now;
  }

  return this->sock_.postSelect(readset, writeset);
}


bool
IRC::onConnect()
{
  Log::Write("Connected to IRC server");

  // Send password (if necessary)
  std::string password(config.serverPassword());
  if (!password.empty())
  {
    this->write("PASS " + password + "\n");
  }

  // Register client
  this->write("USER " + config.username() + " oomon " +
    std::string(OOMON_VERSION) + " :" + config.realName() + "\n");

  // Select nickname
  this->myNick = config.nickname();
  this->write("NICK " + this->myNick + "\n");

  this->amIAnOper = false;

  this->setTimeout(IRC::serverTimeout_);

  return true;
}


int
IRC::write(const std::string & text)
{
#ifdef IRC_DEBUG
  std::cout << "IRC << " << text;
#endif

  return this->sock_.write(text);
}


void
IRC::quit(const std::string & message)
{
#ifdef IRC_DEBUG
  std::cout << "Disconnecting from server." << std::endl;
#endif

  this->write("QUIT :" + message + "\n");
  Log::Write("Disconnecting from server: " + message);
  this->reset();

  this->amIAnOper = 0;
}


bool
IRC::onRead(std::string text)
{
  if (text.empty())
    return true;

#ifdef IRC_DEBUG
  std::cout << "IRC >> " << text << std::endl;
#endif

  StrVector params;

  if (text[0] == ':')
  {
    SplitIRC(params, text.substr(1));
  }
  else
  {
    params.push_back(serverName);

    SplitIRC(params, text);
  }

  std::string from, userhost;
  SplitFrom(params[0], from, userhost);

  std::string command = params[1];

  try
  {
    int numeric = boost::lexical_cast<int>(command);

    switch (numeric)
    {
      case 001:
        serverName = from;
        // Oper up
        this->write("OPER " + config.operName() + " " + config.operPassword() +
            "\n");
        this->klines.Clear();
        this->dlines.Clear();
        this->gettingDlines = false;
        this->gettingKlines = false;
        this->gettingTempDlines = false;
        this->gettingTempKlines = false;
        this->gettingTrace = false;
        {
          std::string channels(config.channels());
          if (!channels.empty())
          {
            this->write("JOIN " + channels + "\n");
          }
        }
	initFloodTables();
        break;
      case 005:
        for (StrVector::size_type idx = 3; idx < params.size(); ++idx)
        {
          if (params[idx] == "ETRACE")
	  {
	    this->supportETrace = true;
	  }
	  else if (params[idx] == "KNOCK")
	  {
	    this->supportKnock = true;
	  }
	  else if (params[idx].substr(0, 12) == "CASEMAPPING=")
	  {
	    std::string map = params[idx].substr(12);

	    if (Same(map, "ascii"))
	    {
	      this->caseMapping = CASEMAP_ASCII;
	    }
	    else if (Same(map, "strict-rfc1459"))
	    {
	      this->caseMapping = CASEMAP_STRICT_RFC1459;
	    }
	    else
	    {
	      this->caseMapping = CASEMAP_RFC1459;
	    }
	  }
	}
	break;
      case 204:	// ontraceuser(body)
      case 205:	// ontraceuser(body)
        if (this->gettingTrace && (params.size() > 6))
        {
          if ((params[6][0] == '[') && (params.size() > 7))
	  {
            // [hybrid7-rc6] IRC >> :plasma.toast.pc 204 OOMon Oper opers OOMon [toast@Plasma.Toast.PC] (192.168.1.1) 0 0
            onTraceUser(params[3], params[4], params[5], params[6], params[7]);
	  }
	  else
	  {
            // [hybrid6] IRC >> :plasma.toast.pc 205 OOMon User 1 Toast[toast@Plasma.Toast.PC] (192.168.1.1) 000000005 000000005
            onTraceUser(params[3], params[4], params[5], params[6]);
	  }
        }
        break;
      case 209:
      case 262:
        if (this->gettingTrace)
        {
          this->gettingTrace = false;
	  users.resetUserCountDelta();
          ::SendAll("*** TRACE complete.", UserFlags::OPER);
        }
        break;
      case 216:
        // IRC >> :plasma.engr.arizona.edu 216 OOMon K *.monkeys.org * * foo llalalala (1998/03/03 11.18)
        // IRC >> :plasma.engr.arizona.edu 216 OOMon K *bork.com * *hork moo la la la (1998/03/03 11.18)
        if ((params.size() > 6) &&
                (((params[3] == "K") && IRC::trackPermKlines_) ||
                 ((params[3] == "k") && IRC::trackTempKlines_)))
        {
	  std::string reason = (params.size() > 7) ? params[7] : "";
	  for (StrVector::size_type pos = 8; pos < params.size(); pos++)
	  {
	    reason += " " + params[pos];
	  }
	  this->klines.Add(params[6] + "@" + params[4], reason,
	    params[3] == "k");
        }
        break;
      case 219:
	if (params.size() >= 3)
	{
          if (params[3] == "K")
          {
            this->gettingKlines = false;
          }
          else if (params[3] == "D")
          {
            this->gettingDlines = false;
          }
          else if (params[3] == "k")
          {
            this->gettingTempKlines = false;
          }
          else if (params[3] == "d")
          {
            this->gettingTempDlines = false;
          }
	}
        break;
      case 225:
        // IRC >> :plasma.toast.pc 225 ToastFOO D 1.2.3.4 :foo (2003/1/9 17.27)
        if ((params.size() > 4) &&
                (((params[3] == "D") && IRC::trackPermDlines_) ||
                 ((params[3] == "d") && IRC::trackTempDlines_)))
        {
	  std::string reason = (params.size() > 5) ? params[5] : "";
	  for (StrVector::size_type pos = 6; pos < params.size(); pos++)
	  {
	    reason += " " + params[pos];
	  }
	  this->dlines.Add(params[4], reason, params[3] == "d");
        }
        break;
      case 303:
        if (params.size() >= 4)
	{
	  services.onIson(params[3]);
        }
        break;
      case 311:	/* RPL_WHOISUSER */
        if (params.size() > 5)
        {
	  // :plasma.toast.pc 311 Toast Toast toast Plasma.Toast.PC * :i
	  std::string nick = params[3];

	  if (this->same(nick, Services::spamtrapNick()))
	  {
	    std::string userhost = params[4] + '@' + params[5];

	    if (this->same(userhost, Services::spamtrapUserhost()))
	    {
	      services.pollSpamtrap();
	    }
	  }
	}
        break;
      case 381:
        {
	  this->amIAnOper = true;
#ifdef USE_FLAGS
	  this->write("FLAGS +ALL\n");
#endif
          this->umode(IRC::umode_);
          this->trace();
	  this->reloadDlines();
	  this->reloadKlines();
        }
	break;
      case 433:
	if (params.size() > 3)
	{
	  std::string to = params[2];
	  std::string usedNick = params[3];

	  if (to == "*")
	  {
	    if (usedNick.length() < 9)
	    {
	      this->myNick = usedNick + "_";
	    }
	    else
	    {
	      this->myNick = usedNick.substr(1) + usedNick.substr(0, 1);
	    }
	    this->write("NICK " + this->myNick + "\n");
	  }
	}
	break;
      case 464:        /* ERR_PASSWDMISMATCH */
        Log::Write("Unable to OPER up due to incorrect password!");
        break;
      case 473:		/* ERR_INVITEONLYCHAN */
	if (params.size() > 3)
	{
	  std::string channel = params[3];

          // If the IRC server supports the KNOCK command and the invite-only
	  // channel is listed in the bot's config file, issue a KNOCK.
	  if (this->supportKnock && config.haveChannel(channel))
	  {
	    this->knock(channel);
	  }
	}
	break;
      case 491:        /* ERR_NOOPERHOST */
        Log::Write("Unable to OPER up due to incorrect oper name or insufficient privileges from IRC server!");
        break;
      case 709:
        if (this->gettingTrace && (params.size() > 9))
        {
          // :plasma.toast.pc 709 toast Oper opers toast toast Plasma.Toast.PC 192.168.1.1 :gecos information goes here
          onETraceUser(params[3], params[4], params[5], params[6], params[7],
	    params[8], params[9]);
        }
	break;
      default:
	// Just ignore all other numerics
	break;
    }
  }
  catch (boost::bad_lexical_cast)
  {
    switch (IRC::getCommand(command))
    {
      case IRC_PING:
	if (params.size() > 2)
	{
          this->write("PONG " + params[2] + "\n");
	}
	else
	{
          this->write("PONG\n");
	}
        break;
      case IRC_NICK:
	if (params.size() > 2)
	{
	  if (this->same(from, this->myNick))
	  {
	    this->myNick = params[2];
	  }
	}
	break;
      case IRC_JOIN:
	if ((params.size() >= 3) && this->same(from, this->myNick))
	{
	  Log::Write("Joined channel " + params[2]);
	}
	break;
      case IRC_PART:
	if ((params.size() >= 3) && this->same(from, this->myNick))
	{
	  Log::Write("Parted channel " + params[2]);
	}
	break;
      case IRC_KICK:
	if ((params.size() >= 5) && this->same(params[3], this->myNick))
	{
	  Log::Write("Kicked from channel " + params[2] + " by " + from + 
	    " (" + params[4] + ')');
	}
	break;
      case IRC_INVITE:
	if (params.size() >= 3)
	{
	  std::string to = params[2];
	  std::string channel = params[3];

          // Make sure the INVITE was directed at the bot
          if (this->same(to, this->myNick))
	  {
	    // Is the channel listed in the config file?
	    if (config.haveChannel(channel))
	    {
	      // Yes, so accept the invite by joining the channel!
	      this->join(channel);
	    }

	    Log::Write("Invited to channel " + params[3] + " by " + from + '.');
	  }
	}
	break;
      case IRC_NOTICE:
	if (params.size() > 3)
        {
	  this->onNotice(from, userhost, params[2], params[3]);
        }
        break;
      case IRC_PRIVMSG:
        if (params.size() > 3)
	{
          this->onPrivmsg(from, userhost, params[2], params[3]);
        }
        break;
      case IRC_WALLOPS:
	if (params.size() > 2)
        {
	  std::string text = params[2];
	  std::string wallopsType = "WALLOPS";

	  if ((text.length() > 11) && (text.substr(0, 11) == "OPERWALL - "))
	  {
	    wallopsType = "OPERWALL";
	    text = text.substr(11, std::string::npos);
	  }
	  else if ((text.length() > 9) && (text.substr(0, 9) == "LOCOPS - "))
	  {
	    wallopsType = "LOCOPS";
	    text = text.substr(9, std::string::npos);
	  }
	  else if ((text.length() > 10) && (text.substr(0, 10) == "WALLOPS - "))
	  {
	    text = text.substr(10, std::string::npos);
	  }

          clients.sendAll("[" + from + ":" + wallopsType + "] " + text,
	    UserFlags::WALLOPS, WATCH_WALLOPS);
        }
        break;
      case IRC_ERROR:
	if (params.size() > 2)
	{
	  Log::Write(params[2]);
	}
	break;
      default:
	// Ignore all other IRC commands
	break;
    }
  }

  return true;
}

void
IRC::onCtcp(const std::string & from, const std::string & userhost,
  const std::string & to, std::string text)
{
  std::string command = this->upCase(FirstWord(text));

  BotSock::Address ip = users.getIP(from, userhost);

  if ((0 == command.compare("DCC")) && (this->same(to, this->myNick)))
  {
    std::string dccCommand = this->upCase(FirstWord(text));

    if (0 == dccCommand.compare("CHAT"))
    {
      FirstWord(text);	// Ignore "chat" parameter

      if (config.mayChat(userhost, ip))
      {
        try
	{
          BotSock::Address address =
            boost::lexical_cast<BotSock::Address>(FirstWord(text));
          BotSock::Port port =
            boost::lexical_cast<BotSock::Port>(FirstWord(text));

          if ((address == INADDR_ANY) || (address == INADDR_NONE))
          {
            Log::Write("Invalid DCC CHAT address from " + from + " (" +
                userhost + ")");
            this->notice(from,
                "Invalid address specified for DCC CHAT. Not funny.");
          }
          else if (port < 1024)
          {
            Log::Write("Invalid DCC CHAT port from " + from + " (" + userhost +
                ")");
            this->notice(from,
                "Invalid port specified for DCC CHAT. Not funny.");
          }
	  else
	  {
            Log::Write("DCC CHAT request from " + from + " (" + userhost + ")");

            if (!clients.connect(address, port, from, userhost, ip))
            {
              Log::Write("DCC CHAT failed for " + from);
	    }
	  }
        }
	catch (boost::bad_lexical_cast)
	{
          Log::Write("Malformed DCC CHAT request from " + from + " (" +
              userhost + ")");
          this->notice(from, "Malformed DCC CHAT request. Not funny.");
	}
      }
      else
      {
        Log::Write("Unauthorized DCC CHAT request from " + from + " (" +
            userhost + ")");
      }
    }
    else
    {
      Log::Write("Unsupported DCC " + dccCommand + " request from " + from +
          " (" + userhost + ")");
    }
  }
  else if (0 == command.compare("CHAT"))
  {
    if (config.mayChat(userhost, ip))
    {
      Log::Write("CTCP CHAT request from " + from + " (" + userhost + ")");

      if (!clients.listen(from, userhost, ip))
      {
        Log::Write("DCC CHAT listen failed for " + from);
        this->notice(from,
          "Unable to initiate DCC CHAT request!  Try again later?");
      }
    }
    else
    {
      Log::Write("Unauthorized CTCP CHAT request from " + from + " (" +
          userhost + ")");
    }
  }
  else if ((command == std::string("PING")) ||
    (command == std::string("ECHO")))
  {
    this->ctcpReply(from, command + " " + text);
  }
  else if (command == std::string("VERSION"))
  {
    this->ctcpReply(from, "VERSION OOMon-" OOMON_VERSION
      " - http://oomon.sourceforge.net/");
  }
}


void
IRC::onCtcpReply(const std::string & from, const std::string & userhost,
  const std::string & to, std::string text)
{
  if (this->same(to, this->myNick))
  {
    std::string command(FirstWord(text));

    if (this->same(command, "VERSION"))
    {
      std::string notice("*** CTCP VERSION reply from ");
      notice += from;
      notice += ": ";
      notice += text;
      ::SendAll(notice, UserFlags::OPER, WATCH_CTCPVERSIONS);

      users.onVersionReply(from, userhost, text);
    }
  }
}


void
IRC::onPrivmsg(const std::string & from, const std::string & userhost,
  const std::string & to, std::string text)
{
  if ((text.length() > 0) && (text[0] == '\001'))
  {
    std::string ctcp = text.substr(1);

    std::string::size_type end = ctcp.find('\001');

    if (end != std::string::npos)
    {
      this->onCtcp(from, userhost, to, ctcp.substr(0, end));
    }
  }
  else if (this->same(to, this->myNick))
  {
    if (Services::spamtrapEnable() &&
      this->same(from, Services::spamtrapNick()) &&
      this->same(userhost, Services::spamtrapUserhost()))
    {
      services.onSpamtrapMessage(text);
    }
    else
    {
      std::string msg("*");
      msg += from;
      msg += "* ";
      msg += text;
      msg += " <";
      msg += userhost;
      msg += '>';

      Log::Write(msg);
      ::SendAll("(IRC) " + msg, UserFlags::OPER, WATCH_MSGS);
      if (IRC::relayMsgsToLocops_)
      {
	this->locops(msg);
      }
      users.onPrivmsg(from, userhost, text);
    }
  }
}


void
IRC::onNotice(const std::string & from, const std::string & userhost,
	const std::string & to, std::string text)
{
  if (!text.empty() && (text[0] == '\001'))
  {
    text.erase(text.begin());

    std::string::size_type end = text.find('\001');

    if (std::string::npos != end)
    {
      text.erase(end, std::string::npos);
    }

    this->onCtcpReply(from, userhost, to, text);
  }
  else if (this->same(from, serverName))
  {
    if ((text.length() > 14) &&
      (0 == text.substr(0, 14).compare("*** Notice -- ")))
    {
      // Server Notice
      this->onServerNotice(text.substr(14));
    }
    else if (0 ==
      this->upCase(text).compare("*** YOU NEED OPER AND N FLAG FOR +N"))
    {
      Log::Write("Unable to set user mode +n!  I can't see nick changes!");
    }
  }
  else if (this->same(from, Services::xoServicesResponse()))
  {
    services.onXoNotice(text);
  }
  else if (this->same(from, CA_SERVICES_RESPONSE))
  {
    services.onCaNotice(text);
  }
  else if (this->same(to, this->myNick))
  {
    if (Services::spamtrapEnable() &&
        this->same(from, Services::spamtrapNick()) &&
        this->same(userhost, Services::spamtrapUserhost()))
    {
      services.onSpamtrapNotice(text);
    }
    else if (userhost.length() > 0)
    {
      // Process all non-server notices
      std::string notice("-");
      notice += from;
      notice += "- ";
      notice += text;
      notice += " <";
      notice += userhost;
      notice += '>';

      Log::Write(notice);
      ::SendAll("(IRC) " + notice, UserFlags::OPER, WATCH_NOTICES);
      users.onNotice(from, userhost, text);
    }
  }
}


void
IRC::notice(const std::string & to, const std::string & text)
{
  this->write("NOTICE " + to + " :" + text + "\n");
}


void
IRC::notice(const std::string & to, const StrList & text)
{
  for (StrList::const_iterator pos = text.begin(); pos != text.end(); ++pos)
  {
    this->notice(to, *pos);
  }
}


void
IRC::msg(const std::string & to, const std::string & text)
{
  this->write("PRIVMSG " + to + " :" + text + "\n");
}


void
IRC::ctcp(const std::string & to, const std::string & text)
{
  this->write("PRIVMSG " + to + " :\001" + text + "\001\n");
}


void
IRC::ctcpReply(const std::string & to, const std::string & text)
{
  this->write("NOTICE " + to + " :\001" + text + "\001\n");
}


void
IRC::isOn(const std::string & text)
{
  this->write("ISON " + text + "\n");
}


void
IRC::kline(const std::string & from, const unsigned int minutes,
  const std::string & target, const std::string & reason)
{
  std::string line("KLINE ");

  if (minutes > 0)
  {
    line += boost::lexical_cast<std::string>(minutes);
    line += ' ';
    line += target;
    line += " :";
    line += reason;
  }
  else
  {
    line += target;
    line += " :";
    line += reason;
  }

  if (IRC::operNickInReason_)
  {
    this->write(line + " [" + from + "]\n");
  }
  else
  {
    this->write(line + "\n");
  }
  Log::Write(line + " [" + from + "]");
}


void
IRC::remoteKline(const std::string & from, const std::string & to,
    const unsigned int minutes, const std::string & target,
    const std::string & reason)
{
  std::string line("KLINE ");

  if (minutes > 0)
  {
    line += boost::lexical_cast<std::string>(minutes);
    line += ' ';
    line += target;
    line += " ON ";
    line += to;
    line += " :";
    line += reason;
  }
  else
  {
    line += target;
    line += " :";
    line += reason;
  }

  if (IRC::operNickInReason_)
  {
    this->write(line + " [" + from + "]\n");
  }
  else
  {
    this->write(line + "\n");
  }
  Log::Write(line + " [" + from + "]");
}


void
IRC::unkline(const std::string & from, const std::string & target)
{
  this->write("UNKLINE " + target + "\n");
  Log::Write("UNKLINE " + target + " [" + from + "]");
}


void
IRC::dline(const std::string & from, const unsigned int minutes,
  const std::string & target, const std::string & reason)
{
  std::string line;

  if (minutes > 0)
  {
    line = "DLINE " + boost::lexical_cast<std::string>(minutes) + " " +
      target + " :" + reason;
  }
  else
  {
    line = "DLINE " + target + " :" + reason;
  }

  if (IRC::operNickInReason_)
  {
    this->write(line + " [" + from + "]\n");
  }
  else
  {
    this->write(line + "\n");
  }
  Log::Write(line + " [" + from + "]");
}


void
IRC::undline(const std::string & from, const std::string & target)
{
  this->write("UNDLINE " + target + "\n");
  Log::Write("UNDLINE " + target + " [" + from + "]");
}


void
IRC::kill(const std::string & from, const std::string & target,
  const std::string & reason)
{
  std::string line = "KILL " + target;

  if (IRC::operNickInReason_)
  {
    this->write(line + " :(" + from + ") " + reason + "\n");
  }
  else
  {
    this->write(line + " :" + reason + "\n");
  }
  Log::Write(line + " ((" + from + ") " + reason + ")");
}


void
IRC::trace(const std::string & target)
{
  if (target.empty())
  {
    users.clear();
    this->gettingTrace = true;
    if (this->supportETrace)
    {
      this->write("ETRACE\n");
    }
    else
    {
      this->write("TRACE\n");
    }
  }
  else
  {
    this->write("TRACE " + target + '\n');
  }
}


void
IRC::statsL(const std::string & nick)
{
  if (!nick.empty())
  {
    this->write("STATS L " + nick + '\n');
  }
}


void
IRC::retrace(const std::string & from)
{
  ::SendAll("Reload USERS requested by " + from, UserFlags::OPER);
  this->trace();
}


void
IRC::reloadKlines(void)
{
  this->klines.Clear();
  if (IRC::trackPermKlines_)
  {
    this->gettingKlines = true;
    this->write("STATS K\n");
  }
  if (IRC::trackTempKlines_)
  {
    this->gettingTempKlines = true;
    this->write("STATS k\n");
  }
}


void
IRC::reloadKlines(const std::string & from)
{
    ::SendAll("Reload KLINES requested by " + from, UserFlags::OPER);
    this->reloadKlines();
}


void
IRC::reloadDlines(void)
{
  this->dlines.Clear();
  if (IRC::trackPermDlines_)
  {
    this->gettingDlines = true;
    this->write("STATS D\n");
  }
  if (IRC::trackTempDlines_)
  {
    this->gettingTempDlines = true;
    this->write("STATS d\n");
  }
}


void
IRC::reloadDlines(const std::string & from)
{
  ::SendAll("Reload DLINES requested by " + from, UserFlags::OPER);
  this->reloadDlines();
}


void
IRC::findK(BotClient * client, const Pattern *userhost, const bool count,
  const bool searchPerms, const bool searchTemps, const bool searchReason) const
{
  this->klines.find(client, userhost, count, searchPerms, searchTemps,
    searchReason);
}


int
IRC::findAndRemoveK(const std::string & from, const Pattern *userhost,
  const bool searchPerms, const bool searchTemps, const bool searchReason)
{
  return this->klines.findAndRemove(from, userhost, searchPerms, searchTemps,
    searchReason);
}


void
IRC::findD(BotClient * client, const Pattern *userhost, const bool count,
  const bool searchPerms, const bool searchTemps, const bool searchReason) const
{
  this->dlines.find(client, userhost, count, searchPerms, searchTemps,
    searchReason);
}


int
IRC::findAndRemoveD(const std::string & from, const Pattern *userhost,
  const bool searchPerms, const bool searchTemps, const bool searchReason)
{
  return this->dlines.findAndRemove(from, userhost, searchPerms, searchTemps,
    searchReason);
}


void
IRC::op(const std::string & channel, const std::string & nick)
{
  this->write("MODE " + channel + " +o " + nick + "\n");
}

//////////////////////////////////////////////////////////////////////
// join(channel)
//
// Description:
//  Sends a JOIN command to the IRC server.
//
// Parameters:
//  channel - The channel to attempt to join.
//
// Return Value:
//  None.
//////////////////////////////////////////////////////////////////////
void
IRC::join(const std::string & channel)
{
  this->write("JOIN " + channel + "\n");
}

//////////////////////////////////////////////////////////////////////
// join(channel, key)
//
// Description:
//  Sends a JOIN command with a key to the IRC server.
//
// Parameters:
//  channel - The channel to attempt to join.
//  key     - The channel key to use when joining.
//
// Return Value:
//  None.
//////////////////////////////////////////////////////////////////////
void
IRC::join(const std::string & channel, const std::string & key)
{
  this->write("JOIN " + channel + " " + key + "\n");
}

//////////////////////////////////////////////////////////////////////
// part(channel)
//
// Description:
//  Sends a PART command to the IRC server.
//
// Parameters:
//  channel - The channel to attempt to leave.
//
// Return Value:
//  None.
//////////////////////////////////////////////////////////////////////
void
IRC::part(const std::string & channel)
{
  this->write("PART " + channel + "\n");
}

//////////////////////////////////////////////////////////////////////
// knock(channel)
//
// Description:
//  Sends a KNOCK command to the IRC server.
//
// Parameters:
//  channel - The channel for which the knock is intended.
//
// Return Value:
//  None.
//////////////////////////////////////////////////////////////////////
void
IRC::knock(const std::string & channel)
{
  this->write("KNOCK " + channel + "\n");

  std::string msg("*** Sent knock to channel ");
  msg += channel;
  Log::Write(msg);
}


void
IRC::status(BotClient * client) const
{
  client->send("Connect Time: " + this->getUptime());

  KlineList::size_type klineCount = this->klines.size();
  if (IRC::trackTempKlines_ && (klineCount > 0))
  {
    client->send("K: lines: " + boost::lexical_cast<std::string>(klineCount) +
      " (" + boost::lexical_cast<std::string>(this->klines.permSize()) +
      " permanent)");
  }
  else if (IRC::trackPermKlines_)
  {
    client->send("K: lines: " + boost::lexical_cast<std::string>(klineCount));
  }

  KlineList::size_type dlineCount = this->dlines.size();
  if (IRC::trackTempDlines_ && (dlineCount > 0))
  {
    client->send("D: lines: " + boost::lexical_cast<std::string>(dlineCount) +
      " (" + boost::lexical_cast<std::string>(this->dlines.permSize()) +
      " permanent)");
  }
  else if (IRC::trackPermDlines_)
  {
    client->send("D: lines: " + boost::lexical_cast<std::string>(dlineCount));
  }
}


void
IRC::subSpamTrap(const bool sub)
{
  if (sub)
  {
    this->msg(Services::spamtrapNick(),
        ".nicksub " +
        boost::lexical_cast<std::string>(Services::spamtrapMinScore()) + " " +
        this->getServerName() + " raw");
  }
  else
  {
    this->msg(Services::spamtrapNick(), ".nickunsub");
  }
}


void
IRC::whois(const std::string & nick)
{
  this->write("WHOIS " + nick + '\n');
}


void
IRC::umode(const std::string & mode)
{
  this->write("MODE " + this->myNick + " :" + mode + '\n');
}


void
IRC::locops(const std::string & text)
{
  this->write("LOCOPS :" + text + '\n');
}


void
IRC::checkUserDelta(void)
{
  std::time_t now = std::time(NULL);
  std::time_t lapse = now - this->lastUserDeltaCheck;

  if (lapse >= 10)
  {
    int delta = users.getUserCountDelta();

    if ((this->lastUserDeltaCheck > 0) && (delta > IRC::userCountDeltaMax_))
    {
      std::string msg("*** User count increased by ");
      msg += boost::lexical_cast<std::string>(delta);
      msg += " in ";
      msg += boost::lexical_cast<std::string>(lapse);
      msg += " seconds.";

      clients.sendAll(msg, UserFlags::OPER);
      Log::Write(msg);
    }
    this->lastUserDeltaCheck = now;
  }
}


IRCCommand
IRC::getCommand(const std::string & text)
{
  if (text == "PING")
    return IRC_PING;
  else if (text == "NICK")
    return IRC_NICK;
  else if (text == "JOIN")
    return IRC_JOIN;
  else if (text == "PART")
    return IRC_PART;
  else if (text == "KICK")
    return IRC_KICK;
  else if (text == "INVITE")
    return IRC_INVITE;
  else if (text == "NOTICE")
    return IRC_NOTICE;
  else if (text == "PRIVMSG")
    return IRC_PRIVMSG;
  else if (text == "WALLOPS")
    return IRC_WALLOPS;
  else if (text == "ERROR")
    return IRC_ERROR;
  else
    return IRC_UNKNOWN;
}


//////////////////////////////////////////////////////////////////////
// IRC::upCase(c)
//
// Description:
//  Converts a character to upper-case.
//
// Parameters:
//  c - An upper or lower-case character.
//
// Return Value:
//  The function returns the upper-case representation of the
//  character.
//////////////////////////////////////////////////////////////////////
char
IRC::upCase(const char c) const
{
  char result;

  if (this->caseMapping == CASEMAP_ASCII)
  {
    if ((c >= 97) && (c <= 122))
    {
      result = static_cast<char>((c - 32));
    }
    else
    {
      result = c;
    }
  }
  else if (this->caseMapping == CASEMAP_STRICT_RFC1459)
  {
    if ((c >= 97) && (c <= 125))
    {
      result = static_cast<char>((c - 32));
    }
    else
    {
      result = c;
    }
  }
  else /* this->caseMapping == CASEMAP_RFC1459 */
  {
    if ((c >= 97) && (c <= 126))
    {
      result = static_cast<char>((c - 32));
    }
    else
    {
      result = c;
    }
  }

  return result;
}


//////////////////////////////////////////////////////////////////////
// IRC::upCase(text)
//
// Description:
//  Converts a string to all upper-case characters.
//
// Parameters:
//  text - A string containing upper and/or lower-case characters.
//
// Return Value:
//  The function returns the upper-case representation of the string.
//////////////////////////////////////////////////////////////////////
std::string
IRC::upCase(const std::string & text) const
{
  std::string result;

  if (this->caseMapping == CASEMAP_ASCII)
  {
    for (std::string::const_iterator pos = text.begin(); pos != text.end();
      ++pos)
    {
      if ((*pos >= 97) && (*pos <= 122))
      {
        result += static_cast<char>((*pos - 32));
      }
      else
      {
        result += static_cast<char>(*pos);
      }
    }
  }
  else if (this->caseMapping == CASEMAP_STRICT_RFC1459)
  {
    for (std::string::const_iterator pos = text.begin(); pos != text.end();
      ++pos)
    {
      if ((*pos >= 97) && (*pos <= 125))
      {
        result += static_cast<char>((*pos - 32));
      }
      else
      {
        result += static_cast<char>(*pos);
      }
    }
  }
  else /* this->caseMapping == CASEMAP_RFC1459 */
  {
    for (std::string::const_iterator pos = text.begin(); pos != text.end();
      ++pos)
    {
      if ((*pos >= 97) && (*pos <= 126))
      {
        result += static_cast<char>((*pos - 32));
      }
      else
      {
        result += static_cast<char>(*pos);
      }
    }
  }

  return result;
}


//////////////////////////////////////////////////////////////////////
// IRC::downCase(c)
//
// Description:
//  Converts a character to lower-case.
//
// Parameters:
//  c - An upper or lower-case character.
//
// Return Value:
//  The function returns the lower-case representation of the
//  character.
//////////////////////////////////////////////////////////////////////
char
IRC::downCase(const char c) const
{
  char result;

  if (this->caseMapping == CASEMAP_ASCII)
  {
    if ((c >= 65) && (c <= 90))
    {
      result = static_cast<char>((c + 32));
    }
    else
    {
      result = c;
    }
  }
  else if (this->caseMapping == CASEMAP_STRICT_RFC1459)
  {
    if ((c >= 65) && (c <= 93))
    {
      result = static_cast<char>((c + 32));
    }
    else
    {
      result = c;
    }
  }
  else /* this->caseMapping == CASEMAP_RFC1459 */
  {
    if ((c >= 65) && (c <= 94))
    {
      result = static_cast<char>((c + 32));
    }
    else
    {
      result = c;
    }
  }

  return result;
}


//////////////////////////////////////////////////////////////////////
// IRC::downCase(text)
//
// Description:
//  Converts a string to all lower-case characters.
//
// Parameters:
//  text - A string containing upper and/or lower-case characters.
//
// Return Value:
//  The function returns the lower-case representation of the string.
//////////////////////////////////////////////////////////////////////
std::string
IRC::downCase(const std::string & text) const
{
  std::string result;

  if (this->caseMapping == CASEMAP_ASCII)
  {
    for (std::string::const_iterator pos = text.begin(); pos != text.end();
      ++pos)
    {
      if ((*pos >= 65) && (*pos <= 90))
      {
        result += static_cast<char>((*pos + 32));
      }
      else
      {
        result += static_cast<char>(*pos);
      }
    }
  }
  else if (this->caseMapping == CASEMAP_STRICT_RFC1459)
  {
    for (std::string::const_iterator pos = text.begin(); pos != text.end();
      ++pos)
    {
      if ((*pos >= 65) && (*pos <= 93))
      {
        result += static_cast<char>((*pos + 32));
      }
      else
      {
        result += static_cast<char>(*pos);
      }
    }
  }
  else /* this->caseMapping == CASEMAP_RFC1459 */
  {
    for (std::string::const_iterator pos = text.begin(); pos != text.end();
      ++pos)
    {
      if ((*pos >= 65) && (*pos <= 94))
      {
        result += static_cast<char>((*pos + 32));
      }
      else
      {
        result += static_cast<char>(*pos);
      }
    }
  }

  return result;
}


//////////////////////////////////////////////////////////////////////
// IRC::same(text1, text2)
//
// Description:
//  Case-insensitively compares two strings.
//
// Parameters:
//  text1  - The first string.
//  text2  - The second string.
//
// Return Value:
//  The function returns true if both strings match.
//////////////////////////////////////////////////////////////////////
bool
IRC::same(const std::string & text1, const std::string & text2) const
{
  return (0 == this->upCase(text1).compare(this->upCase(text2)));
}


IRC::Parser::Parser(const std::string & pattern, const ParserFunction func)
  : func_(func)
{
  this->pattern_.reset(new ClusterPattern(pattern));
}


//////////////////////////////////////////////////////////////////////
// IRC::Parser::match(text)
//
// Description:
//  Compares the text against the parser's pattern.
//
// Parameters:
//  text - The text to compare against the pattern.
//
// Return Value:
//  The function returns true if the text matches the pattern and
//  the caller should stop parsing the text.
//////////////////////////////////////////////////////////////////////
bool
IRC::Parser::match(std::string text) const
{
  bool result = false;

  if (this->pattern_->match(text))
  {
    result = this->func_(text);
  }

  return result;
}


//////////////////////////////////////////////////////////////////////
// IRC::addServerNotice(pattern, func)
//
// Description:
//  Registers a new server notice handler function.
//
// Parameters:
//  pattern - A string containing a cluster-style pattern.
//  func    - The function to call when text matching the pattern is
//            detected.
//
// Return Value:
//  None
//////////////////////////////////////////////////////////////////////
void
IRC::addServerNoticeParser(const std::string & pattern,
  const ParserFunction func)
{
  serverNotices.push_back(Parser(pattern, func));
}


void
IRC::onServerNotice(const std::string & text)
{
  std::find_if(this->serverNotices.begin(), this->serverNotices.end(),
    boost::bind(&Parser::match, _1, text));
}


bool
IRC::validNick(const std::string & nick)
{
  bool result = false;

  if (!nick.empty() && (nick.length() <= 32))
  {
    const std::string invalidFirst("-0123456789");

    if (std::string::npos == invalidFirst.find(nick[0]))
    {
      const std::string validRest("-0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}");

      if (std::string::npos == nick.find_first_not_of(validRest))
      {
        result = true;
      }
    }
  }

  return result;
}


std::string
IRC::getServerTimeout(void)
{
  return boost::lexical_cast<std::string>(IRC::serverTimeout_);
}


std::string
IRC::setServerTimeout(const std::string & newValue)
{
  std::string result;

  try
  {
    int timeout = boost::lexical_cast<int>(newValue);
    if (timeout >= 30)
    {
      IRC::serverTimeout_ = timeout;
      server.setTimeout(timeout);
    }
    else
    {
      result = "*** Numeric value between 30 and ";
      result += boost::lexical_cast<std::string>(INT_MAX);
      result += " expected!";
    }
  }
  catch (boost::bad_lexical_cast)
  {
    result = "*** Numeric value expected!";
  }

  return result;
}


void
IRC::preSelect(fd_set & r, fd_set & w) const
{
  this->sock_.preSelect(r, w);
}


bool
IRC::connect(const std::string & address, const BotSock::Port port)
{
  return this->sock_.connect(address, port);
}


BotSock::Address
IRC::getLocalAddress(void) const
{
  return this->sock_.getLocalAddress();
}


BotSock::Address
IRC::getRemoteAddress(void) const
{
  return this->sock_.getRemoteAddress();
}


void
IRC::init(void)
{
  KlineList::init();

  vars.insert("OPER_NICK_IN_REASON",
      Setting::BooleanSetting(IRC::operNickInReason_));
  vars.insert("RELAY_MSGS_TO_LOCOPS",
      Setting::BooleanSetting(IRC::relayMsgsToLocops_));
  vars.insert("SERVER_TIMEOUT", Setting(IRC::getServerTimeout,
        IRC::setServerTimeout));
  vars.insert("TRACK_PERM_DLINES",
      Setting::BooleanSetting(IRC::trackPermDlines_));
  vars.insert("TRACK_PERM_KLINES",
      Setting::BooleanSetting(IRC::trackPermKlines_));
  vars.insert("TRACK_TEMP_DLINES",
      Setting::BooleanSetting(IRC::trackTempDlines_));
  vars.insert("TRACK_TEMP_KLINES",
      Setting::BooleanSetting(IRC::trackTempKlines_));
  vars.insert("UMODE", Setting::StringSetting(IRC::umode_));
  vars.insert("USER_COUNT_DELTA_MAX",
      Setting::IntegerSetting(IRC::userCountDeltaMax_, 1));
}

