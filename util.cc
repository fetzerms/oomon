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

// C headers
#include <stdio.h>
#include <sys/time.h>
#include <unistd.h>
#include <time.h>

// C++ headers
#include <string>
#include <cctype>

#include "strtype"
#include "oomon.h"
#include "util.h"


//////////////////////////////////////////////////////////////////////
// StrSplit(temp, input, tokens, ignoreEmpties)
//
// Description:
//  Converts a token-separated string into a vector of strings.  This
//  function works similarly to Perl's split() function.
//
// Parameters:
//  temp          - The resulting vector of strings.
//  input         - A token-seprated string.
//  tokens        - A string containing one or more token characters.
//  ignoreEmpties - If true, empty strings will not be inserted into
//                  vector.
//
// Return Value:
//  The function returns the number of nodes inserted into the vector.
//////////////////////////////////////////////////////////////////////
int
StrSplit(StrVector & temp, std::string input, const std::string & tokens,
  bool ignoreEmpties)
{
  int count = 0;

  temp.clear();

  while (input != "")
  {
    std::string::size_type pos = input.find_first_of(tokens);

    if (std::string::npos == pos)
    {
      temp.push_back(input);
      input = "";
      count++;
    }
    else
    {
      std::string unit = input.substr(0, pos);
      std::string rest = input.substr(pos + 1);

      if (!ignoreEmpties || (unit != ""))
      {
        temp.push_back(unit);
        count++;
      }
      input = rest;
    }
  }
  return count;
}


//////////////////////////////////////////////////////////////////////
// SplitIRC(temp, text)
//
// Description:
//  Converts a line received from an IRC server to a vector of strings
//  where each node in the vector is a parameter.  A colon (':') is
//  interpreted to mean the rest of the string is a single parameter.
//
// Parameters:
//  temp - The resulting vector.
//  text - The string received from the IRC server.
//
// Return Value:
//  The function returns the number of nodes inserted into the vector.
//////////////////////////////////////////////////////////////////////
int
SplitIRC(StrVector & temp, std::string text)
{
  int count = 0;

  temp.clear();
  while (text.length() > 0)
  {
    if (text.at(0) == ':')
    {
      text.erase((std::string::size_type) 0, 1);
      temp.push_back(text);
      text = std::string("");
      count++;
    }
    else
    {
      std::string::size_type i = text.find(' ');
      if (std::string::npos != i)
      {
	if (i == 0)
	{
	  text.erase((std::string::size_type) 0, 1);
	}
	else
	{
	  std::string chunk = text.substr(0, i);
          temp.push_back(chunk);
          text.erase(0, i + 1);
          count++;
	}
      }
      else
      {
        temp.push_back(text);
        text = "";
        count++;
      }
    }
  }
  return count;
}


//////////////////////////////////////////////////////////////////////
// SplitFrom(input, Nick, UserHost)
//
// Description:
//  Splits a nick!user@host string into two parts: a nick part and a
//  userhost part.
//
// Parameters:
//  input    - A nick!user@host string received from an IRC server.
//             This can also contain a server name.
//  Nick     - Destination for the nick portion of the string.
//  UserHost - Destination for the userhost portion of the string.
//
// Return Value:
//  None.
//////////////////////////////////////////////////////////////////////
void
SplitFrom(std::string input, std::string & Nick, std::string & UserHost)
{
  std::string::size_type i = input.find('!');
  if (std::string::npos == i)
  {
    // This is a server, not a nick!user@host
    Nick = input;
    UserHost = "";
  }
  else
  {
    // This is a nick!user@host
    Nick = input.substr(0, i);
    UserHost = input.substr(i + 1);
  }
}


//////////////////////////////////////////////////////////////////////
// getNick(text)
//
// Description:
//  Identifies the nickname in a nick!user@host string.
//
// Parameters:
//  text - A string containing a nick!user@host.
//
// Return Value:
//  The function returns the portion of the string up to but not
//  including the first '!' character.
//////////////////////////////////////////////////////////////////////
std::string
getNick(const std::string & text)
{
  std::string::size_type bang = text.find('!');

  if (std::string::npos == bang)
  {
    return text;
  }
  else
  {
    return text.substr(0, bang);
  }
}


