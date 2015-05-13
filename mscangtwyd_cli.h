/*
 * linux/limits.h - PATH_MAX
*/
#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <linux/limits.h>
#include <sys/un.h>
#include <sys/socket.h>

#define SOFTWARE_VERSION 0.01

#define MSCANGTWYD_UXDOSOCK_PATH "/tmp/mscangtwyd.sock"

#define BUF_SIZE 100
#define LINE_BUFFER_SIZE ( 16384 )
#define NUL_BYTE '\0'

#define PRINT_DEBUG printf

enum BINARY_MODE {
  CONSOLE,
  COMMANDLINE,
};



static void usage(const char *);



static void usage(const char *binaryname)
{
  fprintf(stderr,
    "Usage: %s [<options>]\n\n"
    "Options:\n"
    "\t-c         console mode\n"
    "\t-d         enable debug mode\n"
    "\t-h         usage help\n"
    "\t-t <num>   timeout in seconds ( non-console mode only )\n"
    "\t-s <path>  path to server socket file\n"
    "\t-v         version\n"
    "\n",
    binaryname
  );
}
