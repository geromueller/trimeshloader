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
 
#ifndef TRIMESH_LOADER_3DS_H
#define TRIMESH_LOADER_3DS_H

/** 
 @file  tl3ds.h
 @brief Trimeshloader 3DS parser public header file
*/

#ifdef __cplusplus
extern "C" {
#endif

#ifndef TRIMESH_LOADER_EXPORT
	#define TRIMESH_LOADER_API
#else
	#define TRIMESH_LOADER_API extern
#endif

/** @defgroup low_level_3ds_api Trimeshloader low level 3DS API
 * @{
 */

/** Structure describing the parsing state. the user has no direkt access to it. */
typedef struct tl3dsState tl3dsState;

/** Create a new parsing state.
 * \return A new parsing state, which needs to be deleted after parsing. NULL on error.
 */
TRIMESH_LOADER_API tl3dsState *tl3dsCreateState();

/** Reset the parsing state
 * \param state pointer to an previously created state.
 */
TRIMESH_LOADER_API int tl3dsResetState( tl3dsState *state );

/** Destroy a previously created state.
 * \param state pointer to an previously created state.
 */
TRIMESH_LOADER_API void tl3dsDestroyState( tl3dsState *state );

/** Parse a chunk of data.
 * \param state a previously created state.
 * \param buffer pointer to the chunk of data to be parsed
 * \param length number of bytes to be parsed
 * \param last indicator if this is the last chunk. 1 = yes, 0 = no. 
 * \return Returns 0 on success, 1 on error.
 */
TRIMESH_LOADER_API int tl3dsParse(
	tl3dsState *state,
	const char *buffer,
	unsigned int length,
	int last );

/* data access */
TRIMESH_LOADER_API unsigned int tl3dsObjectCount( tl3dsState *state );

TRIMESH_LOADER_API const char *tl3dsObjectName(
	tl3dsState *state,
	unsigned int object );

TRIMESH_LOADER_API unsigned int tl3dsObjectFaceCount(
	tl3dsState *state,
	unsigned int object );

TRIMESH_LOADER_API unsigned int tl3dsObjectFaceIndex(
	tl3dsState *state,
	unsigned int object );

TRIMESH_LOADER_API unsigned int tl3dsMaterialCount( tl3dsState *state );

TRIMESH_LOADER_API const char *tl3dsMaterialName(
	tl3dsState *state,
	unsigned int object );

TRIMESH_LOADER_API int tl3dsGetMaterial(
	tl3dsState *state,
	unsigned int index,
	float *ambient,
	float *diffuse,
	float *specular,
	float *reflect );

TRIMESH_LOADER_API unsigned int tl3dsMaterialReferenceCount( tl3dsState *state );

TRIMESH_LOADER_API const char *tl3dsMaterialReferenceName( 
	tl3dsState *state,
	unsigned int object );

TRIMESH_LOADER_API int tl3dsGetMaterialReference( 
	tl3dsState *state,
	unsigned int index,
	unsigned int *face_index,
	unsigned int *face_count );

TRIMESH_LOADER_API unsigned int tl3dsVertexCount( tl3dsState *state );

TRIMESH_LOADER_API int tl3dsGetVertexDouble(
	tl3dsState *state,
	unsigned int index,
	double *x, double *y, double *z,
	double *tu, double *tv,
	double *nx, double *ny, double *nz );
	
TRIMESH_LOADER_API int tl3dsGetVertex(
	tl3dsState *state,
	unsigned int index,
	float *x, float *y, float *z,
	float *tu, float *tv,
	float *nx, float *ny, float *nz );
	
TRIMESH_LOADER_API unsigned int tl3dsFaceCount(
	tl3dsState *state );

TRIMESH_LOADER_API int tl3dsGetFaceInt(
	tl3dsState *state,
	unsigned int index,
	unsigned int *a,
	unsigned int *b,
	unsigned int *c );
	
TRIMESH_LOADER_API int tl3dsGetFace(
	tl3dsState *state,
	unsigned int index,
	unsigned short *a,
	unsigned short *b,
	unsigned short *c );

TRIMESH_LOADER_API int tl3dsCheckFileExtension( const char *filename );

/** Check if the loaded mesh has normals. 3DS does not support normals. It is for convenience only, and always returns 0.
 * \param state a previously created state.
 * \return Returns 0 if no normals are present, >0 if they are.
 */
TRIMESH_LOADER_API unsigned int tl3dsHasNormals( tl3dsState *state );

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif
