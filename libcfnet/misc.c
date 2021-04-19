/*
  Copyright 2021 Northern.tech AS

  This file is part of CFEngine 3 - written and maintained by Northern.tech AS.

  This program is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License as published by the
  Free Software Foundation; version 3.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA

  To the extent this program is licensed as part of the Enterprise
  versions of CFEngine, the applicable Commercial Open Source License
  (COSL) may apply to this file if you as a licensee so wish it. See
  included file COSL.txt.
*/

#include <cfnet.h>

#include <misc_lib.h>
#include <logging.h>                                         /* GetErrorStr */


int cf_closesocket(int sd)
{
    int res;

#ifdef __MINGW32__
    res = closesocket(sd);
    if (res == SOCKET_ERROR)
    {
        Log(LOG_LEVEL_VERBOSE,
            "Failed to close socket (closesocket: %s)",
            GetErrorStrFromCode(WSAGetLastError()));
    }
#else
    res = close(sd);
    if (res == -1)
    {
        Log(LOG_LEVEL_VERBOSE,
            "Failed to close socket (close: %s)",
            GetErrorStr());
    }
#endif

    return res;
}

/* int sockaddr_AddrCompareMasked(const void *sa1, const void *sa2, int cidr_mask) */
/* { */
/*     assert(cidr_mask >= 0 && cidr_mask <= 128); */

/* } */
