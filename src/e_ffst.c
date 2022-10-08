#include "fig.h"
#include "resources.h"
#include "object.h"
#include "mode.h"
#include "paintop.h"
#include "d_text.h"
#include "u_create.h"
#include "u_elastic.h"
#include "u_list.h"
#include "u_draw.h"
#include "u_drag.h"
#include "u_search.h"
#include "w_canvas.h"
#include "w_drawprim.h"
#include "w_indpanel.h"
#include "w_mousefun.h"
#include "w_msgpanel.h"
#include "w_setup.h"
#include "w_util.h"
#include "w_layers.h"
#include "e_ffst.h"
#include "e_scale.h"
#include "f_read.h"
#include "f_util.h"
#include "u_bound.h"
#include "u_fonts.h"
#include "u_free.h"
#include "u_markers.h"
#include "u_redraw.h"
#include "u_undo.h" 
#include "w_cursor.h"
#include "w_util.h"
#include "e_ffst.h"
#include "u_bound.h"

// #taskFreeSelection
//----------------------------------- Code Starts Here ----------------------------------
/* INFO: This is the file that describes the free selection. The first function 
 *   'ffst_selected' is the one called when the your shortcut is clicked. The left mouse
 *   button is linked to the next function 'init_border_drawing'. That function describes
 *   the general behavior of selection. The left mouse button now is linked to the
 *   function 'create_selectArea'. Inside that function, a elastic box is defined. Then,
 *   a function is called to tag the objects inside this box and a function is called to
 *   create the compound that will move. We called this GLUE CODE. Your work stars here.
 * GOAL: Create the tag codes. You can try create your own code or go to the next hashtag
 *   and get a hint.
 */


//----------------------------------- Code Ends Here ------------------------------------

/**************************************** BASE CODE ************************************/

F_compound *temp = NULL;

void ffst_selected(void)
{
	set_mousefun("corner point", "", "", "", "", "");
	canvas_kbd_proc = null_proc;
	canvas_locmove_proc = null_proc;
	canvas_leftbut_proc = init_border_drawing;
	canvas_middlebut_proc = null_proc;
	canvas_rightbut_proc = null_proc;
	set_cursor(crosshair_cursor);
	reset_action_on();
}

void init_border_drawing(int x, int y)
{
	cur_x = fix_x = x;
	cur_y = fix_y = y;
	set_mousefun("final point", "", "cancel", "", "", "");
	draw_mousefun_canvas();

	// set mouse actions
	canvas_locmove_proc = resizing_box;
	canvas_leftbut_proc = create_selectArea;
	canvas_middlebut_proc = null_proc;
	canvas_rightbut_proc = cancel_box;

	// create box to draw
	elastic_box(fix_x, fix_y, cur_x, cur_y);
	set_cursor(null_cursor);
	set_action_on();
}

static void
cancel_box(void)
{
	elastic_box(fix_x, fix_y, cur_x, cur_y);
	/* erase last lengths if appres.showlengths is true */
	erase_box_lengths();
	ffst_selected();
	draw_mousefun_canvas();
}

static void
create_selectArea(int x, int y)
{
	elastic_box(fix_x, fix_y, cur_x, cur_y);
	int x_min, x_max, y_min, y_max;

	x_min = min2(fix_x, x);
	y_min = min2(fix_y, y);
	x_max = max2(fix_x, x);
	y_max = max2(fix_y, y);
	tag_obj_in_region1(x_min, y_min, x_max, y_max);

	// Create the compound
	create_compoundobject(1, 1);
	draw_mousefun_canvas();
}
static void
break_comp(void)
{
	init_break();
	ffst_selected();
}


/************************************** GLUE CODE **************************************/


// #taskFreeSelection
//-------------------------------------Code Starts Here----------------------------------
/* GOAL: To tag all objects, create a different function to tag each type of object.
 *   Before that, create a general function 'tag_obj_in_region1' that call all the
 *   others. It receives the max and min point position of the selected region, and
 *   returns nothing. 
 *
 * tag_obj_in_region1()
 * {
 *
 * }
 *
 * GOAL: Now, make the functions to tag the objects inside the global compound 'objects'
 *   that matches with the selected area. For each object, you need verifiry if the
 *   object is inside the area, mark tagged and then call the function 
 *   'toggle_[object_type]highlight' from the file 'u_markers.h'.
 * HINT: Think carefully. Analize what information you have about the position of each
 *   object. You can do that. But if you are having trouble, let's work together. Go to
 *   the next hashtag.
 */
   
//------------------------------------ Code Ends Here -----------------------------------

static void
create_compoundobject(int x, int y)
{
	(void)x;
	(void)y;
	F_compound *c;
	if ((c = create_compound()) == NULL)
		return;

	// dont create compound if nothing is selected
	if (compose_compound1(c) == 0)
	{
		free((char *)c);
		ffst_selected();
		draw_mousefun_canvas();
		put_msg("Empty compound, ignored");
		return;
	}
	/*
	 * Make the bounding box exactly match the dimensions of the compound.  */
	compound_bound(c, &c->nwcorner.x, &c->nwcorner.y,
				   &c->secorner.x, &c->secorner.y);

	/* if zero width or height in the compound, adjust to next positioning  grid point or a few pixels if positioning grid is "ANY" */
	if (c->nwcorner.x == c->secorner.x)
	{
		if (cur_pointposn != P_ANY)
		{
			c->secorner.x += point_spacing();
			c->secorner.x = ceil_coords_x(c->secorner.x, c->secorner.y);
		}
	}
	if (c->nwcorner.y == c->secorner.y)
	{
		if (cur_pointposn != P_ANY)
		{
			c->secorner.y += point_spacing();
			c->secorner.y = ceil_coords_y(c->secorner.x, c->secorner.y);
		}
	}

	// cleanup for creating new compound
	c->next = NULL;
	clean_up();
	set_action(F_GLUE);
	toggle_markers_in_compound(c);
	list_add_compound(&objects.compounds, c);
	mask_toggle_compoundmarker(c);
	set_latestcompound(c);
	temp = c;
	set_modifiedflag();
	move_selected1(); // Allows for clean progression of functions  draw_mousefun_canvas();
}

// #taskFreeSelection
//------------------------------------- Code Starts Here --------------------------------
/* HINT: If you do not know what objects are inside a compound, look into the function
 *   that creates a compond in the file 'object.h' and go back to the hashtag before to
 *   finalize you work.      																						  	
 * GOAL: If you do not know anything. Let's work step by step. First, inside a compound
 *   there are a lot of objects lists. For instance, the ellipses are in the elipses 
 *   list, the arcs in the arcs list. To select a ellipse:						
 *   1) look at each object in the linked list objects.ellipses;									
 *   2) verify if the object is in the active layer using 'active_layer'.							
 *   3) verify if center less the radiuses is smaller the minimum point of the selection.		
 *   4) verify if center more the radiuses is bigger them the maximum point of the selection.	
 *   5) If True, tag the object and call 'toggle_ellipsehighlight'.								
 *																								
 *   You need to do that for ellipse, arc, line, spline, text and compound.						
 *   																							
 * HINTS:																						
 *   a) arc or spline: use the fuction 'arc_bound' or 'spline_bound'. It receives the				
 *      object and four variables that will assume the box position of the object.				
 *   b) text: use 'text_bound'. This function receives the same argumets above plus 8 	
 *      variables that are not relevant here.																
 *   c) line: for each line, verify each point. For a line being tagged, all points of it 		
 *      need to be inside the selected area.  													
 *   d) compound: use the fuction 'any_active_in_compound' ('w_layers.c') to check active 
 *      layer.
 */
void tag_obj_in_region1(int xmin, int ymin, int xmax, int ymax)
{
	sel_line(xmin,ymin,xmax,ymax);
	sel_arc(xmin,ymin,xmax,ymax);
	sel_compound(xmin,ymin,xmax,ymax);
	sel_ellipse(xmin,ymin,xmax,ymax);
	sel_spline(xmin,ymin,xmax,ymax);
	sel_text(xmin,ymin,xmax,ymax);
}

static void sel_line(int xmin, int ymin, int xmax, int ymax){
	/*
	F_line e*; F_point *p; bool mark=0;
	for(e=objects.lines;e!=NULL;){
		if(active_layer(e->depth)){
			for(p=e.points;p!=NULL;){
				if(p.x<xmin || p.x>xmax || p.y<ymin || p.y>ymax){
					bool=1;
					break;
				}
				p=p.next;
			}
			if(bool==0){
				toggle_linehighlight(e);
				e->tagged=1-e->tagged;
			}
		}
		e=e->next;
		bool=0;
	}
	*/
	F_line	   *l;
    F_point	   *p;
    int		    inbound;

    for (l = objects.lines; l != NULL; l = l->next) {
	if (!active_layer(l->depth))
	    continue;
	for (inbound = 1, p = l->points; p != NULL && inbound;
	     p = p->next) {
	    inbound = 0;
	    if (xmin > p->x)
		continue;
	    if (xmax < p->x)
		continue;
	    if (ymin > p->y)
		continue;
	    if (ymax < p->y)
		continue;
	    inbound = 1;
	}
	if (!inbound)
	    continue;
	l->tagged = 1 - l->tagged;
	toggle_linehighlight(l);
    }
}

static void sel_arc(int xmin, int ymin, int xmax, int ymax){
    F_arc	   *a;
    int		    urx, ury, llx, lly;

    for (a = objects.arcs; a != NULL; a = a->next) {
	if (!active_layer(a->depth))
	    continue;
	arc_bound(a, &llx, &lly, &urx, &ury);
	if (xmin > llx)
	    continue;
	if (xmax < urx)
	    continue;
	if (ymin > lly)
	    continue;
	if (ymax < ury)
	    continue;
	a->tagged = 1 - a->tagged;
	toggle_archighlight(a);
    }
} 

static void sel_compound(int xmin, int ymin, int xmax, int ymax){
	F_compound	   *c;

    for (c = objects.compounds; c != NULL; c = c->next) {
	if (!any_active_in_compound(c))
	    continue;
	if (xmin > c->nwcorner.x)
	    continue;
	if (xmax < c->secorner.x)
	    continue;
	if (ymin > c->nwcorner.y)
	    continue;
	if (ymax < c->secorner.y)
	    continue;
	c->tagged = 1 - c->tagged;
	toggle_compoundhighlight(c);
    }
} 

static void sel_ellipse(int xmin, int ymin, int xmax, int ymax){
	/*
	F_ellipse *e;
	for(e=objects.elipses;e!=NULL;){
		if(active_layer(e->depth)){
			if(e.center.x-e.radiuses.x>xmin && e.center.x+e.radiuses.x<xmax && e.center.y-e.radiuses.y>ymin && e.center.y+e.radiuses.y<ymax ){
				toggle_ellipsehighlight(e);
				e->tagged=1-e->tagged;
			}
		}
		e=e->next;
	}
	*/
	F_ellipse	   *e;

    for (e = objects.ellipses; e != NULL; e = e->next) {
	if (!active_layer(e->depth))
	    continue;
	if (xmin > e->center.x - e->radiuses.x)
	    continue;
	if (xmax < e->center.x + e->radiuses.x)
	    continue;
	if (ymin > e->center.y - e->radiuses.y)
	    continue;
	if (ymax < e->center.y + e->radiuses.y)
	    continue;
	e->tagged = 1 - e->tagged;
	toggle_ellipsehighlight(e);
    }

}

static void sel_spline(int xmin, int ymin, int xmax, int ymax){
	F_spline	   *s;
    int		    urx, ury, llx, lly;

    for (s = objects.splines; s != NULL; s = s->next) {
	if (!active_layer(s->depth))
	    continue;
	spline_bound(s, &llx, &lly, &urx, &ury);
	if (xmin > llx)
	    continue;
	if (xmax < urx)
	    continue;
	if (ymin > lly)
	    continue;
	if (ymax < ury)
	    continue;
	s->tagged = 1 - s->tagged;
	toggle_splinehighlight(s);
    }
}

static void sel_text(int xmin, int ymin, int xmax, int ymax){
	F_text	   *t;
    int		    txmin, txmax, tymin, tymax;
    int		    dum;

    for (t = objects.texts; t != NULL; t = t->next) {
	if (!active_layer(t->depth))
	    continue;
	text_bound(t, &txmin, &tymin, &txmax, &tymax,
			&dum,&dum,&dum,&dum,&dum,&dum,&dum,&dum);
	if (xmin > txmin || xmax < txmax ||
	    ymin > tymin || ymax < tymax)
		continue;
	t->tagged = 1 - t->tagged;
	toggle_texthighlight(t);
    }
}

//----------------------------------- Code ends Here ------------------------------------


// #taskFreeSelection
//----------------------------------- Code Starts Here ----------------------------------
/* GOAL: The second part of your work is creating the coumpond with the tagged objects.
 *   The main function is 'create_compoundobject' and its done in the code above. Read
 *   this function and create the function 'compose_compound1', that receives the new
 *   compound and put the selected objects. If it did not find anything, return 0, in
 *   other case, return 1. After that, create a function for each object type to get the
 *   tagged ones.
 */

