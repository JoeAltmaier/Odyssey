/*
 * Typical driver template for support of XSNMPv1/MIB2 and XRMON1
 *
 * This driver is a non-functioning template to be used as a reference
 * for intergrators of XSNMPv1 and XRMON1.  No attempt has been made
 * to optimize or take advantage of any hardware specific assist which
 * may be present.  This driver is only a guide.  It does not compile
 * into the XSNMPv1/XRMON subsystems.
 *
 * The approach taken in the transmit and receive routines, which
 * is updating statistics on a per packet basis, may impose
 * a much too severe performance degradation under high traffic. Other
 * approaches which work as well (since MIB2/RMON1 data is not real-time)
 * are timeout routines which update statistics or interval counters
 * which update every N passes thorugh the driver.
 *
 */

/*
 * Some "typical" defines
 */
#define TYPICAL_MAX_IF			4		/* Interfaces supported by  driver	*/
#define TYPICAL_MAX_EPKT_SIZE	1518	/* 1518 bytes, max ethernet size	*/
#define TYPICAL_MIN_EPKT_SIZE	64		/* 64 bytes, min ethernet size		*/
#define TYPICAL_TX_UPDATE		10		/* When to update MIB on tx stats	*/
#define TYPICAL_RX_UPDATE		10		/* When to update MIB on rx stats	*/

#define TYPICAL_SUCCESSFUL		0		/* Dummy define for status			*/
#define TYPICAL_COLLISONS		1		/* Dummy define for status			*/
#define TYPICAL_JABBER   		2		/* Dummy define for status			*/
#define TYPICAL_CRCALIGN 		3		/* Dummy define for status			*/
#define TYPICAL_DEFERRED 		4		/* Dummy define for status			*/
#define TYPICAL_FRAGMENTS		5		/* Dummy define for status			*/

#define TYPICAL_UNICAST			0		/* A unicast packet					*/
#define TYPICAL_MCAST			1		/* A multicast packet				*/
#define TYPICAL_BCAST			2		/* A broadcast packet				*/

/*
 * Typical driver statistics structure; one of these per interface
 */
typedef typical_driver_stats_s {
	u32		index;
	/* rfc1213 */
	u32		out_octets;
	u32		out_mcasts;
	u32		out_ucasts;
	u32		out_discards;
	u32		out_errors;
	u32		out_qlen;
	u32		out_colls;

	/* rfc1213 */
	u32		in_octets;
	u32		in_mcasts;
	u32		in_ucasts;
	u32		in_discards;
	u32		in_errors;
	u32		in_noproto;

	/* rfc1757 */
	u32		octets;
	u32		pkts;
	u32		bcast;
	u32		mcast;
	u32		crcalign;
	u32		runt;     
	u32		giant;
	u32		frags;
	u32		jabber;
	u32		colls;
	u32		pkts64;
	u32		pkts65to127;
	u32		pkts128to255;
	u32		pkts256to511;
	u32		pkts512to1023;
	u32		pkts1024to1518;
} typical_driver_stats_t;

typical_driver_stats_t	dstats[TYPICAL_MAX_IF];

#ifdef XMIB_MIB2
/*
 * XSNMPv1 MIB2 callbacks
 */
extern void xset_1213outQLen           		( u32 unit, u32 value );
extern void xset_1213outErrors         		( u32 unit, u32 value );
extern void xset_1213outDiscards       		( u32 unit, u32 value );
extern void xset_1213outNUcastPkts     		( u32 unit, u32 value );
extern void xset_1213outUcastPkts      		( u32 unit, u32 value );
extern void xset_1213outOctets         		( u32 unit, u32 value );
extern void xset_1213inUcastPkts       		( u32 unit, u32 value );
extern void xset_1213inNUcastPkts      		( u32 unit, u32 value );
extern void xset_1213inUnknownProtos   		( u32 unit, u32 value );
extern void xset_1213inErrors          		( u32 unit, u32 value );
extern void xset_1213inDiscards        		( u32 unit, u32 value );
extern void xset_1213inOctets          		( u32 unit, u32 value );
#endif

#ifdef XMIB_RMON1
/*
 * XRMON1 RMON callbacks
 */
