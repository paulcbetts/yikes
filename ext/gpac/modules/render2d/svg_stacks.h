/*
 *			GPAC - Multimedia Framework C SDK
 *
 *			Copyright (c) Cyril Concolato 2004
 *					All rights reserved
 *
 *  This file is part of GPAC / SVG Rendering sub-project
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
#ifndef _SVG_STACKS_H
#define _SVG_STACKS_H

#include "render2d.h"
/* Reusing generic 2D stacks for rendering */
#include "stacks2d.h"

#ifndef GPAC_DISABLE_SVG

#include <gpac/nodes_svg_da.h>
#ifdef GPAC_ENABLE_SVG_SA
#include <gpac/nodes_svg_sa.h>
#endif
#ifdef GPAC_ENABLE_SVG_SANI
#include <gpac/nodes_svg_sani.h>
#endif


/*functions used by all implementation of SVG*/
void svg_render_node(GF_Node *node, RenderEffect2D *eff);
void svg_render_node_list(GF_ChildNodeItem *children, RenderEffect2D *eff);
void svg_get_nodes_bounds(GF_Node *self, GF_ChildNodeItem *children, RenderEffect2D *eff);

/* Creates a rendering context and Translates the SVG Styling properties into a context
   that the renderer can understand */
DrawableContext *SVG_drawable_init_context(Drawable *node, RenderEffect2D *eff);


#define BASE_IMAGE_STACK 	\
	GF_TextureHandler txh; \
	Drawable *graph; \
	MFURL txurl;	

typedef struct {
	BASE_IMAGE_STACK
} SVG_image_stack;

typedef struct
{
	BASE_IMAGE_STACK
	Bool first_frame_fetched;
	GF_Node *audio;
	Bool stop_requested;
} SVG_video_stack;

typedef struct
{
	GF_AudioInput input;
	Bool is_active;
	MFURL aurl;
} SVG_audio_stack;

typedef struct
{
	GF_TextureHandler txh;
	u32 *cols;
	Fixed *keys;
	u32 nb_col;
} SVG_GradientStack;

typedef struct 
{
	Drawable *draw;
	Fixed prev_size;
	u32 prev_flags;
	u32 prev_anchor;
	char *textToRender;
} SVG_TextStack;


/*regular SVG functions*/
void svg_render_base(GF_Node *node, SVGAllAttributes *all_atts, RenderEffect2D *eff, SVGPropertiesPointers *backup_props, u32 *backup_flags);
void svg_apply_local_transformation(RenderEffect2D *eff, SVGAllAttributes *atts, GF_Matrix2D *backup_matrix);
void svg_restore_parent_transformation(RenderEffect2D *eff, GF_Matrix2D *backup_matrix);

void svg_init_linearGradient(Render2D *sr, GF_Node *node);
void svg_init_radialGradient(Render2D *sr, GF_Node *node);
GF_TextureHandler *svg_gradient_get_texture(GF_Node *node);
void svg_init_solidColor(Render2D *sr, GF_Node *node);
void svg_init_stop(Render2D *sr, GF_Node *node);

void svg_init_svg(Render2D *sr, GF_Node *node);
void svg_init_g(Render2D *sr, GF_Node *node);
void svg_init_switch(Render2D *sr, GF_Node *node);
void svg_init_rect(Render2D *sr, GF_Node *node);
void svg_init_circle(Render2D *sr, GF_Node *node);
void svg_init_ellipse(Render2D *sr, GF_Node *node);
void svg_init_line(Render2D *sr, GF_Node *node);
void svg_init_polyline(Render2D *sr, GF_Node *node);
void svg_init_polygon(Render2D *sr, GF_Node *node);
void svg_init_path(Render2D *sr, GF_Node *node);
void svg_init_text(Render2D *sr, GF_Node *node);
void svg_init_a(Render2D *se, GF_Node *node);

/*WARNING - these are also used by SVG_SA and SVG_SANI*/
void svg_init_image(Render2D *se, GF_Node *node);
void svg_init_video(Render2D *se, GF_Node *node);
void svg_init_audio(Render2D *se, GF_Node *node, Bool slaved_timing);

