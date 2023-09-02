/* Test interger and time functions
 *
 */

import java.util.Date;
import java.io.FileOutputStream;

public class T3Encdec {

	static class DataBlock {
		short sbe0;
		short sbe1;
		short sbe2;
		short sle0;
		short sle1;
		short sle2;
		int ibe0;
		int ibe1;
		int ibe2;
		int ile0;
		int ile1;
		int ile2;
		long lbe0;
		long lbe1;
		long lbe2;
		long lle0;
		long lle1;
		long lle2;
		float fbe0;
		float fbe1;
		float fbe2;
		float fle0;
		float fle1;
		float fle2;
		double dbe0;
		double dbe1;
		double dbe2;
		double dle0;
		double dle1;
		double dle2;
		Date tbe0;
		Date tle0;
		Date tbe1;
		Date tle1;
		Date tle2;
		Date tbe2;
		Date tle3;
		Date tbe3;
	}

	public static void main( String[] args ) throws Exception {
		byte[] buf = new byte[1024];
		int off = 0;
		DataBlock d0 = new DataBlock();
		DataBlock d1 = new DataBlock();

		d0.sbe0 = 1;
		d0.sbe1 = 2345;
		d0.sbe2 = -2;
		d0.sle0 = 7;
		d0.sle1 = 3456;
		d0.sle2 = -5;
		d0.ibe0 = 1;
		d0.ibe1 = 700000;
		d0.ibe2 = -233457;
		d0.ile0 = 7;
		d0.ile1 = 700500;
		d0.ile2 = -5;
		d0.lbe0 = 1L;
		d0.lbe1 = 1800888999000L;
		d0.lbe2 = 999000999000L;
		d0.lle0 = 1L;
		d0.lle1 = 0x70F0F0F0F0F0F0F0L;
		d0.lle2 = 9L;

		d0.fbe0 = -1.92727e-07f;
		d0.fbe1 = 3.388282e+38f;
		d0.fbe2 = 1.172828e-38f;
		d0.fle0 = -1.92727e-07f;
		d0.fle1 = 3.388282e+38f;
		d0.fle2 = 1.172828e-38f;
		d0.dbe0 = -2.218356e-16;
		d0.dbe1 = 1.7782822e+308;
		d0.dbe2 = 2.2100000e-308;
		d0.dle0 = -2.218356e-16;
		d0.dle1 = 1.7782822e+308;
		d0.dle2 = 2.2100000e-308;

                       /* Binary files will differ slightly because of dates */
		d0.tbe0 = d0.tle0 = d0.tbe1 = d0.tle1 = d0.tle2 = d0.tbe2 = d0.tbe3 = d0.tle3 = new Date();

		/*   0 0x00 */
		off += Encdec.enc_uint16be(d0.sbe0, buf, off);
		off += Encdec.enc_uint16be(d0.sbe1, buf, off);
		off += Encdec.enc_uint16be(d0.sbe2, buf, off);
		off += Encdec.enc_uint16le(d0.sle0, buf, off);
		off += Encdec.enc_uint16le(d0.sle1, buf, off);
		off += Encdec.enc_uint16le(d0.sle2, buf, off);
		/*  12 0x0C */	
		off += Encdec.enc_uint32be(d0.ibe0, buf, off);
		off += Encdec.enc_uint32be(d0.ibe1, buf, off);
		off += Encdec.enc_uint32be(d0.ibe2, buf, off);
		off += Encdec.enc_uint32le(d0.ile0, buf, off);
		off += Encdec.enc_uint32le(d0.ile1, buf, off);
		off += Encdec.enc_uint32le(d0.ile2, buf, off);
		/*  36 0x24 */
		off += Encdec.enc_uint64be(d0.lbe0, buf, off);
		off += Encdec.enc_uint64be(d0.lbe1, buf, off);
		off += Encdec.enc_uint64be(d0.lbe2, buf, off);
		off += Encdec.enc_uint64le(d0.lle0, buf, off);
		off += Encdec.enc_uint64le(d0.lle1, buf, off);
		off += Encdec.enc_uint64le(d0.lle2, buf, off);
		/*  84 0x54 */
		off += Encdec.enc_floatbe(d0.fbe0, buf, off);
		off += Encdec.enc_floatbe(d0.fbe1, buf, off);
		off += Encdec.enc_floatbe(d0.fbe2, buf, off);
		off += Encdec.enc_floatle(d0.fle0, buf, off);
		off += Encdec.enc_floatle(d0.fle1, buf, off);
		off += Encdec.enc_floatle(d0.fle2, buf, off);
		/* 108 0x6C */
		off += Encdec.enc_doublebe(d0.dbe0, buf, off);
		off += Encdec.enc_doublebe(d0.dbe1, buf, off);
		off += Encdec.enc_doublebe(d0.dbe2, buf, off);
		off += Encdec.enc_doublele(d0.dle0, buf, off);
		off += Encdec.enc_doublele(d0.dle1, buf, off);
		off += Encdec.enc_doublele(d0.dle2, buf, off);
		/* 156 0x9C */
		off += Encdec.enc_time(d0.tbe0, buf, off, Encdec.TIME_1970_SEC_32BE);
		off += Encdec.enc_time(d0.tle0, buf, off, Encdec.TIME_1970_SEC_32LE);
		off += Encdec.enc_time(d0.tbe1, buf, off, Encdec.TIME_1904_SEC_32BE);
		off += Encdec.enc_time(d0.tle1, buf, off, Encdec.TIME_1904_SEC_32LE);
		off += Encdec.enc_time(d0.tbe2, buf, off, Encdec.TIME_1601_NANOS_64BE);
		off += Encdec.enc_time(d0.tle2, buf, off, Encdec.TIME_1601_NANOS_64LE);
		off += Encdec.enc_time(d0.tbe3, buf, off, Encdec.TIME_1970_MILLIS_64BE);
		off += Encdec.enc_time(d0.tle3, buf, off, Encdec.TIME_1970_MILLIS_64LE);
	    /* 188 0xBC */

		off = 0;
	
		d1.sbe0 = Encdec.dec_uint16be(buf, off); off += 2;
		d1.sbe1 = Encdec.dec_uint16be(buf, off); off += 2;
		d1.sbe2 = Encdec.dec_uint16be(buf, off); off += 2;
		d1.sle0 = Encdec.dec_uint16le(buf, off); off += 2;
		d1.sle1 = Encdec.dec_uint16le(buf, off); off += 2;
		d1.sle2 = Encdec.dec_uint16le(buf, off); off += 2;
	
		d1.ibe0 = Encdec.dec_uint32be(buf, off); off += 4;
		d1.ibe1 = Encdec.dec_uint32be(buf, off); off += 4;
		d1.ibe2 = Encdec.dec_uint32be(buf, off); off += 4;
		d1.ile0 = Encdec.dec_uint32le(buf, off); off += 4;
		d1.ile1 = Encdec.dec_uint32le(buf, off); off += 4;
		d1.ile2 = Encdec.dec_uint32le(buf, off); off += 4;
	
		d1.lbe0 = Encdec.dec_uint64be(buf, off); off += 8;
		d1.lbe1 = Encdec.dec_uint64be(buf, off); off += 8;
		d1.lbe2 = Encdec.dec_uint64be(buf, off); off += 8;
		d1.lle0 = Encdec.dec_uint64le(buf, off); off += 8;
		d1.lle1 = Encdec.dec_uint64le(buf, off); off += 8;
		d1.lle2 = Encdec.dec_uint64le(buf, off); off += 8;
	
		d1.fbe0 = Encdec.dec_floatbe(buf, off); off += 4;
		d1.fbe1 = Encdec.dec_floatbe(buf, off); off += 4;
		d1.fbe2 = Encdec.dec_floatbe(buf, off); off += 4;
		d1.fle0 = Encdec.dec_floatle(buf, off); off += 4;
		d1.fle1 = Encdec.dec_floatle(buf, off); off += 4;
		d1.fle2 = Encdec.dec_floatle(buf, off); off += 4;

		d1.dbe0 = Encdec.dec_doublebe(buf, off); off += 8;
		d1.dbe1 = Encdec.dec_doublebe(buf, off); off += 8;
		d1.dbe2 = Encdec.dec_doublebe(buf, off); off += 8;
		d1.dle0 = Encdec.dec_doublele(buf, off); off += 8;
		d1.dle1 = Encdec.dec_doublele(buf, off); off += 8;
		d1.dle2 = Encdec.dec_doublele(buf, off); off += 8;
	
		d1.tbe0 = Encdec.dec_time(buf, off, Encdec.TIME_1970_SEC_32BE); off += 4;
		d1.tle0 = Encdec.dec_time(buf, off, Encdec.TIME_1970_SEC_32LE); off += 4;
		d1.tbe1 = Encdec.dec_time(buf, off, Encdec.TIME_1904_SEC_32BE); off += 4;
		d1.tle1 = Encdec.dec_time(buf, off, Encdec.TIME_1904_SEC_32LE); off += 4;
		d1.tbe2 = Encdec.dec_time(buf, off, Encdec.TIME_1601_NANOS_64BE); off += 8;
		d1.tle2 = Encdec.dec_time(buf, off, Encdec.TIME_1601_NANOS_64LE); off += 8;
		d1.tbe3 = Encdec.dec_time(buf, off, Encdec.TIME_1970_MILLIS_64BE); off += 8;
		d1.tle3 = Encdec.dec_time(buf, off, Encdec.TIME_1970_MILLIS_64LE); off += 8;

		if( d0.sbe0 != d1.sbe0 ) throw new Exception();
		if( d0.sbe1 != d1.sbe1 ) throw new Exception();
		if( d0.sbe2 != d1.sbe2 ) throw new Exception();
		if( d0.sle0 != d1.sle0 ) throw new Exception();
		if( d0.sle1 != d1.sle1 ) throw new Exception();
		if( d0.sle2 != d1.sle2 ) throw new Exception();
	
		if( d0.ibe0 != d1.ibe0 ) throw new Exception();
		if( d0.ibe1 != d1.ibe1 ) throw new Exception();
		if( d0.ibe2 != d1.ibe2 ) throw new Exception();
		if( d0.ile0 != d1.ile0 ) throw new Exception();
		if( d0.ile1 != d1.ile1 ) throw new Exception();
		if( d0.ile2 != d1.ile2 ) throw new Exception();
	
		if( d0.lbe0 != d1.lbe0 ) throw new Exception();
		if( d0.lbe1 != d1.lbe1 ) throw new Exception();
		if( d0.lbe2 != d1.lbe2 ) throw new Exception();
		if( d0.lle0 != d1.lle0 ) throw new Exception();
		if( d0.lle1 != d1.lle1 ) throw new Exception();
		if( d0.lle2 != d1.lle2 ) throw new Exception();
	
		if( d0.fbe0 != d1.fbe0 ) throw new Exception();
		if( d0.fbe1 != d1.fbe1 ) throw new Exception();
		if( d0.fbe2 != d1.fbe2 ) throw new Exception();
		if( d0.fle0 != d1.fle0 ) throw new Exception();
		if( d0.fle1 != d1.fle1 ) throw new Exception();
		if( d0.fle2 != d1.fle2 ) throw new Exception();
	
		if( d0.dbe0 != d1.dbe0 ) throw new Exception();
		if( d0.dbe1 != d1.dbe1 ) throw new Exception();
		if( d0.dbe2 != d1.dbe2 ) throw new Exception();
		if( d0.dle0 != d1.dle0 ) throw new Exception();
		if( d0.dle1 != d1.dle1 ) throw new Exception();
		if( d0.dle2 != d1.dle2 ) throw new Exception();
	
		if(( d0.tbe0.getTime() - d1.tbe0.getTime() ) > 999 ) throw new Exception();
		if(( d0.tle0.getTime() - d1.tle0.getTime() ) > 999 ) throw new Exception();
		if(( d0.tbe1.getTime() - d1.tbe1.getTime() ) > 999 ) throw new Exception();
		if(( d0.tle1.getTime() - d1.tle1.getTime() ) > 999 ) throw new Exception();
		if( d0.tle2.equals( d1.tle2 ) == false ) throw new Exception();
		if( d0.tbe2.equals( d1.tbe2 ) == false ) throw new Exception();
		if( d0.tle3.equals( d1.tle3 ) == false ) throw new Exception();
		if( d0.tbe3.equals( d1.tbe3 ) == false ) throw new Exception();

		System.err.println( "" + d0.tbe0 );
		System.err.println( "" + d1.tbe0 );

		if (args.length > 0) {
			FileOutputStream out = new FileOutputStream( args[0] );
			out.write( buf, 0, off );
			out.close();
		}
	}
}

