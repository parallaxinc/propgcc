/*
 * struct to pass data to the toggle cog driver
 */

struct toggle_mailbox {
  struct toggle_mailbox *next;
  unsigned int wait_time; /* time to sleep between toggles */
  int basepin;
  int token;
};

/* probably don't need a stack, but provide a small one just in case */
#define STACK_SIZE 16
