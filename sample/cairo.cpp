
#include <cairo\cairo.h>
#include <cairo\cairo-svg.h>


//int main(void)
//{
//	cairo_surface_t *surface;
//	cairo_t *cr;
//
//	surface = cairo_svg_surface_create("svgfile.svg", 390, 60);
//	cr = cairo_create(surface);
//
//	cairo_set_source_rgb(cr, 0, 0, 0);
//	cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL,
//		CAIRO_FONT_WEIGHT_NORMAL);
//	cairo_set_font_size(cr, 40.0);
//
//	cairo_move_to(cr, 10.0, 50.0);
//	cairo_show_text(cr, "Disziplin ist Macht.");
//
//	cairo_surface_destroy(surface);
//	cairo_destroy(cr);
//
//	return 0;
//}

int
main(int argc, char *argv[])
{
	cairo_surface_t *surface2 = cairo_svg_surface_create("testsvg.svg", 180, 180);
	cairo_t *cr2 = cairo_create(surface2);

	cairo_set_line_width(cr2, 5);
	cairo_set_source_rgb(cr2, 0, 0, 0);
	cairo_set_line_cap(cr2, CAIRO_LINE_CAP_SQUARE);
	
	cairo_move_to(cr2, 90, 5);		//middle: vertical line
	cairo_line_to(cr2, 90, 175);

	cairo_move_to(cr2, 5, 90);		//middle: horizontal line
	cairo_line_to(cr2, 175, 90);

	cairo_move_to(cr2, 5, 175);		//top: horizontal line
	cairo_move_to(cr2, 175, 175);

	cairo_move_to(cr2, 5, 175);		//topleft: vertical line
	cairo_line_to(cr2, 5, 5);

	cairo_move_to(cr2, 5, 175);		//bottomleft: diagonal line
	cairo_line_to(cr2, 90, 90);
	
	cairo_stroke_preserve(cr2);				//draw lines, preserve path for later use

	cairo_set_fill_rule(cr2, CAIRO_FILL_RULE_WINDING);	//this is the default fill rule
	cairo_set_source_rgb(cr2, 1, 0, 0.5);	//other color for filling
	cairo_close_path(cr2);					//useful for filling?
	
	cairo_fill(cr2);								//this wont work

	//uncomment this and see the difference, above path was too complex
	//cairo_move_to(cr2, 5, 175);		//bottomleft: diagonal line
	//cairo_line_to(cr2, 90, 90);		
	//cairo_line_to(cr2, 5, 90);		//horizontal line -> left
	//cairo_line_to(cr2, 5, 175);		//vertical line -> bottom
	//cairo_fill(cr2);


	cairo_destroy(cr2);
	cairo_surface_destroy(surface2);

	return 0;
}