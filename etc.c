// Делаем "#if defined" на каждую используемую внешнюю функцию, кроме C89 (иногда делаем даже для C89 для единообразия)

//@ #ifndef _SH_ETC_H
//@ #define _SH_ETC_H
//@
//@ #ifdef __cplusplus
//@ extern "C"
//@ {
//@ #endif
//@
//@ #include <stdio.h>
//@
//@ #include "ex.h"
//@ #include "funcs.h"
//@

#include <errno.h>

#include "etc.h"

//@ /// ---- nointr ----

#if defined (SH_HAVE_select) //@
//@ /// Может не вычислять правильно оставшееся время
int //@
sh_select_nointr (int nfds, fd_set *SH_RESTRICT readfds, fd_set *SH_RESTRICT writefds, fd_set *SH_RESTRICT errorfds, struct timeval *SH_RESTRICT timeout)//@;
{
  int result;

  for (;;)
    {
      result = select (nfds, readfds, writefds, errorfds, timeout);

      if (result == -1 && errno == EINTR)
        ;
      else
        {
          break;
        }
    }

  return result;
}

int //@
sh_x_select (int nfds, fd_set *SH_RESTRICT readfds, fd_set *SH_RESTRICT writefds, fd_set *SH_RESTRICT errorfds, struct timeval *SH_RESTRICT timeout)//@;
{
  int result = sh_select_nointr (nfds, readfds, writefds, errorfds, timeout);

  if (result == -1)
    {
      sh_throw ("select");
    }

  return result;
}
#endif //@

#if defined (SH_HAVE_pselect) //@
//@ /// Не вычисляет правильно оставшееся время
int //@
sh_pselect_nointr (int nfds, fd_set *SH_RESTRICT readfds, fd_set *SH_RESTRICT writefds, fd_set *SH_RESTRICT errorfds, const struct timespec *SH_RESTRICT timeout, const sigset_t *SH_RESTRICT sigmask)//@;
{
  int result;

  for (;;)
    {
      result = pselect (nfds, readfds, writefds, errorfds, timeout, sigmask);

      if (result == -1 && errno == EINTR)
        ;
      else
        {
          break;
        }
    }

  return result;
}

int //@
sh_x_pselect (int nfds, fd_set *SH_RESTRICT readfds, fd_set *SH_RESTRICT writefds, fd_set *SH_RESTRICT errorfds, const struct timespec *SH_RESTRICT timeout, const sigset_t *SH_RESTRICT sigmask)//@;
{
  int result = sh_pselect_nointr (nfds, readfds, writefds, errorfds, timeout, sigmask);

  if (result == -1)
    {
      sh_throw ("pselect");
    }

  return result;
}
#endif //@

#if defined (SH_HAVE_poll) //@
//@ /// Не вычисляет правильно оставшееся время
int //@
sh_poll_nointr (struct pollfd fds[], nfds_t nfds, int timeout)//@;
{
  int result;

  for (;;)
    {
      result = poll (fds, nfds, timeout);

      if (result == -1 && errno == EINTR)
        ;
      else
        {
          break;
        }
    }

  return result;
}

int //@
sh_x_poll (struct pollfd fds[], nfds_t nfds, int timeout)//@;
{
  int result = sh_poll_nointr (fds, nfds, timeout);

  if (result == -1)
    {
      sh_throw ("poll");
    }

  return result;
}
#endif //@

#if defined (SH_HAVE_sleep) //@
#include <unistd.h>
void //@
sh_sleep_nointr (unsigned seconds)//@;
{
  while (seconds != 0)
    {
      seconds = sleep (seconds);
    }
}
#endif //@

#if defined (SH_HAVE_nanosleep) //@
//@ /// sh_nanosleep_nointr, sh_x_nanosleep, sh_clock_nanosleep_nointr, sh_x_clock_nanosleep не имеют последнего аргумента, это исключение из правил libsh
int //@
sh_nanosleep_nointr (const struct timespec *rqtp)//@;
{
  struct timespec rqt = *rqtp;
  int result;

  for (;;)
    {
      result = nanosleep (&rqt, &rqt);

      if (result == -1 && errno == EINTR)
        ;
      else
        {
          break;
        }
    }

  return result;
}

