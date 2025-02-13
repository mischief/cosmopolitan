/*-*- mode:c;indent-tabs-mode:nil;c-basic-offset:2;tab-width:8;coding:utf-8 -*-│
│vi: set net ft=c ts=2 sts=2 sw=2 fenc=utf-8                                :vi│
╞══════════════════════════════════════════════════════════════════════════════╡
│ Copyright 2020 Justine Alexandra Roberts Tunney                              │
│                                                                              │
│ Permission to use, copy, modify, and/or distribute this software for         │
│ any purpose with or without fee is hereby granted, provided that the         │
│ above copyright notice and this permission notice appear in all copies.      │
│                                                                              │
│ THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL                │
│ WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED                │
│ WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE             │
│ AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL         │
│ DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR        │
│ PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER               │
│ TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR             │
│ PERFORMANCE OF THIS SOFTWARE.                                                │
╚─────────────────────────────────────────────────────────────────────────────*/
#include "libc/assert.h"
#include "libc/calls/internal.h"
#include "libc/calls/strace.internal.h"
#include "libc/dce.h"
#include "libc/intrin/asan.internal.h"
#include "libc/sock/internal.h"
#include "libc/sock/sock.h"
#include "libc/sock/sockdebug.h"
#include "libc/str/str.h"
#include "libc/sysv/errfuns.h"

/**
 * Assigns local address and port number to socket.
 *
 * @param fd is the file descriptor returned by socket()
 * @param addr is usually the binary-encoded ip:port on which to listen
 * @param addrsize is the byte-length of addr's true polymorphic form
 * @return socket file descriptor or -1 w/ errno
 * @error ENETDOWN, EPFNOSUPPORT, etc.
 * @asyncsignalsafe
 */
int bind(int fd, const void *addr, uint32_t addrsize) {
  int rc;
  char addrbuf[72];
  if (!addr || (IsAsan() && !__asan_is_valid(addr, addrsize))) {
    rc = efault();
  } else if (addrsize >= sizeof(struct sockaddr_in)) {
    if (!IsWindows()) {
      if (!IsBsd()) {
        rc = sys_bind(fd, addr, addrsize);
      } else {
        char addr2[sizeof(
            struct sockaddr_un_bsd)]; /* sockaddr_un_bsd is the largest */
        assert(addrsize <= sizeof(addr2));
        memcpy(&addr2, addr, addrsize);
        sockaddr2bsd(&addr2[0]);
        rc = sys_bind(fd, &addr2, addrsize);
      }
    } else if (__isfdkind(fd, kFdSocket)) {
      rc = sys_bind_nt(&g_fds.p[fd], addr, addrsize);
    } else {
      rc = ebadf();
    }
  } else {
    rc = einval();
  }
  STRACE("bind(%d, %s) -> %d% m", fd, __describe_sockaddr(addr, addrsize), rc);
  return rc;
}
