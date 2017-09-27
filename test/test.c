#include "tlobj.h"
#include "tl3ds.h"

#include <stdio.h>

int main( int argc, char **argv )
{
	FILE *f = 0;
	
	if( argc < 2 )
		return 1;
		
	f = fopen( argv[1], "r" );
	if( f )
	{
		tl3dsState *state = 0;
		
		state = tl3dsCreateState();
		if( state )
		{
			char buffer[1024*16];
			unsigned int size = 0;
			unsigned int i = 0, count = 0;
			
			while( !feof( f ) )
			{
				size = (unsigned int) fread( buffer, 1, sizeof(buffer), f );
				tl3dsParse( state, buffer, size, size < sizeof(buffer) ? 1 : 0 );
			}

			count = tl3dsObjectCount( state );
			for( i = 0; i < count; i++ )
			{
				unsigned int j = 0;
				unsigned int count = tl3dsObjectFaceCount( state, i );
				unsigned int index = tl3dsObjectFaceIndex( state, i );
				printf( "Object: %s\n", tl3dsObjectName( state, i ) );
				
				
				for( j = 0; j<count; j++ )
				{
					unsigned short a, b, c;
					tl3dsGetFace( state, index + j, &a, &b, &c );
					printf( "   Triangle: (%d, %d, %d)\n", a, b, c );
				}
				
			}
	
			count = tl3dsVertexCount( state );
			for( i = 0; i < count; i++ )
			{
				float x = 0, y = 0, z = 0, u = 0, v = 0, nx = 0, ny = 0, nz = 0;
				tl3dsGetVertex( state, i, &x, &y, &z, &u, &v, &nx, &ny, &nz );
				printf( "   Vertex: (%f, %f, %f), TexCoords: (%f, %f), Normals: (%f, %f, %f)\n", x, y, z, u, v, nx, ny, nz );
			}
			
			tl3dsDestroyState( state );
		}
		
		fclose( f );
	}
	
	return 0;
}

