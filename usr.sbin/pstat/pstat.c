/*
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 */

#if	!defined(lint) && defined(DOSCCS)
char copyright[] =
"@(#) Copyright (c) 1980 Regents of the University of California.\n\
 All rights reserved.\n";

static char sccsid[] = "@(#)pstat.c	5.8.6 (2.11BSD) 1999/9/13";
#endif

/*
 * Print system stuff
 */

#include <sys/param.h>
#define	KERNEL
#include <sys/localopts.h>
#include <sys/file.h>
#include <sys/user.h>
#undef	KERNEL
#include <sys/proc.h>
#include <sys/text.h>
#include <sys/buf.h>
#include <sys/inode.h>
#include <sys/map.h>
#include <sys/ioctl.h>
#include <sys/tty.h>
#include <sys/conf.h>
#include <sys/vm.h>
#include <nlist.h>
#include <stdio.h>

char	*fcore	= "/dev/kmem";
char	*fmem	= "/dev/mem";
char	*fnlist	= "/vmunix";
int	fc, fm;

struct nlist nl[] = {
#define	SINODE	0
	{ "_inode" },
#define	STEXT	1
	{ "_text" },
#define	SPROC	2
	{ "_proc" },
#define	SDZ	3
	{ "_dz_tty" },
#define	SNDZ	4
	{ "_dz_cnt" },
#define	SKL	5
	{ "_cons" },
#define	SFIL	6
	{ "_file" },
#define	SNSWAP	7
	{ "_nswap" },
#define	SNKL	8
	{ "_nkl11" },
#define	SWAPMAP	9
	{ "_swapmap" },
#define	SDH	10
	{ "_dh11" },
#define	SNDH	11
	{ "_ndh11" },
#define	SNPROC	12
	{ "_nproc" },
#define	SNTEXT	13
	{ "_ntext" },
#define	SNFILE	14
	{ "_nfile" },
#define	SNINODE	15
	{ "_ninode" },
#define	SPTY	16
	{ "_pt_tty" },
#define	SNPTY	17
	{ "_npty" },
#define	SDHU	18
	{ "_dhu_tty" },
#define	SNDHU	19
	{ "_ndhu" },
#define	SDHV	20
	{ "_dhv_tty" },
#define	SNDHV	21
	{ "_ndhv" },
#define SCOREMAP 22
	{ "_coremap" },
#define SNBUF	23
	{ "_nbuf" },
#define SBUF 24
	{ "_buf" },
#define SUBMAP 25
	{ "_ub_map" },
	{ "" }
};

int	inof;
int	txtf;
int	prcf;
int	ttyf;
int	usrf;
long	ubase;
int	filf;
int	swpf;
int	coref;
int	buff;
int	ubmapf;
int	totflg;
int	allflg;
int	kflg;
u_short	getw();

main(argc, argv)
char **argv;
{
	register char *argp;
	int allflags;

	argc--, argv++;
	while (argc > 0 && **argv == '-') {
		argp = *argv++;
		argp++;
		argc--;
		while (*argp++)
		switch (argp[-1]) {

		case 'T':
			totflg++;
			break;

		case 'a':
			allflg++;
			break;

		case 'i':
			inof++;
			break;

		case 'k':
			kflg++;
			fcore = fmem = "/core";
			break;

		case 'x':
			txtf++;
			break;

		case 'p':
			prcf++;
			break;

		case 't':
			ttyf++;
			break;

		case 'u':
			if (argc == 0)
				break;
			argc--;
			usrf++;
			sscanf( *argv++, "%lo", &ubase);
			break;

		case 'f':
			filf++;
			break;
		case 's':
			swpf++;
			break;
			break;
		case 'c':
			coref++;
			break;
		case 'm':
			ubmapf++;
			break;
		case 'b':
			buff++;
			break;
		default:
			usage();
			exit(1);
		}
	}
	if (argc>1) {
		fcore = fmem = argv[1];
		kflg++;
	}
	if ((fc = open(fcore, 0)) < 0) {
		printf("Can't find %s\n", fcore);
		exit(1);
	}
	if ((fm = open(fmem, 0)) < 0) {
		printf("Can't find %s\n", fmem);
		exit(1);
	}
	if (argc>0)
		fnlist = argv[0];
	nlist(fnlist, nl);
	if (nl[0].n_type == 0) {
		printf("no namelist, n_type: %d n_value: %o n_name: %s\n", nl[0].n_type, nl[0].n_value, nl[0].n_name);
		exit(1);
	}
	allflags = filf | totflg | inof | prcf | txtf | ttyf | usrf |
			 swpf | coref | ubmapf | buff;
	if (allflags == 0) {
		printf("pstat: one or more of -[aixptfscmbuT] is required\n");
		exit(1);
	}
	if (filf||totflg)
		dofile();
	if (inof||totflg)
		doinode();
	if (prcf||totflg)
		doproc();
	if (txtf||totflg)
		dotext();
	if (ttyf)
		dotty();
	if (usrf)
		dousr();
	if (swpf||totflg)
		domap(swpf, SWAPMAP);
	if (coref||totflg)
		domap(coref, SCOREMAP);
	if (ubmapf||totflg)
		domap(ubmapf, SUBMAP);
	if (buff||totflg)
		dobuf();

	exit(0);
}

usage()
{

	printf("usage: pstat -[aixptfscmbT] [-u [ubase]] [system] [core]\n");
}

doinode()
{
	register struct inode *ip;
	struct inode *xinode;
	register int nin;
	u_int ninode, ainode;

	nin = 0;
	ninode = getw((off_t)nl[SNINODE].n_value);
	xinode = (struct inode *)calloc(ninode, sizeof (struct inode));
	ainode = nl[SINODE].n_value;
	if (ninode < 0 || ninode > 10000) {
		fprintf(stderr, "number of inodes is preposterous (%d)\n",
			ninode);
		return;
	}
	if (xinode == NULL) {
		fprintf(stderr, "can't allocate memory for inode table\n");
		return;
	}
	lseek(fc, (off_t)ainode, 0);
	read(fc, xinode, ninode * sizeof(struct inode));
	for (ip = xinode; ip < &xinode[ninode]; ip++)
		if (ip->i_count)
			nin++;
	if (totflg) {
		printf("%3d/%3d inodes\n", nin, ninode);
		return;
	}
	printf("%d/%d active inodes\n", nin, ninode);
printf("   LOC      FLAGS      CNT  DEVICE  RDC WRC  INO   MODE  NLK  UID  SIZE/DEV FS\n");
	for (ip = xinode; ip < &xinode[ninode]; ip++) {
		if (ip->i_count == 0)
			continue;
		printf("%7.1o ", ainode + (ip - xinode)*sizeof (*ip));
		putf((long)ip->i_flag&ILOCKED, 'L');
		putf((long)ip->i_flag&IUPD, 'U');
		putf((long)ip->i_flag&IACC, 'A');
		putf((long)ip->i_flag&IMOUNT, 'M');
		putf((long)ip->i_flag&IWANT, 'W');
		putf((long)ip->i_flag&ITEXT, 'T');
		putf((long)ip->i_flag&ICHG, 'C');
		putf((long)ip->i_flag&ISHLOCK, 'S');
		putf((long)ip->i_flag&IEXLOCK, 'E');
		putf((long)ip->i_flag&ILWAIT, 'Z');
		putf((long)ip->i_flag&IPIPE, 'P');
		putf((long)ip->i_flag&IMOD, 'm');
		putf((long)ip->i_flag&IRENAME, 'r');
		putf((long)ip->i_flag&IXMOD, 'x');
		printf("%4d", ip->i_count);
		printf("%4d,%3d", major(ip->i_dev), minor(ip->i_dev));
		printf("%4d", ip->i_flag&IPIPE ? 0 : ip->i_shlockc);
		printf("%4d", ip->i_flag&IPIPE ? 0 : ip->i_exlockc);
		printf("%6u ", ip->i_number);
		printf("%7.1o", ip->i_mode);
		printf("%4d", ip->i_nlink);
		printf("%5u", ip->i_uid);
		if ((ip->i_mode&IFMT)==IFBLK || (ip->i_mode&IFMT)==IFCHR)
			printf("%6d,%3d", major(ip->i_rdev), minor(ip->i_rdev));
		else
			printf("%10ld", ip->i_size);
		printf(" %2d", ip->i_fs);
		printf("\n");
	}
	free(xinode);
}

