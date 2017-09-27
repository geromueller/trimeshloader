/*
 * Copyright (c) 2007 Gero Mueller <gero.mueller@cloo.de>
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 *    1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 *
 *    2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 *
 *    3. This notice may not be removed or altered from any source
 *    distribution.
 */

#include "trimeshloader/tl3ds.h"

#include <string.h>
#include <stdlib.h>

/*----------------------------------------------------------------------------*/
typedef enum tl3dsParsingState
{
	TDS_STATE_READ_CHUNK_ID,
	TDS_STATE_READ_CHUNK_LENGTH,
	TDS_STATE_READ_OBJECT_NAME,
	TDS_STATE_SKIP_CHUNK,
	TDS_STATE_READ_POINT_COUNT,
	TDS_STATE_READ_POINTS,
	TDS_STATE_READ_TEXCOORD_COUNT,
	TDS_STATE_READ_TEXCOORDS,
	TDS_STATE_READ_FACE_COUNT,
	TDS_STATE_READ_FACES,
	TDS_STATE_READ_MATERIAL_NAME,
	TDS_STATE_READ_MATERIAL_COLOR,
	TDS_STATE_READ_MATERIAL_PROPERTY,
	TDS_STATE_READ_MATERIAL_LIST_NAME,
	TDS_STATE_READ_MATERIAL_LIST_COUNT,
	TDS_STATE_READ_MATERIAL_LIST,
	TDS_STATE_READ_MAP_NAME,
	TDS_STATE_DONE
} tl3dsParsingState;


/*----------------------------------------------------------------------------*/
typedef struct tl3dsObject
{
	char *name;
	unsigned int index, count;
} tl3dsObject;

/*----------------------------------------------------------------------------*/
typedef struct tl3dsMaterial
{
	char *name;
	float ambient[4], diffuse[4], specular[4], shininess;
} tl3dsMaterial;

/*----------------------------------------------------------------------------*/
typedef struct tl3dsMaterialReference
{
	char *name;
	unsigned int face_index, face_count;
} tl3dsMaterialReference;

/*----------------------------------------------------------------------------*/
struct tl3dsState
{
	unsigned short chunk_id;
	unsigned int chunk_length;

	char *buffer;
	unsigned int buffer_size;
	unsigned int buffer_length;

	unsigned int counter;
	unsigned int item_count;

	tl3dsParsingState parsing_state;

	float *point_buffer;
	unsigned int point_buffer_size;
	unsigned int point_count;
    unsigned int last_point_index;

	float *texcoord_buffer;
	unsigned int texcoord_buffer_size;
	unsigned int texcoord_count;

	unsigned short *face_buffer;
	unsigned int face_buffer_size;
	unsigned int face_count;

	tl3dsMaterial *material_buffer;
	unsigned int material_count;

	tl3dsMaterialReference *material_reference_buffer;
	unsigned int material_reference_count;
	unsigned int last_material_face;

	tl3dsObject **object_buffer;
	unsigned int object_count;

};



/*----------------------------------------------------------------------------*/
static unsigned int tds_le()
{
	const char endian[8] = { 1, 0, 0, 0, 0, 0, 0, 0 };
	unsigned int i = *((unsigned int *)endian);

	/* LE uint64: i = 1 */
	/* LE uint32: i = 1 */
	/* LE uint16: i = 1 */

	/* BE uint32: i > 1 */
	/* BE uint32: i > 1 */
	/* BE uint16: i > 1 */

	if( i == 1 )
		return 1;
	else
		return 0;
}

/*----------------------------------------------------------------------------*/
static float tds_read_le_float( const char *ptr )
{
	float f = 0;
	char *fptr = (char *)&f;

	if( tds_le() )
	{
		fptr[0] = ptr[0];
		fptr[1] = ptr[1];
		fptr[2] = ptr[2];
		fptr[3] = ptr[3];
	}
	else
	{
		fptr[0] = ptr[3];
		fptr[1] = ptr[2];
		fptr[2] = ptr[1];
		fptr[3] = ptr[0];
	}

	return f;
}


