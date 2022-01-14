#include <u.h>
#include <libc.h>
#include <draw.h>
#include "dat.h"
#include "fns.h"

Image *back;
Image *canvas;
Point spos;		/* position on screen */
Point cpos;		/* position on canvas */

static Image **pages;
static int npages;

static int nundo = 0;
static Image *undo[1024];

Point
s2c(Point p)
{
	p = subpt(p, spos);
	if(p.x < 0) p.x -= zoom-1;
	if(p.y < 0) p.y -= zoom-1;
	return addpt(divpt(p, zoom), cpos);
}

Point
c2s(Point p)
{
	return addpt(mulpt(subpt(p, cpos), zoom), spos);
}

Rectangle
c2sr(Rectangle r)
{
	return Rpt(c2s(r.min), c2s(r.max));
}

void
save(Rectangle r, int mark)
{
	Image *tmp;
	int x;

	if(mark){
		x = nundo++ % nelem(undo);
		if(undo[x])
			freeimage(undo[x]);
		undo[x] = nil;
	}
	if(canvas==nil || nundo<0)
		return;
	if(!rectclip(&r, canvas->r))
		return;
	if((tmp = allocimage(display, r, canvas->chan, 0, DNofill)) == nil)
		return;
	draw(tmp, r, canvas, nil, r.min);
	x = nundo++ % nelem(undo);
	if(undo[x])
		freeimage(undo[x]);
	undo[x] = tmp;
}

void
restore(int n)
{
	Image *tmp;
	int x;

	while(nundo > 0){
		if(n-- == 0)
			return;
		x = --nundo % nelem(undo);
		if((tmp = undo[x]) == nil)
			return;
		undo[x] = nil;
		if(canvas == nil || canvas->chan != tmp->chan){
			freeimage(canvas);
			canvas = tmp;
			update(nil);
		} else {
			expand(tmp->r);
			draw(canvas, tmp->r, tmp, nil, tmp->r.min);
			update(&tmp->r);
			freeimage(tmp);
		}
	}
}

void
initcnv(char *file)
{
	int fd;

	if(file == nil)
		return;
	if((fd = open(file, OREAD)) < 0)
		sysfatal("open: %r");
	if((canvas = readimage(display, fd, 0)) == nil)
		sysfatal("readimage: %r");
	close(fd);
}
