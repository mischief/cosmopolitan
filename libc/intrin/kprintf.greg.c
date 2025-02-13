/*-*- mode:c;indent-tabs-mode:nil;c-basic-offset:2;tab-width:8;coding:utf-8 -*-│
│vi: set net ft=c ts=2 sts=2 sw=2 fenc=utf-8                                :vi│
╞══════════════════════════════════════════════════════════════════════════════╡
│ Copyright 2021 Justine Alexandra Roberts Tunney                              │
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
#define ShouldUseMsabiAttribute() 1
#include "libc/bits/bits.h"
#include "libc/bits/likely.h"
#include "libc/bits/safemacros.internal.h"
#include "libc/bits/weaken.h"
#include "libc/calls/calls.h"
#include "libc/calls/internal.h"
#include "libc/dce.h"
#include "libc/errno.h"
#include "libc/fmt/divmod10.internal.h"
#include "libc/fmt/fmt.h"
#include "libc/intrin/kprintf.h"
#include "libc/limits.h"
#include "libc/macros.internal.h"
#include "libc/nexgen32e/rdtsc.h"
#include "libc/nexgen32e/uart.internal.h"
#include "libc/nt/process.h"
#include "libc/nt/runtime.h"
#include "libc/nt/thunk/msabi.h"
#include "libc/runtime/memtrack.internal.h"
#include "libc/runtime/runtime.h"
#include "libc/str/str.h"
#include "libc/str/tpenc.h"
#include "libc/str/utf16.h"
#include "libc/sysv/consts/nr.h"
#include "libc/sysv/consts/prot.h"

#define MAXT (24 * 60 * 60 * 1000000000ull)
#define WRAP ((MAXT + 1) / 10 * 33)

struct Timestamps {
  unsigned long long birth;
  unsigned long long start;
};

extern int __pid;
extern bool __replmode;
extern bool __nomultics;
volatile unsigned long long __kbirth;

privileged static struct Timestamps kenter(void) {
  struct Timestamps ts;
  ts.start = rdtsc();
  ts.birth = __kbirth;
  if (!ts.birth) {
    ts.birth = kStartTsc;
    if (!ts.birth) ts.birth = 1;
    cmpxchg(&__kbirth, 0, ts.birth);
  }
  return ts;
}

privileged static void kleave(struct Timestamps ts) {
  uint64_t finish, elapse, adjust;
  finish = rdtsc();
  elapse = unsignedsubtract(finish, ts.start);
  adjust = ts.birth + elapse;
  if (!adjust) adjust = 1;
  cmpxchg(&__kbirth, ts.birth, adjust); /* ignore overlapping time intervals */
}

privileged static inline char *kadvance(char *p, char *e, long n) {
  intptr_t t = (intptr_t)p;
  if (__builtin_add_overflow(t, n, &t)) t = (intptr_t)e;
  return (char *)t;
}

privileged static char *kemitquote(char *p, char *e, signed char t,
                                   unsigned c) {
  if (t) {
    if (p < e) {
      *p = t < 0 ? 'u' : 'L';
    }
    ++p;
  }
  if (p < e) {
    *p = c;
  }
  ++p;
  return p;
}

privileged static unsigned long long kgetint(va_list va, signed char t,
                                             bool s) {
#ifdef __LP64__
  int bits;
  unsigned long long x;
  x = va_arg(va, unsigned long);
  if (t <= 0) {
    bits = 64 - (32 >> MIN(5, -t));
    x <<= bits;
    if (s) {
      x = (signed long)x >> bits;
    } else {
      x >>= bits;
    }
  }
  return x;
#else
  if (s) {
    switch (t) {
      case -2:
        return (signed char)va_arg(va, int);
      case -1:
        return (signed short)va_arg(va, int);
      default:
        return va_arg(va, signed int);
      case 1:
        return va_arg(va, signed long);
      case 2:
        return va_arg(va, signed long long);
    }
  } else {
    switch (t) {
      case -2:
        return (unsigned char)va_arg(va, int);
      case -1:
        return (unsigned short)va_arg(va, int);
      default:
        return va_arg(va, unsigned int);
      case 1:
        return va_arg(va, unsigned long);
      case 2:
        return va_arg(va, unsigned long long);
    }
  }
#endif
}

privileged static inline bool kiskernelpointer(const void *p) {
  return 0x7f0000000000 <= (intptr_t)p && (intptr_t)p < 0x800000000000;
}

privileged static inline bool kistextpointer(const void *p) {
  return _base <= (const unsigned char *)p && (const unsigned char *)p < _etext;
}

privileged static inline bool kisimagepointer(const void *p) {
  return _base <= (const unsigned char *)p && (const unsigned char *)p < _end;
}

privileged static inline bool kischarmisaligned(const char *p, signed char t) {
  if (t == -1) return (intptr_t)p & 1;
  if (t >= 1) return !!((intptr_t)p & 3);
  return false;
}

privileged static inline bool kismemtrackhosed(void) {
  return !((weaken(_mmi)->i <= weaken(_mmi)->n) &&
           (weaken(_mmi)->p == weaken(_mmi)->s ||
            weaken(_mmi)->p == (struct MemoryInterval *)kMemtrackStart));
}

privileged static bool kismapped(int x) {
  size_t m, r, l = 0;
  if (!weaken(_mmi)) return true;
  if (kismemtrackhosed()) return false;
  r = weaken(_mmi)->i;
  while (l < r) {
    m = (l + r) >> 1;
    if (weaken(_mmi)->p[m].y < x) {
      l = m + 1;
    } else {
      r = m;
    }
  }
  if (l < weaken(_mmi)->i && x >= weaken(_mmi)->p[l].x) {
    return !!(weaken(_mmi)->p[l].prot & PROT_READ);
  } else {
    return false;
  }
}

privileged bool kisdangerous(const void *p) {
  int frame;
  if (kisimagepointer(p)) return false;
  if (kiskernelpointer(p)) return false;
  if (IsLegalPointer(p)) {
    frame = (intptr_t)p >> 16;
    if (IsStackFrame(frame)) return false;
    if (IsOldStackFrame(frame)) return false;
    if (kismapped(frame)) return false;
  }
  return true;
}

privileged static void klog(const char *b, size_t n) {
  int e;
  size_t i;
  uint16_t dx;
  uint32_t wrote;
  unsigned char al;
  long rax, rdi, rsi, rdx;
  if (IsWindows()) {
    e = __imp_GetLastError();
    __imp_WriteFile(__imp_GetStdHandle(kNtStdErrorHandle), b, n, &wrote, 0);
    __imp_SetLastError(e);
  } else if (IsMetal()) {
    for (i = 0; i < n; ++i) {
      for (;;) {
        dx = 0x3F8 + UART_LSR;
        asm("inb\t%1,%0" : "=a"(al) : "dN"(dx));
        if (al & UART_TTYTXR) break;
        asm("pause");
      }
      dx = 0x3F8;
      asm volatile("outb\t%0,%1"
                   : /* no inputs */
                   : "a"(b[i]), "dN"(dx));
    }
  } else {
    asm volatile("syscall"
                 : "=a"(rax), "=D"(rdi), "=S"(rsi), "=d"(rdx)
                 : "0"(__NR_write), "1"(2), "2"(b), "3"(n)
                 : "rcx", "r8", "r9", "r10", "r11", "memory", "cc");
  }
}

privileged static size_t kformat(char *b, size_t n, const char *fmt, va_list va,
                                 struct Timestamps ts) {
  int si;
  wint_t t, u;
  const char *abet;
  signed char type;
  const char *s, *f;
  unsigned long long x;
  unsigned i, j, m, rem, sign, hash, cols, prec;
  char c, *p, *e, pdot, zero, flip, dang, base, quot, z[128];
  if (kistextpointer(b) || kisdangerous(b)) n = 0;
  if (!kistextpointer(fmt)) fmt = "!!WONTFMT";
  p = b;
  f = fmt;
  e = p + n;
  for (;;) {
    for (;;) {
      if (!(c = *f++) || c == '%') break;
    EmitFormatByte:
      if (p < e) *p = c;
      ++p;
    }
    if (!c) break;
    pdot = 0;
    flip = 0;
    dang = 0;
    hash = 0;
    sign = 0;
    prec = 0;
    quot = 0;
    type = 0;
    cols = 0;
    zero = 0;
    abet = "0123456789abcdef";
    for (;;) {
      switch ((c = *f++)) {
        default:
          goto EmitFormatByte;
        case '\0':
          break;
        case '.':
          pdot = 1;
          continue;
        case '-':
          flip = 1;
          continue;
        case '#':
          hash = '0';
          continue;
        case '_':
        case ',':
        case '\'':
          quot = c;
          continue;
        case ' ':
        case '+':
          sign = c;
          continue;
        case 'h':
          --type;
          continue;
        case 'j':
        case 'l':
        case 'z':
          ++type;
          continue;
        case '!':
          dang = 1;
          continue;
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
          si = pdot ? prec : cols;
          si *= 10;
          si += c - '0';
          goto UpdateCols;
        case '*':
          si = va_arg(va, int);
        UpdateCols:
          if (pdot) {
            if (si >= 0) {
              prec = si;
            }
          } else {
            if (si < 0) {
              flip = 1;
              si = -si;
            }
            cols = si;
            if (!cols) {
              zero = 1;
            }
          }
          continue;
        case 'T':
          x = unsignedsubtract(ts.start, ts.birth) % WRAP * 10 / 33;
          goto FormatUnsigned;
        case 'P':
          if (!__vforked) {
            x = __pid;
          } else {
            asm volatile("syscall"
                         : "=a"(x)
                         : "0"(__NR_getpid)
                         : "rcx", "rdx", "r11", "memory", "cc");
          }
          goto FormatUnsigned;
        case 'u':
        case 'd':
          if (UNLIKELY(type <= -3)) {
            s = va_arg(va, int) ? "true" : "false";
            goto FormatString;
          }
          x = kgetint(va, type, c == 'd');
        FormatDecimal:
          if ((long long)x < 0 && c != 'u') {
            x = -x;
            sign = '-';
          }
        FormatUnsigned:
          if (x && hash) sign = hash;
          for (i = j = 0;;) {
            x = DivMod10(x, &rem);
            z[i++ & 127] = '0' + rem;
            if (pdot ? i >= prec : !x) break;
            if (quot && ++j == 3) {
              z[i++ & 127] = quot;
              j = 0;
            }
          }
        EmitNumber:
          if (flip || pdot) zero = 0;
          while (zero && sign) {
            if (p < e) *p = sign;
            if (cols) --cols;
            sign >>= 8;
            ++p;
          }
          t = !!sign + !!(sign >> 8);
          if (!flip && cols >= t) {
            for (j = i; j < cols - t; ++j) {
              if (p < e) {
                *p++ = zero ? '0' : ' ';
              } else {
                p = kadvance(p, e, cols - t - j);
                break;
              }
            }
          }
          while (sign) {
            if (p < e) *p = sign;
            sign >>= 8;
            ++p;
          }
          for (j = i; j; ++p) {
            --j;
            if (p < e) {
              *p = z[j & 127];
            }
          }
          if (flip && cols >= t) {
            for (j = i; j < cols - t; ++j) {
              if (p < e) {
                *p++ = ' ';
              } else {
                p = kadvance(p, e, cols - t - j);
                break;
              }
            }
          }
          break;
        case 'b':
          base = 1;
          if (hash) hash = '0' | 'b' << 8;
        BinaryNumber:
          x = kgetint(va, type, false);
        FormatNumber:
          i = 0;
          m = (1 << base) - 1;
          if (hash && x) sign = hash;
          do z[i++ & 127] = abet[x & m];
          while ((x >>= base) || (pdot && i < prec));
          goto EmitNumber;
        case 'X':
          abet = "0123456789ABCDEF";
          /* fallthrough */
        case 'x':
          base = 4;
          if (hash) hash = '0' | 'x' << 8;
          goto BinaryNumber;
        case 'o':
          base = 3;
          goto BinaryNumber;
        case 'p':
          x = va_arg(va, intptr_t);
          if (!x && pdot) pdot = 0;
          if ((long)x == -1) {
            pdot = 0;
            goto FormatDecimal;
          }
          hash = '0' | 'x' << 8;
          base = 4;
          goto FormatNumber;
        case 'C':
          c = 'c';
          type = 1;
          /* fallthrough */
        case 'c':
          i = 1;
          j = 0;
          x = 0;
          s = (const char *)&x;
          t = va_arg(va, int);
          if (!type) t &= 255;
          if (hash) {
            quot = 1;
            hash = '\'';
            p = kemitquote(p, e, type, hash);
            if (cols && type) --cols; /* u/L */
            if (cols) --cols;         /* start quote */
            if (cols) --cols;         /* end quote */
          }
          goto EmitChar;
        case 'm':
          if (!(x = errno) && sign == ' ' &&
              (!IsWindows() || !__imp_GetLastError())) {
            break;
          } else if (weaken(strerror_r) &&
                     !weaken(strerror_r)(x, z, sizeof(z))) {
            s = z;
            goto FormatString;
          } else {
            goto FormatDecimal;
          }
        case 'n':
          if (__nomultics) {
            if (p < e) *p = '\r';
            ++p;
          }
          if (p < e) *p = '\n';
          ++p;
          break;
        case 'r':
          if (!__replmode) {
            break;
          } else {
            s = "\r\033[K";
            goto FormatString;
          }
        case 'S':
          c = 's';
          type = 1;
          /* fallthrough */
        case 's':
          if (!(s = va_arg(va, const void *))) {
            s = sign != ' ' ? "NULL" : "";
          FormatString:
            type = 0;
            hash = 0;
          } else if (!dang && (kisdangerous(s) || kischarmisaligned(s, type))) {
            if (sign == ' ') {
              if (p < e) *p = ' ';
              ++p;
            }
            x = (intptr_t)s;
            base = 4;
            hash = '!' | '!' << 8;
            goto FormatNumber;
          } else if (hash) {
            quot = 1;
            hash = '"';
            if (cols && type) --cols; /* u/L */
            if (cols) --cols;         /* start quote */
            if (cols) --cols;         /* end quote */
            p = kemitquote(p, e, type, hash);
          }
          if (sign == ' ' && (!pdot || prec) && *s) {
            if (p < e) *p = ' ';
            ++p;
          }
          for (i = j = 0; !pdot || j < prec; ++j) {
            if (UNLIKELY(!((intptr_t)s & (PAGESIZE - 1)))) {
              if (!dang && kisdangerous(s)) break;
            }
            if (!type) {
              if (!(t = *s++ & 255)) break;
              if ((t & 0300) == 0200) goto ActuallyEmitByte;
              ++i;
            EmitByte:
              if (UNLIKELY(quot) && (t == '\\' || ((t == '"' && c == 's') ||
                                                   (t == '\'' && c == 'c')))) {
                if (p + 2 <= e) {
                  p[0] = '\\';
                  p[1] = t;
                }
                p += 2;
                i += 1;
                continue;
              }
              if (pdot ||
                  (t != 0x7F && (t >= 0x20 || (t == '\n' || t == '\t' ||
                                               t == '\r' || t == '\e')))) {
              ActuallyEmitByte:
                if (p < e) *p = t;
                p += 1;
                continue;
              } else if (quot) {
                if (p + 4 <= e) {
                  p[0] = '\\';
                  p[1] = '0' + ((t & 0300) >> 6);
                  p[2] = '0' + ((t & 0070) >> 3);
                  p[3] = '0' + ((t & 0007) >> 0);
                }
                p += 4;
                i += 3;
                continue;
              } else {
                /* Control Pictures
                ═══════════════════════════════════════════════════════
                2400 │ 0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f
                ───────────────────────────────────────────────────────
                2400 │ ␀  ␁  ␂  ␃  ␄  ␅  ␆  ␇  ␈  ␉  ␊  ␋  ␌  ␍  ␎  ␏
                2410 │ ␐  ␑  ␒  ␓  ␔  ␕  ␖  ␗  ␘  ␙  ␚  ␛  ␜  ␝  ␞  ␟
                2420 │ ␠  ␡  ␢  ␣  ␤  ␥  ␦  */
                if (t != 0x7F) {
                  t += 0x2400;
                } else {
                  t = 0x2421;
                }
                goto EmitChar;
              }
            } else if (type < -1) {
              if ((t = *s++ & 255) || prec) {
                t = kCp437[t];
              }
            } else if (type < 0) {
              t = *(const char16_t *)s;
              s += sizeof(char16_t);
              if (IsHighSurrogate(t)) {
                if (!pdot || j + 1 < prec) {
                  if (UNLIKELY(!((intptr_t)s & (PAGESIZE - 1)))) {
                    if (!dang && kisdangerous(s)) break;
                  }
                  u = *(const char16_t *)s;
                  if (IsLowSurrogate(u)) {
                    t = MergeUtf16(t, u);
                    s += sizeof(char16_t);
                    j += 1;
                  }
                } else {
                  break;
                }
              } else if (!t) {
                break;
              }
            } else {
              t = *(const wchar_t *)s;
              s += sizeof(wchar_t);
            }
            if (!t) break;
            ++i;
          EmitChar:
            if (t <= 0x7f) {
              goto EmitByte;
            } else if (t <= 0x7ff) {
              if (p + 2 <= e) {
                p[0] = 0300 | (t >> 6);
                p[1] = 0200 | (t & 077);
              }
              p += 2;
            } else if (t <= 0xffff) {
              if (UNLIKELY(IsSurrogate(t))) {
              EncodeReplacementCharacter:
                t = 0xfffd;
              }
              if (p + 3 <= e) {
                p[0] = 0340 | (t >> 12);
                p[1] = 0200 | ((t >> 6) & 077);
                p[2] = 0200 | (t & 077);
              }
              p += 3;
            } else if (~(t >> 18) & 007) {
              if (p + 4 <= e) {
                p[0] = 0360 | (t >> 18);
                p[1] = 0200 | ((t >> 12) & 077);
                p[2] = 0200 | ((t >> 6) & 077);
                p[3] = 0200 | (t & 077);
              }
              p += 4;
            } else {
              goto EncodeReplacementCharacter;
            }
          }
          if (hash) {
            if (p < e) *p = hash;
            ++p;
          }
          for (; cols > i; --cols) {
            if (p < e) {
              *p++ = ' ';
            } else {
              p = kadvance(p, e, cols - i);
              break;
            }
          }
          break;
      }
      break;
    }
  }
  if (p < e) {
    *p = 0;
  } else if (e > b) {
    u = 0;
    *--e = 0;
    s = "\n...";
    if (!(((f - fmt) >= 2 && f[-2] == '\n') ||
          ((f - fmt) >= 3 && f[-3] == '%' && f[-2] == 'n'))) {
      ++s;
    }
    while ((t = *s++) && e > b) {
      u = *--e;
      *e = t;
    }
    if ((u & 0300) == 0200) {
      while (e > b) {
        u = *--e;
        *e = '.';
        if ((u & 0300) != 0200) {
          break;
        }
      }
    }
  }
  return p - b;
}