//////////////////////////////////////////////////////////////////////
// atoul(text)
//
// Description:
//  Converts a string to an unsigned long.  The function will stop
//  converting at the first non-numeric character.
//
// Parameters:
//  text - A string containing a decimal representation of an unsigned
//         long integer.
//
// Return Value:
//  The function returns the converted unsigned long.  If the string
//  begins with a non-numeric character, the function will return 0.
//////////////////////////////////////////////////////////////////////
unsigned long
atoul(const char *text)
{
  unsigned long temp = 0;
  while ((*text >= '0') && (*text <= '9'))
    temp = temp * 10 + (*(text++) - '0');
  return temp;
}


//////////////////////////////////////////////////////////////////////
// FirstWord(text)
//
// Description:
//  Removes the first word of a string.  The function will stop at
//  the first space character.
//
// Parameters:
//  text - The string from which to remove the first word.
//
// Return Value:
//  The function returns the word that was removed from the beginning
//  of the string.
//////////////////////////////////////////////////////////////////////
std::string
FirstWord(std::string & text)
{
  std::string temp;
  std::string::size_type i = text.find(' ');
  if (std::string::npos != i)
  {
    temp = text.substr(0, i);
    text = text.substr(i + 1);
  }
  else
  {
    temp = text;
    text = "";
  }
  while ((text.length() > 0) && (text[0] == ' '))
    text.erase((std::string::size_type) 0, 1);
  return temp;
}


//////////////////////////////////////////////////////////////////////
// UpCase(c)
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
UpCase(const char c)
{
	return ((c >= 'a') && (c <= 'z')) ? (c - 'a' + 'A') : c;
}


//////////////////////////////////////////////////////////////////////
// UpCase(text)
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
UpCase(const std::string & text)
{
  std::string result = text;
  std::string::size_type len = result.length();

  for (std::string::size_type index = 0; index < len; index++)
  {
	result[index] = UpCase(result[index]);
  }

  return result;
}


//////////////////////////////////////////////////////////////////////
// DownCase(c)
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
DownCase(const char c)
{
	return ((c >= 'A') && (c <= 'Z')) ? (c - 'A' + 'a') : c;
}


//////////////////////////////////////////////////////////////////////
// DownCase(text)
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
DownCase(const std::string & text)
{
  std::string result = text;
  std::string::size_type len = result.length();

  for (std::string::size_type index = 0; index < len; index++)
  {
	result[index] = DownCase(result[index]);
  }

  return result;
}


//////////////////////////////////////////////////////////////////////
// Same(text1, text2)
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
Same(const std::string & text1, const std::string & text2)
{
  return (UpCase(text1) == UpCase(text2));
}


//////////////////////////////////////////////////////////////////////
// Same(text1, text2, length)
//
// Description:
//  Case-insensitively compares up to length characters of two
//  strings.
//
// Parameters:
//  text1  - The first string.
//  text2  - The second string.
//  length - The maximum number of characters to compare.
//
// Return Value:
//  The function returns true if the characters in both strings match.
//////////////////////////////////////////////////////////////////////
bool
Same(const std::string & text1, const std::string & text2,
  const std::string::size_type length)
{
  if ((length > text1.length()) || (length > text2.length()))
  {
    return false;
  }

  return (UpCase(text1.substr(0, length)) == UpCase(text2.substr(0, length)));
}


