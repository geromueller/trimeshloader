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

/** \mainpage trimeshloader-0.1.0
 * \section project_page Project Page
 * \url http://sourceforge.net/projects/trimeshloader
 * \section website Website with tutorials
 * \url http://trimeshloader.sourceforge.net
 */

/**
 @file  trimeshloader.h
 @brief Trimeshloader public header file
*/

#ifndef TRIMESH_LOADER_H
#define TRIMESH_LOADER_H

#include "tlobj.h"
#include "tl3ds.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef TRIMESH_LOADER_EXPORT
	#define TRIMESH_LOADER_API
#else
	#define TRIMESH_LOADER_API extern
#endif


/** @defgroup high_level_api Trimeshloader high level API
 * @{
 */

/** Structure describing an Object (or SubMesh, Batch) */
typedef struct tlObject
{
	/** Name of the Object */
	char *name;

	/** First face in the index list */
	unsigned int face_index;

	/** Face count */
	unsigned int face_count;

} tlObject;

/** Used as format flag in loading functions: load the position of the vertex */
#define TL_FVF_XYZ 1

/** Used as format flag in loading functions: load the texturecoordinate of the vertex */
#define TL_FVF_UV 2

/** Used as format flag in loading functions: load the normal of the vertex */
#define TL_FVF_NORMAL 4

/** Structure describing a Material (Colors and/or Texture) */
typedef struct tlMaterial
{
	/** Name of the Material */
	char    *name;

	/** RGBA Colors */
	float   ambient[4], diffuse[4], specular[4];

	/** Shininess */
	float   shininess;

} tlMaterial;

/** Structure describing the reference to a Material */
typedef struct tlMaterialReference
{
	/** Name of the Material */
	char *name;

	/** First face in the index list */
	unsigned int face_index;

	/** Face count */
	unsigned int face_count;

} tlMaterialReference;

/** Structure describing an Trimesh (index triangle list) containing objects, vertices (point, texture coordinate and normal) and triangle indices */
typedef struct tlTrimesh
{
	/** pointer to the vertex data */
	float *vertices;

	/** number of vertices */
	unsigned int vertex_count;

	/** format of the vertices */
	unsigned int vertex_format;

	/** size/stride of each vertex, in bytes */
	unsigned int vertex_size;

	/** pointer to the face (triangle) indices (3 unsigned shorts) */
	unsigned short *faces;

	/** number of faces */
	unsigned int face_count;

	/** list of objects in this trimesh */
	tlObject *objects;

	/** number of objects */
	unsigned int object_count;

	/** list of materials in this trimesh */
	tlMaterial *materials;

	/** number of materials */
	unsigned int material_count;

	/** list of references to materials in this trimesh */
	tlMaterialReference *material_references;

	/** number of references to materials */
	unsigned int material_reference_count;

} tlTrimesh;


/** Load a 3DS file in an tlTrimesh structure
 * \param filename Pointer to NULL-terminated string containing the filename
 * \param vertex_format Defines the vertex format. any format combination of TL_FVF_XYZ, TL_FVF_UV, TL_FVF_NORMAL
 * \return Returns a new tlTrimesh object, which needs to be deleted with tlDeleteTrimesh. NULL on error.
 */
TRIMESH_LOADER_API tlTrimesh *tlLoad3DS( const char*filename, unsigned int vertex_format );


/** Load a OBJ file in an tlTrimesh structure
 * \param filename Pointer to NULL-terminated string containing the filename
 * \param vertex_format Defines the vertex format. any format combination of TL_FVF_XYZ, TL_FVF_UV, TL_FVF_NORMAL
 * \return Returns a new tlTrimesh object, which needs to be deleted with tlDeleteTrimesh. NULL on error.
 */
TRIMESH_LOADER_API tlTrimesh *tlLoadOBJ( const char*filename, unsigned int vertex_format );

/** Create an a tlTrimesh structure from a tlObjState
 * \param state Pointer to state after parsing.
 * \param vertex_format Defines the vertex format. any format combination of TL_FVF_XYZ, TL_FVF_UV, TL_FVF_NORMAL
 * \return Returns a new tlTrimesh object, which needs to be deleted with tlDeleteTrimesh. NULL on error.
 */
TRIMESH_LOADER_API tlTrimesh *tlCreateTrimeshFromObjState( tlObjState *state, unsigned int vertex_format );

/** Create an a tlTrimesh structure from a tl3dsState
 * \param state Pointer to state after parsing.
 * \param vertex_format Defines the vertex format. any format combination of TL_FVF_XYZ, TL_FVF_UV, TL_FVF_NORMAL
 * \return Returns a new tlTrimesh object, which needs to be deleted with tlDeleteTrimesh. NULL on error.
 */
TRIMESH_LOADER_API tlTrimesh *tlCreateTrimeshFrom3dsState( tl3dsState *state, unsigned int vertex_format );

TRIMESH_LOADER_API void tlTrimeshCreateNormals( tlTrimesh *trimesh );

/** Load an 3DS or OBJ file in an tlTrimesh structure. Automatic extension parsing is done.
 * \param filename Pointer to NULL-terminated string containing the filename
 * \param vertex_format Defines the vertex format. any format combination of TL_FVF_XYZ, TL_FVF_UV, TL_FVF_NORMAL
 * \return Returns a new tlTrimesh object, which needs to be deleted with tlDeleteTrimesh. NULL on error.
 */
TRIMESH_LOADER_API tlTrimesh *tlLoadTrimesh( const char*filename, unsigned int vertex_format );

/** Delete an previously loaded tlTrimesh object
 * \param trimesh Previously loaded tlTrimesh object
 */
TRIMESH_LOADER_API void tlDeleteTrimesh( tlTrimesh *trimesh );

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif /*TRIMESHLOADER_H_*/