u_short
getw(loc)
	off_t loc;
{
	u_short word;

	lseek(fc, loc, 0);
	read(fc, &word, sizeof (word));
	return (word);
}

putf(v, n)
	long	v;
	char	n;
{
	if (v)
		printf("%c", n);
	else
		printf(" ");
}

dotext()
{
	register struct text *xp;
	int ntext;
	struct text *xtext;
	u_int ntx, ntxca, atext;

	ntx = ntxca = 0;
	ntext = getw((off_t)nl[SNTEXT].n_value);
	xtext = (struct text *)calloc(ntext, sizeof (struct text));
	atext = nl[STEXT].n_value;
	if (ntext < 0 || ntext > 10000) {
		fprintf(stderr, "number of texts is preposterous (%d)\n",
			ntext);
		return;
	}
	if (xtext == NULL) {
		fprintf(stderr, "can't allocate memory for text table\n");
		return;
	}
	lseek(fc, (off_t)atext, 0);
	read(fc, xtext, ntext * sizeof (struct text));
	for (xp = xtext; xp < &xtext[ntext]; xp++) {
		if (xp->x_iptr != NULL)
			ntxca++;
		if (xp->x_count != 0)
			ntx++;
	}
	if (totflg) {
		printf("%3d/%3d texts active, %3d used\n", ntx, ntext, ntxca);
		return;
	}
	printf("%d/%d active texts, %d used\n", ntx, ntext, ntxca);
	printf("\
   LOC   FLAGS   DADDR   CADDR    SIZE   IPTR   CNT CCNT   FORW     BACK\n");
	for (xp = xtext; xp < &xtext[ntext]; xp++) {
		if (xp->x_iptr == NULL)
			continue;
		printf("%7.1o ", atext + (xp - xtext)*sizeof (*xp));
		putf((long)xp->x_flag&XPAGI, 'P');
		putf((long)xp->x_flag&XTRC, 'T');
		putf((long)xp->x_flag&XWRIT, 'W');
		putf((long)xp->x_flag&XLOAD, 'L');
		putf((long)xp->x_flag&XLOCK, 'K');
		putf((long)xp->x_flag&XWANT, 'w');
		putf((long)xp->x_flag&XUNUSED, 'u');
		printf("%7.1o ", xp->x_daddr);
		printf("%7.1o ", xp->x_caddr);
		printf("%7.1o ", xp->x_size);
		printf("%7.1o", xp->x_iptr);
		printf("%4u ", xp->x_count);
		printf("%4u ", xp->x_ccount);
		printf("%7.1o ", xp->x_forw);
		printf("%7.1o\n", xp->x_back);
	}
	free(xtext);
}

doproc()
{
	struct proc *xproc;
	u_int nproc, aproc;
	register struct proc *pp;
	register loc, np;

	nproc = getw((off_t)nl[SNPROC].n_value);
	xproc = (struct proc *)calloc(nproc, sizeof (struct proc));
	aproc = nl[SPROC].n_value;
	if (nproc < 0 || nproc > 10000) {
		fprintf(stderr, "number of procs is preposterous (%d)\n",
			nproc);
		return;
	}
	if (xproc == NULL) {
		fprintf(stderr, "can't allocate memory for proc table\n");
		return;
	}
	lseek(fc, (off_t)aproc, 0);
	read(fc, xproc, nproc * sizeof (struct proc));
	np = 0;
	for (pp=xproc; pp < &xproc[nproc]; pp++)
		if (pp->p_stat)
			np++;
	if (totflg) {
		printf("%3d/%3d processes\n", np, nproc);
		return;
	}
	printf("%d/%d processes\n", np, nproc);
printf("   LOC   S       F PRI      SIG   UID SLP TIM  CPU  NI   PGRP    PID   PPID    ADDR   SADDR   DADDR    SIZE   WCHAN    LINK   TEXTP SIGM\n");
	for (pp=xproc; pp<&xproc[nproc]; pp++) {
		if (pp->p_stat==0 && allflg==0)
			continue;
		printf("%7.1o", aproc + (pp - xproc)*sizeof (*pp));
		printf(" %2d", pp->p_stat);
		printf(" %7.1x", pp->p_flag);
		printf(" %3d", pp->p_pri);
		printf(" %8.1lx", pp->p_sig);
		printf(" %5u", pp->p_uid);
		printf(" %3d", pp->p_slptime);
		printf(" %3d", pp->p_time);
		printf(" %4d", pp->p_cpu&0377);
		printf(" %3d", pp->p_nice);
		printf(" %6d", pp->p_pgrp);
		printf(" %6d", pp->p_pid);
		printf(" %6d", pp->p_ppid);
		printf(" %7.1o", pp->p_addr);
		printf(" %7.1o", pp->p_saddr);
		printf(" %7.1o", pp->p_daddr);
		printf(" %7.1o", pp->p_dsize+pp->p_ssize);
		printf(" %7.1o", pp->p_wchan);
		printf(" %7.1o", pp->p_link);
		printf(" %7.1o", pp->p_textp);
		printf(" %8.1lx", pp->p_sigmask);
		printf("\n");
	}
	free(xproc);
}

static char mesg[] =
" # RAW CAN OUT         MODE    ADDR  DEL  COL      STATE       PGRP DISC\n";
static int ttyspace = 64;
static struct tty *tty;

dotty()
{
	extern char *malloc();

	if ((tty = (struct tty *)malloc(ttyspace * sizeof(*tty))) == 0) {
		printf("pstat: out of memory\n");
		return;
	}
	dottytype("kl", SKL, SNKL);
	if (nl[SNDZ].n_type != 0)
		dottytype("dz", SDZ, SNDZ);
	if (nl[SNDH].n_type != 0)
		dottytype("dh", SDH, SNDH);
	if (nl[SNDHU].n_type != 0)
		dottytype("dhu", SDHU, SNDHU);
	if (nl[SNDHV].n_type != 0)
		dottytype("dhv", SDHV, SNDHV);
	if (nl[SNPTY].n_type != 0)
		dottytype("pty", SPTY, SNPTY);
}

dottytype(name, type, number)
char *name;
{
	int ntty;
	register struct tty *tp;
	extern char *realloc();

	lseek(fc, (long)nl[number].n_value, 0);
	read(fc, &ntty, sizeof(ntty));
	printf("%d %s lines\n", ntty, name);
	if (ntty > ttyspace) {
		ttyspace = ntty;
		if ((tty = (struct tty *)realloc(tty, ttyspace * sizeof(*tty))) == 0) {
			printf("pstat: out of memory\n");
			return;
		}
	}
	lseek(fc, (long)nl[type].n_value, 0);
	read(fc, tty, ntty * sizeof(struct tty));
	printf(mesg);
	for (tp = tty; tp < &tty[ntty]; tp++)
		ttyprt(tp, tp - tty);
}

ttyprt(atp, line)
struct tty *atp;
{
	register struct tty *tp;

	printf("%2d", line);
	tp = atp;

	printf("%4d%4d", tp->t_rawq.c_cc, tp->t_canq.c_cc);
	printf("%4d %12.1lo %7.1o %4d %4d ", tp->t_outq.c_cc, tp->t_flags,
		tp->t_addr, tp->t_delct, tp->t_col);
	putf(tp->t_state&TS_TIMEOUT, 'T');
	putf(tp->t_state&TS_WOPEN, 'W');
	putf(tp->t_state&TS_ISOPEN, 'O');
	putf(tp->t_state&TS_FLUSH, 'F');
	putf(tp->t_state&TS_CARR_ON, 'C');
	putf(tp->t_state&TS_BUSY, 'B');
	putf(tp->t_state&TS_ASLEEP, 'A');
	putf(tp->t_state&TS_XCLUDE, 'X');
	putf(tp->t_state&TS_TTSTOP, 'S');
	putf(tp->t_state&TS_HUPCLS, 'H');
	putf(tp->t_state&TS_TBLOCK, 'b');
	putf(tp->t_state&TS_RCOLL, 'r');
	putf(tp->t_state&TS_WCOLL, 'w');
	putf(tp->t_state&TS_ASYNC, 'a');
	printf("%6d", tp->t_pgrp);
	switch (tp->t_line) {

	case OTTYDISC:
		printf("\n");
		break;

	case NTTYDISC:
		printf(" ntty\n");
		break;

	case NETLDISC:
		printf(" berknet\n");
		break;

	case TABLDISC:
		printf(" tab\n");
		break;

	case SLIPDISC:
		printf(" slip\n");
		break;

	default:
		printf(" %d\n", tp->t_line);
	}
}

dousr()
{
	struct user U;
	long	*ip;
	register i, j;

	lseek(fm, ubase << 6, 0);
	read(fm, &U, sizeof(U));
	printf("pcb\t%.1o\n", U.u_pcb.pcb_sigc);
	printf("fps\t%.1o %g %g %g %g %g %g\n", U.u_fps.u_fpsr, 
		U.u_fps.u_fpregs[0], U.u_fps.u_fpregs[1], U.u_fps.u_fpregs[2],
		U.u_fps.u_fpregs[3], U.u_fps.u_fpregs[4], U.u_fps.u_fpregs[5]);
	printf("fpsaved\t%d\n", U.u_fpsaved);
	printf("fperr\t%.1o %.1o\n", U.u_fperr.f_fec, U.u_fperr.f_fea);
	printf("procp\t%.1o\n", U.u_procp);
	printf("ar0\t%.1o\n", U.u_ar0);
	printf("comm\t%s\n", U.u_comm);
	printf("arg\t%.1o %.1o %.1o %.1o %.1o %.1o\n", U.u_arg[0], U.u_arg[1],
		U.u_arg[2], U.u_arg[3], U.u_arg[4], U.u_arg[5]);
	printf("ap\t%.1o\n", U.u_ap);
	printf("qsave\t");
	for	(i = 0; i < sizeof (label_t) / sizeof (int); i++)
		printf("%.1o ", U.u_qsave.val[i]);
	printf("\n");
	printf("r_val1\t%.1o\n", U.u_r.r_val1);
	printf("r_val2\t%.1o\n", U.u_r.r_val2);
	printf("error\t%d\n", U.u_error);
	printf("uids\t%d,%d,%d,%d,%d\n", U.u_uid, U.u_svuid, U.u_ruid,
		U.u_svgid, U.u_rgid);
	printf("groups");
	for	(i = 0; (i < NGROUPS) && (U.u_groups[i] != NOGROUP); i++)
		{
		if	(i%8 == 0) printf("\t");
		printf("%u ", U.u_groups[i]);
		if	(i%8 == 7) printf("\n");
		}
	if	(i%8) printf("\n");
	printf("tsize\t%.1o\n", U.u_tsize);
	printf("dsize\t%.1o\n", U.u_dsize);
	printf("ssize\t%.1o\n", U.u_ssize);
	printf("ssave\t");
	for	(i = 0; i < sizeof (label_t) / sizeof (int); i++)
		printf("%.1o ", U.u_ssave.val[i]);
	printf("\n");
	printf("rsave\t");
	for	(i = 0; i < sizeof (label_t) / sizeof (int); i++)
		printf("%.1o ", U.u_rsave.val[i]);
	printf("\n");
	printf("uisa");
	for	(i = 0; i < sizeof (U.u_uisa) / sizeof (short); i++)
		{
		if	(i%8 == 0) printf("\t");
		printf("%.1o ", U.u_uisa[i]);
		if	(i%8 == 7) printf("\n");
		}
	if	(i%8) printf("\n");
	printf("uisd");
	for	(i = 0; i < sizeof (U.u_uisd) / sizeof (short); i++)
		{
		if	(i%8 == 0) printf("\t");
		printf("%.1o ", U.u_uisd[i]);
		if	(i%8 == 7) printf("\n");
		}
	if	(i%8) printf("\n");
	printf("sep\t%d\n", U.u_sep);
	printf("ovdata\t%d %d %.1o %d\n", U.u_ovdata.uo_curov, 
		U.u_ovdata.uo_ovbase, U.u_ovdata.uo_dbase, U.u_ovdata.uo_nseg);
	printf("ov_offst");
	for	(i = 0; i < NOVL; i++)
		{
		if	(i%8 == 0) printf("\t");
		printf("%.1o ", U.u_ovdata.uo_ov_offst[i]);
		if	(i%8 == 7) printf("\n");
		}
	if	(i%8) printf("\n");
	printf("signal");
	for	(i = 0; i < NSIG; i++)
		{
		if	(i%8 == 0) printf("\t");
		printf("%.1o ", U.u_signal[i]);
		if	(i%8 == 7) printf("\n");
		}
	if	(i%8) printf("\n");
	printf("sigmask");
	for	(i = 0; i < NSIG; i++)
		{
		if	(i%8 == 0) printf("\t");
		printf("%.1lo ", U.u_sigmask[i]);
		if	(i%8 == 7) printf("\n");
		}
	if	(i%8) printf("\n");
	printf("sigonstack\t%.1lo\n", U.u_sigonstack);
	printf("sigintr\t%.1lo\n", U.u_sigintr);
	printf("oldmask\t%.1lo\n", U.u_oldmask);
	printf("code\t%u\n", U.u_code);
	printf("psflags\t%d\n", U.u_psflags);
	printf("ss_base\t%.1o ss_size %.1o ss_flags %.1o\n",
		U.u_sigstk.ss_base, U.u_sigstk.ss_size, U.u_sigstk.ss_flags);
	printf("ofile");
	for	(i = 0; i < NOFILE; i++)
		{
		if	(i%8 == 0) printf("\t");
		printf("%.1o ", U.u_ofile[i]);
		if	(i%8 == 7) printf("\n");
		}
	if	(i%8) printf("\n");
	printf("pofile");
	for	(i = 0; i < NOFILE; i++)
		{
		if	(i%8 == 0) printf("\t");
		printf("%.1o ", U.u_pofile[i]);
		if	(i%8 == 7) printf("\n");
		}
	if	(i%8) printf("\n");
	printf("lastfile\t%d\n", U.u_lastfile);
	printf("cdir\t%.1o\n", U.u_cdir);
	printf("rdir\t%.1o\n", U.u_rdir);
	printf("ttyp\t%.1o\n", U.u_ttyp);
	printf("ttyd\t%d,%d\n", major(U.u_ttyd), minor(U.u_ttyd));
	printf("cmask\t%.1o\n", U.u_cmask);
	printf("ru\t");
	ip = (long *)&U.u_ru;
	for	(i = 0; i < sizeof (U.u_ru) / sizeof (long); i++)
		printf("%ld ", ip[i]);
	printf("\n");
	printf("cru\t");
	ip = (long *)&U.u_cru;
	for	(i = 0; i < sizeof (U.u_cru) / sizeof (long); i++)
		printf("%ld ", ip[i]);
	printf("\n");
	printf("timer\t%ld %ld %ld %ld\n", U.u_timer[0].it_interval,
		U.u_timer[0].it_value, U.u_timer[1].it_interval,
		U.u_timer[1].it_value);
	printf("start\t%ld\n", U.u_start);
	printf("acflag\t%d\n", U.u_acflag);
	printf("prof\t%.1o %u %u %u\n", U.u_prof.pr_base, U.u_prof.pr_size,
		U.u_prof.pr_off, U.u_prof.pr_scale);
	printf("rlimit cur\t");
	for	(i = 0; i < RLIM_NLIMITS; i++)
		{
		if	(U.u_rlimit[i].rlim_cur == RLIM_INFINITY)
			printf("infinite ");
		else
			printf("%ld ", U.u_rlimit[i].rlim_cur);
		}
	printf("\n");
	printf("rlimit max\t");
	for	(i = 0; i < RLIM_NLIMITS; i++)
		{
		if	(U.u_rlimit[i].rlim_max == RLIM_INFINITY)
			printf("infinite ");
		else
			printf("%ld ", U.u_rlimit[i].rlim_max);
		}
	printf("\n");
	printf("quota\t%.1o\n", U.u_quota);
	printf("ncache\t%ld %u %d,%d\n", U.u_ncache.nc_prevoffset,
		U.u_ncache.nc_inumber, major(U.u_ncache.nc_dev),	
		minor(U.u_ncache.nc_dev));
	printf("login\t%*s\n", MAXLOGNAME, U.u_login);
}

