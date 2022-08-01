// Author Jason Reed

#include "fig.h"
#include "resources.h"
#include "mode.h"
#include "object.h"
#include "u_search.h"
#include "u_create.h"
#include "e_depthctrl.h"
#include "u_list.h"
#include "w_canvas.h"
#include "w_mousefun.h"
#include "w_msgpanel.h"
#include "w_layers.h"
#include "d_text.h"
#include "u_bound.h"
#include "u_markers.h"
#include "u_redraw.h"
#include "w_cursor.h"
#include "f_util.h"
#include "u_create.h"


depthctrl_selected(void)
{
	set_mousefun("Bring to Front", "Adjust Depth", "Send to Back", "", "", ""); // control indicator
    canvas_kbd_proc = null_proc;
    canvas_locmove_proc = null_proc;
    canvas_ref_proc = null_proc;
    init_searchproc_left(bringtofront);
    init_searchproc_middle(adjustdepth);
    init_searchproc_right(sendtoback);
    canvas_leftbut_proc = object_search_left;
    canvas_middlebut_proc = object_search_middle;
    canvas_rightbut_proc = object_search_right;
	set_cursor(pick15_cursor);
}

void bringtofront(F_line *p, int type)
{
    if (type == O_COMPOUND) //for compound objects
    {
        int maxc = find_largest_depth(p); //largest depth in compound
        int minc = find_smallest_depth(p);//smallest depth in compound
        int min  = get_min_depth();       //smallest occupied depth
        int offset = 0;
        if ((maxc - minc) < min)
        {
            offset = -1*(maxc - min + 1);
        }
        else
        {
            offset = -1*minc;
        }
        F_compound *c_old = copy_compound(p);
        offset_compound_depth(p, offset);
        add_compound_depth(p);
        remove_compound_depth(c_old);
    }
    else //for single objects
    {
        int old = p->depth; //remember old layer

        if ((get_min_depth() > 0) & (p->depth != 0)) // normal case
        {
            p->depth = get_min_depth() - 1;
            add_depth(type, p->depth);
            remove_depth(type, old);
        }
        else if ((get_min_depth() == 0) & (p->depth != 0)) // lower bound protection
        {
            put_msg("Depth 0 ocupied, moving object anyway");
            p->depth = 0;
            add_depth(type, p->depth);
            remove_depth(type, old);

        }
        else //already in depth 0, do nothing
        {
            put_msg("Object already in depth 0");
        }
    }
    
       redisplay_object(p, type);
}

void sendtoback(F_line* p, int type)
{
    if (type == O_COMPOUND) //for compound objects
    {
        int maxc = find_largest_depth(p); //largest depth in compound
        int minc = find_smallest_depth(p);//smallest depth in compound
        int max = get_max_depth();       //smallest occupied depth
        int offset = 0;
        if ((maxc - minc) < (999 - max)) //default
        {
            offset = (max - minc + 1);
        }
        else //edge case
        {
            if (get_max_depth() == 999)
            {
                put_msg("Depth 999 already occupied, Compound object moved to have max depth of 999");
            }
                offset = 999 - maxc;
        }
        F_compound* c_old = copy_compound(p);
        offset_compound_depth(p, offset);
        add_compound_depth(p);
        remove_compound_depth(c_old);
    }
    else //for single objects
    {
        int old = p->depth;
                if ((get_max_depth() < 999) & (p->depth != 999)) // normal case
        {
            p->depth = get_max_depth() + 1;
            add_depth(type, p->depth);
            remove_depth(type, old);
        }
        else if ((get_max_depth() == 999) & (p->depth != 999)) // lower bound protection
        {
            put_msg("Depth 999 ocupied, moving object anyway");
            p->depth = 999;
            add_depth(type, p->depth);
            remove_depth(type, old);
        }
        else //faster for do nothing case
        {
            put_msg("Object already in depth 999");
        }
    }
    redisplay_object(p, type);
    
}

void adjustdepth(F_line* p, int type) //sets depth to input value from attributes panel
{
    if (type == O_COMPOUND) //for compound objects
    {
        int maxc = find_largest_depth(p); //largest depth in compound
        int minc = find_smallest_depth(p);//smallest depth in compound
        int offset = 0;
        
        //minc becomes cur_depth preserving relative depths

        if ((maxc - minc) < (999 - cur_depth)) // default case
        {
            offset = (cur_depth - minc);
        }
        else //edge case
        {
            offset = 999 - maxc;
        }

        F_compound* c_old = copy_compound(p);
        offset_compound_depth(p, offset);
        add_compound_depth(p);
        remove_compound_depth(c_old);
    }
    else
    {
        int old = p->depth;
        p->depth = cur_depth;
        add_depth(type, p->depth);
        remove_depth(type, old);
    }
    redisplay_object(p, type);
    
}

// local helper functions

int get_min_depth() //minimum depth with an object
{
    int i = 0;
    int ans = -2;
    while (ans == -2)
    {
        if (object_depths[i] != 0)
        {
            ans = i;
        }
        i = i + 1;
    }
    return ans;
}

int get_max_depth() //maximum depth with an object
{
    int i = 999;
    int ans = 1000;
    while (ans == 1000)
    {
        if (object_depths[i] != 0)
        {
            ans = i;
        }
        i = i - 1;
    }
    return ans;
}

void redisplay_object(F_line* p, int type) // call to redraw any object type
{
    switch (type)
    {
    case O_POLYLINE: redisplay_line((F_line*)p);         break;
    case O_ARC: redisplay_arc((F_arc*)p);           break;
    case O_ELLIPSE: redisplay_ellipse((F_ellipse*)p);   break;
    case O_SPLINE: redisplay_spline((F_spline*)p);     break;
    case O_TXT: redisplay_text((F_text*)p);         break;
    case O_COMPOUND: redisplay_compound((F_compound*)p); break;
    }
}

void offset_compound_depth(F_compound *p, int offset)
{
    F_line* l;
    F_arc* a;
    F_ellipse* e;
    F_spline* s;
    F_text* t;
    F_compound* c1;

    for (l = p->lines; l != NULL; l = l->next)
    {
        l->depth = l->depth + offset;
    }
    for (a = p->arcs; a != NULL; a = a->next)
    {
        a->depth = a->depth + offset;
    }
    for (e = p->ellipses; e != NULL; e = e->next)
    {
        e->depth = e->depth + offset;
    }
    for (s = p->splines; s != NULL; s = s->next)
    {
        s->depth = s->depth + offset;
    }
    for (t = p->texts; t != NULL; t = t->next)
    {
        t->depth = t->depth + offset;
    }
    for (c1 = p->compounds; c1 != NULL; c1 = c1->next)
    {
        offset_compound_depth(c1, offset);
    }
}
