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
#include "libc/calls/calls.h"
#include "libc/calls/internal.h"
#include "libc/calls/strace.internal.h"
#include "libc/dce.h"
#include "libc/sysv/errfuns.h"

/**
 * Changes size of file.
 *
 * @param fd must be open for writing
 * @param length may be greater than current current file size, in which
 *      case System V guarantees it'll be zero'd but Windows NT doesn't;
 *      since the prior extends logically and the latter physically
 * @return 0 on success, or -1 w/ errno
 * @asyncsignalsafe
 */
int ftruncate(int fd, int64_t length) {
  int rc;
  if (fd < 0) {
    rc = einval();
  } else if (!IsWindows()) {
    rc = sys_ftruncate(fd, length, length);
  } else {
    if (fd >= g_fds.n) rc = ebadf();
    rc = sys_ftruncate_nt(g_fds.p[fd].handle, length);
  }
  STRACE("ftruncate(%d, %'ld) → %d% m", fd, length, rc);
  return rc;
}
