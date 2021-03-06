/*
 *			GPAC - Multimedia Framework C SDK
 *
 *			Copyright (c) Jean Le Feuvre 2000-2005
 *					All rights reserved
 *
 *  This file is part of GPAC / 3D rendering module
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

#include "render3d_nodes.h"

GF_List *Bindable_GetStack(GF_Node *bindable)
{
	void *st;
	if (!bindable) return 0;
	st = gf_node_get_private(bindable);
	switch (gf_node_get_tag(bindable)) {
	case TAG_MPEG4_Background2D: return ((Background2DStack*)st)->reg_stacks;
	case TAG_MPEG4_Background: case TAG_X3D_Background: return ((BackgroundStack*)st)->reg_stacks;
	case TAG_MPEG4_Viewport: return ((ViewStack*)st)->reg_stacks;
	case TAG_MPEG4_NavigationInfo: case TAG_X3D_NavigationInfo: 
	case TAG_MPEG4_Viewpoint: case TAG_X3D_Viewpoint: 
	case TAG_MPEG4_Fog: case TAG_X3D_Fog:
		return ((ViewStack*)st)->reg_stacks;
	default: return 0;
	}
}

Bool Bindable_GetIsBound(GF_Node *bindable)
{
	if (!bindable) return 0;
	switch (gf_node_get_tag(bindable)) {
	case TAG_MPEG4_Background2D: return ((M_Background2D*)bindable)->isBound;
	case TAG_MPEG4_Viewport: return ((M_Viewport*)bindable)->isBound;
	case TAG_MPEG4_Background: case TAG_X3D_Background: return ((M_Background*)bindable)->isBound;
	case TAG_MPEG4_NavigationInfo: case TAG_X3D_NavigationInfo: return ((M_NavigationInfo*)bindable)->isBound;
	case TAG_MPEG4_Viewpoint: case TAG_X3D_Viewpoint: return ((M_Viewpoint*)bindable)->isBound;
	case TAG_MPEG4_Fog: case TAG_X3D_Fog: return ((M_Fog*)bindable)->isBound;
	default: return 0;
	}
}

void Bindable_SetIsBound(GF_Node *bindable, Bool val)
{
	Bool has_bind_time = 0;
	if (!bindable) return;
	switch (gf_node_get_tag(bindable)) {
	case TAG_MPEG4_Background2D: 
		((M_Background2D*)bindable)->isBound = val;
		break;
	case TAG_MPEG4_Viewport:
		((M_Viewport*)bindable)->isBound = val;
		((M_Viewport*)bindable)->bindTime = gf_node_get_scene_time(bindable);
		has_bind_time = 1;
		break;
	case TAG_X3D_Background:
		((X_Background*)bindable)->bindTime = gf_node_get_scene_time(bindable);
		has_bind_time = 1;
	case TAG_MPEG4_Background:
		((M_Background*)bindable)->isBound = val;
		break;
	case TAG_X3D_NavigationInfo: 
		((X_NavigationInfo*)bindable)->bindTime = gf_node_get_scene_time(bindable);
		has_bind_time = 1;
	case TAG_MPEG4_NavigationInfo: 
		((M_NavigationInfo*)bindable)->isBound = val;
		break;
	case TAG_MPEG4_Viewpoint: case TAG_X3D_Viewpoint: 
		((M_Viewpoint*)bindable)->isBound = val;
		((M_Viewpoint*)bindable)->bindTime = gf_node_get_scene_time(bindable);
		has_bind_time = 1;
		break;
	case TAG_X3D_Fog: 
		((X_Fog*)bindable)->bindTime = gf_node_get_scene_time(bindable);
		has_bind_time = 1;
	case TAG_MPEG4_Fog: 
		((M_Fog*)bindable)->isBound = val;
		break;
	default: return;
	}
	gf_node_event_out_str(bindable, "isBound");
	if (has_bind_time) gf_node_event_out_str(bindable, "bindTime");
}


Bool Bindable_GetSetBind(GF_Node *bindable)
{
	if (!bindable) return 0;
	switch (gf_node_get_tag(bindable)) {
	case TAG_MPEG4_Background2D: return ((M_Background2D*)bindable)->set_bind;
	case TAG_MPEG4_Viewport: return ((M_Viewport*)bindable)->set_bind;
	case TAG_MPEG4_Background: case TAG_X3D_Background: return ((M_Background*)bindable)->set_bind;
	case TAG_MPEG4_NavigationInfo: case TAG_X3D_NavigationInfo: return ((M_NavigationInfo*)bindable)->set_bind;
	case TAG_MPEG4_Viewpoint: case TAG_X3D_Viewpoint: return ((M_Viewpoint*)bindable)->set_bind;
	case TAG_MPEG4_Fog: case TAG_X3D_Fog: return ((M_Fog*)bindable)->set_bind;
	default: return 0;
	}
}

void Bindable_SetSetBind(GF_Node *bindable, Bool val)
{
	if (!bindable) return;
	switch (gf_node_get_tag(bindable)) {
	case TAG_MPEG4_Background2D: 
		((M_Background2D*)bindable)->set_bind = val;
		((M_Background2D*)bindable)->on_set_bind(bindable);
		break;
	case TAG_MPEG4_Viewport: 
		((M_Viewport*)bindable)->set_bind = val;
		((M_Viewport*)bindable)->on_set_bind(bindable);
		break;
	case TAG_MPEG4_Background: case TAG_X3D_Background:
		((M_Background*)bindable)->set_bind = val;
		((M_Background*)bindable)->on_set_bind(bindable);
		break;
	case TAG_MPEG4_NavigationInfo: case TAG_X3D_NavigationInfo: 
		((M_NavigationInfo*)bindable)->set_bind = val;
		((M_NavigationInfo*)bindable)->on_set_bind(bindable);
		break;
	case TAG_MPEG4_Viewpoint: case TAG_X3D_Viewpoint: 
		((M_Viewpoint*)bindable)->set_bind = val;
		((M_Viewpoint*)bindable)->on_set_bind(bindable);
		break;
	case TAG_MPEG4_Fog: case TAG_X3D_Fog: 
		((M_Fog*)bindable)->set_bind = val;
		((M_Fog*)bindable)->on_set_bind(bindable);
		break;
	default: return;
	}
}
#if 0
static void dump_bindable_stack(GF_List *stack, char *label)
{
	u32 j=0;
	GF_Node *n;
	fprintf(stdout, "Bindable stack %s:\n", label);
	while ((n = gf_list_enum(stack, &j))) {
		fprintf(stdout, "%d: %s\n", j, gf_node_get_name(n));
	}
	fprintf(stdout, "\n");
}
#endif

void Bindable_OnSetBind(GF_Node *bindable, GF_List *stack_list)
{
	u32 i;
	Bool on_top, is_bound, set_bind;
	GF_Node *node;
	GF_List *stack;

	set_bind = Bindable_GetSetBind(bindable);
	is_bound = Bindable_GetIsBound(bindable);

	if (!set_bind && !is_bound) return;
	if (set_bind && is_bound) return;

	i=0;
	while ((stack = (GF_List*)gf_list_enum(stack_list, &i))) {
		on_top = (gf_list_get(stack, 0)==bindable) ? 1 : 0;

		if (!set_bind) {
			if (is_bound) Bindable_SetIsBound(bindable, 0);
			if (on_top && (gf_list_count(stack)>1)) {
				gf_list_rem(stack, 0);
				gf_list_add(stack, bindable);
				node = (GF_Node*)gf_list_get(stack, 0);
				Bindable_SetIsBound(node, 1);
			}
		} else {
			if (!is_bound) Bindable_SetIsBound(bindable, 1);
			if (!on_top) {
				/*push old top one down and unbind*/
				node = (GF_Node*)gf_list_get(stack, 0);
				Bindable_SetIsBound(node, 0);
				/*insert new top*/
				gf_list_del_item(stack, bindable);
				gf_list_insert(stack, bindable, 0);
			}
		}
	}
}

void BindableStackDelete(GF_List *stack)
{
	while (gf_list_count(stack)) {
		GF_List *bind_stack_list;
		GF_Node *bindable = (GF_Node*)gf_list_get(stack, 0);
		gf_list_rem(stack, 0);
		bind_stack_list = Bindable_GetStack(bindable);
		if (bind_stack_list) {
			gf_list_del_item(bind_stack_list, stack);
			assert(gf_list_find(bind_stack_list, stack)<0);
		}
	}
	gf_list_del(stack);
}


void PreDestroyBindable(GF_Node *bindable, GF_List *stack_list)
{
	Bool is_bound = Bindable_GetIsBound(bindable);
	Bindable_SetIsBound(bindable, 0);

	while (gf_list_count(stack_list)) {
		GF_Node *stack_top;
		GF_List *stack = (GF_List*)gf_list_get(stack_list, 0);
		gf_list_rem(stack_list, 0);
		gf_list_del_item(stack, bindable);
		if (is_bound) {
			stack_top = (GF_Node*)gf_list_get(stack, 0);
			if (stack_top) Bindable_SetSetBind(stack_top, 1);
		}
	}
}

