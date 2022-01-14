#include <u.h>
#include <libc.h>
#include <draw.h>
#include <mouse.h>


static void
catch(void *, char *msg)
{
	if(strstr(msg, "closed pipe"))
		noted(NCONT);
	noted(NDFLT);
}

int
pipeline(char *fmt, ...)
{
	char buf[1024];
	va_list a;
	int p[2];

	va_start(a, fmt);
	vsnprint(buf, sizeof(buf), fmt, a);
	va_end(a);
	if(pipe(p) < 0)
		return -1;
	switch(rfork(RFPROC|RFMEM|RFFDG|RFNOTEG|RFREND)){
	case -1:
		close(p[0]);
		close(p[1]);
		return -1;
	case 0:
		close(p[1]);
		dup(p[0], 0);
		dup(p[0], 1);
		close(p[0]);
		execl("/bin/rc", "rc", "-c", buf, nil);
		exits("exec");
	}
	close(p[0]);
	return p[1];
}

void
initcmd(void)
{
	notify(catch);
}