oatoi(s)
char *s;
{
	register v;

	v = 0;
	while (*s)
		v = (v<<3) + *s++ - '0';
	return(v);
}

dofile()
{
	int nfile;
	struct file *xfile;
	register struct file *fp;
	register nf;
	u_int loc, afile;
	static char *dtypes[] = { "???", "inode", "socket", "pipe" };

	nf = 0;
	nfile = getw((off_t)nl[SNFILE].n_value);
	xfile = (struct file *)calloc(nfile, sizeof (struct file));
	if (nfile < 0 || nfile > 10000) {
		fprintf(stderr, "number of files is preposterous (%d)\n",
			nfile);
		return;
	}
	if (xfile == NULL) {
		fprintf(stderr, "can't allocate memory for file table\n");
		return;
	}
	afile = nl[SFIL].n_value;
	lseek(fc, (off_t)afile, 0);
	read(fc, xfile, nfile * sizeof (struct file));
	for (fp=xfile; fp < &xfile[nfile]; fp++)
		if (fp->f_count)
			nf++;
	if (totflg) {
		printf("%3d/%3d files\n", nf, nfile);
		return;
	}
	printf("%d/%d open files\n", nf, nfile);
	printf("   LOC   TYPE   FLG        CNT  MSG   DATA    OFFSET\n");
	loc = afile;
	for	(fp=xfile; fp < &xfile[nfile]; fp++, loc += sizeof (*fp))
		{
		if (fp->f_count==0)
			continue;
		printf("%7.1o ", loc);
		if (fp->f_type <= DTYPE_PIPE)
			printf("%-8.8s", dtypes[fp->f_type]);
		else
			printf("8d", fp->f_type);
		putf((long)fp->f_flag&FREAD, 'R');
		putf((long)fp->f_flag&FWRITE, 'W');
		putf((long)fp->f_flag&FAPPEND, 'A');
		putf((long)fp->f_flag&FSHLOCK, 'S');
		putf((long)fp->f_flag&FEXLOCK, 'X');
		putf((long)fp->f_flag&FASYNC, 'I');
		putf((long)fp->f_flag&FNONBLOCK, 'n');
		putf((long)fp->f_flag&FMARK, 'm');
		putf((long)fp->f_flag&FDEFER, 'd');
		printf("  %3d", fp->f_count);
		printf("  %3d", fp->f_msgcount);
		printf("  %7.1o", fp->f_data);
		if (fp->f_offset < 0)
			printf("  0%lo\n", fp->f_offset);
		else
			printf("  %ld\n", fp->f_offset);
	}
	free(xfile);
}

