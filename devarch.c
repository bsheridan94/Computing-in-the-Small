#include "u.h"
#include "../port/lib.h"
#include "mem.h"
#include "dat.h"
#include "fns.h"
#include "../port/error.h"
#include "io.h"


// scp devarch.c bts37@tux.cs.drexel.edu:\home\bts37\CS680\
//
// scp bts37@tux.cs.drexel.edu:\home\bts37\CS680\devarch.c .
enum {
	Qdir = 0,
	Qbase,

	Qmax = 16,
};

typedef long Rdwrfn(Chan*, void*, long, vlong);

static Rdwrfn *readfn[Qmax];
static Rdwrfn *writefn[Qmax];

static Dirtab archdir[Qmax] = {
	".",		{ Qdir, 0, QTDIR },	0,	0555,
};

Lock archwlock;	/* the lock is only for changing archdir */
int narchdir = Qbase;

/*
 * Add a file to the #P listing.  Once added, you can't delete it.
 * You can't add a file with the same name as one already there,
 * and you get a pointer to the Dirtab entry so you can do things
 * like change the Qid version.  Changing the Qid path is disallowed.
 */
Dirtab*
addarchfile(char *name, int perm, Rdwrfn *rdfn, Rdwrfn *wrfn)
{
	int i;
	Dirtab d;
	Dirtab *dp;

	memset(&d, 0, sizeof d);
	strcpy(d.name, name);
	d.perm = perm;

	lock(&archwlock);
	if(narchdir >= Qmax){
		unlock(&archwlock);
		return nil;
	}

	for(i=0; i<narchdir; i++)
		if(strcmp(archdir[i].name, name) == 0){
			unlock(&archwlock);
			return nil;
		}

	d.qid.path = narchdir;
	archdir[narchdir] = d;
	readfn[narchdir] = rdfn;
	writefn[narchdir] = wrfn;
	dp = &archdir[narchdir++];
	unlock(&archwlock);

	return dp;
}

static Chan*
archattach(char* spec)
{
	return devattach('P', spec);
}

Walkqid*
archwalk(Chan* c, Chan *nc, char** name, int nname)
{
	return devwalk(c, nc, name, nname, archdir, narchdir, devgen);
}

static int
archstat(Chan* c, uchar* dp, int n)
{
	return devstat(c, dp, n, archdir, narchdir, devgen);
}

static Chan*
archopen(Chan* c, int omode)
{
	return devopen(c, omode, archdir, narchdir, devgen);
}

static void
archclose(Chan*)
{
}

static long
archread(Chan *c, void *a, long n, vlong offset)
{
	Rdwrfn *fn;

	switch((ulong)c->qid.path){
	case Qdir:
		return devdirread(c, a, n, archdir, narchdir, devgen);

	default:
		if(c->qid.path < narchdir && (fn = readfn[c->qid.path]))
			return fn(c, a, n, offset);
		error(Eperm);
		break;
	}

	return 0;
}

static long
archwrite(Chan *c, void *a, long n, vlong offset)
{
	Rdwrfn *fn;

	if(c->qid.path < narchdir && (fn = writefn[c->qid.path]))
		return fn(c, a, n, offset);
	error(Eperm);

	return 0;
}

void archinit(void);

Dev archdevtab = {
	'P',
	"arch",

	devreset,
	archinit,
	devshutdown,
	archattach,
	archwalk,
	archstat,
	archopen,
	devcreate,
	archclose,
	archread,
	devbread,
	archwrite,
	devbwrite,
	devremove,
	devwstat,
};

static long
cputyperead(Chan*, void *a, long n, vlong offset)
{
	char name[64], str[128];

	cputype2name(name, sizeof name);
	snprint(str, sizeof str, "ARM %s %d\n", name, m->cpumhz);
	return readstr(offset, a, n, str);
}

static long
cputempread(Chan*, void *a, long n, vlong offset)
{
	char str[16];

	snprint(str, sizeof str, "%ud\n", (getcputemp()+500)/1000);
	return readstr(offset, a, n, str);
}


static int ledgpiorun = 0;
static int ledgpiostate = 0;

enum {
	// GPIO registers
	GPFSEL = 0x7e200000,
	GPSET = 0x7e20001c,
	GPCLR = 0x7e200028,
	GPLEV = 0x7e200034,
	GPEDS = 0x7e200040,
	GPREN = 0x7e20004c,
	GPFEN = 0x7e200058,
	GPHEN = 0x7e200064,
	GPLEN = 0x7e200070,
	GPAREN = 0x7e20007c,
	GPAFEN = 0x7e200088,
	GPPUD = 0x7e200094,
	GPPUDCLK = 0x7e200098,
};

