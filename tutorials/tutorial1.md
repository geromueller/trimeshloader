# Tutorial 1: High Level Loading

This tutorial shows how to use the high level loading functions. Nearly all OpenGL and application code is stripped. Only some lines are left for orientation.


## Data structure

First we need the header of course. To make it plain and simple we just use a global variable to hold the trimesh data:

~~~{c}
#include "trimeshloader.h"

tlTrimesh *gTrimesh = NULL;
~~~


## Load/Destroy

load the file provided by the user. if loading fails for some reason, NULL is returned:

~~~{c}
int main()
{
	/* other code, setup opengl */

	gTrimesh = tlLoadTrimesh( argv[1] );
	if( gTrimesh == NULL )
		return 1;

	/* other code */

	while( running == 1 )
	{
		/* other code */
		draw();
		/* other code */
	}

	tlDeleteTrimesh( gTrimesh );

	/* other code */
}
~~~

## Data access

Now we actually draw each object of the the trimesh. bind the vertex data with glVertexPointer, and provide indices to glDrawElements:

~~~{c}
void draw()
{
    glPushMatrix();
    glRotatef( -90.0f, 1.0f, 0.0f, 0.0f );
    glColor3f( 0.0f, 0.0f, 1.0f );
    glEnableClientState( GL_VERTEX_ARRAY );
    glVertexPointer( 3, GL_FLOAT, gTrimesh->verticex_size, gTrimesh->vertices );
    for( i = 0; i < gTrimesh->object_count; i++)
    {
        glDrawElements( GL_TRIANGLES, gTrimesh->objects[i].face_count  * 3, GL_UNSIGNED_SHORT, gTrimesh->faces + gTrimesh->objects[i].face_index );
    }
    glDisableClientState( GL_VERTEX_ARRAY );
    glPopMatrix();
}
~~~
