
/*
 * Rev 5.0 Lempel-Ziv & Welch Compression Support
 * Based on the LZW compression code from libtiff <www.libtiff.org>
 *
 * Copyright (c) 1985, 1986 The Regents of the University of California.
 * All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * James A. Woods, derived from original work by Spencer Thomas
 * and Joseph Orost.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * advertising materials, and other materials related to such
 * distribution and use acknowledge that the software was developed
 * by the University of California, Berkeley.  The name of the
 * University may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#include "../basicconf.h"
#include "../file_io.h"
#include "img2ps.h"

GLELZWByteStream::GLELZWByteStream(GLEByteStream* pipe) : GLEPipedByteStream(pipe) {
	m_RawDatasize = 4096;
	m_RawData = (GLEBYTE*)malloc(m_RawDatasize);
	m_RawCP = m_RawData;
	m_RawCC = 0;
	if (init() && setupEncode() && preEncode()) {
		m_State = GLE_IMAGE_ERROR_NONE;
	} else {
		cleanUp();
	}
}

GLELZWByteStream::~GLELZWByteStream() {
	cleanUp();
	if (m_RawData != NULL) {
		free(m_RawData);
	}
}

int GLELZWByteStream::send(GLEBYTE* bytes, GLEDWORD count) {
	if (encode(bytes, count)) {
		return GLE_IMAGE_ERROR_NONE;
	} else {
		return GLE_IMAGE_ERROR_DATA;
	}
}

int GLELZWByteStream::sendByte(GLEBYTE byte) {
	if (encode(&byte, 1)) {
		return GLE_IMAGE_ERROR_NONE;
	} else {
		return GLE_IMAGE_ERROR_DATA;
	}
}

int GLELZWByteStream::term() {
	if (!postEncode()) return GLE_IMAGE_ERROR_DATA;
	cleanUp();
	if (!flushData()) return GLE_IMAGE_ERROR_DATA;
	return GLEPipedByteStream::term();
}

int GLELZWByteStream::flushData() {
        if (m_RawCC > 0) {
		m_Pipe->send(m_RawData, m_RawCC);
		// FIXME return 0 depending on error code of send

                m_RawCC = 0;
                m_RawCP = m_RawData;
        }
        return 1;
}

#define MAXCODE(n)	((1L<<(n))-1)
#define	BITS_MIN	9	   	     /* start with 9 bits */
#define	BITS_MAX	12		     /* max of 12 bit strings */
#define	CODE_CLEAR	256		     /* code to clear string table */
#define	CODE_EOI	257		     /* end-of-information code */
#define CODE_FIRST	258		     /* first free code entry */
#define	CODE_MAX	MAXCODE(BITS_MAX)
#define	HSIZE		9001L		     /* 91% occupancy */
#define	HSHIFT		(13-8)
/* NB: +1024 is for compatibility with old files */
#define	CSIZE		(MAXCODE(BITS_MAX)+1024L)

typedef	struct {
	unsigned short	nbits;		/* # of bits/code */
	unsigned short	maxcode;	/* maximum code for lzw_nbits */
	unsigned short	free_ent;	/* next free entry in hash table */
	long		nextdata;	/* next bits of i/o */
	long		nextbits;	/* # of valid bits in lzw_nextdata */
} LZWBaseState;

#define	lzw_nbits	base.nbits
#define	lzw_maxcode	base.maxcode
#define	lzw_free_ent	base.free_ent
#define	lzw_nextdata	base.nextdata
#define	lzw_nextbits	base.nextbits

typedef unsigned short hcode_t;			/* codes fit in 16 bits */
typedef struct {
	long	hash;
	hcode_t	code;
} hash_t;

typedef struct {
	LZWBaseState base;
	int	 enc_oldcode;		/* last code encountered */
	long	 enc_checkpoint;	/* point at which to clear table */
#define CHECK_GAP	10000		/* enc_ratio check interval */
	long	 enc_ratio;		/* current compression ratio */
	long	 enc_incount;		/* (input) data bytes encoded */
	long	 enc_outcount;		/* encoded (output) bytes */
	GLEBYTE* enc_rawlimit;	        /* bound on m_RawData buffer */
	hash_t*	 enc_hashtab;		/* kept separate for small machines */
} encodeState;

static void GLELZWEncoderClearHash(encodeState* sp) {
	register hash_t *hp = &sp->enc_hashtab[HSIZE-1];
	register long i = HSIZE-8;
 	do {
		i -= 8;
		hp[-7].hash = -1;
		hp[-6].hash = -1;
		hp[-5].hash = -1;
		hp[-4].hash = -1;
		hp[-3].hash = -1;
		hp[-2].hash = -1;
		hp[-1].hash = -1;
		hp[ 0].hash = -1;
		hp -= 8;
	} while (i >= 0);
    	for (i += 8; i > 0; i--, hp--) {
		hp->hash = -1;
	}
}

