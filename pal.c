#include <u.h>
#include <libc.h>
#include <draw.h>
#include <mouse.h>
#include "dat.h"
#include "fns.h"

Image *ink;

static int palcol[] = {
	0x000000,
	0xFFFFFF, 0xC0C0C0, 0x7F7F7F,
	0xFF0000, 0xC00000, 0x7F0000,
	0xFF7F00, 0xC06000, 0x7F3F00,
	0xFFFF00, 0xC0C000, 0x7F7F00,
	0x00FF00, 0x00C000, 0x007F00,
	0x00FFFF, 0x00C0C0, 0x007F7F,
	0x0000FF, 0x0000C0, 0x00007F,
	0x7F00FF, 0x6000C0, 0x3F007F,
	0xFF00FF, 0xC000C0, 0x7F007F,
};
static Image *pal[nelem(palcol)];
static Rectangle palr, penr;

void
drawpal(void)
{
	Rectangle r, rr;
	int i;

	r = screen->r;
	r.min.y = r.max.y - 20;
	replclipr(screen, 0, r);

	penr = r;
	penr.min.x = r.max.x - NBRUSH*Dy(r);

	palr = r;
	palr.max.x = penr.min.x;

	r = penr;
	draw(screen, r, back, nil, ZP);
	for(i=0; i<NBRUSH; i++){
		r.max.x = penr.min.x + (i+1)*Dx(penr) / NBRUSH;
		rr = r;
		if(i == brush)
			rr.min.y += Dy(r)/3;
		if(i == NBRUSH-1){
			/* last is special brush for fill draw */
			draw(screen, rr, ink, nil, ZP);
		} else {
			rr.min = addpt(rr.min, divpt(subpt(rr.max, rr.min), 2));
			rr.max = rr.min;
			strokedraw(screen, rr, ink, i);
		}
		r.min.x = r.max.x;
	}

	r = palr;
	for(i=1; i<=nelem(pal); i++){
		r.max.x = palr.min.x + i*Dx(palr) / nelem(pal);
		rr = r;
		if(ink == pal[i-1])
			rr.min.y += Dy(r)/3;
		draw(screen, rr, pal[i-1], nil, ZP);
		gendrawdiff(screen, r, rr, back, ZP, nil, ZP, SoverD);
		r.min.x = r.max.x;
	}

	r = screen->r;
	r.max.y -= Dy(palr);
	replclipr(screen, 0, r);
}

int
hitpal(Mouse m)
{
	int i;

	if(ptinrect(m.xy, penr)){
		if(m.buttons & 7){
			brush = ((m.xy.x - penr.min.x) * NBRUSH) / Dx(penr);
			drawpal();
		}
		return 1;
	}
	if(ptinrect(m.xy, palr)){
		Image *col;

		i = (m.xy.x - palr.min.x) * nelem(pal) / Dx(palr);
		col = pal[i];
		switch(m.buttons & 7){
		case 1:
			ink = col;
			drawpal();
			break;
		case 2:
			back = col;
			drawpal();
			update(nil);
			break;
		/* -> picker
		case 4:
			snprint(buf, sizeof(buf), "%06x", palcol[i]);
			if(eenter("Hex", buf, sizeof(buf), &m) == 6){
				c = strtoll(buf, &e, 16);
				if(*e == 0){
					palcol[i] = c;
					freeimage(pal[i]);
					pal[i] = allocimage(display, Rect(0, 0, 1, 1), RGB24, 1, c<<8|0xff);
					drawpal();
				}
			}
			break;
		*/
		}
		return 1;
	}
	return 0;
}

void
initpal(int invbg)
{
	int i;

	for(i=0; i<nelem(pal); i++){
		pal[i] = allocimage(display, Rect(0, 0, 1, 1), RGB24, 1,
			palcol[i % nelem(palcol)]<<8 | 0xFF);
		if(pal[i] == nil)
			sysfatal("allocimage: %r");
	}
	if(invbg){
		ink = pal[1];
		back = pal[0];
	}else{
		ink = pal[0];
		back = pal[1];
	}
}
