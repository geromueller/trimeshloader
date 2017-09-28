# Tutorial 3: Flexible Low Level Loading

This tutorial shows how to use the low level loading functions and how to fill custom data structures. Nearly all ODE and application code is stripped. Only some lines are left for orientation.


## Custom data structure

Ode Trimesh creators need simple (only points, floats or doubles) vertices and indices (int). The high level structure tlTrimesh provides complex vertices (point + uv map + normal) and unsigned short indices instead, which would be a waste of memory. We'd also need to copy the data again.

~~~{c}
typedef struct CollisionTrimesh
{
    float *vertex_buffer;
    unsigned int vertex_count;

    int *face_buffer;
    unsigned int face_count;
} CollisionTrimesh;
~~~


## Parsing and flexible data access

The file is parsed the same way it was done in @ref tutorials/tutorial2.md, except that we now only load 3DS files, for readability. It is the same scheme:

- open the file
- create a parser state
- pass chunks to the parser
- access data via low level access functions @ref tl3dsGetVertex / @ref tl3dsGetFace
- destroy state and close file


~~~{c}
#include "tl3ds.h"

CollisionTrimesh *loadCollisionTrimesh( const char *filename )
{
    FILE *f = 0;
    CollisionMesh *mesh = 0;
   
    mesh = malloc( sizeof(CollisionMesh)  );
   
    f = fopen( filename, "r" );
    if( f )
    {
        char buffer[1024];
        tl3dsState *state = 0;
        unsigned int size = 0;
        unsigned int i = 0;

        state = tl3dsCreateState();

        while( !feof( f ) )
        {
            size = (unsigned int) fread( buffer, 1, sizeof(buffer), f );
            tl3dsParse( state, buffer, size, size < sizeof(buffer) ? 1 : 0 );
        }

        mesh->vertex_count = tl3dsVertexCount( state );
        mesh->vertex_buffer = malloc( sizeof(float) * 3 * mesh->vertex_count );
        for( i = 0; i < mesh->vertex_count; i++ )
            tl3dsGetVertex( state, i, &mesh->vertex_buffer[3*i], &mesh->vertex_buffer[3*i+1], &mesh->vertex_buffer[3*i+2], 0, 0, 0, 0, 0 );

        mesh->face_count = tl3dsFaceCount( state );
        mesh->face_buffer = malloc( sizeof(int) * 3 * mesh->face_count );
        for( i = 0; i < mesh->face_count; i++ )
        {
            unsigned int a = 0, b = 0, c = 0;
            tl3dsGetFace( state, i, &a, &b, &c );
           
            /* convert from unsigned to signed */
            mesh->index_buffer[3*i] = a;
            mesh->index_buffer[3*i+1] = b;
            mesh->index_buffer[3*i+2] = c;
        }

        tl3dsDestroyState( state );

        fclose( f );
    }

    return mesh;
}
~~~

Quicklink: @ref tutorials/tutorial1.md

Quicklink: @ref tutorials/tutorial2.md


## Building the Trimesh Collision Shape

This is for convinience only. Look at ODE documentation!


~~~{c}
#include "ode.h"

void createCollisionShape( dSpace space,  dBody body, const char *filename, dReal density )
{
    CollisionTrimesh *trimesh = loadCollisionTrimesh( "test.3ds" )
    dTriMeshDataID tridata = dGeomTriMeshDataCreate();
    dGeomTriMeshDataBuildSingle (
            tridata,
            trimesh->vertex_buffer,
            3 * sizeof ( float ),
            trimesh->vertex_count,
            trimesh->face_buffer,
            trimesh->face_count * 3,
            3 * sizeof ( int ) );

    dGeomID mesh = dCreateTriMesh ( space, tridata, 0, 0, 0 );
    dGeomSetBody ( mesh, body );

    dMass mass;
    dMassSetTrimesh ( &mass, density, mesh );
    dBodySetMass ( body, &mass );
} 
~~~
