/*
 *			GPAC - Multimedia Framework C SDK
 *
 *			Copyright (c) Jean Le Feuvre 2000-2005 
 *					All rights reserved
 *
 *  This file is part of GPAC / Authoring Tools sub-project
 *
 *  GPAC is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *   
 *  GPAC is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *   
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA. 
 *
 */

#ifndef _GF_SCENE_MANAGER_H_
#define _GF_SCENE_MANAGER_H_


#ifdef __cplusplus
extern "C" {
#endif

#include <gpac/isomedia.h>
#include <gpac/scenegraph_vrml.h>

/*
		Memory scene management

*/

/*NDT check - return 1 if node belongs to given NDT. Handles proto, and assumes undefined nodes
always belong to the desired NDT*/
Bool gf_node_in_table(GF_Node *node, u32 NDTType);

/*generic systems access unit context*/
typedef struct
{	
	/*AU timing in TimeStampResolution*/
	u64 timing;
	/*timing in sec - used if timing isn't set*/
	Double timing_sec;
	/*random access indication - may be overriden by encoder*/
	Bool is_rap;
	/*opaque command list per stream type*/
	GF_List *commands;

	/*pointer to owner stream*/
	struct _stream_context *owner;
} GF_AUContext;

/*generic stream context*/
typedef struct _stream_context
{
	/*ESID of stream, or 0 if unknown in which case it is automatically updated at encode stage*/
	u16 ESID;
	/*stream type - used as a hint, the encoder(s) may override it*/
	u8 streamType;
	u8 objectType;
	u32 timeScale;
	GF_List *AUs;

	/*last stream AU time, when playing the context directly*/
	u64 last_au_time;
} GF_StreamContext;

/*generic presentation context*/
typedef struct 
{
	/*the one and only scene graph used by the scene manager.*/
	GF_SceneGraph *scene_graph;

	/*all systems streams used in presentation*/
	GF_List *streams;
	/*(initial) object descriptor if any - if not set the encoder will generate it*/
	GF_ObjectDescriptor *root_od;

	/*scene resolution*/
	u32 scene_width, scene_height;
	Bool is_pixel_metrics;

	/*BIFS encoding - these is needed for:
	- protos in protos which define subscene graph, hence seperate namespace, but are coded with the same IDs
	- route insertions which are not tracked by the scene graph
	we could do this by hand (counting protos & route insert) but let's be lazy
	*/
	u32 max_node_id, max_route_id, max_proto_id;
} GF_SceneManager;

/*scene manager constructor - @scene_graph: scene graph used by the manager. */
GF_SceneManager *gf_sm_new(GF_SceneGraph *scene_graph);
/*scene manager destructor - does not destroy the attached scene graph*/
void gf_sm_del(GF_SceneManager *ctx);
/*retrive or create a stream context in the presentation context
WARNING: if a stream with the same streamType and no ESID already exists in the context, 
it is assigned the requested ES_ID - this is needed to solve base layer*/
GF_StreamContext *gf_sm_stream_new(GF_SceneManager *ctx, u16 ES_ID, u8 streamType, u8 objectType);
/*removes and destroy stream context from presentation context*/
void gf_sm_stream_del(GF_SceneManager *ctx, GF_StreamContext *sc);
/*locate a stream based on its id*/
GF_StreamContext *gf_sm_stream_find(GF_SceneManager *ctx, u16 ES_ID);
/*create a new AU context in the given stream context*/
GF_AUContext *gf_sm_stream_au_new(GF_StreamContext *stream, u64 timing, Double time_ms, Bool isRap);


/*applies all commands in all streams (only BIFS for now): the context manager will only have one command per
stream, this command being a random access*/
GF_Err gf_sm_make_random_access(GF_SceneManager *ctx);

/*translates SRT/SUB (TTXT not supported) source into BIFS command stream source
	@src: GF_ESD of new stream (MUST be created before to store TS resolution)
	@mux: GF_MuxInfo of src stream - shall contain a valid file, and at least the textNode member set
*/
GF_Err gf_sm_import_bifs_subtitle(GF_SceneManager *ctx, GF_ESD *src, GF_MuxInfo *mux);


/*SWF to MPEG-4 flags*/
enum
{
	/*all data in dictionary is in first frame*/
	GF_SM_SWF_STATIC_DICT = 1,
	/*remove all text*/
	GF_SM_SWF_NO_TEXT = (1<<1),
	/*remove embedded fonts (force device font usage)*/
	GF_SM_SWF_NO_FONT = (1<<2),
	/*forces XCurve2D which supports quadratic bezier*/
	GF_SM_SWF_QUAD_CURVE = (1<<3),
	/*forces line remove*/
	GF_SM_SWF_NO_LINE = (1<<4),
	/*forces XLineProperties (supports scalable lines)*/
	GF_SM_SWF_SCALABLE_LINE = (1<<5),
	/*forces gradient remove (using center color) */
	GF_SM_SWF_NO_GRADIENT = (1<<6),
	/*use a dedicated BIFS stream to control display list. This allows positioning in the movie
	(jump to frame, etc..) as well as looping from inside the movie (set by default)*/
	GF_SM_SWF_SPLIT_TIMELINE = (1<<7),
	/*when using SplitTimeline, this flag will prevent generating an AnimationStream in the scene (this is used
	by direct playback only)*/
	GF_SM_SWF_NO_ANIM_STREAM = (1<<8),
	/*enable appearance reuse*/
	GF_SM_SWF_REUSE_APPEARANCE = (1<<9)
};

/*general loader flags*/
enum
{
	/*if set, always load MPEG-4 nodes, otherwise X3D versions are used for vrml/x3d*/
	GF_SM_LOAD_MPEG4_STRICT = 1,
	/*signal loading is done for playback:	
		scrips will be queued in their parent command for later loading
		SFTime (MPEG-4 only) fields will be handled correctly when inserting/creating nodes based on AU timing
	*/
	GF_SM_LOAD_FOR_PLAYBACK = 2,

	/*special flag indicating that the context is already loaded & valid (eg no default stream creations & co)
	this is used when performing diff encoding (eg the file to load only has updates).
	When set, gf_sm_load_init will NOT attempt to parse first frame*/
	GF_SM_LOAD_CONTEXT_READY = 4
};

/*loader type, usually detected based on file ext*/
enum
{	
	GF_SM_LOAD_BT = 1, /*BT loader*/
	GF_SM_LOAD_VRML, /*VRML97 loader*/
	GF_SM_LOAD_X3DV, /*X3D VRML loader*/
	GF_SM_LOAD_XMTA, /*XMT-A loader*/
	GF_SM_LOAD_X3D, /*X3D XML loader*/
	GF_SM_LOAD_SVG_DA, /*SVG loader with dynamic allocation of attributes */
	GF_SM_LOAD_XSR, /*LASeR+XML loader*/
#ifdef GPAC_ENABLE_SVG_SA
	GF_SM_LOAD_SVG_SA, /* SVG loader with static allocation of attributes */
#endif
#ifdef GPAC_ENABLE_SVG_SANI
	GF_SM_LOAD_SVG_SANI, /*SVG loader with static allocation of attributes and no property inheritance */
#endif
	GF_SM_LOAD_SWF, /*SWF->MPEG-4 converter*/
	GF_SM_LOAD_QT, /*MOV->MPEG-4 converter (only cubic QTVR for now)*/
	GF_SM_LOAD_MP4 /*MP4 memory loader*/
};

typedef struct
{
	/*scene graph worked on - may be NULL if ctx is present*/
	GF_SceneGraph *scene_graph;
	/*context manager to load (MUST BE RESETED BEFORE if needed) - may be NULL for loaders not using commands, 
	in which case the graph will be directly updated*/
	GF_SceneManager *ctx;
	/*file to import except IsoMedia files*/
	const char *fileName;
	/*IsoMedia file to import (we need to be able to load from an opened file for scene stats)*/
	GF_ISOFile *isom;
	/*swf import flags*/
	u32 swf_import_flags;
	/*swf flatten limit: angle limit below which 2 lines are considered as aligned, 
	in which case the lines are merged as one. If 0, no flattening happens*/
	Float swf_flatten_limit;
	/*swf extraction path: if set, swf media (mp3, jpeg) are extracted to this path. If not set
	media are extracted to original file directory*/
	const char *localPath;

	/*loader flags*/
	u32 flags;

	/*private to loader*/
	void *loader_priv;
	/*loader type, one of the above value. If not set, detected based on file extension*/
	u32 type;
} GF_SceneLoader;

/*initializes the context loader - this will load any IOD and the first frame of the main scene*/
GF_Err gf_sm_load_init(GF_SceneLoader *load);
/*completely loads context*/
GF_Err gf_sm_load_run(GF_SceneLoader *load);
/*terminates the context loader*/
void gf_sm_load_done(GF_SceneLoader *load);

/*parses memory scene (any textural format) into the context
!! THE LOADER TYPE MUST BE ASSIGNED (BT/WRL/XMT/X3D/SVG only) !!
The string MUST be at least 4 bytes long in order to detect BOM (unicode encoding). 
The string can ba either UTF-8 or UTF-16 data
if clean_at_end is set, associated parser is destroyed. Otherwise, a call to gf_sm_load_done must be done 
to clean ressources (needed for SAX progressive loading)
*/
GF_Err gf_sm_load_string(GF_SceneLoader *load, char *str, Bool clean_at_end);


/*scene dump mode*/
enum
{
	/*BT*/
	GF_SM_DUMP_BT = 0,
	/*XMT-A*/
	GF_SM_DUMP_XMTA,
	/*VRML Text (WRL)*/
	GF_SM_DUMP_VRML,
	/*X3D Text (x3dv)*/
	GF_SM_DUMP_X3D_VRML,
	/*X3D XML*/
	GF_SM_DUMP_X3D_XML,
	/*LASeR XML*/
	GF_SM_DUMP_LASER,
	/*SVG dump (only dumps svg root of the first LASeR unit*/
	GF_SM_DUMP_SVG,
	/*automatic selection of MPEG4 vs X3D, text mode*/
	GF_SM_DUMP_AUTO_TXT,
	/*automatic selection of MPEG4 vs X3D, xml mode*/
	GF_SM_DUMP_AUTO_XML,
};

#ifndef GPAC_READ_ONLY

/*dumps scene context to BT or XMT
@rad_name: file name & loc without extension - if NULL dump will happen in stdout
@dump_mode: one of the above*/
GF_Err gf_sm_dump(GF_SceneManager *ctx, char *rad_name, u32 dump_mode);

#endif


/*encoding flags*/
enum
{
	/*if flag set, DEF names are encoded*/
	GF_SM_ENCODE_USE_NAMES =	1,
	/*if flag set, RAP are generated inband rather than as redundant samples*/
	GF_SM_ENCODE_RAP_INBAND = 2,
	/*if flag set, RAP are generated inband rather than as sync shadow samples*/
	GF_SM_ENCODE_RAP_SHADOW = 4,
};

typedef struct
{
	/*encoding flags*/
	u32 flags;
	/*delay between 2 RAP in ms. If 0 RAPs are not forced - BIFS and LASeR only for now*/
	u32 rap_freq;
	/*if set, any unknown stream in the scene will be looked for in @mediaSource (MP4 only)*/
	char *mediaSource;
	/*LASeR */
	/*resolution*/
	s32 resolution;
	/*coordBits, scaleBits*/
	u32 coord_bits, scale_bits;
	Bool auto_qant;
} GF_SMEncodeOptions;

/*
encodes scene context into @mp4.
if @log is set, generates BIFS encoder log file
*/
GF_Err gf_sm_encode_to_file(GF_SceneManager *ctx, GF_ISOFile *mp4, GF_SMEncodeOptions *opt);

/*Dumping tools*/
typedef struct _scenedump GF_SceneDumper;
/*create a scene dumper 
@graph: scene graph being dumped
@rad_name: file radical (NULL for stdout) - if not NULL MUST BE GF_MAX_PATH length
@indent_char: indent format
@XMLDump: if set, dumps in XML format otherwise regular text
returns NULL if can't create a file
*/
GF_SceneDumper *gf_sm_dumper_new(GF_SceneGraph *graph, char *rad_name, char indent_char, Bool XMLDump);
void gf_sm_dumper_del(GF_SceneDumper *bd);

/*dumps commands list
@indent: indent to use
@skip_replace_scene_header: if set and dumping in BT mode, the initial REPLACE SCENE header is skipped
*/
GF_Err gf_sm_dump_command_list(GF_SceneDumper *sdump, GF_List *comList, u32 indent, Bool skip_first_replace);

/*dumps complete graph - 
@skip_proto: proto declarations are skipped
@skip_routes: routes are not dumped
*/
GF_Err gf_sm_dump_graph(GF_SceneDumper *sdump, Bool skip_proto, Bool skip_routes);


#ifndef GPAC_READ_ONLY

/*stat object - to refine :)*/

/*store nodes or proto stats*/
typedef struct
{
	/*node type or protoID*/
	u32 tag;
	const char *name;
	/*number of created nodes*/
	u32 nb_created;
	/*number of used nodes*/
	u32 nb_used;
	/*number of used nodes*/
	u32 nb_del;
} GF_NodeStats;

typedef struct _scenestat
{
	GF_List *node_stats;
	GF_List *proto_stats;
	
	/*ranges of all SFVec2fs for points only (MFVec2fs)*/
	SFVec2f max_2d, min_2d;
	/* resolution of 2D points (nb bits for integer part and decimal part)*/
	u32 int_res_2d, frac_res_2d;
	/* resolution of scale coefficient (nb bits for integer part)*/
	u32 scale_int_res_2d, scale_frac_res_2d;

	Fixed max_fixed, min_fixed;

	/*number of parsed 2D points*/
	u32 count_2d;
	/*number of deleted 2D points*/
	u32 rem_2d;

	/*ranges of all SFVec3fs for points only (MFVec3fs)*/
	SFVec3f max_3d, min_3d;
	u32 count_3d;
	/*number of deleted 3D points*/
	u32 rem_3d;

	u32 count_float, rem_float;
	u32 count_color, rem_color;
	/*all SFVec2f other than MFVec2fs elements*/
	u32 count_2f;
	/*all SFVec3f other than MFVec3fs elements*/
	u32 count_3f;
} GF_SceneStatistics;

typedef struct _statman GF_StatManager;

/*creates new stat handler*/
GF_StatManager *gf_sm_stats_new();
/*deletes stat handler*/
void gf_sm_stats_del(GF_StatManager *stat);
/*reset statistics*/
void gf_sm_stats_reset(GF_StatManager *stat);

/*get statistics - do NOT modify the returned structure*/
GF_SceneStatistics *gf_sm_stats_get(GF_StatManager *stat);

/*produces stat report for a complete graph*/
GF_Err gf_sm_stats_for_graph(GF_StatManager *stat, GF_SceneGraph *sg);

/*produces stat report for the full scene*/
GF_Err gf_sm_stats_for_scene(GF_StatManager *stat, GF_SceneManager *sm);

/*produces stat report for the given command*/
GF_Err gf_sm_stats_for_command(GF_StatManager *stat, GF_Command *com);

#endif


#ifdef __cplusplus
}
#endif


#endif	/*_GF_SCENE_MANAGER_H_*/

