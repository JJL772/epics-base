/* $Id */
/************************************************************************/
/*									*/
/*	        	      L O S  A L A M O S			*/
/*		        Los Alamos National Laboratory			*/
/*		         Los Alamos, New Mexico 87545			*/
/*									*/
/*	Copyright, 1986, The Regents of the University of California.	*/
/*									*/
/*									*/
/*	History								*/
/*	-------								*/
/*	.01 08xx87 joh	Init Release					*/
/*	.02 01xx90 joh	fd_set in the UNIX version only			*/
/*	.03 060691 joh	Rearanged buffer struct for SPARC port		*/
/*	.04 072391 joh	new lock prevents event preemption on vxWorks	*/
/*	.05 082791 joh	declaration of ca_request_event()		*/
/*	.06 082791 joh	added send message in progress flag		*/
/*	.07 091691 joh	moved channel_state enum to cadef.h for export	*/
/*	.08 102991 joh	added sprintf buffer				*/
/*	.09 111891 joh	added event task id for vxWorks			*/
/*	.10 111991 joh	mobe IODONESUB macro to service.c		*/
/*	.11 031692 joh	added declaration for post_msg()		*/
/*	.12 031892 joh	initial rebroadcast delay is now a #define	*/
/*	.13 050492 joh	added exception channel fd set			*/
/*	.14 111792 joh	increased MAXCONNTRIES from 3 to 30		*/
/*	.15 112092 joh	added AST lck count var for VMS			*/
/*	.16 120992 joh	switched to dll list routines			*/
/*	.17 121892 joh	added TCP send buf size var			*/
/*	.18 122192 joh	added outstanding ack var			*/
/*	.19 012094 joh	added minor version (for each server)		*/
/* $Log$								*/
/*									*/
/*_begin								*/
/************************************************************************/
/*									*/
/*	Title:	channel access TCPIP interface include file		*/
/*	File:	atcs:[ca]iocinf.h					*/
/*	Environment: VMS, UNIX, VRTX					*/
/*									*/
/*									*/
/*	Purpose								*/
/*	-------								*/
/*									*/
/*	ioc interface include						*/
/*									*/
/*									*/
/*	Special comments						*/
/*	------- --------						*/
/*	Use GLBLTYPE to define externals so we can change the all at	*/
/*	once from VAX globals to generic externals			*/
/*									*/
/************************************************************************/
/*_end									*/
#ifndef INCiocinfh  
#define INCiocinfh

#ifdef CA_GLBLSOURCE
#    	define GLBLTYPE
#else
#    	define GLBLTYPE extern
#endif

#ifdef __STDC__
#	define VERSIONID(NAME,VERS) \
       		char *EPICS_CAS_VID_ ## NAME = VERS;
#else /*__STDC__*/
#	define VERSIONID(NAME,VERS) \
       		char *EPICS_CAS_VID_/* */NAME = VERS;
#endif /*__STDC__*/

#ifdef CAC_VERSION_GLOBAL
#	define HDRVERSIONID(NAME,VERS) VERSIONID(NAME,VERS)
#else /*CAC_VERSION_GLOBAL*/
#       define HDRVERSIONID(NAME,VERS)
#endif /*CAC_VERSION_GLOBAL*/

HDRVERSIONID(iocinfh, "$Id$")

/*
 * ANSI C includes
 */
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <stdlib.h>
#include <limits.h>
#include <stdarg.h>

#include <shareLib.h>

/*
 * OS dependent includes
 */
#include "os_depen.h"

/*
 * EPICS includes
 */
#include <epicsAssert.h>
#include <cadef.h>
#include <bucketLib.h>
#include <ellLib.h> 
#include <envDefs.h> 
#include <epicsPrint.h> 

/*
 * CA private includes 
 */
#include "addrList.h"
#include "iocmsg.h"

#ifndef min
#define min(A,B) ((A)>(B)?(B):(A))
#endif

#ifndef max 
#define max(A,B) ((A)<(B)?(B):(A))
#endif

#ifndef NBBY
# define NBBY 8 /* number of bits per byte */
#endif

#define MSEC_PER_SEC 	1000L
#define USEC_PER_SEC 	1000000L

/*
 * catch when they use really large strings
 */
#define STRING_LIMIT	512

#define INITCHK \
if(!ca_static){ \
        int     s; \
        s = ca_task_initialize(); \
        if(s != ECA_NORMAL){ \
                return s; \
        } \
}

/* throw out requests prior to last ECA_TIMEOUT from ca_pend */
#define	VALID_MSG(PIIU) (piiu->read_seq == piiu->cur_read_seq)

#define SETPENDRECV		{pndrecvcnt++;}
#define CLRPENDRECV(LOCK)	{if(--pndrecvcnt<1){cac_io_done(LOCK); POST_IO_EV;}}

struct udpmsglog{
	long                    nbytes;
	int			valid;
	struct sockaddr_in      addr;
};

struct putCvrtBuf{
	ELLNODE			node;
	unsigned long		size;
	void			*pBuf;
};

/*
 * for use with cac_select_io()
 */
#define CA_DO_SENDS	1
#define CA_DO_RECVS	2

struct pending_io_event{
  	ELLNODE			node;
  	void			(*io_done_sub)();
  	void			*io_done_arg;
};

typedef struct timeval ca_time;

#define LD_CA_TIME(FLOAT_TIME,PCATIME) \
((PCATIME)->tv_sec = (long) (FLOAT_TIME), \
(PCATIME)->tv_usec = (long) ( ((FLOAT_TIME)-(PCATIME)->tv_sec)*USEC_PER_SEC ))

/*
 * dont adjust
 */
#ifdef CA_GLBLSOURCE
const ca_time CA_CURRENT_TIME = {0,0};
#else /*CA_GLBLSOURCE*/
extern const ca_time CA_CURRENT_TIME;
#endif /*CA_GLBLSOURCE*/

/*
 * these control the duration and period of name resolution
 * broadcasts
 *
 * MAXCONNTRIES must be less than the number of bits in type long
 */
#define MAXCONNTRIES 		30	/* N conn retries on unchanged net */

#define SELECT_POLL 		(0.05) 	/* units sec - polls into recast */
#define CA_RECAST_DELAY 	(0.1)   /* initial delay to next recast (sec) */
#define CA_RECAST_PORT_MASK	0xff	/* random retry interval off port */
#define CA_RECAST_PERIOD 	(5.0)	/* ul on retry period long term (sec) */

/*
 * these two control the period of connection verifies
 * (echo requests) - CA_CONN_VERIFY_PERIOD - and how
 * long we will wait for an echo reply before we
 * give up and flag the connection for disconnect
 * - CA_ECHO_TIMEOUT.
 *
 * CA_CONN_VERIFY_PERIOD is normally obtained from an
 * EPICS environment variable.
 */
#define CA_ECHO_TIMEOUT		5.0	/* (sec) disconn no echo reply tmo */ 
#define CA_CONN_VERIFY_PERIOD	30.0	/* (sec) how often to request echo */

/*
 * only used when communicating with old servers
 */
#define CA_RETRY_PERIOD		5	/* int sec to next keepalive */

#define N_REPEATER_TRIES_PRIOR_TO_MSG	50
#define REPEATER_TRY_PERIOD		(1.0) 

#ifdef vxWorks
typedef struct caclient_put_notify{
	ELLNODE			node;
	PUTNOTIFY		dbPutNotify;
	unsigned long		valueSize; /* size of block pointed to by dbPutNotify */
	void			(*caUserCallback)(struct event_handler_args);
	void			*caUserArg;
	struct ca_static	*pcas;
	int			busy;
}CACLIENTPUTNOTIFY;
#endif /*vxWorks*/

#define MAX_CONTIGUOUS_MSG_COUNT 2

/*
 * ! lock needs to be applied when an id is allocated !
 */
#define CLIENT_HASH_TBL_SIZE	(1<<12)
#define CLIENT_ID_WIDTH 	28 /* bits */
#define CLIENT_ID_COUNT		(1<<CLIENT_ID_WIDTH)
#define CLIENT_ID_MASK		(CLIENT_ID_COUNT-1)
#define CLIENT_FAST_ID_ALLOC	(CLIENT_ID_MASK&nextFastBucketId++)
#define CLIENT_SLOW_ID_ALLOC	(CLIENT_ID_MASK&nextSlowBucketId++)

#define SEND_RETRY_COUNT_INIT	100

#define iiuList 	(ca_static->ca_iiuList)
#define piiuCast 	(ca_static->ca_piiuCast)
#define pndrecvcnt	(ca_static->ca_pndrecvcnt)
#define chidlist_pend	(ca_static->ca_chidlist_pend)
#define chidlist_conn	(ca_static->ca_chidlist_conn)
#define chidlist_noreply\
			(ca_static->ca_chidlist_noreply)
#define ioeventlist	(ca_static->ca_ioeventlist)
#define nxtiiu		(ca_static->ca_nxtiiu)
#define free_event_list	(ca_static->ca_free_event_list)
#define pend_read_list	(ca_static->ca_pend_read_list)
#define pend_write_list	(ca_static->ca_pend_write_list)
#define fd_register_func\
			(ca_static->ca_fd_register_func)
#define fd_register_arg	(ca_static->ca_fd_register_arg)
#define post_msg_active	(ca_static->ca_post_msg_active)
#define sprintf_buf	(ca_static->ca_sprintf_buf)
#define pSlowBucket	(ca_static->ca_pSlowBucket)
#define pFastBucket	(ca_static->ca_pFastBucket)
#define nextSlowBucketId (ca_static->ca_nextSlowBucketId)
#define nextFastBucketId (ca_static->ca_nextFastBucketId)

#if defined(vxWorks)
#	define io_done_sem	(ca_static->ca_io_done_sem)
#	define evuser		(ca_static->ca_evuser)
#	define client_lock	(ca_static->ca_client_lock)
#	define event_lock	(ca_static->ca_event_lock)
#	define local_chidlist	(ca_static->ca_local_chidlist)
#	define dbfree_ev_list	(ca_static->ca_dbfree_ev_list)
#	define lcl_buff_list	(ca_static->ca_lcl_buff_list)
#	define event_tid	(ca_static->ca_event_tid)
#endif

/*
 * one for each task that does a ca import
 */
typedef struct task_var_list{
	ELLNODE			node;
	int			tid;
}TVIU;


/*
 * Ring buffering for both sends and recvs
 */
struct ca_buffer{
  	char			buf[MAX_MSG_SIZE]; /* from iocmsg.h */
	unsigned long		max_msg;
	unsigned long		rdix;
  	unsigned long		wtix;
	int			readLast;
};

#define CAC_RING_BUFFER_INIT(PRBUF, SIZE) \
	assert((SIZE)>sizeof((PRBUF)->buf)); \
	(PRBUF)->max_msg = (SIZE); \
	(PRBUF)->rdix = 0; \
	(PRBUF)->wtix = 0; \
	(PRBUF)->readLast = TRUE; 

#define CAC_RING_BUFFER_READ_ADVANCE(PRBUF, DELTA) \
( 	(PRBUF)->readLast = TRUE, \
	((PRBUF)->rdix += (DELTA)) >= (PRBUF)->max_msg ? \
	(PRBUF)->rdix = 0 :  \
	(PRBUF)->rdix  \
) 

#define CAC_RING_BUFFER_WRITE_ADVANCE(PRBUF, DELTA) \
( 	(PRBUF)->readLast = FALSE, \
	((PRBUF)->wtix += (DELTA)) >= (PRBUF)->max_msg ? \
	(PRBUF)->wtix = 0 :  \
	(PRBUF)->wtix  \
) 

#define TAG_CONN_DOWN(PIIU) \
( \
/* ca_printf("Tagging connection down at %d in %s\n", __LINE__, __FILE__), */ \
(PIIU)->conn_up = FALSE \
)

/*
 * One per IOC
 */
typedef struct ioc_in_use{
	ELLNODE			node;
	ELLLIST			chidlist;	/* chans on this connection */
	ELLLIST			destAddr;
	ca_time			timeAtLastRecv;
	ca_time			timeAtSendBlock;
	ca_time			timeAtEchoRequest;
	unsigned long		curDataMax;
	unsigned long		curDataBytes;
	struct ca_buffer	send;
	struct ca_buffer	recv;
	struct extmsg		curMsg;
	struct ca_static	*pcas;
	void			*pCurData;
	void			(*sendBytes)(struct ioc_in_use *);
	void			(*recvBytes)(struct ioc_in_use *);
	void			(*procInput)(struct ioc_in_use *);
	SOCKET			sock_chan;
	int			sock_proto;
	unsigned		minor_version_number;
	unsigned		contiguous_msg_count;
	unsigned		curMsgBytes;
	unsigned		read_seq;
	unsigned 		cur_read_seq;
	unsigned		minfreespace;
	char			host_name_str[32];

	/*
	 * bit fields placed together for better packing density
	 */
	unsigned		client_busy:1;
	unsigned		echoPending:1; 
	unsigned		send_needed:1;	/* CA needs a send */
	unsigned		conn_up:1;   /* boolean: T-conn /F-disconn */
	unsigned		sendPending:1;
}IIU;

/*
 * for the beacon's recvd hash table
 */
#define BHT_INET_ADDR_MASK		0x7f
typedef struct beaconHashEntry{
	struct beaconHashEntry	*pNext;
	IIU			*piiu;
	struct in_addr 		inetAddr;
	ca_time			timeStamp;
	ca_real			averagePeriod;
}bhe;

/*
 * This struct allocated off of a free list 
 * so that the select() ctx is thread safe
 */
typedef struct {
	ELLNODE		node;
	fd_set          readMask;  
	fd_set          writeMask;  
}caFDInfo;

struct  ca_static{
	ELLLIST		ca_iiuList;
	ELLLIST		ca_ioeventlist;
	ELLLIST		ca_free_event_list;
	ELLLIST		ca_pend_read_list;
	ELLLIST		ca_pend_write_list;
	ELLLIST		activeCASG;
	ELLLIST		freeCASG;
	ELLLIST		activeCASGOP;
	ELLLIST		freeCASGOP;
	ELLLIST		putCvrtBuf;
	ELLLIST		fdInfoFreeList;
	ELLLIST		fdInfoList;
	ca_time		currentTime;
	ca_time		ca_conn_next_retry;
	ca_time		ca_conn_retry_delay;
	ca_time		ca_last_repeater_try;
	ca_real		ca_connectTMO;
	long		ca_pndrecvcnt;
	unsigned long	ca_nextSlowBucketId;
	unsigned long	ca_nextFastBucketId;
	IIU		*ca_piiuCast;
	void		(*ca_exception_func)
				(struct exception_handler_args);
	void		*ca_exception_arg;
	void		(*ca_connection_func)
				(struct connection_handler_args);
	void		*ca_connection_arg;
	int		(*ca_printf_func)(const char *pformat, va_list args);
	void		(*ca_fd_register_func)
				(void *, SOCKET, int);
	void		*ca_fd_register_arg;
	char		*ca_pUserName;
	char		*ca_pHostName;
	BUCKET		*ca_pSlowBucket;
	BUCKET		*ca_pFastBucket;
	bhe		*ca_beaconHash[BHT_INET_ADDR_MASK+1];
	unsigned	ca_repeater_tries;
	unsigned	ca_search_retry; /* search retry seq number */
	unsigned	ca_search_responses; /* num search resp within seq # */
	unsigned short	ca_server_port;
	unsigned short	ca_repeater_port;
	char		ca_sprintf_buf[256];
	unsigned 	ca_post_msg_active:1; 
	unsigned 	ca_manage_conn_active:1; 
	unsigned 	ca_repeater_contacted:1;
#if defined(vxWorks)
	SEM_ID		ca_io_done_sem;
	SEM_ID		ca_blockSem;
	SEM_ID		ca_client_lock; 
	SEM_ID		ca_event_lock; /* dont allow events to preempt */
	SEM_ID		ca_putNotifyLock;
	ELLLIST		ca_local_chidlist;
	ELLLIST		ca_dbfree_ev_list;
	ELLLIST		ca_lcl_buff_list;
	ELLLIST		ca_putNotifyQue;
	ELLLIST		ca_taskVarList;
	void		*ca_evuser;
	int		ca_event_tid;
	int		ca_tid;
    	int		recv_tid;
	unsigned	ca_local_ticks;
#endif
};



#define CASG_MAGIC      0xFAB4CAFE

/*
 * one per outstanding op
 */
typedef struct{
        ELLNODE                 node;
        WRITEABLE_CA_SYNC_GID   id;
        void                    *pValue;
        unsigned long           magic;
        unsigned long           seqNo;
}CASGOP;


/*
 * one per synch group
 */
typedef struct{
        ELLNODE                 node;
        WRITEABLE_CA_SYNC_GID   id;
        unsigned long           magic;
        unsigned long           opPendCount;
        unsigned long           seqNo;
        /*
         * Asynch Notification
         */
#	ifdef vxWorks
        SEM_ID          sem;
#	endif /*vxWorks*/
}CASG;


/*
 *	GLOBAL VARIABLES
 *	There should only be one - add others to ca_static above -joh
 */

GLBLTYPE
struct ca_static *ca_static;

/*
 * CA internal functions
 *
 */

void 	cac_send_msg(void);
void 	cac_mux_io(struct timeval *ptimeout);
int	repeater_installed(void);
int	search_msg(chid chix, int reply_type);
int	ca_request_event(evid monix);
void 	ca_busy_message(struct ioc_in_use *piiu);
void	ca_ready_message(struct ioc_in_use *piiu);
void	noop_msg(struct ioc_in_use *piiu);
int 	echo_request(struct ioc_in_use *piiu, ca_time *pCurrentTime);
void 	issue_claim_channel(struct ioc_in_use *piiu, chid pchan);
void 	issue_identify_client(struct ioc_in_use *piiu);
void 	issue_client_host_name(struct ioc_in_use *piiu);
int	ca_defunct(void);
int 	ca_printf(char *pformat, ...);
void 	manage_conn(int silent);
void 	mark_server_available(const struct in_addr *pnet_addr);
void	flow_control(struct ioc_in_use *piiu);
int	broadcast_addr(struct in_addr *pcastaddr);
void	ca_repeater(void);
void 	cac_recv_task(int tid);
void 	cac_io_done(int lock);
void 	ca_sg_init(void);
void	ca_sg_shutdown(struct ca_static *ca_temp);
int 	cac_select_io(struct timeval *ptimeout, int flags);
void caHostFromInetAddr(
	const struct in_addr 	*pnet_addr,
	char			*pBuf,
	unsigned		size
);
int post_msg(
	struct ioc_in_use       *piiu,
	const struct in_addr	*pnet_addr,
	char			*pInBuf,
	unsigned long		blockSize
);
int alloc_ioc(
	const struct in_addr	*pnet_addr,
	int			port,
	struct ioc_in_use	**ppiiu
);
unsigned long cacRingBufferWrite(
	struct ca_buffer        *pRing,
	void                    *pBuf,
	unsigned long           nBytes);

unsigned long cacRingBufferRead(
	struct ca_buffer        *pRing,
	void                    *pBuf,
	unsigned long           nBytes);

unsigned long cacRingBufferWriteSize(
	struct ca_buffer 	*pBuf, 
	int 			contiguous);

unsigned long cacRingBufferReadSize(
	struct ca_buffer 	*pBuf, 
	int 			contiguous);

void caIOBlockListFree(
	ELLLIST *pList,
	chid    chan,
	int     cbRequired,
	int     status);

char *localUserName(void);

char *localHostName(void);

int create_net_chan(
struct ioc_in_use       **ppiiu,
const struct in_addr	*pnet_addr,	/* only used by TCP connections */
int			port,
int                     net_proto
);

int ca_check_for_fp(void);

void caSetupBCastAddrList (ELLLIST *pList, SOCKET sock, unsigned port);

int ca_os_independent_init (void);

void freeBeaconHash(struct ca_static *ca_temp);
void removeBeaconInetAddr(const struct in_addr *pnet_addr);
bhe *lookupBeaconInetAddr(const struct in_addr *pnet_addr);
bhe *createBeaconHashEntry(const struct in_addr *pnet_addr);
void close_ioc(IIU *piiu);
void notify_ca_repeater(void);
void cac_clean_iiu_list(void);

void ca_process_input_queue(void);
void cac_flush_internal(void);
void cac_block_for_io_completion(struct timeval *pTV);
void cac_block_for_sg_completion(CASG *pcasg, struct timeval *pTV);
void os_specific_sg_create(CASG *pcasg);
void os_specific_sg_delete(CASG *pcasg);
void os_specific_sg_io_complete(CASG *pcasg);
int cac_os_depen_init(struct ca_static *pcas);
void cac_os_depen_exit (struct ca_static *pcas);
void ca_process_exit();
void ca_spawn_repeater(void);
typedef void CACVRTFUNC(void *pSrc, void *pDest, int hton, unsigned long count);
void cac_gettimeval(struct timeval  *pt);
/* returns A - B in floating secs */
ca_real cac_time_diff(ca_time *pTVA, ca_time *pTVB);
/* returns A + B in integer secs & integer usec */
ca_time cac_time_sum(ca_time *pTVA, ca_time *pTVB);
void caIOBlockFree(evid pIOBlock);
void clearChannelResources(unsigned id);
void caSetDefaultPrintfHandler ();

/*
 * !!KLUDGE!!
 *
 * this was extracted from dbAccess.h because we are unable
 * to include both dbAccess.h and db_access.h at the
 * same time.
 */
#define M_dbAccess      (501 <<16) /*Database Access Routines */
#define S_db_Blocked (M_dbAccess|39) /*Request is Blocked*/
#define S_db_Pending (M_dbAccess|37) /*Request is pending*/

#endif /* this must be the last line in this file */