/* 
 * common function to handle -s, -c and -m: swapmap, coremap and ub_map. 
 * 2008-12-30 - wfjm 
*/

domap(flag, nlind)
int flag;
int nlind;
{
	size_t	nswap, size, freetot, freemax;
	int	i, num, mshift;
	struct	map	maphdr;
	struct	mapent	*maptbl;
	char	*name;

	switch(nlind) {
	case SWAPMAP:
		name = "swapmap";
		mshift = -1;	/* disk blocks -> kBytes */
		break;
	case SCOREMAP:
		name = "coremap";
		mshift = -4;	/* clocks -> kBytes */
		break;
	case SUBMAP:
		name = "ub_map";
		mshift = 3;	/* ubmap pages -> kBytes */
		break;
	default:
		return;
	}

	if (nl[nlind].n_type == 0) return;

	lseek(fc, (off_t)nl[nlind].n_value, 0);
	read(fc, &maphdr, sizeof (maphdr));

	num = (maphdr.m_limit - maphdr.m_map);
	maptbl = (struct mapent *)calloc(num, sizeof (*maptbl));
	if (maptbl == NULL) {
		fprintf(stderr, "can't allocate memory for %s table\n", name);
		return;
	}

	lseek(fc, (off_t)maphdr.m_map, 0);
	read(fc, maptbl, num * sizeof (*maptbl));


	if (flag) printf("   ADDR   SIZE     kB\n");	
	for	(freetot = 0, freemax = 0, i = 0; maptbl[i].m_size; i++)
	{
		size = maptbl[i].m_size;
		freetot += size;
		if (size>freemax) freemax = size;
		if(flag) {
			printf("%7.1o %6u %6u\n",
				maptbl[i].m_addr, size, size<<mshift);
		}
	}

	printf("%3d/%3d %7s entries, ", i, num, name);

	switch(nlind) {
	case SWAPMAP:
		nswap = getw((off_t)nl[SNSWAP].n_value);
		printf("%4u kB used, %4u kB free, %4u kB max\n",
		       (nswap-freetot)/2, freetot/2, freemax/2);
		break;
	case SCOREMAP:
		printf("%4u kB free, %4u kB max\n", 
			freetot/16, freemax/16);
		break;
	case SUBMAP:
		printf("%4u    free, %4u    max\n", 
			freetot, freemax);
		break;
	}

	free(maptbl);
}

dobuf()
{
	register struct buf *bp;
	int nbuf;
	struct buf *xbuf;
	u_int abuf, abufe;
	u_long l_baddr;
	int i;
	
	nbuf = getw((off_t)nl[SNBUF].n_value);
	if (nbuf <= 0 || nbuf >= 2000) {
		fprintf(stderr, "number of buffers is preposterous (%d)\n",
			nbuf);
		return;
	}

	xbuf = (struct buf *) calloc(nbuf, sizeof (struct buf));
	if (xbuf == NULL) {
		fprintf(stderr, "can't allocate memory for buf table\n");
		return;
	}

	abuf = nl[SBUF].n_value;
	lseek(fc, (off_t)abuf, 0);
	read(fc, xbuf, nbuf * sizeof (struct buf));
	abufe = abuf + nbuf * sizeof (struct buf);

	if (buff) {
		printf("IND    LOC       FLAGS         ");
		printf("FORW   BACKW   AFORW  ABACKW   DEVICE   BLKNO\n");

		for (bp=xbuf, i=0; bp<&xbuf[nbuf]; bp++, i++) {
			printf("%3d ", i);
			printf("%7.1o ", abuf + (bp - xbuf)*sizeof(*bp));
			putf((long)bp->b_flags&B_READ,   'R');
			putf((long)bp->b_flags&B_DONE,   'D');
			putf((long)bp->b_flags&B_ERROR,  'E');
			putf((long)bp->b_flags&B_BUSY,   'B');
			putf((long)bp->b_flags&B_PHYS,   'P');
			putf((long)bp->b_flags&B_MAP,    'M');
			putf((long)bp->b_flags&B_WANTED, 'W');
			putf((long)bp->b_flags&B_AGE,    'A');
			putf((long)bp->b_flags&B_ASYNC,  'a');
			putf((long)bp->b_flags&B_DELWRI, 'd');
			putf((long)bp->b_flags&B_TAPE,   'T');
			putf((long)bp->b_flags&B_INVAL,  'I');
			putf((long)bp->b_flags&B_BAD,    'b');
			putf((long)bp->b_flags&B_LOCKED, 'L');
			putf((long)bp->b_flags&B_UBAREMAP,'u');
			putf((long)bp->b_flags&B_RAMREMAP,'r');
			pbufbp(abuf, abufe, bp->b_forw);
			pbufbp(abuf, abufe, bp->b_back);
			pbufbp(abuf, abufe, bp->av_forw);
			pbufbp(abuf, abufe, bp->av_back);
			printf("%4d,%3d", major(bp->b_dev), minor(bp->b_dev));
			printf("%8ld ", bp->b_blkno);
			printf("\n");
		}
	}
}

pbufbp(abuf, abufe, bp)
struct buf *abuf, *abufe;
struct buf *bp;
{
	u_int ind;
	ind = ((u_int)bp - (u_int)abuf) / (u_int)sizeof(*bp);
	if (bp >= abuf && bp < abufe) {
		printf("  ->%3d ", ind);
	} else {
		printf("%7.1o ", bp);
	}
}

