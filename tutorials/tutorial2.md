# Tutorial 2: Low Level Loading

This tutorial shows how to use the low level loading functions to load from an archive. It is based on the first tutorial. Nearly all OpenGL and application code is stripped. Only some lines are left for orientation.


## Data structure

We also need the PhysicsFS header:

~~~{c}
#include "trimeshloader.h"
#include "physfs.h"

tlTrimesh *gTrimesh = NULL;
~~~


## Parse

Load the file provided by the user with PhysicsFS. The API for OBJ and 3DS files is the same, just the names differ. The rough steps are:

- create a state (3ds or Obj)
- pass the data loaded from the file to parser
- set the last flag when we reach the end of the file.
- create a high level tlTrimesh from the state.
- cleanup

~~~{c}
int load( const char *filename )
{
    PHYSFS_file* myfile = PHYSFS_openRead( filename ); 
    if( myfile )
    {
        char buffer[1024];
        unsigned int size = 0;

        if( tlObjCheckFileExtension( filename ) )
        {
            tlObjState *state = tlObjCreateState();
            while( PHYSFS_eof( myfile ) == 0 )
            {
                size = (unsigned int) PHYSFS_read( myfile, buffer, 1, sizeof(buffer) );
                tlObjParse( state, buffer, size, size < sizeof(buffer) ? 1 : 0 );
            }
            gTrimesh = tlCreateTrimeshFromObjState( state );
            tlObjDestroyState( state );
        }
        else if( tl3dsCheckFileExtension( filename ) )
        {
            tl3dsState *state = tl3dsCreateState();
            while( PHYSFS_eof( myfile ) == 0 )
            {
                size = (unsigned int) PHYSFS_read( myfile, buffer, 1, sizeof(buffer) );
                tl3dsParse( state, buffer, size, size < sizeof(buffer) ? 1 : 0 );
            }
            gTrimesh = tlCreateTrimeshFrom3dsState( state );
            tl3dsDestroyState( state );
        }
       
        PHYSFS_close( myfile );   
    }
    else
    {
        return 1;
    }

    if( gTrimesh == 0 )
        return 1;

    return 0;
}
~~~


## Init/Destroy

Iniitalise PhysicsFS and load the trimesh.

~~~{c}
int main()
{
    PHYSFS_init(argv[0]);
    PHYSFS_AddToSearchPath("myzip.zip", 1);

    /* other code, setup opengl */

    if( load() != 0 )
        return 1;

    /* other code */
   
    while( running == 1 )
    {
        /* other code */
        draw();
        /* other code */
    }

    tlDeleteTrimesh( gTrimesh );
    PHYSFS_deinit();

    /* other code */
}

~~~

## Data access

The draw functions stays the same:

~~~{c}
void draw()
{
    glPushMatrix();
    glRotatef( -90.0f, 1.0f, 0.0f, 0.0f );
    glColor3f( 0.0f, 0.0f, 1.0f );
    glEnableClientState( GL_VERTEX_ARRAY );
    glVertexPointer( 3, GL_FLOAT, sizeof(float) * 8, gTrimesh->vertices );
    for( i = 0; i < gTrimesh->object_count; i++)
    {
        glDrawElements( GL_TRIANGLES, gTrimesh->objects[i].face_count  * 3, GL_UNSIGNED_SHORT, &gTrimesh->faces[gTrimesh->objects[i].face_index] );
    }
    glDisableClientState( GL_VERTEX_ARRAY );
    glPopMatrix();
}
~~~