void //@
sh_x_nanosleep (const struct timespec *rqtp)//@;
{
  if (sh_nanosleep_nointr (rqtp) == -1)
    {
      sh_throw ("nanosleep");
    }
}
#endif //@

#if defined (SH_HAVE_clock_nanosleep) //@
int //@
sh_clock_nanosleep_nointr (clockid_t clock_id, int flags, const struct timespec *rqtp)//@;
{
  struct timespec rqt = *rqtp;
  int result;

  for (;;)
    {
      result = clock_nanosleep (clock_id, flags, &rqt, &rqt);

      if (result == EINTR)
        ;
      else
        {
          break;
        }
    }

  return result;
}

//@ /// Пишет в errno
void //@
sh_x_clock_nanosleep (clockid_t clock_id, int flags, const struct timespec *rqtp)//@;
{
  int result = sh_clock_nanosleep_nointr (clock_id, flags, rqtp);

  if (result != 0)
    {
      errno = result;
      sh_throw ("clock_nanosleep");
    }
}
#endif //@

//@ /// ---- external ----

//@ #if defined (SH_HAVE_execl) && defined (SH_HAVE_execle) && defined (SH_HAVE_execlp)
//@ # include <unistd.h>
//@ /// В случае успеха все exec-функции не возвращают, path/file вычисляется дважды
//@ # define sh_x_execl(path,  ...) (execl  (path, __VA_ARGS__), sh_throw ("execl: %s",  path))
//@ # define sh_x_execle(path, ...) (execle (path, __VA_ARGS__), sh_throw ("execle: %s", path))
//@ # define sh_x_execlp(file, ...) (execlp (file, __VA_ARGS__), sh_throw ("execlp: %s", file))
//@ #endif
//@

#if defined (SH_HAVE_fcntl) //@
//@ #include <fcntl.h>
int //@
_sh_after_fcntl (int result)//@;
{
  if (result == -1)
    {
      sh_throw ("fcntl");
    }

  return result;
}
//@ # define sh_x_fcntl(fildes, ...) _sh_after_fcntl (fcntl (fildes, __VA_ARGS__))
#endif //@

//@ /// ---- xx without x ----

#include <string.h>
//@ /// x_memchr нет, есть HAVE, то же для strchr
void * //@
sh_xx_memchr (const void *s, int c, size_t n)//@;
{
  void *result = memchr (s, c, n);

  if (result == NULL)
    {
      sh_throwx ("memchr: byte not found");
    }

  return result;
}

#include <string.h>
char * //@
sh_xx_strchr (const char *s, int c)//@;
{
  char *result = strchr (s, c);

  if (result == NULL)
    {
      sh_throwx ("strchr: character not found");
    }

  return result;
}

//@ /// ---- xx ----

int //@
sh_xx_vsnprintf (char *SH_RESTRICT s, size_t n, const char *SH_RESTRICT format, va_list ap)//@;
{
  int result = sh_x_vsnprintf (s, n, format, ap);

  if ((unsigned) result >= n)
    {
      sh_throwx ("vsnprintf: truncated");
    }

  return result;
}

int //@
sh_xx_snprintf (char *SH_RESTRICT s, size_t n, const char *SH_RESTRICT format, ...)//@;
{
  _SH_V (format, int, sh_xx_vsnprintf (s, n, format, ap));
}

char //@
sh_xx_fgetc (FILE *stream)//@;
{
  int result = sh_x_fgetc (stream);

  if (result == EOF)
    {
      sh_throwx ("fgetc: end of file");
    }

  return (char) result;
}

char //@
sh_xx_getc (FILE *stream)//@;
{
  return sh_xx_fgetc (stream);
}