/**
 * Privileged snprintf().
 *
 * @param b is buffer, and guaranteed a NUL-terminator if `n>0`
 * @param n is number of bytes available in buffer
 * @return length of output excluding NUL, which may exceed `n`
 * @see kprintf() for documentation
 * @asyncsignalsafe
 * @vforksafe
 */
privileged size_t ksnprintf(char *b, size_t n, const char *fmt, ...) {
  size_t m;
  va_list v;
  struct Timestamps t = {0};
  va_start(v, fmt);
  m = kformat(b, n, fmt, v, t);
  va_end(v);
  return m;
}

/**
 * Privileged vsnprintf().
 *
 * @param b is buffer, and guaranteed a NUL-terminator if `n>0`
 * @param n is number of bytes available in buffer
 * @return length of output excluding NUL, which may exceed `n`
 * @see kprintf() for documentation
 * @asyncsignalsafe
 * @vforksafe
 */
privileged size_t kvsnprintf(char *b, size_t n, const char *fmt, va_list v) {
  struct Timestamps t = {0};
  return kformat(b, n, fmt, v, t);
}

/**
 * Privileged vprintf.
 *
 * @see kprintf() for documentation
 * @asyncsignalsafe
 * @vforksafe
 */
privileged void kvprintf(const char *fmt, va_list v) {
  size_t n;
  char b[2048];
  struct Timestamps t;
  if (!v) return;
  t = kenter();
  n = kformat(b, sizeof(b), fmt, v, t);
  klog(b, MIN(n, sizeof(b)));
  kleave(t);
}

