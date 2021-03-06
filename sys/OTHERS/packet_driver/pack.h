struct header {
	char	sync;
	char	ksize;
	short	sum;
	char	cntl;
	char	ccntl;
};
#define	HDRSIZ	6

struct pack {
	short	p_state;	/* line state */
	short	p_bits;		/* mask for getepack */
	short	p_rsize;	/* input packet size */
	short	p_xsize;	/* output packet size */
	struct	header p_ihbuf;	/* input header */
	struct	header p_ohbuf; /* output header */
	char	*p_rptr;
	char	p_mode;
	char	**p_ipool;
	char	p_xcount;	/* # active output buffers */
	char	p_rcount;
	char	p_nout,p_tout;
	char	p_lpsize;	/* log(psize/32) */
	char	p_timer;
	char	p_obusy;
	char	p_srxmit;
	char	p_rwindow;	/* window size */
	char	p_swindow;
	char	p_msg;		/* control msg */
	char	p_rmsg;		/* repeated control msg */
	char	p_ps,p_pr;	/* last packet sent, recv'd */
	char	p_rpr;
	char	p_nxtps;	/* next output seq number */
	char	p_imap;		/* bit map of input buffers */
	char	p_pscopy;	/* newest output packet */
	char	*p_ob[8];	/* output buffers */
	char	*p_ib[8];	/* input buffers */
	char	p_os[8];	/* output buffer status */
	char	p_is[8];	/* input                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                 fine	PKIPRI	30

#define	NPLINES	8

/*
 * packet ioctl buf
 */
struct	piocb {
	u_short	t;
	short	psize;
	short	mode;
	short	state;
	char	window;
};

short pkdebug;
short pksizes[];