int GLELZWByteStream::setupEncode() {
	encodeState* sp = (encodeState*)m_Data;
	sp->enc_hashtab = (hash_t*) malloc(HSIZE*sizeof (hash_t));
	if (sp->enc_hashtab == NULL) {
		return 0;
	}
	return 1;
}

int GLELZWByteStream::preEncode() {
	encodeState *sp = (encodeState*)m_Data;
	sp->lzw_nbits = BITS_MIN;
	sp->lzw_maxcode = MAXCODE(BITS_MIN);
	sp->lzw_free_ent = CODE_FIRST;
	sp->lzw_nextbits = 0;
	sp->lzw_nextdata = 0;
	sp->enc_checkpoint = CHECK_GAP;
	sp->enc_ratio = 0;
	sp->enc_incount = 0;
	sp->enc_outcount = 0;
	/*
	 * The 4 here insures there is space for 2 max-sized
	 * codes in encode and LZWPostDecode.
	 */
	sp->enc_rawlimit = m_RawData + m_RawDatasize-1 - 4;
	GLELZWEncoderClearHash(sp);
	sp->enc_oldcode = (hcode_t) -1;	/* generates CODE_CLEAR in encode */
	return 1;
}

#define	CALCRATIO(sp, rat) {					  \
	if (incount > 0x007fffff) { /* NB: shift will overflow */ \
		rat = outcount >> 8;				  \
		rat = (rat == 0 ? 0x7fffffff : incount/rat);	  \
	} else							  \
		rat = (incount<<8) / outcount;			  \
}
#define	PutNextCode(op, c) {					   \
	nextdata = (nextdata << nbits) | c;			   \
	nextbits += nbits;					   \
	*op++ = (unsigned char)(nextdata >> (nextbits-8));	   \
	nextbits -= 8;						   \
	if (nextbits >= 8) {					   \
		*op++ = (unsigned char)(nextdata >> (nextbits-8)); \
		nextbits -= 8;					   \
	}							   \
	outcount += nbits;					   \
}

/*
 * Encode a chunk of pixels.
 *
 * Uses an open addressing double hashing (no chaining) on the
 * prefix code/next character combination.  We do a variant of
 * Knuth's algorithm D (vol. 3, sec. 6.4) along with G. Knott's
 * relatively-prime secondary probe.  Here, the modular division
 * first probe is gives way to a faster exclusive-or manipulation.
 * Also do block compression with an adaptive reset, whereby the
 * code table is cleared when the compression ratio decreases,
 * but after the table fills.  The variable-length output codes
 * are re-sized at this point, and a CODE_CLEAR is generated
 * for the decoder.
 */