/*----------------------------------------------------------------------------*/
static unsigned short tds_read_le_ushort( const char *ptr )
{
	unsigned short s = 0;
	char *sptr = (char *)&s;

	if( tds_le() )
	{
		sptr[0] = ptr[0];
		sptr[1] = ptr[1];
	}
	else
	{
		sptr[0] = ptr[1];
		sptr[1] = ptr[0];
	}

	return s;

}

/*----------------------------------------------------------------------------*/
static unsigned int tds_read_le_uint( const char *ptr )
{
	unsigned int i = 0;
	char *iptr = (char *)&i;

	if( tds_le() )
	{
		iptr[0] = ptr[0];
		iptr[1] = ptr[1];
		iptr[2] = ptr[2];
		iptr[3] = ptr[3];
	}
	else
	{
		iptr[0] = ptr[3];
		iptr[1] = ptr[2];
		iptr[2] = ptr[1];
		iptr[3] = ptr[0];
	}

	return i;

}


/*----------------------------------------------------------------------------*/
static int tds_buffer_reserve( tl3dsState *state, unsigned int size )
{
	unsigned int new_size = 1;
	char *new_buffer = 0;

	if( state == NULL )
		return 1;

	if( state->buffer_size >= size )
		return 0;

	while( new_size < size )
		new_size = new_size * 2;

	new_buffer = (char *)realloc( state->buffer, new_size );
	if( new_buffer )
	{
		state->buffer = new_buffer;
		state->buffer_size = new_size;
	}

	return 0;
}


/*----------------------------------------------------------------------------*/
static void tds_buffer_add( tl3dsState *state, char c )
{
	if( tds_buffer_reserve( state, state->buffer_length + 1 ) != 0 )
		return;

	state->buffer[ state->buffer_length ] = c;
	state->buffer_length++;
}

/*----------------------------------------------------------------------------*/
static void tds_material_buffer_add( tl3dsState *state, char *name )
{
    float           defAmbient[4]   = {1.0, 1.0, 1.0, 1.0};
    float           defSpecular[4]  = {0.0, 0.0, 0.0, 1.0};
	unsigned int    new_size    = (state->material_count + 1 ) * sizeof(tl3dsMaterial);
	tl3dsMaterial   *new_buffer = realloc( state->material_buffer, new_size );

	if( new_buffer == NULL )
        return;

	state->material_buffer = new_buffer;
	state->material_buffer[state->material_count].name = malloc( strlen( name ) + 1 );
	strcpy(state->material_buffer[state->material_count].name, name);

    memcpy(state->material_buffer[state->material_count].ambient, defAmbient, sizeof(float) * 4);
    memcpy(state->material_buffer[state->material_count].diffuse, defAmbient, sizeof(float) * 4);
    memcpy(state->material_buffer[state->material_count].specular, defSpecular, sizeof(float) * 4);
    state->material_buffer[state->material_count].shininess   = 1.0;

	state->material_count++;
}

/*----------------------------------------------------------------------------*/
static void tds_material_set_property( tl3dsState *state, float *props )
{
    int last_material_index  = state->material_count - 1;

    switch (state->chunk_id)
    {
    case 0xA010:
        memcpy(state->material_buffer[last_material_index].ambient, props, sizeof(float) * 3);
        break;
    case 0xA020:
        memcpy(state->material_buffer[last_material_index].diffuse, props, sizeof(float) * 3);
        break;
    case 0xA030:
        memcpy(state->material_buffer[last_material_index].specular, props, sizeof(float) * 3);
        break;
    case 0xA040:
        state->material_buffer[last_material_index].shininess = *props;
        break;
    case 0xA050:
        {
            float   opacity = 1.0f - *props;

            state->material_buffer[last_material_index].ambient[3]   = opacity;
            state->material_buffer[last_material_index].diffuse[3]   = opacity;
            state->material_buffer[last_material_index].specular[3]  = opacity;
        }
       break;
    }
}

/*----------------------------------------------------------------------------*/
static void tds_material_reference_buffer_add( tl3dsState *state, char *name)
{
	unsigned int new_size = (state->material_reference_count + 1 ) * sizeof(tl3dsMaterialReference);
	tl3dsMaterialReference *new_buffer = realloc( state->material_reference_buffer, new_size );

	if( new_buffer == NULL )
        return;

	state->material_reference_buffer = new_buffer;
	state->material_reference_buffer[state->material_reference_count].name = malloc( strlen(name) + 1 );
	strcpy( state->material_reference_buffer[state->material_reference_count].name, name );

	state->material_reference_count++;
}