compose_compound1() 
{
/* 	1) Set 'defer_update_layers' to defer updating of layer buttons until we've composed
 *	   the entire compound. 
 *	2) Call the funtions to get all tagged objects. 
 *	3) Now update the layer buttons setting 'defer_update_layers' and calling the  
 *	   function ('w_layers.c'). 
 *  4) Verify the lists of objects inside de compound. If one of them is not null
 *  return 1, or return 0. 
 */
	c->ellipses = NULL;
    c->lines = NULL;
    c->texts = NULL;
    c->splines = NULL;
    c->arcs = NULL;
    c->comments = NULL;
    c->compounds = NULL;
    /* defer updating of layer buttons until we've composed the entire compound */
    defer_update_layers = True;
    get_ellipse(&c->ellipses);
    get_line(&c->lines);
    get_spline(&c->splines);
    get_text(&c->texts);
    get_arc(&c->arcs);
    get_compound(&c->compounds);
    /* now update the layer buttons */
    defer_update_layers = False;
    update_layers();
    if (c->ellipses != NULL)
	return (1);
    if (c->splines != NULL)
	return (1);
    if (c->lines != NULL)
	return (1);
    if (c->texts != NULL)
	return (1);
    if (c->arcs != NULL)
	return (1);
    if (c->compounds != NULL)
	return (1);
    return (0);
}

/* HINT: In the function to get the tagged objects, when you find a tagged object, before
 *   remove from the original compound and put into the new one, call the function 
 *   'remove_depth' from the file 'u_list.c'. For compounds, use the function
 *   'remove_compound_depth'.
 */
static void
get_arc(F_arc **list)
{
    F_arc	   *a, *arc, *aa;

    for (a = objects.arcs; a != NULL;) {
	if (!a->tagged) {
	    aa = a;
	    a = a->next;
	    continue;
	}
	remove_depth(O_ARC, a->depth);
	if (*list == NULL)
	    *list = a;
	else
	    arc->next = a;
	arc = a;
	if (a == objects.arcs)
	    a = objects.arcs = objects.arcs->next;
	else
	    a = aa->next = a->next;
	arc->next = NULL;
    }
}
static void
get_line(F_line **list)
{
    F_line	   *line, *l, *ll;

    for (l = objects.lines; l != NULL;) {
	if (!l->tagged) {
	    ll = l;
	    l = l->next;
	    continue;
	}
	remove_depth(O_POLYLINE, l->depth);
	if (*list == NULL)
	    *list = l;
	else
	    line->next = l;
	line = l;
	if (l == objects.lines)
	    l = objects.lines = objects.lines->next;
	else
	    l = ll->next = l->next;
	line->next = NULL;
    }
}
static void
get_spline(F_spline **list)
{
    F_spline	   *spline, *s, *ss;

    for (s = objects.splines; s != NULL;) {
	if (!s->tagged) {
	    ss = s;
	    s = s->next;
	    continue;
	}
	remove_depth(O_SPLINE, s->depth);
	if (*list == NULL)
	    *list = s;
	else
	    spline->next = s;
	spline = s;
	if (s == objects.splines)
	    s = objects.splines = objects.splines->next;
	else
	    s = ss->next = s->next;
	spline->next = NULL;
    }
}
static void
get_text(F_text **list)
{
    F_text	   *text, *t, *tt;

    for (t = objects.texts; t != NULL;) {
	if (!t->tagged) {
	    tt = t;
	    t = t->next;
	    continue;
	}
	remove_depth(O_TXT, t->depth);
	if (*list == NULL)
	    *list = t;
	else
	    text->next = t;
	text = t;
	if (t == objects.texts)
	    t = objects.texts = objects.texts->next;
	else
	    t = tt->next = t->next;
	text->next = NULL;
    }
}
static void
get_compound(F_compound **list)
{
    F_compound	   *compd, *c, *cc;

    for (c = objects.compounds; c != NULL;) {
	if (!c->tagged) {
	    cc = c;
	    c = c->next;
	    continue;
	}
	remove_compound_depth(c);
	if (*list == NULL)
	    *list = c;
	else
	    compd->next = c;
	compd = c;
	if (c == objects.compounds)
	    c = objects.compounds = objects.compounds->next;
	else
	    c = cc->next = c->next;
	compd->next = NULL;
    }
}

//----------------------------------- Code ends Here ------------------------------------


static void get_ellipse(F_ellipse **list)
{
	F_ellipse *e, *ee, *ellipse;
	for (e = objects.ellipses; e != NULL;)
	{
		if (!e->tagged)
		{
			ee = e;
			e = e->next;
			continue;
		}
		remove_depth(O_ELLIPSE, e->depth); 
		if (*list == NULL)
			*list = e;
		else
			ellipse->next = e;
		ellipse = e;
		if (e == objects.ellipses)
			e = objects.ellipses = objects.ellipses->next;
		else
			e = ee->next = e->next;
		ellipse->next = NULL;
	}
}


/************************************** MOVE  CODE *************************************/


