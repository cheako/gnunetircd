/************************************************************************
 *   IRC - Internet Relay Chat, src/s_user.c
 *   Copyright (C) 1990 Jarkko Oikarinen and
 *                      University of Oulu, Computing Center
 *
 *   See file AUTHORS in IRC package for additional names of
 *   the programmers. 
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 1, or (at your option)
 *   any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *  $Id$
 */

#include "irc_string.h"
#include "config.h"      /* RFC1035_ANAL */
#include "common.h"

#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <assert.h>
#include <time.h>
#include <sys/stat.h>

/*
** m_functions execute protocol messages on this server:
**
**      cptr    is always NON-NULL, pointing to a *LOCAL* client
**              structure (with an open socket connected!). This
**              identifies the physical socket where the message
**              originated (or which caused the m_function to be
**              executed--some m_functions may call others...).
**
**      sptr    is the source of the message, defined by the
**              prefix part of the message if present. If not
**              or prefix not found, then sptr==cptr.
**
**              (!IsServer(cptr)) => (cptr == sptr), because
**              prefixes are taken *only* from servers...
**
**              (IsServer(cptr))
**                      (sptr == cptr) => the message didn't
**                      have the prefix.
**
**                      (sptr != cptr && IsServer(sptr) means
**                      the prefix specified servername. (?)
**
**                      (sptr != cptr && !IsServer(sptr) means
**                      that message originated from a remote
**                      user (not local).
**
**              combining
**
**              (!IsServer(sptr)) means that, sptr can safely
**              taken as defining the target structure of the
**              message in this server.
**
**      *Always* true (if 'parse' and others are working correct):
**
**      1)      sptr->from == cptr  (note: cptr->from == cptr)
**
**      2)      MyConnect(sptr) <=> sptr == cptr (e.g. sptr
**              *cannot* be a local connection, unless it's
**              actually cptr!). [MyConnect(x) should probably
**              be defined as (x == x->from) --msa ]
**
**      parc    number of variable parameter strings (if zero,
**              parv is allowed to be NULL)
**
**      parv    a NULL terminated list of parameter pointers,
**
**                      parv[0], sender (prefix string), if not present
**                              this points to an empty string.
**                      parv[1]...parv[parc-1]
**                              pointers to additional parameters
**                      parv[parc] == NULL, *always*
**
**              note:   it is guaranteed that parv[0]..parv[parc-1] are all
**                      non-NULL pointers.
*/

/*
 * clean_nick_name - ensures that the given parameter (nick) is
 * really a proper string for a nickname (note, the 'nick'
 * may be modified in the process...)
 *
 *      RETURNS the length of the final NICKNAME (0, if
 *      nickname is illegal)
 *
 *  Nickname characters are in range
 *      'A'..'}', '_', '-', '0'..'9'
 *  anything outside the above set will terminate nickname.
 *  In addition, the first character cannot be '-'
 *  or a Digit.
 *
 *  Note:
 *      '~'-character should be allowed, but
 *      a change should be global, some confusion would
 *      result if only few servers allowed it...
 */
int clean_nick_name(char* nick)
{
  char* ch   = nick;
  char* endp = ch + NICKLEN;
  assert(0 != nick);

  if (*nick == '-' || IsDigit(*nick)) /* first character in [0..9-] */
    return 0;
  
  for ( ; ch < endp && *ch; ++ch) {
    if (!IsNickChar(*ch))
      break;
  }
  *ch = '\0';

  return (ch - nick);
}

/* 
 * valid_hostname - check hostname for validity
 *
 * Inputs       - pointer to user
 * Output       - YES if valid, NO if not
 * Side effects - NONE
 *
 * NOTE: this doesn't allow a hostname to begin with a dot and
 * will not allow more dots than chars.
 */
int valid_hostname(const char* hostname)
{
  int         dots  = 0;
  int         chars = 0;
  const char* p     = hostname;

  assert(0 != p);

  if ('.' == *p)
    return NO;

  while (*p) {
    if (!IsHostChar(*p))
      return NO;
    if ('.' == *p++)
      ++dots;
    else
      ++chars;
  }
  return (0 == dots || chars < dots) ? NO : YES;
}

/* 
 * valid_username - check username for validity
 *
 * Inputs       - pointer to user
 * Output       - YES if valid, NO if not
 * Side effects - NONE
 * 
 * Absolutely always reject any '*' '!' '?' '@' '.' in an user name
 * reject any odd control characters names.
 */
int valid_username(const char* username)
{
  const char *p = username;
  assert(0 != p);

  if ('~' == *p)
    ++p;
  /* 
   * reject usernames that don't start with an alphanum
   * i.e. reject jokers who have '-@somehost' or '.@somehost'
   * or "-hi-@somehost", "h-----@somehost" would still be accepted.
   *
   * -Dianora
   */
  if (!IsAlNum(*p))
    return NO;

  while (*++p) {
    if (!IsUserChar(*p))
      return NO;
  }
  return YES;
}

