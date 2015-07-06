/*
 * linux/limits.h - PATH_MAX
*/
#include <assert.h>
#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <linux/limits.h>
#include <sys/un.h>
#include <sys/socket.h>

#define FALSE 0
#define TRUE !FALSE

#define SOFTWARE_VERSION 0.01

#define MSCANGTWYD_UXDOSOCK_PATH "/tmp/mscangtwyd.sock"

#define BUF_SIZE 100
#define LINE_BUFFER_SIZE ( 16384 )
#define NUL_BYTE '\0'
#define TIMEOUT_MIN 1
#define TIMEOUT_MAX 5

#define PRINT_DEBUG printf

enum BINARY_MODE {
  CONSOLE,
  COMMANDLINE,
};



static void usage(const char *);
static void *socketReader( void * );
static void *socketWriter( void * );
