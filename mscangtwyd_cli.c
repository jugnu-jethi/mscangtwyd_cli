/*
 * 
*/
#include "mscangtwyd_cli.h"

int main( int argc, char *argv[] ){
  
  struct sockaddr_un addr;
  int optchar, debug, sfd;
  ssize_t numRead;
  char buf[BUF_SIZE], sockPathBuffer[ PATH_MAX ];
  enum BINARY_MODE clientMode = COMMANDLINE;
  
  memset( sockPathBuffer, NUL_BYTE, sizeof( sockPathBuffer ) );
  strncpy( sockPathBuffer, MSCANGTWYD_UXDOSOCK_PATH, sizeof( MSCANGTWYD_UXDOSOCK_PATH ) );
    
    /* command line args parsing - getopt */
  while( ( optchar = getopt(argc, argv, "chds:t:v" ) ) != -1 ){
    switch( optchar ){

      /* console mode */
      case 'c':
        PRINT_DEBUG( "getopt: %c\n", optchar );
        clientMode = CONSOLE;
        break;
      /* show help or usage */
      case 'h':
        PRINT_DEBUG( "getopt: %c\n", optchar );
        usage( argv[0] );
        exit( EXIT_FAILURE );
        break;
      /* debug mode */
      case 'd':
        debug = 1;
        PRINT_DEBUG( "getopt: %c: %d\n", (char)optchar, debug );
        break;
      /* unix domain socket path */
      case 's':
        PRINT_DEBUG( "getopt: %c: %s\n", (char)optchar, optarg );
        memset( sockPathBuffer, NUL_BYTE, sizeof( sockPathBuffer ) );
        strncpy( sockPathBuffer, optarg, ( size_t )( PATH_MAX - 1 ) );
        PRINT_DEBUG( "%s\n", sockPathBuffer );
        break;
      /* timeout in seconds */ 
      case 't':
        PRINT_DEBUG( "getopt: %c: %s\n", (char)optchar, optarg );
        fprintf( stderr, "timeout option unimplemented\n" );
        exit( EXIT_FAILURE );
        break;
      case 'v':
        PRINT_DEBUG( "getopt: %c: %s\n", (char)optchar, optarg );
        fprintf( stderr, "%s %.2f\n", argv[0], SOFTWARE_VERSION );
        exit( EXIT_SUCCESS );
        break;
      default:
        usage( argv[0] );
        exit( EXIT_FAILURE );

    }

  }
  
  /* catch none or non-valid cmdline options */
  if( ( 1 == argc ) || ( optind < argc ) ){

    usage( argv[0] );
    exit( EXIT_FAILURE );

  }

  /* Create client unix domain socket */
  sfd = socket(AF_UNIX, SOCK_STREAM, 0);
  if( -1 == sfd ){

    perror("socket");
      exit( EXIT_FAILURE );

  }
      
  /* Construct server address, and make the connection */
  memset( &addr, 0, sizeof( struct sockaddr_un ) );
  addr.sun_family = AF_UNIX;
  strncpy(addr.sun_path, MSCANGTWYD_UXDOSOCK_PATH, sizeof( addr.sun_path ) - 1 );

  if( connect( sfd, ( struct sockaddr * )&addr, sizeof( struct sockaddr_un ) ) == -1){

    perror( "connect" );
    exit( EXIT_FAILURE );

  }

  /* Copy stdin to socket */

  while ((numRead = read(STDIN_FILENO, buf, BUF_SIZE)) > 0){
    if (write(sfd, buf, numRead) != numRead){

      PRINT_DEBUG("partial/failed write");

    }

  }

  if( -1 == numRead ){

    perror( "read" );
    exit( EXIT_FAILURE );

  }

  exit( EXIT_SUCCESS );         /* Closes our socket; server sees EOF */

}
