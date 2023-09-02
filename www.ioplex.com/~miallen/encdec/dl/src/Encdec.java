/* encdec - encode and decode integers, times, and
 * internationalized strings to and from popular binary formats
 * Copyright (c) 2003 Michael B. Allen <mballen@erols.com>
 *
 * The GNU Library General Public License
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 * 
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free
 * Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,
 * MA 02111-1307, USA
 */

import java.util.Date;
import jcifs.util.Log;

public class Encdec {

	public static final long MILLISECONDS_BETWEEN_1970_AND_1601 = 11644473600000L;
	public static final long SEC_BETWEEEN_1904_AND_1970 = 2082844800L;
	public static final int TIME_1970_SEC_32BE = 1;
	public static final int TIME_1970_SEC_32LE = 2;
	public static final int TIME_1904_SEC_32BE = 3;
	public static final int TIME_1904_SEC_32LE = 4;
	public static final int TIME_1601_NANOS_64LE = 5;
	public static final int TIME_1601_NANOS_64BE = 6;
	public static final int TIME_1970_MILLIS_64BE = 7;
	public static final int TIME_1970_MILLIS_64LE = 8;

	/* Encode integers
	 */

	public static int enc_uint16be( short s, byte[] dst, int di ) {
		dst[di++] = (byte)((s >> 8) & 0xFF);
		dst[di] = (byte)(s & 0xFF);
		return 2;
	}
	public static int enc_uint32be( int i, byte[] dst, int di ) {
		dst[di++] = (byte)((i >> 24) & 0xFF);
		dst[di++] = (byte)((i >> 16) & 0xFF);
		dst[di++] = (byte)((i >> 8) & 0xFF);
		dst[di] = (byte)(i & 0xFF);
		return 4;
	}
	public static int enc_uint16le( short s, byte[] dst, int di )
	{
		dst[di++] = (byte)(s & 0xFF);
		dst[di] = (byte)((s >> 8) & 0xFF);
		return 2;
	}
	public static int enc_uint32le( int i, byte[] dst, int di )
	{
		dst[di++] = (byte)(i & 0xFF);
		dst[di++] = (byte)((i >> 8) & 0xFF);
		dst[di++] = (byte)((i >> 16) & 0xFF);
		dst[di] = (byte)((i >> 24) & 0xFF);
		return 4;
	}

	/* Decode integers
	 */

	public static short dec_uint16be( byte[] src, int si )
	{
		return (short)(((src[si] & 0xFF) << 8) | (src[si + 1] & 0xFF));
	}
	public static int dec_uint32be( byte[] src, int si )
	{
		return ((src[si] & 0xFF) << 24) | ((src[si + 1] & 0xFF) << 16) |
	           ((src[si + 2] & 0xFF) << 8) | (src[si + 3] & 0xFF);
	}
	public static short dec_uint16le( byte[] src, int si )
	{
		return (short)((src[si] & 0xFF) | ((src[si + 1] & 0xFF) << 8));
	}
	public static int dec_uint32le( byte[] src, int si )
	{
		return (src[si] & 0xFF) | ((src[si + 1] & 0xFF) << 8) |
	           ((src[si + 2] & 0xFF) << 16) | ((src[si + 3] & 0xFF) << 24);
	}

	/* Encode and decode 64 bit integers
	 */

	public static int enc_uint64be( long l, byte[] dst, int di )
	{
		enc_uint32be( (int)(l & 0xFFFFFFFFL), dst, di + 4 );
		enc_uint32be( (int)(( l >> 32L ) & 0xFFFFFFFFL), dst, di );
		return 8;
	}
	public static int enc_uint64le( long l, byte[] dst, int di )
	{
		enc_uint32le( (int)(l & 0xFFFFFFFFL), dst, di );
		enc_uint32le( (int)(( l >> 32L ) & 0xFFFFFFFFL), dst, di + 4 );
		return 8;
	}
	public static long dec_uint64be( byte[] src, int si )
	{
		long l;
		l = dec_uint32be( src, si ) & 0xFFFFFFFFL;
		l <<= 32L;
		l |= dec_uint32be( src, si + 4 ) & 0xFFFFFFFFL;
		return l;
	}
	public static long dec_uint64le( byte[] src, int si )
	{
		long l;
		l = dec_uint32le( src, si + 4 ) & 0xFFFFFFFFL;
		l <<= 32L;
		l |= dec_uint32le( src, si ) & 0xFFFFFFFFL;
		return l;
	}