//////////////////////////////////////////////////////////////////////
// ChkPass(CORRECT, TEST)
//
// Description:
//  Compares a string to a secret password.
//
// Parameters:
//  CORRECT - The secret password (encrypted).
//  TEST    - A user-entered value to check against the password.
//
// Return Value:
//  The function returns true if the passwords match and false
//  otherwise.
//////////////////////////////////////////////////////////////////////
bool
ChkPass(std::string CORRECT, std::string TEST)
{
  if (CORRECT == "*")
    return false;
  if ((CORRECT == "") && (TEST == ""))
    return true;
#ifdef USE_CRYPT
  if (CORRECT == crypt(TEST.c_str(), CORRECT.c_str()))
    return true;
#else
  return (CORRECT == TEST);
#endif
  return false;
}


//////////////////////////////////////////////////////////////////////
// timeStamp(format)
//
// Description:
//  Produces a string containing a textual representation of the
//  current date and time.
//
// Parameters:
//  format - Either TIMESTAMP_KLINE or TIMESTAMP_LOG.
//
// Return Value:
//  The function returns a string containing a textual representation
//  of the current date and time.
//////////////////////////////////////////////////////////////////////
std::string
timeStamp(const TimeStampFormat format)
{
  char ts_holder[MAX_BUFF];
  struct tm *time_val;
  time_t t;
  
  t = time(NULL);
  time_val = localtime(&t);

  switch (format)
  {
    case TIMESTAMP_KLINE:
#ifdef HAVE_SNPRINTF
      snprintf(ts_holder, sizeof(ts_holder),
#else
      sprintf(ts_holder,
#endif
	"%d/%d/%d %2.2d.%2.2d", time_val->tm_year + 1900, time_val->tm_mon + 1,
	time_val->tm_mday, time_val->tm_hour, time_val->tm_min);
	break;
    case TIMESTAMP_LOG:
    default:
#ifdef HAVE_SNPRINTF
      snprintf(ts_holder, sizeof(ts_holder),
#else
      sprintf(ts_holder,
#endif
	"%d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d", time_val->tm_year + 1900,
	time_val->tm_mon + 1, time_val->tm_mday, time_val->tm_hour,
	time_val->tm_min, time_val->tm_sec);
      break;
  }

  return std::string(ts_holder);
}


//////////////////////////////////////////////////////////////////////
// IntToStr(i, len)
//
// Description:
//  Converts an int to a string.
//
// Parameters:
//  i   - The int to convert.
//  len - The desired width of the resulting string.
//
// Return Value:
//  The function returns string containing the decimal representation
//  of the int, right-aligned with spaces to fill the desired width if
//  necessary.
//////////////////////////////////////////////////////////////////////
std::string
IntToStr(int i, std::string::size_type len)
{
  if (i < 0)
  {
    return std::string("-") + IntToStr(-i, (len > 1) ? (len - 1) : 0);
  }
  else
  {
    std::string result;

    if (i == 0)
    {
      result = "0";
    }
    else
    {
      while (i > 0)
      {
        result.insert(result.begin(), 1,
	  (std::string::value_type)((i % 10) + '0'));
        i /= 10;
      }
    }

    std::string::size_type l = result.length();
    if (l < len)
    {
      result.insert(result.begin(), len - l, ' ');
    }
    return result;
  }
}


//////////////////////////////////////////////////////////////////////
// ULongToStr(i, len)
//
// Description:
//  Converts an unsigned long to a string.
//
// Parameters:
//  i   - The unsigned long to convert.
//  len - The desired width of the resulting string.
//
// Return Value:
//  The function returns string containing the decimal representation
//  of the unsigned long, right-aligned with spaces to fill the
//  desired width if necessary.
//////////////////////////////////////////////////////////////////////
std::string
ULongToStr(unsigned long i, std::string::size_type len)
{
  std::string result;

  if (i == 0)
  {
    result = "0";
  }
  else
  {
    while (i != 0)
    {
      result.insert(result.begin(), 1,
	(std::string::value_type)((i % 10) + '0'));
      i /= 10;
    }
  }

  std::string::size_type l = result.length();
  if (l < len)
  {
    result.insert(result.begin(), len - l, ' ');
  }
  return result;
}


//////////////////////////////////////////////////////////////////////
// GetHandleHB(text)
//
// Description:
//  Identifies the botname in a handle@botname string.
//
// Parameters:
//  text - A string containing a handle@botname.
//
// Return Value:
//  The function returns the botname portion of the handle@botname.
//////////////////////////////////////////////////////////////////////
std::string
GetBotHB(std::string text)
{
  if (text == "*")
    return text;

  std::string::size_type i = text.find('@');
  if (i != std::string::npos)
    return text.substr(i + 1);
  else
    return "";
}


//////////////////////////////////////////////////////////////////////
// GetHandleHB(text)
//
// Description:
//  Identifies the handle in a handle@botname string.
//
// Parameters:
//  text - A string containing a handle@botname.
//
// Return Value:
//  The function returns the handle portion of the handle@botname.
//////////////////////////////////////////////////////////////////////
std::string
GetHandleHB(std::string text)
{
  if (text == "*")
    return text;

  std::string::size_type i = text.find('@');
  if (i != std::string::npos)
    return text.substr(0, i);
  else
    return "";
}


//////////////////////////////////////////////////////////////////////
// CountChars(text, ch)
//
// Description:
//  Counts how many of a particular character exist in a string.
//
// Parameters:
//  text - The string to scan.
//  ch   - The character to count.
//
// Return Value:
//  The function returns the number of characters counted.
//////////////////////////////////////////////////////////////////////
std::string::size_type
CountChars(const std::string & text, const std::string::value_type ch)
{
  std::string::size_type length = text.length();
  std::string::size_type count = 0;

  for (std::string::size_type index = 0; index < length; index++)
  {
    if (text[index] == ch)
    {
      count++;
    }
  }

  return count;
}


//////////////////////////////////////////////////////////////////////
// isIP(host)
//
// Description:
//  Determines if a hostname is an IP address.  The checking is fairly
//  ignorant about correctness of an IP address.  It merely checks if
//  the string contains 3 dots and the remaining characters are all
//  numeric.  So, the following are all considered IPs by this
//  function:
//    1.2.3.4
//    999999999.2.3.4
//    1...2
//  The probably ought to be changed so that only valid IP addresses
//  are identified.
//
// Parameters:
//  host - A string containing a hostname or IP address.
//
// Return Value:
//  The function returns true if host contains an IP address or false
//  if it contains a hostname.
//////////////////////////////////////////////////////////////////////
bool
isIP(const std::string & host)
{
  return ((std::string::npos == host.find_first_not_of(".0123456789")) &&
    (3 == CountChars(host, '.')));
}


//////////////////////////////////////////////////////////////////////
// getDomain(host, withDot)
//
// Description:
//  Identifies the domain (or subnet) of a hostname (or IP address).
//
// Parameters:
//  host    - A string containing a hostname or IP address.
//  withDot - If true, the resulting domain will include an extra dot.
//
// Return Value:
//  The function returns a string containing a domain or subnet.
//////////////////////////////////////////////////////////////////////
std::string
getDomain(std::string host, bool withDot)
{
  if (isIP(host))
  {
    // 1.2.3.4
    std::string::size_type lastDot = host.rfind('.');
    if (std::string::npos != lastDot)
    {
      return (host.substr(0, lastDot + (withDot ? 1 : 0)));
    }
    else
    {
      // This should never happen
      return host;
    }
  }
  else
  {
    std::string::size_type lastDot = host.rfind('.');
    if (std::string::npos == lastDot)
    {
      // domain
      return host;
    }
    else
    {
      std::string::size_type prevDot = host.rfind('.', lastDot - 1);
      if (std::string::npos == prevDot)
      {
        // domain.tld
        return host;
      }
      else
      {
        // *.domain.tld
        std::string tld = host.substr(lastDot + 1, std::string::npos);
        if (tld.length() == 2)
        {
          // *.domain.xx
          std::string::size_type triDot = host.rfind('.', prevDot - 1);
          if (std::string::npos == triDot)
          {
            // user@foo.domain.xx
            if (std::string::npos == host.substr(0,
              prevDot).find_first_of("0123456789"))
            {
              // alpha.domain.xx
	      return (host);
	    }
	    else
	    {
              // al55pha.domain.xx
	      return (host.substr(prevDot + (withDot ? 0 : 1),
		std::string::npos));
	    }
	  }
	  else
	  {
	    // *.foo.domain.xx
	    if (std::string::npos == host.substr(triDot + 1,
              prevDot - triDot - 1).find_first_of("0123456789"))
            {
              // *.alpha.domain.xx
	      return (host.substr(triDot + (withDot ? 0 : 1),
		std::string::npos));
	    }
	    else
	    {
              // *.al55pha.domain.xx
	      return (host.substr(prevDot + (withDot ? 0 : 1),
		std::string::npos));
	    }
	  }
	}
	else
	{
	  // *.domain.tld
	  return (host.substr(prevDot + (withDot ? 0 : 1), std::string::npos));
	}
      }
    }
  }
}


//////////////////////////////////////////////////////////////////////
// klineMask(userhost)
//
// Description:
//  Generates a suitable k-line mask for a particular user@host.
//
// Parameters:
//  userhost - A string containing a user@host.
//
// Return Value:
//  The function returns a k-line mask that matches the passed
//  user@host.
//////////////////////////////////////////////////////////////////////
std::string klineMask(const std::string & userhost)
{
  std::string::size_type at = userhost.find('@');
  if (at != std::string::npos)
  {
    std::string user = userhost.substr(0, at);
    std::string host = userhost.substr(at + 1, std::string::npos);

    if ((user.length() > 0) && (user[0] == '~'))
    {
      user = "~*";
    }
    else if (user.length() > 8)
    {
      user = "*" + user.substr(user.length() - 8);
    }
    else
    {
      user = "*" + user;
    }

    std::string domain = getDomain(host, true);

    if (isIP(host))
    {
      return (user + "@" + domain + "*");
    }
    else
    {
      return (user + "@*" + domain);
    }
  }
  else
  {
    return userhost;
  }
}


//////////////////////////////////////////////////////////////////////
// classCMask(ip)
//
// Description:
//  Creates a class-C style mask of an IP address.
//
// Parameters:
//  ip - A string representation of an IP address.
//
// Return Value:
//  The function returns a string containing a class-C mask of the IP
//  address.
//////////////////////////////////////////////////////////////////////
std::string classCMask(const std::string & ip)
{
  std::string::size_type lastDot = ip.rfind('.');

  return (ip.substr(0, lastDot) + ".*");
}


//////////////////////////////////////////////////////////////////////
// StrJoin(token, items)
//
// Description:
//  Converts a vector of strings to a single string, where each node
//  of the vector is separated by a token character.  This function
//  works similarly to Perl's join() function.
//
// Parameters:
//  token - A character to separate each node of the vector.
//  items - A vector of strings.
//
// Return Value:
//  The resulting token-separated string.
//////////////////////////////////////////////////////////////////////
std::string
StrJoin(char token, const StrVector & items)
{
  std::string result;

  for (StrVector::size_type index = 0; index < items.size(); index++)
  {
    if (index == 0)
      result = items[index];
    else
      result += " " + items[index];
  }

  return result;
}


//////////////////////////////////////////////////////////////////////
// isNumeric(text)
//
// Description:
//  Determines whether a string contains only numeric characters.
//
// Parameters:
//  text - A string to check.
//
// Return Value:
//  The function returns true if the string contains only numeric
//  characters or false otherwise.
//////////////////////////////////////////////////////////////////////
bool
isNumeric(const std::string & text)
{
  for (std::string::size_type pos = 0; pos < text.length(); ++pos)
  {
    if (!isdigit(text[pos]))
    {
      return false;
    }
  }
  return true;
}


//////////////////////////////////////////////////////////////////////
// isDynamic(user, host)
//
// Description:
//  Given a user and hostname, this function will determine whether
//  the client is connecting from a dynamic host.  For now, this uses
//  a VERY simple algorithm.  If the client is not running identd or
//  the hostname contains numeric characters, it is considered
//  dynamic.  This works about 90% of the time.  There are problems,
//  however, as AOL dialup hostnames use the form foo.ipt.aol.com,
//  where "foo" is a hexadecimal representation of the IP address.
//  Since hex includes alpha characters, an AOL dialup hostname can
//  be completely without numeric characters (I've seen it).
//
//  Eventually, I'll rewrite this so that users can specify patterns
//  to match against that determine whether a hostname is dynamic.
//
// Parameters:
//  user - a client's username
//  host - a client's hostname
//
// Return value:
//  true - the client is connecting from a dynamic host
//  false - the client is NOT connecting from a dynamic host
//////////////////////////////////////////////////////////////////////
bool
isDynamic(const std::string & user, const std::string & host)
{
  bool result = false;

  if (((user.length() > 0) && (user[0] == '~')) ||
    (std::string::npos != host.find_first_of("0123456789")))
  {
    result = true;
  }

  return result;
}


//////////////////////////////////////////////////////////////////////
// timeDiff(diff)
//
// Description:
//  Convert a time difference to a textual representation of weeks,
//  days, hours, minutes, and seconds.
//
// Parameters:
//  diff - A time difference.
//
// Return Value:
//  A string containing the textual representation.
//////////////////////////////////////////////////////////////////////
std::string
timeDiff(time_t diff)
{
  if (diff == 0)
 {
    	return "none";
  }

  std::string result;
  int amount;

  // Seconds
  amount = diff % 60;
  if (amount != 0)
  {
    result = IntToStr(amount) + "s";
  }
  diff /= 60;

  // Minutes
  amount = diff % 60;
  if (amount != 0)
  {
    result = IntToStr(amount) + "m" + result;
  }
  diff /= 60;

  // Hours
  amount = diff % 24;
  if (amount != 0)
  {
    result = IntToStr(amount) + "h" + result;
  }
  diff /= 24;

  // Days
  amount = diff % 7;
  if (amount != 0)
  {
    result = IntToStr(amount) + "d" + result;
  }
  diff /= 7;

  // Weeks
  if (diff != 0)
  {
    result = IntToStr(diff) + "w" + result;
  }

  return result;
}


//////////////////////////////////////////////////////////////////////
// trimLeft(text)
//
// Description:
//  Removes the whitespace from the beginning of a string.
//
// Parameters:
//  text - Any string.
//
// Return Value:
//  The function returns the string with its leading whitespace
//  removed.
//////////////////////////////////////////////////////////////////////
std::string
trimLeft(const std::string & text)
{
  std::string::size_type pos = text.find_first_not_of(" \t");

  if (pos == std::string::npos)
  {
    return "";
  }

  return text.substr(pos);
}


//////////////////////////////////////////////////////////////////////
// hexDump(buffer, size)
//
// Description:
//  Converts an array of bytes to their hexadecimal representations.
//
// Parameters:
//  buffer - A pointer to the start of the array.
//  size   - The size, in bytes, of the array.
//
// Return Value:
//  The function returns a string containing a space-separated list
//  of hexadecimal values suitable for printing, eating, etc.  May
//  cause drowsiness.
//////////////////////////////////////////////////////////////////////
std::string
hexDump(const void *buffer, const int size)
{
  const unsigned char *copy = reinterpret_cast<const unsigned char *>(buffer);
  std::string result;

  if (copy != NULL)
  {
    for (int i = 0; i < size; ++i)
    {
      char *nibble = "0123456789ABCDEF";

      if (result.length() > 0)
      {
        result += " ";
      }
      result += nibble[copy[i] >> 4];
      result += nibble[copy[i] & 15];
    }
  }

  return result;
}


//////////////////////////////////////////////////////////////////////
// chopNick(NUH)
//
// Description:
//  Takes the nick out of a nick[user@host].  Unfortunately, this
//  isn't as easy to do as you might expect because both the nickname
//  and username can contain '[' characters.  Therefore, we do a
//  little guesswork to figure out where the nickname ends and the
//  username begins.
//
// Parameters:
//  NUH - A string containing a nickname, username, and hostname in
//        the format, "nick[user@host]"
//
// Return value:
//  The function returns the nickname portion of the string.
//////////////////////////////////////////////////////////////////////
std::string
chopNick(std::string NUH)
{
  std::string result;

  // How many '[' characters are in the string?
  if (CountChars(NUH, '[') > 1)
  {
    // More than one '[' character.  Yuck.  Let's do some guesswork.

    // Is there a "[~" sequence?  That's usually a pretty good indicator.
    std::string::size_type pos = NUH.find("[~");

    if (pos != std::string::npos)
    {
      result = NUH.substr(0, pos);
    }
    else
    {
      // No luck.  Okay, assume all of the '[' characters are in the
      // nickname.  Don't expect this to be correct every time.
      int nicklen = 0;

      while (nicklen < 9)
      {
        if ((NUH[nicklen] == '[') &&
	  (CountChars(NUH.substr(nicklen), '[') == 1))
        {
	  break;
        }
        nicklen++;
      }
      result = NUH.substr(0, nicklen);
    }
  }
  else
  {
    // Only 1 '[' character and that must be the nickname/username divider.
    std::string::size_type pos = NUH.find('[');

    if (pos != std::string::npos)
    {
      result = NUH.substr(0, pos);
    }
  }
  return result;
}


//////////////////////////////////////////////////////////////////////
// chopUserHost(NUH)
//
// Description:
//  Takes the user@host out of a nick[user@host].  Unfortunately, this
//  isn't as easy to do as you might expect because both the nickname
//  and username can contain '[' characters.  Therefore, we do a
//  little guesswork to figure out where the nickname ends and the
//  username begins.
//
// Parameters:
//  NUH - A string containing a nickname, username, and hostname in
//        the format, "nick[user@host]"
//
// Return value:
//  The function returns the user@host portion of the string.
//////////////////////////////////////////////////////////////////////
std::string
chopUserhost(std::string NUH)
{
  if (NUH != "")
  {
    std::string::size_type pos = NUH.find('[');
    if (pos != std::string::npos)
    {
      if (CountChars(NUH, '[') > 1)
      {
	// Moron has '[' characters in his nick or username
	// Look for '~' to mark the start of an non-ident'ed
	// username...
	std::string::size_type tilde = NUH.find("[~");
	if (tilde != std::string::npos)
	{
	  NUH = NUH.substr(tilde + 1);
	}
	else
	{
	  // Blah. Loser is running identd with a screwy
	  // username or nick.
	  // Find the last '[' in the string and assume it is
	  // the divider, unless this produces an illegal
	  // length nick :P
	  int nicklen = 0;
	  while ((nicklen <= 9) && ((pos = NUH.find('[')) != std::string::npos))
	  {
	    NUH = NUH.substr(1);
	    nicklen++;
	  }
	}
      }
      else
      {
        NUH = NUH.substr(pos + 1);
      }
      // Remove the trailing ']'
      NUH.erase(NUH.length() - 1, 1);
      return NUH;
    }
  }
  return "";
}


//////////////////////////////////////////////////////////////////////
// expandPath(filename, cwd)
//
// Description:
//  This function determines whether a filename contains an absolute
//  path or if it is located relative to a working directory.
//
// Parameters:
//  filename - a filename
//  cwd      - the current working directory
//
// Return value:
//  The function returns the full pathname of the desired file.
//////////////////////////////////////////////////////////////////////
std::string
expandPath(const std::string & filename, const std::string & cwd)
{
  std::string result;

  if ((filename.length() > 0) && (filename[0] != '/'))
  {
    result = cwd + '/' + filename;
  }
  else
  {
    result = filename;
  }

  return result;
}