int GLELZWByteStream::encode(GLEBYTE* bp, GLEDWORD cc) {
	register encodeState *sp = (encodeState*)m_Data;
	register long fcode;
	register hash_t *hp;
	register int h, c;
	hcode_t ent;
	long disp;
	long incount, outcount, checkpoint;
	long nextdata, nextbits;
	int free_ent, maxcode, nbits;
	GLEBYTE* op, *limit;
	if (sp == NULL)	return 0;
	/*
	 * Load local state.
	 */
	incount = sp->enc_incount;
	outcount = sp->enc_outcount;
	checkpoint = sp->enc_checkpoint;
	nextdata = sp->lzw_nextdata;
	nextbits = sp->lzw_nextbits;
	free_ent = sp->lzw_free_ent;
	maxcode = sp->lzw_maxcode;
	nbits = sp->lzw_nbits;
	op = m_RawCP;
	limit = sp->enc_rawlimit;
	ent = sp->enc_oldcode;
	if (ent == (hcode_t) -1 && cc > 0) {
		/*
		 * NB: This is safe because it can only happen
		 *     at the start of a strip where we know there
		 *     is space in the data buffer.
		 */
		PutNextCode(op, CODE_CLEAR);
		ent = *bp++; cc--; incount++;
	}
	while (cc > 0) {
		c = *bp++; cc--; incount++;
		fcode = ((long)c << BITS_MAX) + ent;
		h = (c << HSHIFT) ^ ent;	/* xor hashing */
// #ifdef _WINDOWS
// 		/*
// 		 * Check hash index for an overflow.
// 		 */
// 		if (h >= HSIZE)
// 			h -= HSIZE;
// #endif
		hp = &sp->enc_hashtab[h];
		if (hp->hash == fcode) {
			ent = hp->code;
			continue;
		}
		if (hp->hash >= 0) {
			/*
			 * Primary hash failed, check secondary hash.
			 */
			disp = HSIZE - h;
			if (h == 0) disp = 1;
			do {
				/*
				 * Avoid pointer arithmetic 'cuz of
				 * wraparound problems with segments.
				 */
				if ((h -= disp) < 0) h += HSIZE;
				hp = &sp->enc_hashtab[h];
				if (hp->hash == fcode) {
					ent = hp->code;
					goto hit;
				}
			} while (hp->hash >= 0);
		}
		/*
		 * New entry, emit code and add to table.
		 */
		/*
		 * Verify there is space in the buffer for the code
		 * and any potential Clear code that might be emitted
		 * below.  The value of limit is setup so that there
		 * are at least 4 bytes free--room for 2 codes.
		 */
		if (op > limit) {
			m_RawCC = (GLEDWORD)(op - m_RawData);
			flushData();
			op = m_RawData;
		}
		PutNextCode(op, ent);
		ent = c;
		hp->code = free_ent++;
		hp->hash = fcode;
		if (free_ent == CODE_MAX-1) {
			/* table is full, emit clear code and reset */
			GLELZWEncoderClearHash(sp);
			sp->enc_ratio = 0;
			incount = 0;
			outcount = 0;
			free_ent = CODE_FIRST;
			PutNextCode(op, CODE_CLEAR);
			nbits = BITS_MIN;
			maxcode = MAXCODE(BITS_MIN);
		} else {
			/*
			 * If the next entry is going to be too big for
			 * the code size, then increase it, if possible.
			 */
			if (free_ent > maxcode) {
				nbits++;
				// assert(nbits <= BITS_MAX);
				maxcode = (int) MAXCODE(nbits);
			} else if (incount >= checkpoint) {
				long rat;
				/*
				 * Check compression ratio and, if things seem
				 * to be slipping, clear the hash table and
				 * reset state.  The compression ratio is a
				 * 24+8-bit fractional number.
				 */
				checkpoint = incount+CHECK_GAP;
				CALCRATIO(sp, rat);
				if (rat <= sp->enc_ratio) {
					GLELZWEncoderClearHash(sp);
					sp->enc_ratio = 0;
					incount = 0;
					outcount = 0;
					free_ent = CODE_FIRST;
					PutNextCode(op, CODE_CLEAR);
					nbits = BITS_MIN;
					maxcode = MAXCODE(BITS_MIN);
				} else {
					sp->enc_ratio = rat;
				}
			}
		}
	hit:
		;
	}
	/*
	 * Restore global state.
	 */
	sp->enc_incount = incount;
	sp->enc_outcount = outcount;
	sp->enc_checkpoint = checkpoint;
	sp->enc_oldcode = ent;
	sp->lzw_nextdata = nextdata;
	sp->lzw_nextbits = nextbits;
	sp->lzw_free_ent = free_ent;
	sp->lzw_maxcode = maxcode;
	sp->lzw_nbits = nbits;
	m_RawCP = op;
	return 1;
}

/*
 * Finish off an encoded strip by flushing the last
 * string and tacking on an End Of Information code.
 */
int GLELZWByteStream::postEncode() {
	register encodeState *sp = (encodeState*)m_Data;
	GLEBYTE* op = m_RawCP;
	long nextbits = sp->lzw_nextbits;
	long nextdata = sp->lzw_nextdata;
	long outcount = sp->enc_outcount;
	int nbits = sp->lzw_nbits;
	if (op > sp->enc_rawlimit) {
		m_RawCC = (GLEDWORD)(op - m_RawData);
		flushData();
		op = m_RawData;
	}
	if (sp->enc_oldcode != (hcode_t) -1) {
		PutNextCode(op, sp->enc_oldcode);
		sp->enc_oldcode = (hcode_t) -1;
	}
	PutNextCode(op, CODE_EOI);
	if (nextbits > 0) {
		*op++ = (unsigned char)(nextdata << (8-nextbits));
	}
	m_RawCC = (GLEDWORD)(op - m_RawData);
	return 1;
}

void GLELZWByteStream::cleanUp() {
	if (m_Data != NULL) {
		if (((encodeState*)m_Data)->enc_hashtab != NULL) {
			free(((encodeState*)m_Data)->enc_hashtab);
		}
		free(m_Data);
		m_Data = NULL;
	}
}

int GLELZWByteStream::init() {
	m_Data = (GLEBYTE*) malloc(sizeof (encodeState));
	if (m_Data == NULL) {
		return 0;
	}
	((encodeState*)m_Data)->enc_hashtab = NULL;
	return 1;
}