enum {
	CMBlink,
	CMStart,
	CMStop,
	CMOn,
	CMOff,
};

static Cmdtab ledgpiocmd[] = {
	{CMBlink, "blink", 1},
	{CMStart, "start", 1},
	{CMStop, "stop", 1},
	{CMOn, "on", 1},
	{CMOff, "off", 1},
};


void
ledcontrol()
{
	ulong *ppLed;
	ulong *ppButton;
	int i = 1;

	int buttonPin = 22, maskButton;
	int ledPin = 27, maskLed;

	i <<= (ledPin % 10) * 3;
	maskLed = 7 << ((ledPin % 10) * 3);
	ppLed = (ulong *)GPFSEL +ledPin / 10;
	*ppLed = (*ppLed & maskLed) | i;
	
	i = 0;
	i <<= (buttonPin % 10) * 3;
	maskButton = 7 << ((buttonPin % 10) * 3);
	ppButton = (ulong *)GPFSEL + buttonPin / 10;
	*ppButton = (*ppButton & maskButton) | i;

	print("LED CONTROL EXECUTED\n");
	while(ledgpiorun){

		ulong buttonStatusMask = 0x00400000;
		
		if(( ((ulong *)GPLEV)[0] & buttonStatusMask) == buttonStatusMask  ){
			tsleep(&up->sleep, return0, 0, 100);
			ledgpiostate = ledgpiostate + 1;
			if(ledgpiostate==3)
				ledgpiostate = 0;

		}
		if(ledgpiostate == 0){
		// set pin select to low				
			ppLed = (ulong*)GPCLR + ledPin / 32;
			*ppLed = 1 << ledPin % 32;	
		}


		if(ledgpiostate == 1){
		// set pin select to high
			ppLed = (ulong*)GPSET + ledPin / 32;
			*ppLed = 1 << ledPin % 32;	
		}

		if(ledgpiostate == 2){
			ppLed = (ulong*)GPCLR + ledPin / 32;
			*ppLed = 1 << ledPin % 32;	
		// set pin select low
			tsleep(&up->sleep, return0, 0, 100);
		// set pin select to high
			ppLed = (ulong*)GPSET + ledPin / 32;
			*ppLed = 1 << ledPin % 32;	
		}

	
	tsleep(&up->sleep, return0, 0, 100);
	}
	ppLed = (ulong*)GPCLR + ledPin / 32;
	*ppLed = 1 << ledPin % 32;	

	pexit("", 1);


}


static long
ledgpioread(Chan*c,  void *a, long n, vlong offset)
{
	char str[128];

	if(ledgpiostate == 0 && ledgpiorun == 0)
	snprint(str, sizeof str, "LED: OFF\t Scanning: Off \n");
	
	if(ledgpiostate == 1 && ledgpiorun == 0)
	snprint(str, sizeof str, "LED: ON\t Scanning: Off \n");

	if(ledgpiostate == 2 && ledgpiorun == 0)
	snprint(str, sizeof str, "LED: BLINKING\t Scanning: Off \n");
	
	if(ledgpiostate == 0 && ledgpiorun == 1)
	snprint(str, sizeof str, "LED: OFF\t Scanning: On \n");
	
	if(ledgpiostate == 1 && ledgpiorun == 1)
	snprint(str, sizeof str, "LED: ON\t Scanning: On \n");

	if(ledgpiostate == 2 && ledgpiorun == 1)
	snprint(str, sizeof str, "LED: BLINKING\t Scanning: On \n");
	
	return readstr(offset, a, n, str);
}

static long
ledgpiowrite(Chan* c, void *buf, long n, vlong)
{
	Cmdbuf *cb;
	Cmdtab *ct;

	if(c->qid.type & QTDIR)
		error(Eperm);
	cb = parsecmd(buf, n);
	ct = lookupcmd(cb, ledgpiocmd, nelem(ledgpiocmd));
	switch(ct->index) {
	case CMBlink:
		print("Blink! \n");
		ledgpiostate = 2; 
		break;	
	case CMStart:
		print("Start! \n");
		ledgpiorun = 1;
		kproc("LED Driver", ledcontrol, nil);
		break;
		
	case CMStop:
		ledgpiostate = 0;
		print("Stop! \n");
		ledgpiorun = 0;
		break;
	case CMOn:
		print("On! \n");
		ledgpiostate = 1;
		break;

	case CMOff:
		print("Off! \n");
		ledgpiostate = 0;
		break;

	}
	free(cb);
	return n;
}

void
archinit(void)
{
	addarchfile("cputype", 0444, cputyperead, nil);
	addarchfile("cputemp", 0444, cputempread, nil);
	addarchfile("ledgpio", 0777, ledgpioread, ledgpiowrite );
}

