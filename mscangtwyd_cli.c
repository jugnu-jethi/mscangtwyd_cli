/*
 * 
*/
#include "mscangtwyd_cli.h"



int sfd;



int main( int argc, char *argv[] ){
  
  struct sockaddr_un addr;
  int optchar, debug;
  unsigned int timeout = 0;
  ssize_t numRead;
  char buf[BUF_SIZE], sockPathBuffer[ PATH_MAX ];
  enum BINARY_MODE clientMode = COMMANDLINE;
  pthread_t THreaderSocket, THwriterSocket;
  
  memset( sockPathBuffer, NUL_BYTE, sizeof( sockPathBuffer ) );
  strncpy( sockPathBuffer, MSCANGTWYD_UXDOSOCK_PATH, sizeof( MSCANGTWYD_UXDOSOCK_PATH ) );
    
    /* command line args parsing - getopt */
  while( ( optchar = getopt( argc, argv, "chs:t:v" ) ) != -1 ){
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
        
      /* unix domain socket path */
      case 's':
        PRINT_DEBUG( "getopt: %c: %s\n", ( char )optchar, optarg );
        if( PATH_MAX < strlen( optarg ) ){
          PRINT_DEBUG( "Path longer than %d\n", PATH_MAX );
          usage( argv[0] );
          exit( EXIT_FAILURE );
        }
        memset( sockPathBuffer, NUL_BYTE, sizeof( sockPathBuffer ) );
        strncpy( sockPathBuffer, optarg, ( size_t )( PATH_MAX - 1 ) );
        PRINT_DEBUG( "%s\n", sockPathBuffer );
        break;
        
      /* timeout in seconds */ 
      case 't':
        PRINT_DEBUG( "getopt: %c: %s\n", ( char )optchar, optarg );
        timeout = atoi( optarg );
        if( ( TIMEOUT_MAX < timeout ) || ( TIMEOUT_MIN > timeout ) ){
          
          fprintf( stderr, "Invaid timeout duration specified\n" );
          usage( argv[0] );
          exit( EXIT_FAILURE );
        }
        break;
        
      case 'v':
        PRINT_DEBUG( "getopt: %c: %s\n", ( char )optchar, optarg );
        fprintf( stderr, "%s %.2f\n", argv[0], SOFTWARE_VERSION );
        exit( EXIT_SUCCESS );
        break;
        
      default:
        usage( argv[0] );
        exit( EXIT_FAILURE );
    }
  }
  
  /* catch none or non-valid cmdline options in console mode */
  if( CONSOLE == clientMode ){
    
    PRINT_DEBUG( "optind: %d argc: %d\n", optind, argc );
    if( ( 1 == argc ) || ( optind != argc ) ){
      
      usage( argv[0] );
      exit( EXIT_FAILURE );
    }
    
    if( 0 != timeout ){
      
      fprintf( stderr, "Timeout is an invalid option in console mode\n" );
      usage( argv[0] );
      exit( EXIT_FAILURE );
    }
  }
  
  if( COMMANDLINE == clientMode ){
    
    PRINT_DEBUG( "optind: %d argc: %d\n", optind, argc );
    if( ( 1 == argc ) || ( optind == argc ) ){
      
      usage( argv[0] );
      exit( EXIT_FAILURE );
    }
    
    if( 0 == timeout ){
      
      fprintf( stderr, "Timeout must be specified in command line mode\n" );
      usage( argv[0] );
      exit( EXIT_FAILURE );
    }
  }
  

  PRINT_DEBUG( "optind: %d argc: %d\n", optind, argc );
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
  
  pthread_create( &THwriterSocket, NULL, &socketWriter, NULL);
  pthread_create( &THreaderSocket, NULL, &socketReader, NULL);

  if( CONSOLE == clientMode ){
    
    pthread_join( THwriterSocket, NULL);
    
    pthread_cancel( THreaderSocket );
   
    pthread_join( THreaderSocket, NULL);
    
  }
  
  close( sfd );
  
  /* Closes our socket; server sees EOF */
  exit( EXIT_SUCCESS );

}



static void usage( const char *binaryname ){
  
  fprintf(stderr,
    "Usage: %s [<options>] [<mscan-packets>]\n\n"
    "Options:\n"
    "\t-c         console mode\n"
    "\t             -default: commmand line mode\n"
    "\t-h         usage help\n"
    "\t-t <num>   timeout in seconds ( non-console mode only )\n"
    "\t             -duration: %d to %d seconds\n"
    "\t-s <path>  path to server socket file\n"
    "\t             -max: %d characters\n"
    "\t-v         version\n"
    "\n",
    binaryname, TIMEOUT_MIN, TIMEOUT_MAX, PATH_MAX
  );
}



static void *socketReader( void *sockReadArg ){
  
  char readBuffer[ LINE_BUFFER_SIZE ];
  int threadReturn = 0;
  ssize_t bytesRead;

  
  memset( readBuffer, NUL_BYTE, sizeof( readBuffer ) );
  
  while( ( bytesRead = read( sfd, readBuffer, LINE_BUFFER_SIZE ) ) > 0 ){
    
    if( bytesRead != write( STDOUT_FILENO, readBuffer, ( size_t )bytesRead ) ){
      perror( "write" );
      
      threadReturn = 1;
      pthread_exit( &threadReturn );
    }
    
    memset( readBuffer, NUL_BYTE, sizeof( readBuffer ) );
  }
  
  threadReturn = 0;
  pthread_exit( &threadReturn );
}



static void *socketWriter( void *sockWriteArg ){
  
  char *line;
  int threadReturn = 0;
  size_t len = ( size_t )LINE_BUFFER_SIZE;
  ssize_t bytesRead, bytesWrote;
  
  line = ( char * )malloc( ( size_t )( LINE_BUFFER_SIZE * sizeof( char ) ) );
  if( NULL == line ){
    perror( "malloc" );
    
    threadReturn = 1;
    pthread_exit( &threadReturn );
  }
  
  assert( NULL != line );

  printf( "Type 'exit' to quit.\n" );
  while( -1 != ( bytesRead = getline( &line, &len, stdin ) ) ){
    
    if( 0 == strncmp( line, "exit", (  sizeof( "exit" ) - sizeof( NUL_BYTE ) ) ) ){
      
      break;
    }
    
    printf("Retrieved line of length %zu :\n", bytesRead );
    printf("%s", line);
    if( -1 == ( bytesWrote = write( sfd, line, bytesRead ) ) ){
      
      perror( "write" );
      PRINT_DEBUG( "Error while writing to socket.\n" );
    }
    
    if( bytesWrote < bytesRead ){
      
      PRINT_DEBUG( "Partial or incomplete write to socket.\n" );
    }
    
    memset( line, NUL_BYTE, sizeof( line ) );
  }

  free( line );
  threadReturn = 0;
  pthread_exit( &threadReturn );
}



