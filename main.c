#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/time.h>

#define PASSES      2
#define BUFFER_SIZE	65536

// ranrot-b prng
uint32_t rrb_rand( uint32_t seed )
{
    static uint32_t rrb_lo, rrb_hi;

    if( seed )
    {
        rrb_lo = seed;
        rrb_hi = ~seed;
    }

    rrb_hi = ( rrb_hi << 16 ) + ( rrb_hi >> 16 );
    rrb_hi += rrb_lo;
    rrb_lo += rrb_hi;

    return rrb_hi;
}

int main( int argc, char **argv )
{
    printf( "\nSaferase v0.1 (11-jan-2003) by Robin Leffmann\n\n" );

    if( argc == 1 )
    {
        printf( "Usage: drag files from their original locations and drop them onto Saferase.\n\n"
                "Press enter to quit" );
        getchar();

        return 0;
    }

    // random-ish seed for the prng
    uint32_t seed = 0;
    struct timeval tod;
    while( seed == 0 || seed == -1 )
    {
        gettimeofday( &tod, NULL );
        seed = tod.tv_sec * tod.tv_usec;
    }
    rrb_rand( seed );

    printf( "%i file(s) selected for unrecoverable erasure - are you really sure? (y/n) ", argc - 1 );
    if( getchar() != 'y' )
        return 0;

    // buffer for the random data
    uint8_t *buffer = (uint8_t*)malloc( BUFFER_SIZE );
    if( !buffer )
        return 1;

    // delete the whole list of files each pass, to try ensure flushing to disk
    printf( "\n" );
    int p = PASSES, c = 0;
    while( p-- )
    {
        int f = argc;
        while( --f )
        {
            FILE *file = fopen( argv[f], "rb+" );
            if( !file )
                continue;

            fseek( file, 0, SEEK_END );
            int size = ftell( file );
            if( size == 0 )
            {
                fclose( file );
                continue;
            }
            fseek( file, 0, SEEK_SET );

            // fill buffer with random data, then flush to disk
           	while( size )
           	{
           	    int portion = size > BUFFER_SIZE ? BUFFER_SIZE : size;
                for( int i = 0; i < portion; i++ )
                    buffer[i] = (uint8_t)rrb_rand( 0 );
                fwrite( buffer, 1, portion, file );
                size -= portion;
            }
            fflush( file );
            fclose( file );

            if( !p )
            {
                // rename file before deletion
                char new[strlen(argv[f])];
                strcpy( new, argv[f] );
                char *p = strrchr( new, '/' );
                if( !p )
                	p = new;
                int n = strlen( p ) - 1;
                while( n )
                    p[n--] = 97 + rrb_rand( 0 ) % 26;
                if( !rename(argv[f], new) && !remove(new) )
                {
                    printf( "%s ... erased\n", strrchr(argv[f], '/') + 1 );              
                    c++;
                }
            }
        }
    }

    free( buffer );
    printf( "\n%i of %i file(s) erased\n", c, argc - 1 );

    return 0;
}
