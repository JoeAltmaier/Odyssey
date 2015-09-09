#ifndef IMGHDR_H
#define IMGHDR_H


#ifdef  __cplusplus
extern "C" {
#endif

#define HEADER_VERSION		2
#define IMG_SIGNATURE		0x5A5AA5A5

#define    ROUNDUP(a, n)	(((a) + ((n) - 1)) & ~((n) - 1))

typedef struct {
	unsigned long	i_signature;		/* Signature IMG_MAGIC */
	unsigned long	i_headerversion;	/* Version of the header */
	unsigned long	i_mjr_ver;			/* Major Version */
	unsigned long	i_mnr_ver;			/* Minor Version */
	unsigned long	i_cksum;			/* Checksum of the image*/
	unsigned long	i_zipsize;			/* Size of zippped image */
	unsigned long	i_imagesize;		/* Image size */
	unsigned char	i_imagename[16];	/* Image name */
	unsigned long	i_type;				/* Type of Image. HBC/NAC/RAC/SSD */ 
	unsigned long	i_imageoffset;		/* Where the image is. Word Aligned */ 
	unsigned long	i_day;				/* Day this image was created */
	unsigned long	i_month;			/* Month */
	unsigned long	i_year;				/* Year */
	unsigned long	i_sec;				/* Seconds */
	unsigned long	i_min;				/* minutes */
	unsigned long	i_hour;				/* hour */
	unsigned long	i_sym_table_size;	/* If we attach symbols */
	unsigned long	i_n_syms;			/* Number of symbols */
	unsigned long	i_entry_point;		/* entry point of the image */ 
	/*
	 * Add additional Fields Here!
	 */
} img_hdr_t;

typedef struct symrec {
	unsigned long	vaddr;
	unsigned char	sym;
} symrec_t;

#ifdef  __cplusplus
}
#endif


#endif		/* IMGHDR_H */
