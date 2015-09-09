/*************************************************************************/
/*                                                                       */
/*       Copyright (c) 1993-1996 Accelerated Technology, Inc.            */
/*                                                                       */
/* PROPRIETARY RIGHTS of Accelerated Technology are involved in the      */
/* subject matter of this material.  All manufacturing, reproduction,    */
/* use, and sales rights pertaining to this subject matter are governed  */
/* by the license agreement.  The recipient of this software implicitly  */
/* accepts the terms of the license.                                     */
/*                                                                       */
/*************************************************************************/

/*************************************************************************/
/*                                                                       */
/* FILE NAME                                            VERSION          */
/*                                                                       */
/*      ps_comp.c                                       WEBSERV 1.0      */
/*                                                                       */
/* COMPONENT                                                             */
/*                                                                       */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This file holds functions that are used in the process of        */
/*      compressing and decompressing files.                             */
/*                                                                       */
/*                                                                       */
/* AUTHOR                                                                */
/*                                                                       */
/*                                                                       */
/*                                                                       */
/*     PicoServer Embedded Web Server                                    */
/*                                                                       */
/*     Copywrite (c) 1995 1996 1997 CNiT                                 */
/*                                                                       */
/*     Communication and Information Technology                          */
/*                                                                       */
/*                                                                       */
/*                                                                       */
/*                                                                       */
/* DATA STRUCTURES                                                       */
/*                                                                       */
/*      BIT_FILE                        Structure to hold information    */
/*                                      relative to Bit files.           */
/*      SYMBOL                          Structure to hold symbol         */
/*                                      scale, symbol high count, and    */
/*                                      symbol low count.                */
/*      Totals                          A global array to hold the       */
/*                                      cumulatice totals.               */
/*                                                                       */
/* FUNCTIONS                                                             */
/*                                                                       */
/*      ps_compress                     Compresses a file in memory or   */
/*                                      magnetic media.                  */                                                                
/*      ps_decompress                   Decompresses a file in memory or */
/*                                      magnetic media.                  */
/*      fatal_error                     Prints an Error message to a     */
/*                                      file.                            */
/*      CloseOutputBitFile              Closes Output Bit File.          */
/*      CompressFile                    Compresses Input Bit File.       */
/*      OpenInputBitFile                Open Input Bit file.             */
/*      OpenOutputBitFile               Open the Output Bit file.        */
/*      ExpandFile                      Expand compressed file.          */
/*      build_model                     Builds the compressed file model.*/
/*      initialize_arithmetic_encoder   Initializes the state of the     */
/*                                      Arithmetic Encoder.              */
/*      convert_int_to_symbol           Converts an integer count into a */
/*                                      symbol structure.                */
/*      encode_symbol                   Encodes a symbol for file        */
/*                                      compression.                     */
/*      flush_arithmetic_encoder        Flushes Arithmetic encoder.      */
/*      OutputBits                      Outputs a bit based on a count   */
/*                                      and a specific code.             */
/*      input_counts                    Reads the number of counts from  */
/*                                      the input file builds the table  */
/*                                      of cumulative counts.            */
/*      initialize_arithmetic_decoder   Initializes the state of the     */
/*                                      Arithmetic Decoder.              */
/*      get_symbol_scale                Retrieves symbol scale.          */
/*      get_current_count               Retrieves current Count.         */
/*      convert_symbol_to_int           Converts symbol to integer.      */
/*      remove_symbol_from_stream       Removes a charcter from the input*/
/*                                      stream.                          */
/*      buf_putc                        Puts a character in a buffer.    */
/*      count_bytes                     Counts number of bytes.          */
/*      scale_counts                    Scales the number of byte counts.*/
/*      output_counts                   Outputs the byte counts.         */
/*      build_totals                    Builds the table of cumulative   */
/*                                      totals.                          */
/*      buf_getc                        Gets a character from the input  */
/*                                      bit_file.                        */
/*      InputBit                        Reads a bit from an input file   */
/*                                      stream.                          */
/*      InputBits                       Inputs a bit based on a count.   */
/*      CloseInputBitFile               Close the Input Bit File.        */
/*      OutputBit                       Outputs a Specific bit in the    */
/*                                      file stream.                     */
/*      flush_buf_putc                  Flush the putc buffer.           */
/*                                                                       */
/* DEPENDENCIES                                                          */
/*                                                                       */
/*     ps_pico.h                        Defines and Data Structures      */
/*                                      related to Nucleus Webserv       */    
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         NAME            DATE                    REMARKS               */
/*                                                                       */
/*                                                                       */
/*                                                                       */
/*************************************************************************/

#include "ps_pico.h"

#ifdef FILE_COMPRESSION

 
typedef struct bit_file
{
    char * fstart;                                  /* start of "file" */
    char * cur_pos;                                 /* current position in "file" */
    int    length;                                  /* length of "file" */
    unsigned char mask;
    int rack;
    int pacifier_counter;
    int mode;
    Request *  fd;
} BIT_FILE;
 

void            fatal_error( char *s);
void            CompressFile( BIT_FILE * buf_input, BIT_FILE * buf_output );
void            ExpandFile( BIT_FILE *buf_input, BIT_FILE *buf_output );
BIT_FILE        *OpenInputBitFile( BIT_FILE *name,char * buf, int length );
BIT_FILE        *OpenOutputBitFile( BIT_FILE *name, char * buf, int length );
void            OutputBit( BIT_FILE *bit_file, int bit );
void            OutputBits( BIT_FILE *bit_file, unsigned long code, int count );

int             InputBit( BIT_FILE *bit_file );
unsigned long   InputBits( BIT_FILE *bit_file, int bit_count );
void            CloseInputBitFile( BIT_FILE *bit_file );
void            CloseOutputBitFile( BIT_FILE *bit_file );
int             buf_getc( BIT_FILE *bitfile );
int             buf_putc( unsigned int out, BIT_FILE *bitfile );
void            flush_buf_putc( BIT_FILE *b );

typedef struct
{
    unsigned short int low_count;
    unsigned short int high_count;
    unsigned short int scale;
}   SYMBOL;

/*
 * Internal function prototypes, with or without ANSI prototypes.
 */
				       

void            build_model( BIT_FILE *input, BIT_FILE *output );
void            scale_counts( unsigned long counts[], unsigned char scaled_counts[] );
void            build_totals( unsigned char scaled_counts[] );
void            count_bytes( BIT_FILE *input, unsigned long counts[] );
void            output_counts( BIT_FILE *output, unsigned char scaled_counts[] );
void            input_counts( BIT_FILE *stream );
void            convert_int_to_symbol( int symbol, SYMBOL *s );
void            get_symbol_scale( SYMBOL *s );
int             convert_symbol_to_int( int count, SYMBOL *s );
void            initialize_arithmetic_encoder( void );
void            encode_symbol( BIT_FILE *stream, SYMBOL *s );
void            flush_arithmetic_encoder( BIT_FILE *stream );
short int       get_current_count( SYMBOL *s );
void            initialize_arithmetic_decoder( BIT_FILE *stream );
void            remove_symbol_from_stream( BIT_FILE *stream, SYMBOL *s );


#define END_OF_STREAM 256
short int totals[ 258 ];                            /* The cumulative totals */



/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      ps_compress                                                      */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      Function used to compress an html, gif, jpeg, or text files. This*/
/*      function compresses the files for the in memory file system and  */
/*      os dependent file system.                                        */
/*                                                                       */ 
/* AUTHOR                                                                */
/*                                                                       */
/*                                                                       */
/*                                                                       */
/*     PicoServer Embedded Web Server                                    */
/*                                                                       */
/*     Copywrite (c) 1995 1996 1997 CNiT                                 */
/*                                                                       */
/*     Communication and Information Technology                          */
/*                                                                       */
/*                                                                       */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      save_memfs                      Save file in memory.             */
/*      save_os_fs                      Saves a file in an os supported  */
/*                                      file system.                     */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      CloseOutputBitFile              Closes Output Bit File.          */
/*      CompressFile                    Compresses Input Bit File.       */
/*      OpenInputBitFile                Open Input Bit file.             */
/*      OpenOutputBitFile               Open the Output Bit file.        */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      inbuf                           The input buffer where the       */
/*                                      file is coming from. Usually     */
/*                                      memory.                          */
/*      length                          The length of the data to be     */
/*                                      compressed.                      */
/*      mode                            Flag that idicates whether       */
/*                                      Not Output or Output             */
/*      outbuf                          The output buffer where the      */
/*                                      the memory is to be stored.      */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      out.length+1                    Pointer to next position to be   */
/*                                      compressed.                      */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         NAME            DATE                    REMARKS               */
/*                                                                       */
/*************************************************************************/
int ps_compress( int mode,char * inbuf, char *outbuf,int length )
{
    BIT_FILE out;
    BIT_FILE in;

    OpenOutputBitFile( &out, outbuf, length );
    out.mode = mode;
    OpenInputBitFile( &in, inbuf, length );

    CompressFile( &in,&out );
    CloseOutputBitFile( &out );

    return( out.length+1 );
}

/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      ps_decompress                                                    */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      Function to decompress files resident in the in memory file      */
/*      system or os file system.                                        */
/*                                                                       */
/* AUTHOR                                                                */
/*                                                                       */
/*                                                                       */
/*                                                                       */
/*     PicoServer Embedded Web Server                                    */
/*                                                                       */
/*     Copywrite (c) 1995 1996 1997 CNiT                                 */
/*                                                                       */
/*     Communication and Information Technology                          */
/*                                                                       */
/*                                                                       */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      md_send_file                    Send file to browser over        */
/*                                      network.                         */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      CloseOutputBitFile              Closes Output Bit File.          */
/*      ExpandFile                      Expand compressed file.          */
/*      OpenInputBitFile                Open Input Bit file.             */
/*      OpenOutputBitFile               Open the Output Bit file.        */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      req                             Pointer to Request structure that*/
/*                                      holds all information pertaining */
/*                                      to  the HTTP request.            */
/*      inbuf                           Input Buffer to be decompressed. */
/*      outbuf                          Output Buffer of decompressed    */
/*                                      file.                            */
/*      ilen                            Length of the input buffer.      */
/*      olen                            Length of the output buffer.     */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      Always returns Zero.                                             */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         NAME            DATE                    REMARKS               */
/*                                                                       */
/*************************************************************************/

int ps_decompress(Request * req,char * inbuf, char *outbuf,int ilen, int olen)
{
    BIT_FILE in;
    BIT_FILE out;


#ifdef DEBUG 
printf( "ps_decompress(%x %x %d %d) \n",inbuf,outbuf,ilen,olen );
#endif

    OpenOutputBitFile( &out, outbuf, olen );
    OpenInputBitFile( &in, inbuf, ilen );

    out.fd=req;
    out.mode = NET_OUTPUT | DONT_OUTPUT;
    out.length = 0;

    ExpandFile( &in,&out );
    CloseOutputBitFile( &out );

    return(0);

}

/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      CompressFile                                                     */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*                                                                       */
/* AUTHOR                                                                */
/*                                                                       */
/*                                                                       */
/*                                                                       */
/*     PicoServer Embedded Web Server                                    */
/*                                                                       */
/*     Copywrite (c) 1995 1996 1997 CNiT                                 */
/*                                                                       */
/*     Communication and Information Technology                          */
/*                                                                       */
/*                                                                       */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      ps_compress                     Compresses a file in memory or   */
/*                                      magnetic media.                  */                                                                
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      build_model                     Builds the compressed file model.*/
/*      initialize_arithmetic_encoder   Initializes the state of the     */
/*                                      Arithmetic Encoder.              */
/*      convert_int_to_symbol           Converts an integer count into a */
/*                                      symbol structure.                */
/*      encode_symbol                   Encodes a symbol for file        */
/*                                      compression.                     */
/*      flush_arithmetic_encoder        Flushes Arithmetic encoder.      */
/*      OutputBits                      Outputs a bit based on a count   */
/*                                      and a specific code.             */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      input                           Pointer to BIT_FILE structure.   */
/*                                      This structure holds all the     */
/*                                      related to the input file to be  */
/*                                      compressed.                      */
/*      output                          Pointer to BIT_FILE structure.   */
/*                                      This structure holds all the     */
/*                                      related to the output file to be */
/*                                      compressed.                      */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      None.                                                            */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         NAME            DATE                    REMARKS               */
/*                                                                       */
/*************************************************************************/

void  CompressFile( input, output)
BIT_FILE *input;
BIT_FILE *output;
{
    int c;
    SYMBOL s;

    build_model( input, output);
    initialize_arithmetic_encoder();

    while ( ( c = buf_getc( input ) ) != EOF )
    {
	convert_int_to_symbol( c, &s );
	encode_symbol( output, &s );
    }
    convert_int_to_symbol( END_OF_STREAM, &s );
    encode_symbol( output, &s );
    flush_arithmetic_encoder( output );
    OutputBits( output, 0L, 16 );

}

/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      ExpandFile                                                       */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*                                                                       */
/* AUTHOR                                                                */
/*                                                                       */
/*                                                                       */
/*                                                                       */
/*     PicoServer Embedded Web Server                                    */
/*                                                                       */
/*     Copywrite (c) 1995 1996 1997 CNiT                                 */
/*                                                                       */
/*     Communication and Information Technology                          */
/*                                                                       */
/*                                                                       */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      ps_decompress                   Decompresses a file in memory or */
/*                                      magnetic media.                  */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      input_counts                    Reads the number of counts from  */
/*                                      the input file builds the table  */
/*                                      of cumulative counts.            */
/*      initialize_arithmetic_decoder   Initializes the state of the     */
/*                                      Arithmetic Decoder.              */
/*      get_symbol_scale                Retrieves symbol scale.          */
/*      get_current_count               Retrieves current Count.         */
/*      convert_symbol_to_int           Converts symbol to integer.      */
/*      remove_symbol_from_stream       Removes a charcter from the input*/
/*                                      stream.                          */
/*      buf_putc                        Puts a character in a buffer.    */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      input                           Pointer to BIT_FILE structure.   */
/*                                      This structure holds all the     */
/*                                      related to the input file to be  */
/*                                      compressed.                      */
/*      output                          Pointer to BIT_FILE structure.   */
/*                                      This structure holds all the     */
/*                                      related to the output file to be */
/*                                      compressed.                      */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      None.                                                            */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         NAME            DATE                    REMARKS               */
/*                                                                       */
/*************************************************************************/

void  ExpandFile( input, output)
BIT_FILE *input;
BIT_FILE *output;
{
    SYMBOL s;
    int c;
    int count;

    input_counts( input );
    initialize_arithmetic_decoder( input );
    for ( ; ; )
    {
	get_symbol_scale( &s );
	count = get_current_count( &s );
	c = convert_symbol_to_int( count, &s );
	if ( c == END_OF_STREAM )
	    break;
	remove_symbol_from_stream( input, &s );
	buf_putc( ( char ) c, output );
    }
}

/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      build_model                                                      */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This is the routine that is called to scan the input file,       */
/*      scale the counts, build the totals array, then outputs the       */
/*      scaled counts to the output file.                                */
/*                                                                       */
/*                                                                       */
/* AUTHOR                                                                */
/*                                                                       */
/*                                                                       */
/*                                                                       */
/*     PicoServer Embedded Web Server                                    */
/*                                                                       */
/*     Copywrite (c) 1995 1996 1997 CNiT                                 */
/*                                                                       */
/*     Communication and Information Technology                          */
/*                                                                       */
/*                                                                       */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      CompressFile                    Compresses an Input file.        */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      count_bytes                     Counts number of bytes.          */
/*      scale_counts                    Scales the number of byte counts.*/
/*      output_counts                   Outputs the byte counts.         */
/*      build_totals                    Builds the table of cumulative   */
/*                                      totals.                          */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      input                           Pointer to BIT_FILE structure.   */
/*                                      This structure holds all the     */
/*                                      related to the input file to be  */
/*                                      compressed.                      */
/*      output                          Pointer to BIT_FILE structure.   */
/*                                      This structure holds all the     */
/*                                      related to the output file to be */
/*                                      compressed.                      */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      None.                                                            */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         NAME            DATE                    REMARKS               */
/*                                                                       */
/*************************************************************************/

void  build_model( input, output )
BIT_FILE *input;
BIT_FILE *output;
{
    unsigned long counts[ 256 ];
    unsigned char scaled_counts[ 256 ];

    count_bytes( input, counts );
    scale_counts( counts, scaled_counts );
    output_counts( output, scaled_counts );
    build_totals( scaled_counts );
}

#ifndef SEEK_SET
#define SEEK_SET 0
#endif

/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      count_byte                                                       */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This routine runs through the file and counts the appearances    */
/*      of each character.                                               */
/*                                                                       */
/* AUTHOR                                                                */
/*                                                                       */
/*                                                                       */
/*                                                                       */
/*     PicoServer Embedded Web Server                                    */
/*                                                                       */
/*     Copywrite (c) 1995 1996 1997 CNiT                                 */
/*                                                                       */
/*     Communication and Information Technology                          */
/*                                                                       */
/*                                                                       */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      build_model                     Builds the compressed file model.*/
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      buf_getc                        Gets a character from the input  */
/*                                      bit_file.                        */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      input                           Pointer to BIT_FILE structure.   */
/*                                      This structure holds all the     */
/*                                      related to the input file to be */
/*                                      compressed.                      */
/*      counts                          Number of bytes that are found   */
/*                                      in this segement of the file.    */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      None.                                                            */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         NAME            DATE                    REMARKS               */
/*                                                                       */
/*************************************************************************/

void  count_bytes( input, counts )
BIT_FILE *input;
unsigned long counts[];
{
    int i;
    int c;

    for ( i = 0 ; i < 256 ; i++ )
	counts[ i ] = 0;

    while ( ( c = buf_getc( input )) != EOF )
	counts[ c ]++;

	input->cur_pos= input->fstart;              /* "REWIND" input stream */
}


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      scale_counts                                                     */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This routine is called to scale the counts down. There are       */
/*      two types of scaling that must be done.  First, the counts       */
/*      need to be scaled down so that the individual counts fit         */
/*      into a single unsigned char. Then, the counts need to be         */
/*      rescaled so that the total of all counts is less than 16384.     */
/*                                                                       */
/*                                                                       */
/* AUTHOR                                                                */
/*                                                                       */
/*                                                                       */
/*                                                                       */
/*     PicoServer Embedded Web Server                                    */
/*                                                                       */
/*     Copywrite (c) 1995 1996 1997 CNiT                                 */
/*                                                                       */
/*     Communication and Information Technology                          */
/*                                                                       */
/*                                                                       */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      build_model                     Builds the compressed file model.*/
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      None.                                                            */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      counts                          Number of bytes in file.         */
/*      scaled_counts                   Scaled Number of bytes in file.  */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      None.                                                            */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         NAME            DATE                    REMARKS               */
/*                                                                       */
/*************************************************************************/

void  scale_counts( counts, scaled_counts )
unsigned long counts[];
unsigned char scaled_counts[];
{
    int i;
    unsigned long max_count;
    unsigned int total;
    unsigned long scale;

/*
 * The first section of code makes sure each count fits into a single byte.
 */
    max_count = 0;
    for ( i = 0 ; i < 256 ; i++ )
	if ( counts[ i ] > max_count )
	    max_count = counts[ i ];
    scale = max_count / 256;
    scale = scale + 1;
    for ( i = 0 ; i < 256 ; i++ )
    {
	scaled_counts[ i ] = (unsigned char ) ( counts[ i ] / scale );
	if ( scaled_counts[ i ] == 0 && counts[ i ] != 0 )
	    scaled_counts[ i ] = 1;
    }
/*
 * This next section makes sure the total is less than 16384.  I initialize
 * the total to 1 instead of 0 because there will be an additional 1 added
 * in for the END_OF_STREAM symbol;
 */
    total = 1;
    for ( i = 0 ; i < 256 ; i++ )
	total += scaled_counts[ i ];
    if ( total > ( 32767 - 256 ) )
	scale = 4;
    else if ( total > 16383 )
	scale = 2;
    else
	return;
    for ( i = 0 ; i < 256 ; i++ )
	scaled_counts[ i ] /= (unsigned char)scale;
}


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      build_totals                                                     */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This routine is used by both the encoder and decoder to          */
/*      build the table of cumulative totals. The counts for the         */
/*      characters in the file are in the counts array, and we know      */
/*      that there will be a single instance of the EOF symbol.          */
/*                                                                       */
/* AUTHOR                                                                */
/*                                                                       */
/*                                                                       */
/*                                                                       */
/*     PicoServer Embedded Web Server                                    */
/*                                                                       */
/*     Copywrite (c) 1995 1996 1997 CNiT                                 */
/*                                                                       */
/*     Communication and Information Technology                          */
/*                                                                       */
/*                                                                       */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      build_model                     Builds the compressed file model.*/
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      None.                                                            */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      scaled_counts                   Scaled Number of bytes in file.  */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      None.                                                            */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         NAME            DATE                    REMARKS               */
/*                                                                       */
/*************************************************************************/

void  build_totals( scaled_counts )
unsigned char scaled_counts[];
{
    int i;

    totals[ 0 ] = 0;
    for ( i = 0 ; i < END_OF_STREAM ; i++ )
	totals[ i + 1 ] = totals[ i ] + scaled_counts[ i ];
    totals[ END_OF_STREAM + 1 ] = totals[ END_OF_STREAM ] + 1;
}

/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      output_counts                                                    */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      In order for the compressor to build the same model, I have      */
/*      to store the symbol counts in the compressed file so the         */
/*      expander can read them in.  In order to save space, I don't      */
/*      save all 256 symbols unconditionally. The format used to store   */
/*      counts looks like this:                                          */
/*                                                                       */
/*      start, stop, counts, start, stop, counts, ... 0                  */
/*                                                                       */
/*      This means that I store runs of counts, until all the            */
/*      non-zero counts have been stored.  At this time the list is      */
/*      terminated by storing a start value of 0.  Note that at least    */
/*      1 run of counts has to be stored, so even if the first start     */
/*      value is 0, I read it in. It also means that even in an empty    */
/*      file that has no counts, I have to pass at least one count.      */
/*                                                                       */
/*      In order to efficiently use this format, I have to identify      */
/*      runs of non-zero counts.  Because of the format used, I don't    */
/*      want to stop a run because of just one or two zeros in the count */
/*      stream. So I have to sit in a loop looking for strings of three  */
/*      or more zero values in a row.                                    */
/*                                                                       */
/*      This is simple in concept, but it ends up being one of the most  */
/*      complicated routines in the whole program.  A routine that just  */
/*      writes out 256 values without attempting to optimize would be    */
/*      much simpler, but would hurt compression quite a bit on small    */
/*      files.                                                           */
/*                                                                       */
/* AUTHOR                                                                */
/*                                                                       */
/*                                                                       */
/*                                                                       */
/*     PicoServer Embedded Web Server                                    */
/*                                                                       */
/*     Copywrite (c) 1995 1996 1997 CNiT                                 */
/*                                                                       */
/*     Communication and Information Technology                          */
/*                                                                       */
/*                                                                       */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      build_model                     Builds the compressed file model.*/
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      buf_putc                        Puts a character in a buffer.    */
/*      fatal_error                     Prints an Error message to a     */
/*                                      file.                            */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      output                          Pointer to BIT_FILE structure.   */
/*                                      This structure holds all the     */
/*                                      related to the output file to be */
/*                                      compressed.                      */
/*      scaled_counts                   Scaled Number of bytes in file.  */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      None.                                                            */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         NAME            DATE                    REMARKS               */
/*                                                                       */
/*************************************************************************/

void  output_counts( output, scaled_counts )
BIT_FILE *output;
unsigned char scaled_counts[];
{
    int first;
    int last;
    int next;
    int i;

    first = 0;
    while ( first < 255 && scaled_counts[ first ] == 0 )
	    first++;
/*
 * Each time I hit the start of the loop, I assume that first is the
 * number for a run of non-zero values.  The rest of the loop is
 * concerned with finding the value for last, which is the end of the
 * run, and the value of next, which is the start of the next run.
 * At the end of the loop, I assign next to first, so it starts in on
 * the next run.
 */
    for ( ; first < 256 ; first = next )
    {
	last = first + 1;
	for ( ; ; )
	{
	    for ( ; last < 256 ; last++ )
		if ( scaled_counts[ last ] == 0 )
		    break;
	    last--;
	    for ( next = last + 1; next < 256 ; next++ )
		if ( scaled_counts[ next ] != 0 )
		    break;
	    if ( next > 255 )
		break;
	    if ( ( next - last ) > 3 )
		break;
	    last = next;
	};
/*
 * Here is where I output first, last, and all the counts in between.
 */
	if ( buf_putc( first, output ) != first )
	    fatal_error( "Error writing byte counts\n" );
	if ( buf_putc( last, output ) != last )
	    fatal_error( "Error writing byte counts\n" );
	for ( i = first ; i <= last ; i++ ) {
	    if ( buf_putc( scaled_counts[ i ], output ) !=
		 (int) scaled_counts[ i ] )
		fatal_error( "Error writing byte counts\n" );
	}
    }
    if ( buf_putc( 0, output ) != 0 )
	    fatal_error( "Error writing byte counts\n" );
}


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      input_counts                                                     */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      When expanding, I have to read in the same set of counts.        */
/*      This is quite a bit easier that the process of writing           */
/*      them out, since no decision making needs to be done. All         */
/*      I do is read in first, check to see if I am all done, and        */
/*      if not, read in last and a string of counts.                     */
/*                                                                       */
/* AUTHOR                                                                */
/*                                                                       */
/*                                                                       */
/*                                                                       */
/*     PicoServer Embedded Web Server                                    */
/*                                                                       */
/*     Copywrite (c) 1995 1996 1997 CNiT                                 */
/*                                                                       */
/*     Communication and Information Technology                          */
/*                                                                       */
/*                                                                       */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      ExpandFile                      Expands a compressed file.       */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      fatal_error                     Prints an Error message to a     */
/*                                      file.                            */
/*      build_totals                    Builds the table of cumulative   */
/*                                      totals.                          */
/*      buf_getc                        Gets a character from the input  */
/*                                      bit_file.                        */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      input                           Pointer to BIT_FILE structure.   */
/*                                      This structure holds all the     */
/*                                      related to the input file to be */
/*                                      compressed.                      */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      None.                                                            */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         NAME            DATE                    REMARKS               */
/*                                                                       */
/*************************************************************************/

void  input_counts( input )
BIT_FILE *input;
{
    int first;
    int last;
    int i;
    int c;
    unsigned char scaled_counts[ 256 ];

    for ( i = 0 ; i < 256 ; i++ )
	scaled_counts[ i ] = 0;
    if ( ( first = buf_getc( input ) ) == EOF )
	fatal_error( "Error reading byte counts\n" );
    if ( ( last = buf_getc( input ) ) == EOF )
	fatal_error( "Error reading byte counts\n" );
    for ( ; ; )
    {
	for ( i = first ; i <= last ; i++ )
	    if ( ( c = buf_getc( input ) ) == EOF )
		fatal_error( "Error reading byte counts\n" );
	    else
		scaled_counts[ i ] = ( unsigned char ) c;
	if ( ( first = buf_getc( input ) ) == EOF )
	    fatal_error( "Error reading byte counts\n" );
	if ( first == 0 )
	    break;
	if ( ( last = buf_getc( input ) ) == EOF )
	    fatal_error( "Error reading byte counts\n" );
    }
    build_totals( scaled_counts );
}

/*
 * Everything from here down define the arithmetic coder section
 * of the program.
 */

/*
 * These four variables define the current state of the arithmetic
 * coder/decoder.  They are assumed to be 16 bits long.  Note that
 * by declaring them as short ints, they will actually be 16 bits
 * on most 80X86 and 680X0 machines, as well as VAXen.
 */
static unsigned short int code;                     /* The present input code value     */
static unsigned short int low;                      /* Start of the current code range  */
static unsigned short int high;                     /* End of the current code range    */
long underflow_bits;                                /* Number of underflow bits pending */


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      initialize_arithmetic_encoder                                    */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This routine must be called to initialize the encoding           */
/*      process. The high register is initialized to all 1s, and         */
/*      it is assumed that it has an infinite string of 1s to be         */
/*      shifted into the lower bit positions when needed.                */
/*                                                                       */
/* AUTHOR                                                                */
/*                                                                       */
/*                                                                       */
/*                                                                       */
/*     PicoServer Embedded Web Server                                    */
/*                                                                       */
/*     Copywrite (c) 1995 1996 1997 CNiT                                 */
/*                                                                       */
/*     Communication and Information Technology                          */
/*                                                                       */
/*                                                                       */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      CompressFile                    Compresses an Input file.        */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      None.                                                            */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      None.                                                            */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      None.                                                            */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         NAME            DATE                    REMARKS               */
/*                                                                       */
/*************************************************************************/

void  initialize_arithmetic_encoder()
{
    low = 0;
    high = 0xffff;
    underflow_bits = 0;
}


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      flush_arithimetic_encoder                                        */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      At the end of the encoding process, there are still              */
/*      significant bits left in the high and low registers.             */
/*      We output two bits, plus as many underflow bits as               */
/*      are necessary.                                                   */
/*                                                                       */
/* AUTHOR                                                                */
/*                                                                       */
/*                                                                       */
/*                                                                       */
/*     PicoServer Embedded Web Server                                    */
/*                                                                       */
/*     Copywrite (c) 1995 1996 1997 CNiT                                 */
/*                                                                       */
/*     Communication and Information Technology                          */
/*                                                                       */
/*                                                                       */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      CompressFile                    Compresses an Input file.        */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      OutputBit                       Outputs a Specific bit in the    */
/*                                      file stream.                     */
/*                                                                       */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      stream                          Pointer to BIT_FILE structure.   */
/*                                      This structure holds all the     */
/*                                      related to the output file to be */
/*                                      compressed.                      */
/*                                                                       */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      None.                                                            */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         NAME            DATE                    REMARKS               */
/*                                                                       */
/*************************************************************************/

void  flush_arithmetic_encoder( stream )
BIT_FILE *stream;
{
    OutputBit( stream, low & 0x4000 );
    underflow_bits++;
    while ( underflow_bits-- > 0 )
	OutputBit( stream, ~low & 0x4000 );
}


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      convert_int_to_symbol                                            */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      Finding the low count, high count, and scale for                 */
/*      a symbol is really easy, because of the way the totals           */
/*      are stored. This is the one redeeming feature of the             */
/*      data structure used in this implementation.                      */
/*                                                                       */
/* AUTHOR                                                                */
/*                                                                       */
/*                                                                       */
/*                                                                       */
/*     PicoServer Embedded Web Server                                    */
/*                                                                       */
/*     Copywrite (c) 1995 1996 1997 CNiT                                 */
/*                                                                       */
/*     Communication and Information Technology                          */
/*                                                                       */
/*                                                                       */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      CompressFile                    Compresses an Input file.        */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      None.                                                            */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      c                               Integer to be converted.         */
/*      s                               Pointer to the SYMBOL structure  */
/*                                      that contains the converted      */
/*                                      symbol information.              */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      None.                                                            */ 
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         NAME            DATE                    REMARKS               */
/*                                                                       */
/*************************************************************************/

void  convert_int_to_symbol( c, s )
int c;
SYMBOL *s;
{
    s->scale = totals[ END_OF_STREAM + 1 ];
    s->low_count = totals[ c ];
    s->high_count = totals[ c + 1 ];
}


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      get_symbol_scale                                                 */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      Getting the scale for the current context is easy.               */
/*                                                                       */
/* AUTHOR                                                                */
/*                                                                       */
/*                                                                       */
/*                                                                       */
/*     PicoServer Embedded Web Server                                    */
/*                                                                       */
/*     Copywrite (c) 1995 1996 1997 CNiT                                 */
/*                                                                       */
/*     Communication and Information Technology                          */
/*                                                                       */
/*                                                                       */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      ExpandFile                      Expands a compressed file.       */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      None.                                                            */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      s                               Pointer to the SYMBOL structure  */
/*                                      that contains the converted      */
/*                                      symbol information.              */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      None.                                                            */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         NAME            DATE                    REMARKS               */
/*                                                                       */
/*************************************************************************/

void  get_symbol_scale( s )
SYMBOL *s;
{
    s->scale = totals[ END_OF_STREAM + 1 ];
}


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      convert_symbol_to_int                                            */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      During decompression, we have to search through the              */
/*      table until we find the symbol that straddles the                */
/*      "count" parameter. When it is found, it is returned.             */
/*      The reason for also setting the high count and low count         */
/*      is so that symbol can be properly removed from the               */
/*      arithmetic coded input.                                          */
/*                                                                       */
/* AUTHOR                                                                */
/*                                                                       */
/*                                                                       */
/*                                                                       */
/*     PicoServer Embedded Web Server                                    */
/*                                                                       */
/*     Copywrite (c) 1995 1996 1997 CNiT                                 */
/*                                                                       */
/*     Communication and Information Technology                          */
/*                                                                       */
/*                                                                       */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      ExpandFile                      Expands a compressed file.       */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      None.                                                            */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      count                           The current count.               */
/*      s                               Pointer to the SYMBOL structure  */
/*                                      that contains the converted      */
/*                                      symbol information.              */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      c                               Integer form of converted symbol.*/
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         NAME            DATE                    REMARKS               */
/*                                                                       */
/*************************************************************************/

int convert_symbol_to_int( count, s)
int count;
SYMBOL *s;
{
    int c;

    for ( c = END_OF_STREAM ; count < totals[ c ] ; c-- );
    s->high_count = totals[ c + 1 ];
    s->low_count = totals[ c ];
    return( c );
}


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      encode_symbol                                                    */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This routine is called to encode a symbol. The symbol            */
/*      is passed in the SYMBOL structure as a low count, a              */
/*      high count, and a range, instead of the more conventional        */
/*      probability ranges. The encoding process takes two steps.        */
/*      First, the values of high and low are updated to take            */
/*      into account the range restriction created by the  new           */
/*      symbol. Then, as many bits as possible are shifted out to        */
/*      the output stream.  Finally, high and low are stable again       */
/*      and the routine returns.                                         */
/*                                                                       */
/* AUTHOR                                                                */
/*                                                                       */
/*                                                                       */
/*                                                                       */
/*     PicoServer Embedded Web Server                                    */
/*                                                                       */
/*     Copywrite (c) 1995 1996 1997 CNiT                                 */
/*                                                                       */
/*     Communication and Information Technology                          */
/*                                                                       */
/*                                                                       */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      CompressFile                    Compresses an Input file.        */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      OutputBit                       Outputs a Specific bit in the    */
/*                                      file stream.                     */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      stream                          Pointer to BIT_FILE structure.   */
/*                                      This structure holds all the     */
/*                                      related to the output file to be */
/*                                      compressed.                      */
/*      s                               Pointer to the SYMBOL structure  */
/*                                      that contains the converted      */
/*                                      symbol information.              */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      None.                                                            */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         NAME            DATE                    REMARKS               */
/*                                                                       */
/*************************************************************************/

void  encode_symbol( stream, s )
BIT_FILE *stream;
SYMBOL *s;
{
    long range;
/*
 * These three lines rescale high and low for the new symbol.
 */
    range = (long) ( high-low ) + 1;
    high = low + (unsigned short int)
		 (( range * s->high_count ) / s->scale - 1 );
    low = low + (unsigned short int)
		(( range * s->low_count ) / s->scale );
/*
 * This loop turns out new bits until high and low are far enough
 * apart to have stabilized.
 */
    for ( ; ; )
    {
/*
 * If this test passes, it means that the MSDigits match, and can
 * be sent to the output stream.
 */
	if ( ( high & 0x8000 ) == ( low & 0x8000 ) )
	{
	    OutputBit( stream, high & 0x8000 );
	    while ( underflow_bits > 0 )
	    {
		OutputBit( stream, ~high & 0x8000 );
		underflow_bits--;
	    }
	}
/*
 * If this test passes, the numbers are in danger of underflow, because
 * the MSDigits don't match, and the 2nd digits are just one apart.
 */
	else if ( ( low & 0x4000 ) && !( high & 0x4000 ))
	{
	    underflow_bits += 1;
	    low &= 0x3fff;
	    high |= 0x4000;
	} else
	    return ;
	low <<= 1;
	high <<= 1;
	high |= 1;
    }
}


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      get_current_count                                                */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      When decoding, this routine is called to figure out              */
/*      which symbol is presently waiting to be decoded. This            */
/*      routine expects to get the current model scale in the            */
/*      s->scale parameter, and it returns  a count that                 */
/*      corresponds to the present floating point code:                  */
/*                                                                       */
/*      code = count / s->scale                                          */
/*                                                                       */
/*                                                                       */
/* AUTHOR                                                                */
/*                                                                       */
/*                                                                       */
/*                                                                       */
/*     PicoServer Embedded Web Server                                    */
/*                                                                       */
/*     Copywrite (c) 1995 1996 1997 CNiT                                 */
/*                                                                       */
/*     Communication and Information Technology                          */
/*                                                                       */
/*                                                                       */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      ExpandFile                      Expands a compressed file.       */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      None.                                                            */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      s                               Pointer to the SYMBOL structure  */
/*                                      that contains the converted      */
/*                                      symbol information.              */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      count                           Current Count.                   */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         NAME            DATE                    REMARKS               */
/*                                                                       */
/*************************************************************************/

short int get_current_count( s )
SYMBOL *s;
{
    long range;
    short int count;

    range = (long) ( high - low ) + 1;
    count = (short int)
	    ((((long) ( code - low ) + 1 ) * s->scale-1 ) / range );
    return( count );
}


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      initialize_arithmetic_decoder                                    */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*       This routine is called to initialize the state                  */
/*       of the arithmetic decoder.  This involves initializing          */
/*       the high and low registers to their conventional                */
/*       starting values, plus reading the first 16 bits from            */
/*       the input stream into the code value.                           */
/*                                                                       */
/* AUTHOR                                                                */
/*                                                                       */
/*                                                                       */
/*                                                                       */
/*     PicoServer Embedded Web Server                                    */
/*                                                                       */
/*     Copywrite (c) 1995 1996 1997 CNiT                                 */
/*                                                                       */
/*     Communication and Information Technology                          */
/*                                                                       */
/*                                                                       */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      ExpandFile                      Expands a compressed file.       */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      InputBit                        Reads a bit from an input file   */
/*                                      stream.                          */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      stream                          Pointer to BIT_FILE structure.   */
/*                                      This structure holds all the     */
/*                                      related to the input file to be  */
/*                                      compressed.                      */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      None.                                                            */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         NAME            DATE                    REMARKS               */
/*                                                                       */
/*************************************************************************/

void  initialize_arithmetic_decoder( stream )
BIT_FILE *stream;
{
    int i;

    code = 0;
    for ( i = 0 ; i < 16 ; i++ )
    {
	code <<= 1;
	code += InputBit( stream );
    }
    low = 0;
    high = 0xffff;
}


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      remove_symbol_from_stream                                        */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      Just figuring out what the present symbol is                     */
/*      doesn't remove it from the input bit stream. After               */
/*      the character has been decoded, this routine has to              */
/*      be called to remove it from the input stream.                    */
/*                                                                       */
/* AUTHOR                                                                */
/*                                                                       */
/*                                                                       */
/*                                                                       */
/*     PicoServer Embedded Web Server                                    */
/*                                                                       */
/*     Copywrite (c) 1995 1996 1997 CNiT                                 */
/*                                                                       */
/*     Communication and Information Technology                          */
/*                                                                       */
/*                                                                       */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      ExpandFile                      Expands a compressed file.       */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      InputBit                        Reads a bit from an input file   */
/*                                      stream.                          */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      stream                          Pointer to BIT_FILE structure.   */
/*                                      This structure holds all the     */
/*                                      related to the input file to be  */
/*                                      compressed.                      */
/*      s                               Pointer to the SYMBOL structure  */
/*                                      that contains the converted      */
/*                                      symbol information.              */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      None.                                                            */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         NAME            DATE                    REMARKS               */
/*                                                                       */
/*************************************************************************/

void  remove_symbol_from_stream( stream, s )
BIT_FILE *stream;
SYMBOL *s;
{
    long range;

/*
 * First, the range is expanded to account for the symbol removal.
 */
    range = (long)( high - low ) + 1;
    high = low + (unsigned short int)
		 (( range * s->high_count ) / s->scale - 1 );
    low = low + (unsigned short int)
		 (( range * s->low_count ) / s->scale );
/*
 * Next, any possible bits are shipped out.
 */
    for ( ; ; )
    {
/*
 * If the MSDigits match, the bits will be shifted out.
 */
	if ( ( high & 0x8000 ) == ( low & 0x8000 ) )
	{
	}
/*
 * Else, if underflow is threatening, shift out the 2nd MSDigit.
 */
	else if ( (low & 0x4000) == 0x4000  && (high & 0x4000) == 0 )
	{
	    code ^= 0x4000;
	    low   &= 0x3fff;
	    high  |= 0x4000;
	} else
 /*
 * Otherwise, nothing can be shifted out, so I return.
 */
	    return;
	low <<= 1;
	high <<= 1;
	high |= 1;
	code <<= 1;
	code += InputBit( stream );
    }
}





#define PACIFIER_COUNT 2047

/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      OpenOutputBitFile                                                */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function initializes the Output Bit File that is to be      */
/*      compressed or decompressed.                                      */                                                                 
/*                                                                       */
/*                                                                       */
/* AUTHOR                                                                */
/*                                                                       */
/*                                                                       */
/*                                                                       */
/*     PicoServer Embedded Web Server                                    */
/*                                                                       */
/*     Copywrite (c) 1995 1996 1997 CNiT                                 */
/*                                                                       */
/*     Communication and Information Technology                          */
/*                                                                       */
/*                                                                       */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      ps_decompress                   Decompresses a file in memory or */
/*                                      magnetic media.                  */
/*      ps_compress                     Compresses a file in memory or   */
/*                                      magnetic media.                  */                                                                
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      None.                                                            */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      bitfile                         Pointer to structure that        */
/*                                      holds all information about      */
/*                                      the file.                        */
/*      buf                             Output Bit Buffer to be opened.  */
/*      length                          Output Bit Buffer length.        */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      Always returns Zero.                                             */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         NAME            DATE                    REMARKS               */
/*                                                                       */
/*************************************************************************/


BIT_FILE * OpenOutputBitFile( BIT_FILE * bit_file, char *buf,int length )
{
    length = length;
    bit_file->fstart =  buf;
    bit_file->cur_pos = buf;
    bit_file->length = 0;
    bit_file->rack = 0;
    bit_file->mask = 0x80;
    bit_file->pacifier_counter = 0;

	return(0);
}

/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      OpenInputBitFile                                                 */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function initializes the Input Bit File that is to be       */
/*      compressed or decompressed.                                      */                                                                 
/*                                                                       */
/* AUTHOR                                                                */
/*                                                                       */
/*                                                                       */
/*                                                                       */
/*     PicoServer Embedded Web Server                                    */
/*                                                                       */
/*     Copywrite (c) 1995 1996 1997 CNiT                                 */
/*                                                                       */
/*     Communication and Information Technology                          */
/*                                                                       */
/*                                                                       */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      ps_decompress                   Decompresses a file in memory or */
/*                                      magnetic media.                  */
/*      ps_compress                     Compresses a file in memory or   */
/*                                      magnetic media.                  */                                                                
/* CALLS                                                                 */
/*                                                                       */
/*      None.                                                            */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      bitfile                         Pointer to structure that        */
/*                                      holds all information about      */
/*                                      the file.                        */
/*      buf                             Input Bit Buffer to be opened.   */
/*      length                          Input Bit Buffer length.         */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      Always returns Zero.                                             */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         NAME            DATE                    REMARKS               */
/*                                                                       */
/*************************************************************************/

BIT_FILE *
OpenInputBitFile( BIT_FILE * bit_file, char * buf, int length)
{
    bit_file->fstart = buf;
    bit_file->cur_pos = buf;
    bit_file->length = length;
    bit_file->rack = 0;
    bit_file->mask = 0x80;
    bit_file->pacifier_counter = 0;
	return(0);
}

/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      CloseOutputBitFile                                               */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*       Function that closes the bit file and then flushes the buffer.  */                                                                
/* AUTHOR                                                                */
/*                                                                       */
/*                                                                       */
/*                                                                       */
/*     PicoServer Embedded Web Server                                    */
/*                                                                       */
/*     Copywrite (c) 1995 1996 1997 CNiT                                 */
/*                                                                       */
/*     Communication and Information Technology                          */
/*                                                                       */
/*                                                                       */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      ps_decompress                   Decompresses a file in memory or */
/*                                      magnetic media.                  */
/*      ps_compress                     Compresses a file in memory or   */
/*                                      magnetic media.                  */                                                                
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      buf_putc                        Puts a character in a buffer.    */
/*      fatal_error                     Prints an Error message to a     */
/*                                      file.                            */
/*      flush_buf_putc                  Flush the putc buffer.           */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      bitfile                         Pointer to structure that        */
/*                                      holds all information about      */
/*                                      the file.                        */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      None.                                                            */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         NAME            DATE                    REMARKS               */
/*                                                                       */
/*************************************************************************/

void  CloseOutputBitFile( bit_file )
BIT_FILE *bit_file;
{
    if ( bit_file->mask != 0x80 )
	if ( buf_putc( bit_file->rack, bit_file ) != bit_file->rack )
	    fatal_error( "Fatal error in CloseBitFile!\n" );

	flush_buf_putc( bit_file );
}

/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*     CloseInputBitFile                                                 */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*     Function to Close the Input Bit File.  This currently is a dummy  */
/*     function, and is used used to fit the API.                        */                        
/*                                                                       */
/* AUTHOR                                                                */
/*                                                                       */
/*                                                                       */
/*                                                                       */
/*     PicoServer Embedded Web Server                                    */
/*                                                                       */
/*     Copywrite (c) 1995 1996 1997 CNiT                                 */
/*                                                                       */
/*     Communication and Information Technology                          */
/*                                                                       */
/*                                                                       */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      None.                                                            */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      None.                                                            */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      bitfile                         Pointer to structure that        */
/*                                      holds all information about      */
/*                                      the file.                        */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      None.                                                            */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         NAME            DATE                    REMARKS               */
/*                                                                       */
/*************************************************************************/

void  CloseInputBitFile( bit_file )
BIT_FILE *bit_file;
{
  /*  Remove Warnings */
      bit_file = bit_file;
}

/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      OutputBit                                                        */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      Function to output a specific bit into the file stream.          */
/*                                                                       */
/* AUTHOR                                                                */
/*                                                                       */
/*                                                                       */
/*                                                                       */
/*     PicoServer Embedded Web Server                                    */
/*                                                                       */
/*     Copywrite (c) 1995 1996 1997 CNiT                                 */
/*                                                                       */
/*     Communication and Information Technology                          */
/*                                                                       */
/*                                                                       */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      flush_arithmetic_encoder        Flushes Arithmetic encoder.      */
/*      encode_symbol                   Encodes a symbol for file        */
/*                                      compression.                     */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      buf_putc                        Puts a character in a buffer.    */
/*      fatal_error                     Prints an Error message to a     */
/*                                      file.                            */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      bit_file                        Pointer to structure that        */
/*                                      holds all information about      */
/*                                      the file.                        */
/*      bit                             Bit to be outputted              */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      None.                                                            */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         NAME            DATE                    REMARKS               */
/*                                                                       */
/*************************************************************************/

void  OutputBit( bit_file, bit )
BIT_FILE *bit_file;
int bit;
{
    if ( bit )
	bit_file->rack |= bit_file->mask;
    bit_file->mask >>= 1;
    if ( bit_file->mask == 0 )
    {
	if ( buf_putc( bit_file->rack, bit_file ) != bit_file->rack )
	    fatal_error( "Fatal error in OutputBit!\n" );
	else
	if ( ( bit_file->pacifier_counter++ & PACIFIER_COUNT ) == 0 );
		/*null statement*/
	bit_file->rack = 0;
	bit_file->mask = 0x80;
    }
}

/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      OutputBits                                                       */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      Function to output a code a certain number of counts. It also    */
/*      checks to make sure the value that is outputted is correct.  If  */
/*      it is not then write to file an error message.                   */
/*                                                                       */
/* AUTHOR                                                                */
/*                                                                       */
/*                                                                       */
/*                                                                       */
/*     PicoServer Embedded Web Server                                    */
/*                                                                       */
/*     Copywrite (c) 1995 1996 1997 CNiT                                 */
/*                                                                       */
/*     Communication and Information Technology                          */
/*                                                                       */
/*                                                                       */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      CompressFile                    Compresses an Input file.        */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      buf_putc                        Puts a character in a buffer.    */
/*      fatal_error                     Prints an Error message to a     */
/*                                      file.                            */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      bitfile                         Pointer to structure that        */
/*                                      holds all information about      */
/*                                      the file.                        */
/*      code                            A flag used in the selection of  */
/*                                      how the data is output.          */    
/*      count                           Represents the number of         */
/*                                      left shifts when building the    */
/*                                      mask.                            */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      None.                                                            */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         NAME            DATE                    REMARKS               */
/*                                                                       */
/*************************************************************************/

void  OutputBits( bit_file, code, count )
BIT_FILE *bit_file;
unsigned long code;
int count;
{
    unsigned long mask;

    mask = 1L << ( count - 1 );
    while ( mask != 0)
    {
	if ( mask & code )
	    bit_file->rack |= bit_file->mask;
	bit_file->mask >>= 1;
	if ( bit_file->mask == 0 )
	{
		if ( buf_putc( bit_file->rack, bit_file ) != bit_file->rack )
			fatal_error( "Fatal error in OutputBit!\n" );
	    else if ( ( bit_file->pacifier_counter++ & PACIFIER_COUNT ) == 0 );
		/* null statement*/
		bit_file->rack = 0;
	    bit_file->mask = 0x80;
	}
	mask >>= 1;
    }
}

/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      InputBit                                                         */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function gets a bit from an input file stream. It returns   */
/*      one or zero depending on the bit value.                          */
/*                                                                       */
/*                                                                       */
/* AUTHOR                                                                */
/*                                                                       */
/*                                                                       */
/*                                                                       */
/*     PicoServer Embedded Web Server                                    */
/*                                                                       */
/*     Copywrite (c) 1995 1996 1997 CNiT                                 */
/*                                                                       */
/*     Communication and Information Technology                          */
/*                                                                       */
/*                                                                       */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      initialize_arithmetic_decoder   Initializes the state of the     */
/*                                      Arithmetic Decoder.              */
/*      remove_symbol_from_stream       Removes a charcter from the input*/
/*                                      stream.                          */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      buf_getc                        Gets a character from the input  */
/*                                      bit_file.                        */
/*      fatal_error                     Prints an Error message to a     */
/*                                      file.                            */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      bitfile                         Pointer to structure that        */
/*                                      holds all information about      */
/*                                      the file.                        */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      value                           Holds the bit value.             */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         NAME            DATE                    REMARKS               */
/*                                                                       */
/*************************************************************************/

int InputBit( bit_file )
BIT_FILE *bit_file;
{
    int value;

    if ( bit_file->mask == 0x80 )
    {
	bit_file->rack = buf_getc( bit_file );
	if ( bit_file->rack == EOF )
	    fatal_error( "Fatal error in InputBit!\n" );
	if ( ( bit_file->pacifier_counter++ & PACIFIER_COUNT ) == 0 );
		/*null statement */
    }
    value = bit_file->rack & bit_file->mask;
    bit_file->mask >>= 1;
    if ( bit_file->mask == 0 )
	bit_file->mask = 0x80;
    return( value ? 1 : 0 );
}

/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      InputBits                                                        */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      Function to input 32 bits from an input file.  It returns a 32   */
/*      bit value.  It is currently not used. It is here for future      */
/*      updates.                                                         */
/*                                                                       */
/* AUTHOR                                                                */
/*                                                                       */
/*                                                                       */
/*                                                                       */
/*     PicoServer Embedded Web Server                                    */
/*                                                                       */
/*     Copywrite (c) 1995 1996 1997 CNiT                                 */
/*                                                                       */
/*     Communication and Information Technology                          */
/*                                                                       */
/*                                                                       */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      None.                                                            */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      fatal_error                     Prints an Error message to a     */
/*                                      file.                            */
/*      buf_getc                        Gets a character from the input  */
/*                                      bit_file.                        */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      bitfile                         Pointer to structure that        */
/*                                      holds all information about      */
/*                                      the file.                        */
/*      bit_count                       A specific count used for        */
/*                                      createing the mask.              */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      return_value                    32 bit value.                    */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         NAME            DATE                    REMARKS               */
/*                                                                       */
/*************************************************************************/

unsigned long InputBits( bit_file, bit_count )
BIT_FILE *bit_file;
int bit_count;
{
    unsigned long mask;
    unsigned long return_value;

    mask = 1L << ( bit_count - 1 );
    return_value = 0;
    while ( mask != 0)
    {
	    if ( bit_file->mask == 0x80 )
	{
		bit_file->rack = buf_getc( bit_file );
		if ( bit_file->rack == EOF )
			fatal_error( "Fatal error in InputBit!" );
	    if ( ( bit_file->pacifier_counter++ & PACIFIER_COUNT ) == 0 );
		/* null statement */
	    }
	    if ( bit_file->rack & bit_file->mask )
	    return_value |= mask;
	mask >>= 1;
	bit_file->mask >>= 1;
	if ( bit_file->mask == 0 )
	    bit_file->mask = 0x80;
    }
    return( return_value );
}

/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      buf_putc                                                         */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function is used to take a character in memory and put the  */
/*      the data in a buffer. Then based on the mode it either puts the  */ 
/*      value in a string or it transmits the character over the network.*/ 
/*                                                                       */
/*                                                                       */
/* AUTHOR                                                                */
/*                                                                       */
/*                                                                       */
/*                                                                       */
/*     PicoServer Embedded Web Server                                    */
/*                                                                       */
/*     Copywrite (c) 1995 1996 1997 CNiT                                 */
/*                                                                       */
/*     Communication and Information Technology                          */
/*                                                                       */
/*                                                                       */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      ExpandFile                      Expands a compressed file.       */
/*      output_counts                   Outputs the byte counts.         */
/*      CloseOutputBitFile              Closes Output Bit File.          */
/*      OutputBit                       Outputs a Specific bit in the    */
/*                                      file stream.                     */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*                                                                       */
/*      ps_net_write                    Writes data out to the Network.  */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      bitfile                         Pointer to structure that        */
/*                                      holds all information about      */
/*                                      the file.                        */
/*                                                                       */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      out                             String that the character was    */
/*                                      placed in.                       */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         NAME            DATE                    REMARKS               */
/*                                                                       */
/*************************************************************************/

int buf_putc( unsigned int out, BIT_FILE *bitfile )
{
    char c;

    if(( bitfile->mode & DONT_OUTPUT ) == 0  )
    {
	*( bitfile->cur_pos ) = ( char )out;

    }

	if( bitfile->mode  & NET_OUTPUT )
    {
		c=(char)out;
	ps_net_write( bitfile->fd,&c,1 );
	}

	bitfile->cur_pos++;
	bitfile->length++;

	return(out);
}

/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      flush_buf_putc                                                   */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      Hook for flushing the putc buffer.  This function has not been   */
/*      written.  It has been used to follow the API.                    */
/*                                                                       */
/* AUTHOR                                                                */
/*                                                                       */
/*                                                                       */
/*                                                                       */
/*     PicoServer Embedded Web Server                                    */
/*                                                                       */
/*     Copywrite (c) 1995 1996 1997 CNiT                                 */
/*                                                                       */
/*     Communication and Information Technology                          */
/*                                                                       */
/*                                                                       */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      CloseOutputBitFile              Closes Output Bit File.          */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      None.                                                            */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      b                               Pointer to structure that        */
/*                                      holds all information about      */
/*                                      the file.                        */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      None.                                                            */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         NAME            DATE                    REMARKS               */
/*                                                                       */
/*************************************************************************/

void  flush_buf_putc(BIT_FILE *b)
{
    /*  Remove Warnings */
    b = b;
}

/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      buf_getc                                                         */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      Function that gets a character from an input bitfile. It checks  */
/*      if the character is equal to EOF.  If it is not then it returns  */
/*      the character position to the calling routine.                   */ 
/*                                                                       */
/* AUTHOR                                                                */
/*                                                                       */
/*                                                                       */
/*                                                                       */
/*     PicoServer Embedded Web Server                                    */
/*                                                                       */
/*     Copywrite (c) 1995 1996 1997 CNiT                                 */
/*                                                                       */
/*     Communication and Information Technology                          */
/*                                                                       */
/*                                                                       */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      input_counts                    Reads the number of counts from  */
/*                                      the input file builds the table  */
/*                                      of cumulative counts.            */
/*      count_bytes                     Counts number of bytes.          */
/*      InputBit                        Reads a bit from an input file   */
/*                                      stream.                          */
/*      InputBits                       Inputs a bit based on a count.   */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      None.                                                            */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      bitfile                         Pointer to structure that        */
/*                                      holds all information about      */
/*                                      the file.                        */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      EOF                             EOF character                    */
/*      i                               Indicates that a character was   */
/*                                      found and gives position.        */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         NAME            DATE                    REMARKS               */
/*                                                                       */
/*************************************************************************/

int buf_getc( BIT_FILE *bitfile )
{
    int i;

    if( ( bitfile->cur_pos - bitfile->fstart ) > (( bitfile->length ))-1 )
    {
	return( EOF);
	};
		
	i = *(bitfile->cur_pos) & 0xff;
	bitfile->cur_pos++;

	return(i);
}


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      fatal_error                                                      */
/*                                                                       */
/* DESCRIPTION                                                           */
/*      Function to printf error message to file during creation of      */
/*      of in-memory file system(pkmfs).                                 */
/*                                                                       */
/* AUTHOR                                                                */
/*                                                                       */
/*                                                                       */
/*                                                                       */
/*     PicoServer Embedded Web Server                                    */
/*                                                                       */
/*     Copywrite (c) 1995 1996 1997 CNiT                                 */
/*                                                                       */
/*     Communication and Information Technology                          */
/*                                                                       */
/*                                                                       */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      output_counts                   Outputs the byte counts.         */
/*      input_counts                    Reads the number of counts from  */
/*                                      the input file builds the table  */
/*                                      of cumulative counts.            */
/*      CloseOutputBitFile              Closes Output Bit File.          */
/*      OutputBit                       Outputs a Specific bit in the    */
/*                                      file stream.                     */
/*      OutputBits                      Outputs a bit based on a count   */
/*                                      and a specific code.             */
/*      InputBit                        Reads a bit from an input file   */
/*                                      stream.                          */
/*      InputBits                       Inputs a bit based on a count.   */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      None.                                                            */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      s                               Holds the message                */
/*                                      that is to be printed to the     */
/*                                      file.                            */                                                
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      None.                                                            */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         NAME            DATE                    REMARKS               */
/*                                                                       */
/*************************************************************************/

void  fatal_error( char *s)
{   /*  remove warnings */
     s = s;
#ifdef DEBUG
    printf("fatal_error: Bit error %s\n",s);
#endif

}

#endif /* FILE_COMPRESSION */
