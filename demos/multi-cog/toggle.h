/*
 * struct to pass data to the toggle cog driver
 */

struct toggle_mailbox {
  struct toggle_mailbox *next;
  unsigned int wait_time; /* time to sleep between toggles */
  int pin;
  int token;
};