extern void xset_1757Octets                ( u32 unit, u32 value );
extern void xset_1757Pkts                  ( u32 unit, u32 value );
extern void xset_1757BroadcastPkts         ( u32 unit, u32 value );
extern void xset_1757MulticastPkts         ( u32 unit, u32 value );
extern void xset_1757CRCAlignErrors        ( u32 unit, u32 value );
extern void xset_1757UndersizePkts         ( u32 unit, u32 value );
extern void xset_1757OversizePkts          ( u32 unit, u32 value );
extern void xset_1757Fragments             ( u32 unit, u32 value );
extern void xset_1757Jabbers               ( u32 unit, u32 value );
extern void xset_1757Collisions            ( u32 unit, u32 value );
extern void xset_1757Pkts64Octets          ( u32 unit, u32 value );
extern void xset_1757Pkts65to127Octets     ( u32 unit, u32 value );
extern void xset_1757Pkts128to255Octets    ( u32 unit, u32 value );
extern void xset_1757Pkts256to511Octets    ( u32 unit, u32 value );
extern void xset_1757Pkts512to1023Octets   ( u32 unit, u32 value );
extern void xset_1757Pkts1024to1518Octets  ( u32 unit, u32 value );
#endif

/*
 * Support routines for the typical driver.  Exact form depends
 * upon OS/STACK/INTERFACE capabilities
 */
	
/*
 * This routine transmits a packet pointed to by "pkt" of length "len"
 */
u32
typical_tx_pkt( u8 *pkt, u32 len )
{
	/*
	 * Send the packet and return status on operation.
	 *
	 * Specific to environment.  User must supply
     * details of this function
	 */
	return( TYPICAL_SUCCESSFUL );
}

/*
 * This routine returns status on last received pkt
 */
u32
typical_pkt_status()
{
	/*
	 * Retrieve status on last received packet
 	 *
	 * Specific to environment.  User must supply
     * details of this function
	 */
	return( TYPICAL_SUCCESSFUL );
}

/*
 * This routine frees the packet buffer resource
 */
void
typical_free_pkt( u8 *pkt )
{
	/*
 	 * Free the packet resource back to the system
	 *
	 * Specific to environment.  User must supply
	 * details of this function
	 */
}

/*
 * Typical driver transmit and receive routines.  Exact forms of
 * these functions depend upon OS/STACK/INTERFACE capabilities
 */

/*
 * typical_transmit
 *
 * Typical Interface Transmit routine
 *
 * unit:	The number of the interface (0 relative)
 * pkt:		Pointer to the packet to be transmitted (dest, src, data, fcs)
 * len:		Length in bytes of the packet (including FCS)
 */
typical_transmit( u32 unit, u8 *pkt, u32 len )
{
u32	status;

	typical_tx_update++;

	status = typical_tx_pkt(pkt,len);		/* send the packet, get status	*/

	if(status == SUCCESSFUL) {
#ifdef XMIB_MIB2
		dstats[unit].out_octets += len;
		xset_1213outOctets(unit, dstats[unit].out_octets);

		switch( xsnmp_pkttype(unit, pkt) ) {
			default:
			case TYPICAL_UNICAST:
				dstats[unit].out_uncasts++;
				xset_1213outUcastPkts(unit, dstats[unit].out_ucasts);
				break;
			case TYPICAL_MCAST:
				dstats[unit].out_mcasts++;
				xset_1213inNUcastPkts(unit, dstats[unit].out_mcasts);
				break;
			case TYPICAL_BCAST:
				dstats[unit].out_bcast++;
				xset_1213inNUcastPkts(unit, dstats[unit].out_mcasts);
				break;
		}
	} else {
		dstats[unit].out_errors++;
		if(status == TYPICAL_DEFERRED) {
			dstats[unit].out_discards++;
			xset_1213outErrors(unit, dstats[unit].out_errors);
			xset_1213outDiscards(unit, dstats[unit].out_discards);
		}
		if(status == TYPICAL_COLLISIONS) {
			dstats[unit].out_colls++;
			xset_1213outErrors(unit, dstats[unit].out_errors);
			xset_1213outDiscards(unit, dstats[unit].out_discards);
		}
#endif
	}

}

/*
 * typical_receive
 *
 * Interface Receive handler
 *
 * unit:	The number of the interface (0 relative)
 * pkt:		Pointer to the received packet (dst, src, data, fcs)
 * len:		Length in bytes of the packet (including FCS)
 */
typical_receive( u32 unit, u8 *pkt, u32 len )
{
#ifdef XMIB_MIB2 
	dstats[unit].in_pkts++;					/* count every packet incoming	*/
#endif
#ifdef XMIB_RMON1
	dstats[unit].pkts++;
	xset_1757Pkts(unit, dstats[unit].pkts);
#endif

	status = typical_pkt_status();			/* get GOOD/BAD packet status	*/

	if(status == TYPICAL_SUCCESSFUL) {		/* non-error packet rcv'd	*/
#ifdef XMIB_RMON1
		switch( xrmon1_pktsize(unit,size) ) {
			case RMON_SIZE_RUNT:
				dstats[unit].runts++;
				xset_1757UndersizePkts(unit, dstats[unit].runts);
				break;
			case RMON_SIZE_64:
				dstats[unit].pkts64++;
				xset_1757Pkts64Octets(unit, dstats[unit].pkts64);
				break;
			case RMON_SIZE_65to127:
				dstats[unit].pkts65to127++;
				xset_1757Pkts65to127Octets(unit, dstats[unit].pkts65to127);
				break;
			case RMON_SIZE_128to255:
				dstats[unit].pkts128to255++;
				xset_1757Pkts128to255Octets(unit, dstats[unit].pkts128to255);
				break;
			case RMON_SIZE_256to511:
				dstats[unit].pkts256to511++;
				xset_1757Pkts256to511Octets(unit, dstats[unit].pkts256to511);
				break;
			case RMON_SIZE_512to1023:
				dstats[unit].pkts512to1023++;
				xset_1757Pkts512to1023Octets(unit, dstats[unit].pkts512to1023);
				break;
			case RMON_SIZE_1024to1518:
				dstats[unit].pkts1024to1518++;
				xset_1757Pkts1024to1518Octets(unit, dstats[unit].pkts1024to1518);
				break;
			case RMON_SIZE_GIANT:
				dstats[unit].giants++;
				xset_1757OversizePkts(unit, dstats[unit].giants);
				break;
		}
#endif

		switch( xsnmp_pkttype(unit, pkt) ) {
			default:
#ifdef XMIB_MIB2
			case TYPICAL_UNICAST:
				dstats[unit].in_uncasts++;
				xset_1213inUcastPkts(unit, dstats[unit].in_ucasts);
#endif
				break;
			case TYPICAL_MCAST:
#ifdef XMIB_MIB2
				dstats[unit].in_mcasts++;
				xset_1213inNUcastPkts(unit, dstats[unit].in_mcasts);
#endif
#ifdef XMIB_RMON1
				dstats[unit].mcast++;
				xset_1757MulticastPkts(unit, dstats[unit].mcast);
#endif
				break;
			case TYPICAL_BCAST:
#ifdef XMIB_MIB2
				dstats[unit].in_bcast++;
				xset_1213inNUcastPkts(unit, dstats[unit].in_mcasts);
#endif
#ifdef XMIB_RMON1
				dstats[unit].bcast++;
				xset_1757BroadcastPkts(unit, dstats[unit].bcast);
#endif
				break;
		}

#ifdef XMIB_RMON1
  	    xrmon1_pktcontent(unit, len, pkt);

		dstats[unit].octets += len;				/* accumulate the octets	*/
		xset_1757Octets(unit, dstats[unit].octets);
#endif

#ifdef XMIB_MIB2
		dstats[unit].in_octets += len;			/* accumulate the octets	*/
		xset_1213inOctets(unit, dstats[unit].in_octets);
#endif

		free_pkt(pkt);						/* free the good packet			*/

	} else {								/* an error packet received		*/
#ifdef XMIB_MIB2
		dstats[unit].in_errors++;			/* count the error				*/
		xset_1213inErrors(unit, dstats[unit].in_errors);
		xset_1213inDiscards(unit, dstats[unit].in_errors);
#endif
#ifdef XMIB_RMON1
		if( status == TYPICAL_COLLISION ) {
			dstats[unit].colls++;			/* count collisions				*/
			xset_1757Collisions(unit, dstats[unit].colls);
		}
		if( status == TYPICAL_JABBER ) {
			dstats[unit].jabber++;			/* count jabbers				*/
			xset_1757Jabbers(unit, dstats[unit].jabber);
		}
		if( status == TYPICAL_CRCALIGN ) {
			dstats[unit].crcalign++;		/* count CRC/Aligns				*/
			xset_1757CRCAlignErrors(unit, dstats[unit].crcalign);
		}
		if( status == TYPICAL_FRAGMENTS ) {
			dstats[unit].frags++;			/* count Fragmetns 				*/
			xset_1757Fragments(unit, dstats[unit].frags);
		}
#endif
		free_pkt(pkt);						/* dump the bad packet			*/
	}
}