/*moves to next/prev in the focus list. The start/end of the focus list is NULL, ie UA focus*/
u32 svg_focus_switch_ring(Render2D *sr, Bool move_prev);
/*moves focus to indicated node in the given direction, if any*/
u32 svg_focus_navigate(Render2D *sr, u32 key_code);
/*checks if the display is on or off*/
Bool svg_is_display_off(SVGPropertiesPointers *props);



/*common functions to SVG_SA and SVG_SANI*/
#ifdef GPAC_ENABLE_SVG_SA_BASE
void svg_sa_apply_local_transformation(RenderEffect2D *eff, GF_Node *node, GF_Matrix2D *backup_matrix);
void svg_sa_restore_parent_transformation(RenderEffect2D *eff, GF_Matrix2D *backup_matrix);
void svg_sa_render_base(GF_Node *node, RenderEffect2D *eff, SVGPropertiesPointers *backup_props, u32 *backup_flags);
#endif

/*SVG_SA functions */
#ifdef GPAC_ENABLE_SVG_SA
void svg_sa_init_svg(Render2D *sr, GF_Node *node);
void svg_sa_init_g(Render2D *sr, GF_Node *node);
void svg_sa_init_switch(Render2D *sr, GF_Node *node);
void svg_sa_init_rect(Render2D *sr, GF_Node *node);
void svg_sa_init_circle(Render2D *sr, GF_Node *node);
void svg_sa_init_ellipse(Render2D *sr, GF_Node *node);
void svg_sa_init_line(Render2D *sr, GF_Node *node);
void svg_sa_init_polyline(Render2D *sr, GF_Node *node);
void svg_sa_init_polygon(Render2D *sr, GF_Node *node);
void svg_sa_init_path(Render2D *sr, GF_Node *node);
void svg_sa_init_text(Render2D *sr, GF_Node *node);
void svg_sa_init_a(Render2D *se, GF_Node *node);
void svg_sa_init_linearGradient(Render2D *sr, GF_Node *node);
void svg_sa_init_radialGradient(Render2D *sr, GF_Node *node);
GF_TextureHandler *svg_sa_gradient_get_texture(GF_Node *node);
void svg_sa_init_solidColor(Render2D *sr, GF_Node *node);
void svg_sa_init_stop(Render2D *sr, GF_Node *node);
#endif


/*SVG_SA functions */
#ifdef GPAC_ENABLE_SVG_SANI

/* Creates a rendering context and Translates the SVG Styling properties into a context
   that the renderer can understand */
DrawableContext *svg_sani_drawable_init_context(Drawable *node, RenderEffect2D *eff);

void svg_sani_init_svg(Render2D *sr, GF_Node *node);
void svg_sani_init_g(Render2D *sr, GF_Node *node);
void svg_sani_init_switch(Render2D *sr, GF_Node *node);
void svg_sani_init_rect(Render2D *sr, GF_Node *node);
void svg_sani_init_circle(Render2D *sr, GF_Node *node);
void svg_sani_init_ellipse(Render2D *sr, GF_Node *node);
void svg_sani_init_line(Render2D *sr, GF_Node *node);
void svg_sani_init_polyline(Render2D *sr, GF_Node *node);
void svg_sani_init_polygon(Render2D *sr, GF_Node *node);
void svg_sani_init_path(Render2D *sr, GF_Node *node);
void svg_sani_init_a(Render2D *se, GF_Node *node);
void svg_sani_init_linearGradient(Render2D *sr, GF_Node *node);
void svg_sani_init_radialGradient(Render2D *sr, GF_Node *node);
GF_TextureHandler *svg_sani_gradient_get_texture(GF_Node *node);
void svg_sani_init_solidColor(Render2D *sr, GF_Node *node);
void svg_sani_init_stop(Render2D *sr, GF_Node *node);

void svg_sani_render_base(GF_Node *node, RenderEffect2D *eff);
void svg_sani_render_node(GF_Node *node, RenderEffect2D *eff);
void svg_sani_render_node_list(GF_List *children, RenderEffect2D *eff);
void svg_sani_get_nodes_bounds(GF_Node *self, GF_List *children, RenderEffect2D *eff);
void gf_svg_sani_apply_local_transformation(RenderEffect2D *eff, GF_Node *node, GF_Matrix2D *backup_matrix);
void gf_svg_sani_restore_parent_transformation(RenderEffect2D *eff, GF_Matrix2D *backup_matrix);

#endif


#endif //GPAC_DISABLE_SVG

#endif //_SVG_STACKS_H