/*----------------------------------------------------------------------------*/
static void tds_material_reference_set_range( tl3dsState *state, unsigned int face_index, unsigned int face_count )
{
	if( state->material_reference_count > 0 )
	{
		state->material_reference_buffer[state->material_reference_count - 1].face_index = face_index;
		state->material_reference_buffer[state->material_reference_count - 1].face_count = face_count;
	}
}


/*----------------------------------------------------------------------------*/
static int tds_object_buffer_add(
	tl3dsState *state,
	const char *name,
	unsigned int name_length )
{
	tl3dsObject **new_object_buffer = 0;
	unsigned int new_object_count = state->object_count + 1;

	new_object_buffer = (tl3dsObject **)realloc(
		state->object_buffer,
		sizeof(tl3dsObject *) * new_object_count );

	if( new_object_buffer )
	{
		/* create the new object */
		tl3dsObject *new_object = (tl3dsObject *)malloc( sizeof(tl3dsObject) );
		memset(	new_object, 0, sizeof(tl3dsObject) );

		/* copy the name */
		new_object->name = (char *)malloc( name_length );
		memcpy( new_object->name, name, name_length );

		/* add the new object */
		new_object_buffer[ new_object_count - 1 ] = new_object;

		/* update state */
		state->object_buffer = new_object_buffer;
		state->object_count = new_object_count;

		return 0;
	}

	return 1;
}


/*----------------------------------------------------------------------------*/
static void tds_point_buffer_grow( tl3dsState *state, unsigned int count )
{
	unsigned int new_size = (state->point_count + count ) * 3 * sizeof(float);

	float *new_buffer = realloc( state->point_buffer, new_size );
	if( new_buffer )
	{
		state->point_buffer = new_buffer;
		state->point_buffer_size = new_size;
	}
}


/*----------------------------------------------------------------------------*/
static void tds_point_buffer_add( tl3dsState *state, float x, float y, float z )
{
	unsigned int new_size = (state->point_count + 1 ) * 3 * sizeof(float);

	if( state->point_buffer_size < new_size )
		return;

	state->point_buffer[state->point_count * 3] = x;
	state->point_buffer[state->point_count * 3 + 1] = y;
	state->point_buffer[state->point_count * 3 + 2] = z;
	state->point_count++;
}


/*----------------------------------------------------------------------------*/
static void tds_texcoord_buffer_grow( tl3dsState *state, unsigned int count )
{
	unsigned int new_size = (state->texcoord_count + count ) * 2 * sizeof(float);
	float *new_buffer = realloc( state->texcoord_buffer, new_size );

	if( new_buffer )
	{
		state->texcoord_buffer = new_buffer;
		state->texcoord_buffer_size = new_size;
	}
}


/*----------------------------------------------------------------------------*/
static void tds_texcoord_buffer_add( tl3dsState *state, float u, float v )
{
	unsigned int new_size = (state->texcoord_count + 1 ) * 2 * sizeof(float);

	if( state->texcoord_buffer_size < new_size )
		return;

	state->texcoord_buffer[state->texcoord_count * 2] = u;
	state->texcoord_buffer[state->texcoord_count * 2 + 1] = v;
	state->texcoord_count++;
}


/*----------------------------------------------------------------------------*/
static void tds_face_buffer_grow( tl3dsState *state, unsigned int count )
{
	unsigned int new_size
		= (state->face_count + count ) * 3 * sizeof(unsigned short);

	unsigned short *new_buffer = realloc( state->face_buffer, new_size );
	if( new_buffer )
	{
		state->face_buffer = new_buffer;
		state->face_buffer_size = new_size;
	}
}


/*----------------------------------------------------------------------------*/
static void tds_face_buffer_add(
	tl3dsState *state,
	unsigned short a,
	unsigned short b,
	unsigned short c )
{
	unsigned int new_size
		= (state->face_count + 1 ) * 3 * sizeof(unsigned short);

	if( state->face_buffer_size < new_size )
		return;

	state->face_buffer[state->face_count * 3] = a + state->last_point_index;
	state->face_buffer[state->face_count * 3 + 1] = b + state->last_point_index;
	state->face_buffer[state->face_count * 3 + 2] = c + state->last_point_index;
	state->face_count++;
}


