extern void _exit(int status);

void
exit(int status)
{
  _exit(status);
}
