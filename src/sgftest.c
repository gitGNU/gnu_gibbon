#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "sgf.h"

static void Indent( int n ) {
    while( n-- )
	putchar( ' ' );
}

static void PrintProperty( property *pp, int n ) {

    listOLD *pl;
    
    Indent( n );
    putchar( pp->ach[ 0 ] );
    putchar( pp->ach[ 1 ] );

    for( pl = pp->pl->plNext; pl->p; pl = pl->plNext )
	printf( "[%s]", (char *) pl->p );
    
    putchar( '\n' );
}

static void PrintNode( listOLD *pl, int n ) {

    for( pl = pl->plNext; pl->p; pl = pl->plNext )
	PrintProperty( pl->p, n );
    
    Indent( n ); puts( "-" );
}

static void PrintSequence( listOLD *pl, int n ) {

    for( pl = pl->plNext; pl->p; pl = pl->plNext )
	PrintNode( pl->p, n );
}

static void PrintGameTreeSeq( listOLD *pl, int n );

static void PrintGameTree( listOLD *pl, int n ) {

    pl = pl->plNext;

    Indent( n ); puts( "<<<<<" );
    PrintSequence( pl->p, n );
    PrintGameTreeSeq( pl, n + 1 );
    Indent( n ); puts( ">>>>>" );
}

static void PrintGameTreeSeq( listOLD *pl, int n ) {

    for( pl = pl->plNext; pl->p; pl = pl->plNext )
	PrintGameTree( pl->p, n );
}

void Error( char *s, int f ) {

    fprintf( stderr, "sgf error: %s\n", s );
}

int main( int argc, char *argv[] ) {

    FILE *pf = NULL;
    listOLD *pl;
    
    SGFErrorHandler = Error;

    if( argc > 1 )
	if( !( pf = fopen( argv[ 1 ], "r" ) ) ) {
	    fprintf( stderr, "%s: %s\n", argv[ 1 ], strerror(errno) );
	    return 1;
	}
    
    if( ( pl = SGFParse( pf ? pf : stdin ) ) )
	PrintGameTreeSeq( pl, 0 );
    else {
	puts( "Fatal error; can't print collection." );
	return 2;
    }
    
    return 0;
}