/*----------------------------------------------------------------------------*/
tl3dsState *tl3dsCreateState()
{
	tl3dsState *state = malloc( sizeof(tl3dsState) );

	if( state )
	{
		memset( state, 0, sizeof(tl3dsState) );
		state->parsing_state = TDS_STATE_READ_CHUNK_ID;
	}

	return state;
}


/*----------------------------------------------------------------------------*/
int tl3dsResetState( tl3dsState *state )
{
	unsigned int i;

	if( state->buffer )
		free( state->buffer );

	if( state->object_buffer )
	{
		for( i = 0; i < state->object_count; i++ )
		{
			tl3dsObject *obj = state->object_buffer[i];
			if( obj == 0 )
				continue;

			if( obj->name )
				free( obj->name );

			free( obj );
		}

		free( state->object_buffer );
	}

	if( state->point_buffer )
		free( state->point_buffer );

	if( state->texcoord_buffer )
		free( state->texcoord_buffer );

	if( state->face_buffer )
		free( state->face_buffer );

	if( state->material_buffer )
		free( state->material_buffer );

	if( state->material_reference_buffer )
		free( state->material_reference_buffer );

	memset( state, 0, sizeof(tl3dsState) );

	state->parsing_state = TDS_STATE_READ_CHUNK_ID;

	return 0;
}


/*----------------------------------------------------------------------------*/
void tl3dsDestroyState( tl3dsState *state )
{
	if( state )
	{
		tl3dsResetState( state );
		free( state );
	}
}


