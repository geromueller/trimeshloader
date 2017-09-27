#include "trimeshloader.h"

#include <stdio.h>

void print_info()
{
	printf("tlobjloader <filename>");
}

int main( int argc, char **argv )
{
	tlTrimesh *trimesh = NULL;

	if( argc < 2 )
	{
		print_info();
		return 1;
	}

	trimesh = tlLoadTrimesh( argv[1] , TL_FVF_XYZ | TL_FVF_UV | TL_FVF_NORMAL );
	if( trimesh )
	{
		printf( "Objects: %d\n", trimesh->object_count );
		printf( "Vertices: %d\n", trimesh->vertex_count );
		printf( "Faces: %d\n", trimesh->face_count );
		printf( "Materials: %d\n", trimesh->material_count );

		tlDeleteTrimesh( trimesh );
	}

	return 0;
}