/**
 * Privileged printf().
 *
 * This function is intended for crash reporting. It's designed to be as
 * unbreakable as possible, so that error messages can always be printed
 * even when the rest of the runtime is broken. As such, it has continue
 * on error semantics, doesn't support buffering between invocations and
 * floating point is not supported. Output is also truncated if the line
 * gets too long, but care is taken to preserve your newline characters.
 * Your errno and GetLastError() state will not be clobbered, and ftrace
 * and other runtime magic won't be invoked, since all the runtime magic
 * depends on this function.
 *
 * Directives:
 *
 *     %[FLAGS][WIDTH|*][.[PRECISION|*]][TYPE]SPECIFIER
 *
 * Specifiers:
 *
 * - `P` pid
 * - `c` char
 * - `o` octal
 * - `b` binary
 * - `s` string
 * - `p` pointer
 * - `d` decimal
 * - `n` newline
 * - `u` unsigned
 * - `r` carriage
 * - `m` strerror
 * - `X` uppercase
 * - `T` timestamp
 * - `x` hexadecimal
 *
 * Types:
 *
 * - `hhh` bool
 * - `hh`  char or cp437
 * - `h`   short or char16_t
 * - `l`   long or wchar_t
 * - `ll`  long long
 *
 * Flags:
 *
 * - `0` zero padding
 * - `-` flip alignment
 * - `!` bypass memory safety
 * - `,` thousands grouping w/ comma
 * - `'` thousands grouping w/ apostrophe
 * - `_` thousands grouping w/ underscore
 * - `+` plus leftpad if positive (aligns w/ negatives)
 * - ` ` space leftpad if positive (aligns w/ negatives)
 * - `#` represent value with literal syntax, e.g. 0x, 0b, quotes
 *
 * @asyncsignalsafe
 * @vforksafe
 */
privileged void kprintf(const char *fmt, ...) {
  /* system call support runtime depends on this function */
  /* function tracing runtime depends on this function */
  /* asan runtime depends on this function */
  va_list v;
  va_start(v, fmt);
  kvprintf(fmt, v);
  va_end(v);
}