/*----------------------------------------------------------------------------*/
int tl3dsParse(
	tl3dsState *state,
	const char *buffer,
	unsigned int length,
	int last )
{
	unsigned int i = 0;

	if( state == NULL )
		return 1;

	while( i < length )
	{
		char c = buffer[i];

		switch( state->parsing_state )
		{
		case TDS_STATE_READ_CHUNK_ID:
			tds_buffer_add( state, c );

			if( state->buffer_length == 2 )
			{
				state->chunk_id = tds_read_le_ushort( state->buffer );
				state->buffer_length = 0;

				state->parsing_state = TDS_STATE_READ_CHUNK_LENGTH;
			}
			++i;
			break;

		case TDS_STATE_READ_CHUNK_LENGTH:
			tds_buffer_add( state, c );

			if( state->buffer_length == 4 )
			{
				state->chunk_length = tds_read_le_uint( state->buffer );
				state->buffer_length = 0;

				switch( state->chunk_id )
				{
				case 0x4d4d: /* MAIN CHUNK */
				case 0x4100: /* TRI_OBJECT */
				case 0x3d3d: /* 3D EDITOR CHUNK */
				case 0xAFFF: /* MATERIAL CHUNK */
				case 0xA200: /* MAT_TEXMAP */
				case 0xA230: /* MAT_BUMPMAP */
				case 0xA33A: /* MAT_TEX2MAP */
					state->parsing_state = TDS_STATE_READ_CHUNK_ID;
					break;

				case 0x4000: /* OBJECT */
					state->parsing_state = TDS_STATE_READ_OBJECT_NAME;
					break;

				case 0x4110: /* POINT_ARRAY */
					state->parsing_state = TDS_STATE_READ_POINT_COUNT;
					break;

				case 0x4111: /* POINT_FLAG_ARRAY */
					state->counter = 6;
                    state->parsing_state = TDS_STATE_SKIP_CHUNK;
                    break;

				case 0x4120: /* FACE_ARRAY */
					state->parsing_state = TDS_STATE_READ_FACE_COUNT;
					break;

				case 0x4130: /* MSH_MAT_GROUP */
					state->parsing_state = TDS_STATE_READ_MATERIAL_LIST_NAME;
                    break;

				case 0x4140: /* TEX_ARRAY */
					state->parsing_state = TDS_STATE_READ_TEXCOORD_COUNT;
					break;

				case 0x4150: /* SMOOTH_GROUP */
                    state->counter = 6;
					state->parsing_state = TDS_STATE_SKIP_CHUNK;

				case 0x4160: /* MESH_MATRIX */
 					state->counter = 6;
                    state->parsing_state = TDS_STATE_SKIP_CHUNK;
                    break;

                case 0xA000: /* MATERIAL_NAME */
                    state->parsing_state = TDS_STATE_READ_MATERIAL_NAME;
                    break;

                case 0xA010: /* AMBIENT_COLOR */
                case 0xA020: /* DIFFUSE_COLOR */
                case 0xA030: /* SPECULAR_COLOR */
                    state->parsing_state = TDS_STATE_READ_MATERIAL_COLOR;
                    break;

                case 0xA040: /* SHININESS */
                case 0xA050: /* TRASPARENCY */
                    state->parsing_state = TDS_STATE_READ_MATERIAL_PROPERTY;
                    break;

                case 0xA300: /* MAT_MAPNAME */
                    state->parsing_state = TDS_STATE_READ_MAP_NAME;
                    break;

				default:
					state->counter = 6;
                    if( state->counter >= state->chunk_length )
                        state->parsing_state = TDS_STATE_READ_CHUNK_ID;
                    else
                        state->parsing_state = TDS_STATE_SKIP_CHUNK;
					break;
				}
			}
			++i;
			break;

		case TDS_STATE_READ_OBJECT_NAME:
			tds_buffer_add( state, c );

			if( c == 0 )
			{
				tds_object_buffer_add(
					state,
					state->buffer,
					state->buffer_length );

				/* continue with chunks */
				state->parsing_state = TDS_STATE_READ_CHUNK_ID;
				state->buffer_length = 0;
			}
			++i;
			break;

		case TDS_STATE_READ_POINT_COUNT:
			tds_buffer_add( state, c );

			if( state->buffer_length == 2 )
			{
				state->item_count = tds_read_le_ushort( state->buffer );
				tds_point_buffer_grow( state, state->item_count );

				state->parsing_state = TDS_STATE_READ_POINTS;
				state->buffer_length = 0;
				state->counter = 0;
			}
			++i;
			break;

		case TDS_STATE_READ_POINTS:
			tds_buffer_add( state, c );

			if( state->buffer_length == 12 )
			{
				tds_point_buffer_add(
					state,
					tds_read_le_float( state->buffer ),
					tds_read_le_float( state->buffer + 4 ),
					tds_read_le_float( state->buffer + 8 ) );

				state->counter++;
				state->buffer_length = 0;

				if( state->counter >= state->item_count )
				{
					state->parsing_state = TDS_STATE_READ_CHUNK_ID;
					state->buffer_length = 0;
					state->last_point_index = state->point_count - state->item_count;
				}
			}

			++i;
			break;

		case TDS_STATE_READ_TEXCOORD_COUNT:
			tds_buffer_add( state, c );

			if( state->buffer_length == 2 )
			{
				state->item_count = tds_read_le_ushort( state->buffer );
				tds_texcoord_buffer_grow( state, state->item_count );

				state->parsing_state = TDS_STATE_READ_TEXCOORDS;
				state->buffer_length = 0;
				state->counter = 0;
			}
			++i;
			break;

		case TDS_STATE_READ_TEXCOORDS:
			tds_buffer_add( state, c );

			if( state->buffer_length == 8 )
			{
				tds_texcoord_buffer_add(
					state,
					tds_read_le_float( state->buffer ),
					tds_read_le_float( state->buffer + 4 ) );

				state->counter++;
				state->buffer_length = 0;

				if( state->counter >= state->item_count )
					state->parsing_state = TDS_STATE_READ_CHUNK_ID;
			}

			++i;
			break;

		case TDS_STATE_READ_FACE_COUNT:
			tds_buffer_add( state, c );

			if( state->buffer_length == 2 )
			{
				state->item_count = tds_read_le_ushort( state->buffer );
				tds_face_buffer_grow( state, state->item_count );

				state->object_buffer[state->object_count-1]->count
					= state->item_count;

				if( state->object_count > 1 )
					state->object_buffer[state->object_count - 1]->index
						= state->object_buffer[state->object_count-2]->index
						+  state->object_buffer[state->object_count-2]->count;


				state->parsing_state = TDS_STATE_READ_FACES;
				state->buffer_length = 0;
				state->counter = 0;

				if( state->counter >= state->item_count )
					state->parsing_state = TDS_STATE_READ_CHUNK_ID;
			}
			++i;

			break;

		case TDS_STATE_READ_FACES:
			tds_buffer_add( state, c );

			if( state->buffer_length == 8 )
			{
				tds_face_buffer_add(
					state,
					tds_read_le_ushort( state->buffer ),
					tds_read_le_ushort( state->buffer + 2),
					tds_read_le_ushort( state->buffer + 4 ) );

				state->counter++;
				state->buffer_length = 0;

				if( state->counter >= state->item_count )
					state->parsing_state = TDS_STATE_READ_CHUNK_ID;
			}

			++i;
			break;

		case TDS_STATE_READ_MATERIAL_NAME:
			tds_buffer_add( state, c );

			if( c == 0 )
			{
                tds_material_buffer_add(state, state->buffer);

				/* continue with chunks */
				state->parsing_state = TDS_STATE_READ_CHUNK_ID;
				state->buffer_length = 0;
			}
            ++i;
			break;

		case TDS_STATE_READ_MATERIAL_COLOR:
			tds_buffer_add( state, c );

			if( state->buffer_length == state->chunk_length - 6 )
			{
				float   color[3];

                if( state->buffer[0] == 0x10 )
				{
					color[0] = tds_read_le_float( state->buffer + 6 );
					color[1] = tds_read_le_float( state->buffer + 10 );
					color[2] = tds_read_le_float( state->buffer + 14 );
				}
				else if( state->buffer[0] == 0x11 )
				{
                   color[0] = (float)((unsigned char)(state->buffer[6])) / 255.0f;
                   color[1] = (float)((unsigned char)(state->buffer[7])) / 255.0f;
                   color[2] = (float)((unsigned char)(state->buffer[8])) / 255.0f;
                }

                tds_material_set_property(state, color);

                state->parsing_state = TDS_STATE_READ_CHUNK_ID;
				state->buffer_length = 0;
			}
            ++i;
			break;

		case TDS_STATE_READ_MATERIAL_PROPERTY:
			tds_buffer_add( state, c );

			if( state->buffer_length == state->chunk_length - 6 )
			{
				float param;

                if( state->buffer[0] == 0x30 ) /* percent int */
				{
                    unsigned short percent = tds_read_le_ushort( state->buffer + 6 );
                    param = (float)percent / 100.0f;
                }
				else if( state->buffer[0] == 0x11 ) /* percent float */
				{
                    float percent = tds_read_le_float( state->buffer + 6 );
                    param = percent / 100.0f;
                }

                tds_material_set_property(state, &param);
				state->parsing_state = TDS_STATE_READ_CHUNK_ID;
				state->buffer_length = 0;
			}
            ++i;
			break;

		case TDS_STATE_READ_MATERIAL_LIST_NAME:
			tds_buffer_add( state, c );

			if( c == 0 )
			{
				tds_material_reference_buffer_add( state, state->buffer );

                /* continue with chunks */
				state->parsing_state = TDS_STATE_READ_MATERIAL_LIST_COUNT;
				state->buffer_length = 0;
			}
            ++i;
			break;

		case TDS_STATE_READ_MATERIAL_LIST_COUNT:
			tds_buffer_add( state, c );

			if( state->buffer_length == 2 )
			{
				state->item_count = tds_read_le_ushort( state->buffer );
                tds_material_reference_set_range(state, state->last_material_face, state->item_count);
                state->last_material_face += state->item_count;

				state->parsing_state = TDS_STATE_READ_MATERIAL_LIST;
				state->buffer_length = 0;
				state->counter = 0;
			}
            ++i;
			break;

		case TDS_STATE_READ_MATERIAL_LIST:
			tds_buffer_add( state, c );

			if( state->buffer_length == 2 )
			{
				/* we don't care about the actual face list, we assume that materials refs come in the right order */
                state->counter++;
				state->buffer_length = 0;

 				if( state->counter >= state->item_count )
 					state->parsing_state = TDS_STATE_READ_CHUNK_ID;
 			}
 			++i;
 			break;

		case TDS_STATE_READ_MAP_NAME:
			tds_buffer_add( state, c );

			if( c == 0 )
			{
				/* continue with chunks */
				state->parsing_state = TDS_STATE_READ_CHUNK_ID;
				state->buffer_length = 0;
			}
            ++i;
			break;

		case TDS_STATE_SKIP_CHUNK:
			++i;
			++state->counter;
			if( state->counter >= state->chunk_length )
				state->parsing_state = TDS_STATE_READ_CHUNK_ID;
			break;

		default:
			++i;
			break;
		}
	}

	if( last )
		state->parsing_state = TDS_STATE_DONE;

	return 0;
}


