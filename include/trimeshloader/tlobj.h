/*
 * Copyright (c) 2007-2017 Gero Mueller <post@geromueller.de>
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

#ifndef TRIMESH_LOADER_OBJ_H
#define TRIMESH_LOADER_OBJ_H

/** 
 @file  tlobj.h
 @brief Trimeshloader OBJ parser public header file
*/

#ifdef __cplusplus
extern "C" {
#endif

#ifndef TRIMESH_LOADER_EXPORT
	#define TRIMESH_LOADER_API
#else
	#define TRIMESH_LOADER_API extern
#endif

/** @defgroup low_level_obj_api Trimeshloader low level OBJ API
 * @{
 */
 
typedef struct tlObjState tlObjState;

/* state handling */
TRIMESH_LOADER_API tlObjState *tlObjCreateState();

TRIMESH_LOADER_API int tlObjResetState( tlObjState *state );

TRIMESH_LOADER_API void tlObjDestroyState( tlObjState *state );

/* parsing */
TRIMESH_LOADER_API int tlObjParse(
	tlObjState *state,
	const char *buffer,
	unsigned int length,
	int last );

/* data access */
TRIMESH_LOADER_API unsigned int tlObjObjectCount( tlObjState *state );

TRIMESH_LOADER_API const char *tlObjObjectName(
	tlObjState *state,
	unsigned int object );

TRIMESH_LOADER_API unsigned int tlObjObjectFaceCount(
	tlObjState *state,
	unsigned int object );

TRIMESH_LOADER_API unsigned int tlObjObjectFaceIndex(
	tlObjState *state,
	unsigned int object );

TRIMESH_LOADER_API unsigned int tlObjMaterialCount( tlObjState *state );

TRIMESH_LOADER_API const char *tlObjMaterialName(
	tlObjState *state,
	unsigned int object );

TRIMESH_LOADER_API unsigned int tlObjMaterialLibCount( tlObjState *state );

TRIMESH_LOADER_API const char *tlObjMaterialLibName(
	tlObjState *state,
	unsigned int object );

TRIMESH_LOADER_API int tlObjGetMaterial(
	tlObjState *state,
	unsigned int index,
	float *ambient,
	float *diffuse,
	float *specular,
	float *reflect );

TRIMESH_LOADER_API unsigned int tlObjMaterialReferenceCount( tlObjState *state );

TRIMESH_LOADER_API const char *tlObjMaterialReferenceName( 
	tlObjState *state,
	unsigned int object );

TRIMESH_LOADER_API int tlObjGetMaterialReference( 
	tlObjState *state,
	unsigned int index,
	unsigned int *face_index,
	unsigned int *face_count );

TRIMESH_LOADER_API unsigned int tlObjVertexCount( tlObjState *state );

TRIMESH_LOADER_API int tlObjGetVertexDouble(
	tlObjState *state,
	unsigned int index,
	double *x, double *y, double *z,
	double *tu, double *tv,
	double *nx, double *ny, double *nz );
	
TRIMESH_LOADER_API int tlObjGetVertex(
	tlObjState *state,
	unsigned int index,
	float *x, float *y, float *z,
	float *tu, float *tv,
	float *nx, float *ny, float *nz );
	
TRIMESH_LOADER_API unsigned int tlObjFaceCount(
	tlObjState *state );

TRIMESH_LOADER_API int tlObjGetFaceInt(
	tlObjState *state,
	unsigned int index,
	unsigned int *a,
	unsigned int *b,
	unsigned int *c );
	
TRIMESH_LOADER_API int tlObjGetFace(
	tlObjState *state,
	unsigned int index,
	unsigned short *a,
	unsigned short *b,
	unsigned short *c );

TRIMESH_LOADER_API int tlObjCheckFileExtension( const char *filename );

/** Check if the loaded mesh has normals.
 * \param state a previously created state.
 * \return Returns 0 if no normals are present, >0 if they are.
 */
TRIMESH_LOADER_API unsigned int tlObjHasNormals( tlObjState *state );

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif
