/*
 * struct to pass data to the toggle cog driver
 */

struct toggle_mailbox {
  unsigned int wait_time; /* time to sleep between toggles */
};

/* probably don't need a stack, but provide a small one just in case */
#define STACK_SIZE 16