/*----------------------------------------------------------------------------*/
unsigned int tl3dsObjectCount( tl3dsState *state )
{
	if( state == NULL )
		return 0;

	if( state->parsing_state != TDS_STATE_DONE )
		return 0;

	return state->object_count;
}


/*----------------------------------------------------------------------------*/
const char *tl3dsObjectName( tl3dsState *state, unsigned int object )
{
	if( state == NULL )
		return NULL;

	if( state->parsing_state != TDS_STATE_DONE )
		return NULL;

	if( object >= state->object_count )
		return NULL;

	return state->object_buffer[object]->name;
}


/*----------------------------------------------------------------------------*/
unsigned int tl3dsObjectFaceCount( tl3dsState *state, unsigned int object )
{

	if( state == NULL )
		return 0;

	if( state->parsing_state != TDS_STATE_DONE )
		return 0;

	if( object >= state->object_count )
		return 0;

	return state->object_buffer[object]->count;
}


/*----------------------------------------------------------------------------*/
unsigned int tl3dsObjectFaceIndex( tl3dsState *state, unsigned int object )
{

	if( state == NULL )
		return 0;

	if( state->parsing_state != TDS_STATE_DONE )
		return 0;

	if( object >= state->object_count )
		return 0;

	return state->object_buffer[object]->index;
}


