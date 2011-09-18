#include <errno.h>
#include <reent.h>

_ssize_t _write_r(struct _reent *reent, int fd, const void *buf_p, size_t bytes)
{
  extern int _putc(int);
  unsigned char *buf = buf_p;
  _ssize_t sent = 0;

  while (bytes > 0) {
    _putc(*buf);
    buf++;
    sent++;
  }
  return sent;
}