char //@
sh_xx_getchar (void)//@;
{
  return sh_xx_fgetc (stdin);
}

size_t //@
sh_xx_fread (void *SH_RESTRICT ptr, size_t size, size_t nitems, FILE *SH_RESTRICT stream)//@;
{
  if (sh_x_fread (ptr, size, nitems, stream) != nitems)
    {
      sh_throwx ("fread: end of file");
    }

  return nitems;
}

#include <ctype.h>

#define _SH_XX_STRTO(str, func, type, args) \
  if (str[0] == '\0') \
    { \
      sh_throwx (#func ": empty string"); \
    } \
  \
  if (isspace (str[0])) \
    { \
      sh_throwx (#func ": white-space at start"); \
    } \
  \
  char *end; \
  type result = sh_x_ ## func args; \
  \
  if (end[0] != '\0') \
    { \
      sh_throwx (#func ": some garbage after number"); \
    } \
  \
  return result

#if defined (SH_HAVE_strtod) //@
double //@
sh_xx_strtod (const char *nptr)//@;
{
  _SH_XX_STRTO (nptr, strtod, double, (nptr, &end));
}
#endif //@

#if defined (SH_HAVE_strtof) //@
float //@
sh_xx_strtof (const char *nptr)//@;
{
  _SH_XX_STRTO (nptr, strtof, float, (nptr, &end));
}
#endif //@

#if defined (SH_HAVE_strtold) //@
long double //@
sh_xx_strtold (const char *nptr)//@;
{
  _SH_XX_STRTO (nptr, strtold, long double, (nptr, &end));
}
#endif //@

#if defined (SH_HAVE_strtoimax) //@
intmax_t //@
sh_xx_strtoimax (const char *nptr, int base)//@;
{
  _SH_XX_STRTO (nptr, strtoimax, intmax_t, (nptr, &end, base));
}
#endif //@

#if defined (SH_HAVE_strtoumax) //@
uintmax_t //@
sh_xx_strtoumax (const char *nptr, int base)//@;
{
  _SH_XX_STRTO (nptr, strtoumax, uintmax_t, (nptr, &end, base));
}
#endif //@

#if defined (SH_HAVE_strtol) //@
long //@
sh_xx_strtol (const char *str, int base)//@;
{
  _SH_XX_STRTO (str, strtol, long, (str, &end, base));
}
#endif //@

#if defined (SH_HAVE_strtoll) //@
long long //@
sh_xx_strtoll (const char *str, int base)//@;
{
  _SH_XX_STRTO (str, strtoll, long long, (str, &end, base));
}
#endif //@

#if defined (SH_HAVE_strtoul) //@
unsigned long //@
sh_xx_strtoul (const char *str, int base)//@;
{
  _SH_XX_STRTO (str, strtoul, unsigned long, (str, &end, base));
}
#endif //@

#if defined (SH_HAVE_strtoull) //@
unsigned long long //@
sh_xx_strtoull (const char *str, int base)//@;
{
  _SH_XX_STRTO (str, strtoull, unsigned long long, (str, &end, base));
}
#endif //@

//@ /// ---- Высокоуровневые функции ----

#define _SH_CONV(min, max, func) \
  if (x < min || x > max) \
    { \
      sh_throwx (func ": out of range"); \
    } \
  \
  return x

#include <limits.h>
#include <stdint.h>

int //@
sh_long2int (long x)//@;
{
  _SH_CONV (INT_MIN, INT_MAX, "sh_long2int");
}

size_t //@
sh_umax2size_t (uintmax_t x)//@;
{
  _SH_CONV (0, SIZE_MAX, "sh_umax2size_t");
}

#if defined (SH_HAVE_write) //@
//@ /// write вызывается как минимум один раз, поэтому делается проверка возможности чтения/записи
void //@
sh_repeat_write (int fildes, const void *buf, size_t nbyte)//@;
{
  for (;;)
    {
      ssize_t written = sh_x_write (fildes, buf, nbyte);

      nbyte -= written;

      if (nbyte == 0)
        {
          break;
        }

      buf = (const char *) buf + written;
    }
}
#endif //@

#if defined (SH_HAVE_waitpid) //@
//@ /// запускать с реальным положительным pid и без WNOHANG
int //@
sh_waitpid_status (pid_t pid, int options)//@;
{
  int status;

  sh_x_waitpid (pid, &status, options);

  return status;
}
#endif //@

#if defined (SH_HAVE_fork) //@
pid_t //@
sh_safe_fork (void)//@;
{
  pid_t result = sh_x_fork ();

  if (result == 0)
    {
      sh_reset ();
      sh_set_terminate (&sh_exit_failure);
    }

    return result;
}
#endif //@

#if defined (SH_HAVE_wait) && defined (SH_HAVE_strsignal) //@
#include <string.h>
int //@
sh_code (int status)//@;
{
  if (WIFEXITED (status))
    {
      return WEXITSTATUS (status);
    }
  else if (WIFSIGNALED (status))
    {
      // SOMEDAY: strsignal is not thread-safe
      sh_throwx ("child process killed by signal: %s", strsignal (WTERMSIG (status)));
    }
  else if (WIFSTOPPED (status))
    {
      sh_throwx ("child process stopped by signal: %s", strsignal (WSTOPSIG (status)));
    }
#ifdef WIFCONTINUED
  else if (WIFCONTINUED (status))
    {
      sh_throwx ("child process continued");
    }
#endif
  else
    {
      // POSIX 2013 edition:
      // There may be additional implementation-defined circumstances under which wait() or waitpid()
      // report status. This shall not occur unless the calling process or one of its child processes
      // explicitly makes use of a non-standard extension. In these cases the interpretation of the
      // reported status is implementation-defined.

      sh_throwx ("child process terminated using method not specified in POSIX 2013 edition");
    }
}

#include <stdlib.h>
void //@
sh_success (int status)//@;
{
  int code = sh_code (status);

  if (code != EXIT_SUCCESS)
    {
      sh_throwx ("child process terminated with exit code %d", code);
    }
}
#endif //@

#if defined (SH_HAVE_close) && defined (SH_HAVE_shutdown) && defined (SH_HAVE_poll) && defined (SH_HAVE_read) && defined (SH_HAVE_write) //@
#include <stdlib.h>

#include <sys/socket.h>
#include <unistd.h>
#include <poll.h>

//@ struct sh_multicat_t
//@ {
//@   int src;
//@   int dst;
//@   int flags;
//@ };

//@ /// При EOF fd всегда исключается из working set, т. е. sh_multicat не продолжает читать из него дальше. Есть ситуации, когда продолжение чтения возможно, например, если это терминал. Но я не могу придумать use case'а, когда такое чтение (и игнорирование EOF'а!) было бы нужно
//@ /// При EOF'е я не всегда закрываю input fd, т. к. это может быть терминал. И я не всегда закрываю output fd, т. к. в него просто можно писать дальше, например, сделать туда ещё один sh_multicat
//@ /// SH_WSHUTDOWNW и SH_DONE, т. к. netcat in mind. Но даже в самом гибком netcat'е я не могу придумать use case'ы на остальные комбинации, т. е. регекс SH_[RW]SHUTDOWN[RW]
//@ /// нет RSHUTDOWNR, т. к. EOF на сокете - это уже RSHUTDOWNR
//@ /// Вообще, эти действия придуманы вот для чего:
//@ /// * Для более краткой записи
//@ /// * Чтобы закрыть fd сразу же, как только он перестал быть нужен (чтобы можно было отмонтировать fs и т. д.)
//@ /// * Чтобы можно было реализовать корректный netcat, который посылает FIN сразу же после EOF'а на stdin и закрывает stdout сразу же после FIN
//@ /// * Если не закрывать сразу же, это может привести к deadlock'ам. Например:
//@ ///     sh_multicat ({{0, 3, SH_RCLOSE | SH_WCLOSE}, {4, 1, SH_RCLOSE | SH_WCLOSE}}); // Псевдокод, предположим, что 3 и 4 - это концы одного пайпа
//@ ///   Тогда, если не было бы SH_WCLOSE, был бы deadlock
//@ #define SH_RCLOSE     (1 << 0)
//@ #define SH_WSHUTDOWNW (1 << 1)
//@ #define SH_WCLOSE     (1 << 2)
//@ #define SH_DONE       (1 << 3)

static sh_bool
multicat_done (const struct sh_multicat_t *what)
{
  sh_bool result = SH_TRUE;

  // Сперва должны быть все shutdown'ы, а потом все close'ы, т. к. это может быть один fd

  if (what->flags & SH_WSHUTDOWNW)
    {
      SH_CTRY
        {
          sh_x_shutdown (what->dst, SHUT_WR);
        }
      SH_CATCH
        {
          result = SH_FALSE;
        }
      SH_CEND;
    }

  if (what->flags & SH_RCLOSE)
    {
      SH_CTRY
        {
          sh_x_close (what->src);
        }
      SH_CATCH
        {
          result = SH_FALSE;
        }
      SH_CEND;
    }

  if (what->flags & SH_WCLOSE)
    {
      SH_CTRY
        {
          sh_x_close (what->dst);
        }
      SH_CATCH
        {
          result = SH_FALSE;
        }
      SH_CEND;
    }

  return result;
}

//@ /// Это НЕ параллельный cat, т. к. всё происходит в одном потоке и синхронно (но с мультиплексированием I/O)
//@ /// sh_multicat modifies what
//@ /// size >= 0
//@ /// Любые fd могут быть одинаковыми с оговоркой: после закрывания fd его нельзя использовать, т. е. нельзя {{3, 3, SH_RCLOSE | SH_WCLOSE}}, нельзя {{3, 4, SH_RCLOSE}, {3, 5, 0}}
//@ /// sh_multicat нужен в том числе для правильной реализации "cat file - | prog"
//@ /// sh_multicat применяет действия в любом случае
//@ /// sh_multicat всегда завершается в случае любой ошибки, т. к. нет контрюзекейса
//@ /// Данные могут потеряться, т. е. прочитали, но не записали
//@ /// sh_multicat делает poll только на чтение и ошибки, но не на запись. Т. е. write may block
//@ /// sh_multicat обычно не лочится в read и поэтому сразу же чувствует ошибки write
//@ /// sh_multicat позволяет избежать получения SIGPIPE в большинстве случаев. Но SIGPIPE всё же возможен, если пайп закрылся между poll и write
//@ /// Мой cat круче вашего. В моём случае в "cat | prog" cat сможет сдетектить завершение prog и сразу же завершиться
// Не удалось доказать экспериментом, что системные вызовы Linux sendfile и splice быстрее обычного read/write
void //@
sh_multicat (struct sh_multicat_t what[], int size)//@;
{
  int working = size;

  SH_FTRY // what
    {
      struct pollfd *p = (struct pollfd *) sh_x_malloc (2 * size * sizeof (struct pollfd));

      SH_FTRY // p
        {
          // 1 MiB, т. к. это быстрее, чем, скажем, 16 KiB, проверено на опыте. В то же время значения больше 1 MiB могут привести к появлению бага 12309 ядра Linux
          const size_t buf_size = 1024 * 1024;

          // Делаем malloc, т. к. 1 MiB может не поместиться на стеке
          char *buf = (char *) sh_x_malloc (buf_size);

          SH_FTRY // buf
            {
              for (int i = 0; i != size; ++i)
                {
                  p[2 * i].fd = what[i].src;
                  p[2 * i].events = POLLIN;
                  p[2 * i + 1].fd = what[i].dst;
                  p[2 * i + 1].events = 0;
                }

              while (working != 0)
                {
                  // Делаем poll, т. к. select может не работать с большими fd
                  sh_x_poll (p, 2 * size, -1);

                  for (int i = 0; i != size; ++i)
                    {
                      // Проверка на POLLHUP идёт раньше проверки на POLLERR, т. к. на GNU/Linux при получении RST on TCP выставляется POLLERR | POLLHUP, и мне хочется узнать о POLLHUP раньше, чтобы выдать более точное сообщение об ошибке

                      if (p[2 * i + 1].revents & POLLHUP)
                        {
                          sh_throwx ("sh_multicat: hang up on an output file descriptor (maybe RST on TCP, for example, peer rejected data we sent)");
                        }

                      if (p[2 * i + 1].revents & POLLERR)
                        {
                          sh_throwx ("sh_multicat: error on an output file descriptor (maybe broken pipe)");
                        }

                      if (p[2 * i + 1].revents & POLLNVAL)
                        {
                          sh_throwx ("sh_multicat: an output file descriptor is invalid");
                        }

                      // Input: unfortunately, if there is no writers to pipe, then this is POLLHUP. So, we assume POLLHUP == POLLIN. For the same reason, just in case, we assume POLLERR and POLLNVAL are equal to POLLIN
                      if (p[2 * i].revents & POLLIN || p[2 * i].revents & POLLERR || p[2 * i].revents & POLLHUP || p[2 * i].revents & POLLNVAL)
                        {
                          size_t got = sh_x_read (what[i].src, buf, buf_size);

                          if (got == 0)
                            {
                              p[2 * i].fd = -1;
                              p[2 * i + 1].fd = -1;
                              --working;

                              if (!multicat_done (&(what[i])))
                                {
                                  what[i].src = -1;
                                  SH_THROW;
                                }

                              what[i].src = -1;

                              if (what[i].flags & SH_DONE)
                                {
                                  goto done;
                                }

                              continue;
                            }

                          sh_repeat_write (what[i].dst, buf, got);
                        }
                    }
                }

              done: ;
            }
          SH_FINALLY // buf
            {
              free (buf);
            }
          SH_FEND; // buf
        }
      SH_FINALLY // p
        {
          free (p);
        }
      SH_FEND; // p
    }
  SH_FINALLY // what
    {
      if (working != 0)
        {
          sh_bool result = SH_TRUE;

          for (int i = 0; i != size; ++i)
            {
              if (what[i].src != -1)
                {
                  result = result && multicat_done (&(what[i]));
                }
            }

          if (!result)
            {
              SH_THROW;
            }
        }
    }
  SH_FEND; // what
}

//@ /// "cat a b > out" равен следующему коду (не exception-safe):
//@ /// sh_cat (sh_x_open ("b", O_RDONLY), sh_cat (sh_x_open ("a", O_RDONLY), sh_x_creat ("out", 0666), SH_RCLOSE), SH_RCLOSE | SH_WCLOSE)
//@ /// (такой сжатый стиль - особенность C, см. например, i++ и ++i, strcpy)
//@ /// Или следующему exception-safe, который выглядит даже красивее:
//@ /// int out = sh_x_creat ("out", 0666);
//@ /// SH_FTRY
//@ ///   {
//@ ///     sh_cat (sh_x_open ("a", O_RDONLY), out, SH_RCLOSE);
//@ ///     sh_cat (sh_x_open ("b", O_RDONLY), out, SH_RCLOSE);
//@ ///   }
//@ /// SH_FINALLY
//@ ///   {
//@ ///     sh_close (out);
//@ ///   }
//@ /// SH_FEND;
int //@
sh_cat (int src, int dst, int flags)//@;
{
  struct sh_multicat_t what;

  what.src = src;
  what.dst = dst;
  what.flags = flags;

  sh_multicat (&what, 1);

  return dst;
}
#endif //@

//@
//@ #ifdef __cplusplus
//@ }
//@ #endif
//@
//@ #endif // ! _SH_ETC_H