/*----------------------------------------------------------------------------*/
unsigned int tl3dsMaterialCount( tl3dsState *state )
{
	if( state == NULL )
		return 0;

	if( state->parsing_state != TDS_STATE_DONE )
		return 0;

    return state->material_count;
}


/*----------------------------------------------------------------------------*/
const char *tl3dsMaterialName(   tl3dsState *state,
                                 unsigned int material )
{
	if( state == NULL )
		return NULL;

	if( state->parsing_state != TDS_STATE_DONE )
		return NULL;

	if( material >= state->material_count )
		return NULL;

    return state->material_buffer[material].name;
}

/*----------------------------------------------------------------------------*/
int tl3dsGetMaterial(    tl3dsState *state,
                         unsigned int index,
                         float *ambient, float *diffuse, float *specular,
                         float *shininess )
{
	if( state == NULL )
		return 1;

	if( index >= state->material_count )
		return 1;

	if( ambient )
		memcpy(ambient, state->material_buffer[index].ambient, sizeof(float) * 4);

	if( diffuse )
    	memcpy(diffuse, state->material_buffer[index].diffuse, sizeof(float) * 4);

    if( specular )
    	memcpy(specular, state->material_buffer[index].specular, sizeof(float) * 4);

    if( shininess )
    	*shininess    = state->material_buffer[index].shininess;

    return 0;
}

/*----------------------------------------------------------------------------*/
unsigned int tl3dsMaterialReferenceCount( tl3dsState *state )
{
	if( state == NULL )
		return 0;

    return state->material_reference_count;
}

/*----------------------------------------------------------------------------*/
const char *tl3dsMaterialReferenceName(   tl3dsState *state,
                               unsigned int object )
{
	if( state == NULL )
		return NULL;

	if( object >= state->material_reference_count )
		return NULL;

    return state->material_reference_buffer[object].name;
}

/*----------------------------------------------------------------------------*/
int tl3dsGetMaterialReference(    tl3dsState *state,
                             unsigned int index,
                             unsigned int *face_index,
                             unsigned int *face_count)
{
	if( state == NULL )
		return 1;

	if( index >= state->material_reference_count )
		return 1;

    if( face_index )
    	*face_index = state->material_reference_buffer[index].face_index;

    if( face_count )
    	*face_count = state->material_reference_buffer[index].face_count;

    return 0;

}

/*----------------------------------------------------------------------------*/
unsigned int tl3dsVertexCount( tl3dsState *state )
{
	if( state == NULL )
		return 0;

	if( state->parsing_state != TDS_STATE_DONE )
		return 0;

	return state->point_count;
}