	/* Encode floats
	 */

	public static int enc_floatle( float f, byte[] dst, int di )
	{
		return enc_uint32le( Float.floatToIntBits( f ), dst, di );
	}
	public static int enc_floatbe( float f, byte[] dst, int di )
	{
		return enc_uint32be( Float.floatToIntBits( f ), dst, di );
	}

	/* Decode floating point numbers
	 */

	public static float dec_floatle( byte[] src, int si )
	{
		return Float.intBitsToFloat( dec_uint32le( src, si ));
	}
	public static float dec_floatbe( byte[] src, int si )
	{
		return Float.intBitsToFloat( dec_uint32be( src, si ));
	}

	/* Encode and decode doubles
	 */

	public static int enc_doublele( double d, byte[] dst, int di )
	{
		return enc_uint64le( Double.doubleToLongBits( d ), dst, di );
	}
	public static int enc_doublebe( double d, byte[] dst, int di )
	{
		return enc_uint64be( Double.doubleToLongBits( d ), dst, di );
	}
	public static double dec_doublele( byte[] src, int si )
	{
		return Double.longBitsToDouble( dec_uint64le( src, si ));
	}
	public static double dec_doublebe( byte[] src, int si )
	{
		return Double.longBitsToDouble( dec_uint64be( src, si ));
	}

	/* Encode times
	 */

	public static int enc_time( Date date, byte[] dst, int di, int enc )
	{
		long t;

		switch( enc ) {
			case TIME_1970_SEC_32BE:
				return enc_uint32be( (int)(date.getTime() / 1000L), dst, di );
			case TIME_1970_SEC_32LE:
				return enc_uint32le( (int)(date.getTime() / 1000L), dst, di );
			case TIME_1904_SEC_32BE:
				return enc_uint32be( (int)((date.getTime() / 1000L +
					SEC_BETWEEEN_1904_AND_1970) & 0xFFFFFFFF), dst, di );
			case TIME_1904_SEC_32LE:
				return enc_uint32le( (int)((date.getTime() / 1000L +
					SEC_BETWEEEN_1904_AND_1970) & 0xFFFFFFFF), dst, di );
			case TIME_1601_NANOS_64BE:
				t = (date.getTime() + MILLISECONDS_BETWEEN_1970_AND_1601) * 10000L;
				return enc_uint64be( t, dst, di );
			case TIME_1601_NANOS_64LE:
				t = (date.getTime() + MILLISECONDS_BETWEEN_1970_AND_1601) * 10000L;
				return enc_uint64le( t, dst, di );
			case TIME_1970_MILLIS_64BE:
				return enc_uint64be( date.getTime(), dst, di );
			case TIME_1970_MILLIS_64LE:
				return enc_uint64le( date.getTime(), dst, di );
			default:
				throw new IllegalArgumentException( "Unsupported time encoding" );
		}
	}

	/* Decode times
	 */

	public static Date dec_time( byte[] src, int si, int enc )
	{
		long t;
	
		switch( enc ) {
			case TIME_1970_SEC_32BE:
				return new Date( dec_uint32be( src, si ) * 1000L );
			case TIME_1970_SEC_32LE:
				return new Date( dec_uint32le( src, si ) * 1000L );
			case TIME_1904_SEC_32BE:
				return new Date((( dec_uint32be( src, si ) & 0xFFFFFFFFL) -
					SEC_BETWEEEN_1904_AND_1970 ) * 1000L );
			case TIME_1904_SEC_32LE:
				return new Date((( dec_uint32le( src, si ) & 0xFFFFFFFFL) -
					SEC_BETWEEEN_1904_AND_1970 ) * 1000L );
			case TIME_1601_NANOS_64BE:
				t = dec_uint64be( src, si );
				return new Date( t / 10000L - MILLISECONDS_BETWEEN_1970_AND_1601);
			case TIME_1601_NANOS_64LE:
				t = dec_uint64le( src, si );
				return new Date( t / 10000L - MILLISECONDS_BETWEEN_1970_AND_1601);
			case TIME_1970_MILLIS_64BE:
				return new Date( dec_uint64be( src, si ));
			case TIME_1970_MILLIS_64LE:
				return new Date( dec_uint64le( src, si ));
			default:
				throw new IllegalArgumentException( "Unsupported time encoding" );
		}
	}
}