void move_selected1(void)
{
	set_mousefun("move object", "horiz/vert move", "", LOC_OBJ, LOC_OBJ, LOC_OBJ);
	canvas_kbd_proc = null_proc;
	canvas_locmove_proc = null_proc;
	init_searchproc_left(init_arb_move);
	init_searchproc_middle(init_constrained_move);
	canvas_leftbut_proc = object_search_left;
	canvas_middlebut_proc = object_search_middle;
	canvas_rightbut_proc = null_proc;
	return_proc = break_comp;
	set_cursor(pick9_cursor);
	reset_action_on();
}

static void
init_arb_move(F_line *p, int type, int x, int y, int px, int py)
{
	constrained = MOVE_ARB;
	init_move(p, type, x, y, px, py);
	canvas_middlebut_proc = null_proc;
	set_mousefun("place object", "", "cancel", LOC_OBJ, LOC_OBJ, LOC_OBJ);
	draw_mousefun_canvas();
}

static void
init_constrained_move(F_line *p, int type, int x, int y, int px, int py)
{
	constrained = MOVE_HORIZ_VERT;
	init_move(p, type, x, y, px, py);
	canvas_middlebut_proc = canvas_leftbut_proc;
	canvas_leftbut_proc = null_proc;
	set_mousefun("", "place object", "cancel", LOC_OBJ, LOC_OBJ, LOC_OBJ);
	draw_mousefun_canvas();
}

// #taskFreeSelection
//------------------------------------- Code Starts Here---------------------------------
// GOAL: At the end. read the MOVE code above and create the 'init_move' function. 

static void
init_move(F_line *p, int type, int x, int y, int px, int py)
{
//  The line below turn off all markers 
	update_markers(0);
/*	GOAL: Now, for each type of object:
 *  1) set the cursor to wait;
 *	2) populate the current variable with the object p;
 *  3) call the function list_delete ('u_list.c');
 * 	4) call the fuction that redraw anything near the object (redisplay);
 * 	5) set the cursor to null;
 * 	6) init the dragging using the apropriated function ('u_drag.c'). For O_TXT objects,
 *     use x and y. For the others, use px and py.
 * 
 * 	The default case should just return the funcion.
 */
switch (type) {
    case O_COMPOUND:
	set_cursor(wait_cursor);
	cur_c = (F_compound *) p;
	list_delete_compound(&objects.compounds, cur_c);
	redisplay_compound(cur_c);
	set_cursor(null_cursor);
	init_compounddragging(cur_c, px, py);
	break;
    case O_POLYLINE:
	set_cursor(wait_cursor);
	cur_l = (F_line *) p;
	list_delete_line(&objects.lines, cur_l);
	redisplay_line(cur_l);
	set_cursor(null_cursor);
	init_linedragging(cur_l, px, py);
	break;
    case O_TXT:
	set_cursor(wait_cursor);
	cur_t = (F_text *) p;
	list_delete_text(&objects.texts, cur_t);
	redisplay_text(cur_t);
	set_cursor(null_cursor);
	init_textdragging(cur_t, x, y);
	break;
    case O_ELLIPSE:
	set_cursor(wait_cursor);
	cur_e = (F_ellipse *) p;
	list_delete_ellipse(&objects.ellipses, cur_e);
	redisplay_ellipse(cur_e);
	set_cursor(null_cursor);
	init_ellipsedragging(cur_e, px, py);
	break;
    case O_ARC:
	set_cursor(wait_cursor);
	cur_a = (F_arc *) p;
	list_delete_arc(&objects.arcs, cur_a);
	redisplay_arc(cur_a);
	set_cursor(null_cursor);
	init_arcdragging(cur_a, px, py);
	break;
    case O_SPLINE:
	set_cursor(wait_cursor);
	cur_s = (F_spline *) p;
	list_delete_spline(&objects.splines, cur_s);
	redisplay_spline(cur_s);
	set_cursor(null_cursor);
	init_splinedragging(cur_s, px, py);
	break;
    default:
	return;
    }
}	

//----------------------------------- Code Ends Here ------------------------------------


/************************************** BREAK CODE **************************************/

static void
init_break(void)
{
	cur_c = temp;
	mask_toggle_compoundmarker(cur_c);
	clean_up();
	list_delete_compound(&objects.compounds, cur_c);
	tail(&objects, &object_tails);
	append_objects(&objects, cur_c, &object_tails);
	toggle_markers_in_compound(cur_c);
	set_tags(cur_c, 0);
	set_action(F_BREAK);
	set_latestcompound(cur_c);
	set_modifiedflag();
	ffst_selected();
}