/*----------------------------------------------------------------------------*/
int tl3dsGetVertexDouble(
	tl3dsState *state,
	unsigned int index,
	double *x, double *y, double *z,
	double *tu, double *tv,
	double *nx, double *ny, double *nz )
{
	if( state == NULL )
		return 1;

	if( index >= state->point_count )
		return 1;

	if( state->point_buffer && index < state->point_count )
	{
		if( x )
			*x = (float)state->point_buffer[ index * 3 ];

		if( y )
			*y = (float)state->point_buffer[ index * 3 + 1];

		if( z )
			*z = (float)state->point_buffer[ index * 3 + 2];
	}

	if( state->texcoord_buffer && index < state->texcoord_count )
	{
		if( tu )
			*tu = (float)state->texcoord_buffer[ index * 2 ];

		if( tv )
			*tv = (float)state->texcoord_buffer[ index * 2 + 1];
	}

	if( nx )
		*nx = 0;

	if( ny )
		*ny = 0;

	if( nz )
		*nz = 0;

	return 0;
}


/*----------------------------------------------------------------------------*/
int tl3dsGetVertex(
	tl3dsState *state,
	unsigned int index,
	float *x, float *y, float *z,
	float *tu, float *tv,
	float *nx, float *ny, float *nz )
{
	if( state == NULL )
		return 1;

	if( index >= state->point_count )
		return 1;

	if( state->point_buffer && index < state->point_count )
	{
		if( x )
			*x = (float)state->point_buffer[ index * 3 ];

		if( y )
			*y = (float)state->point_buffer[ index * 3 + 1];

		if( z )
			*z = (float)state->point_buffer[ index * 3 + 2];
	}

	if( state->texcoord_buffer && index < state->texcoord_count )
	{
		if( tu )
			*tu = (float)state->texcoord_buffer[ index * 2 ];

		if( tv )
			*tv = (float)state->texcoord_buffer[ index * 2 + 1];
	}

	if( nx )
		*nx = 0;

	if( ny )
		*ny = 0;

	if( nz )
		*nz = 0;

	return 0;
}


/*----------------------------------------------------------------------------*/
unsigned int tl3dsFaceCount( tl3dsState *state )
{
	if( state == NULL )
		return 0;

	return state->face_count;
}


/*----------------------------------------------------------------------------*/
int tl3dsGetFaceInt(
	tl3dsState *state,
	unsigned int index,
	unsigned int *a,
	unsigned int *b,
	unsigned int *c )
{
	unsigned int face = 0;

	if( state == NULL )
		return 1;

	if( state->face_buffer && face <= state->face_count )
	{
		if( a )
			*a = state->face_buffer[ index * 3 ];

		if( b )
			*b = state->face_buffer[ index * 3 + 1 ];

		if( c )
			*c = state->face_buffer[ index * 3 + 2 ];
	}

	return 0;
}

/*----------------------------------------------------------------------------*/
int tl3dsGetFace(
	tl3dsState *state,
	unsigned int index,
	unsigned short *a,
	unsigned short *b,
	unsigned short *c )
{
	unsigned int face = 0;

	if( state == NULL )
		return 1;

	if( state->face_buffer && face <= state->face_count )
	{
		if( a )
			*a = state->face_buffer[ index * 3 ];

		if( b )
			*b = state->face_buffer[ index * 3 + 1 ];

		if( c )
			*c = state->face_buffer[ index * 3 + 2 ];
	}

	return 0;
}


/*----------------------------------------------------------------------------*/
int tl3dsCheckFileExtension( const char *filename )
{
	const char *ext = 0, *tmp = filename;

	if( filename == NULL )
		return 1;

	while( *tmp != 0 )
	{
		if( *tmp == '.' )
			ext = tmp + 1;

		tmp++;
	}

	/* no extension found */
	if( ext == 0 )
		return 1;

	if( (ext[0] == '3')
		&& (ext[1] == 'd' || ext[1] == 'D')
		&& (ext[2] == 's' || ext[2] == 'S')
		&& (ext[3] == 0) )
		return 0;

	return 1;
}

/*----------------------------------------------------------------------------*/
unsigned int tl3dsHasNormals( tl3dsState *state )
{
    return 0;
}
