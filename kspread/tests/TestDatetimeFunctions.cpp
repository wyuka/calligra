/* This file is part of the KDE project
   Copyright 2007 Sascha Pfau <MrPeacock@web.de>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; only
   version 2 of the License.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "TestKspreadCommon.h"

#include "TestDatetimeFunctions.h"

#define CHECK_EVAL(x,y) { Value z(RoundNumber(y)); QCOMPARE(evaluate(x,z), (z)); }

#define CHECK_FAIL(x,y,txt) { Value z(RoundNumber(y)); QEXPECT_FAIL("", txt, Continue); QCOMPARE(evaluate(x,z), (z));}


// round to get at most 10-digits number
static Value RoundNumber(double f)
{
  return Value( QString::number(f, 'g', 9) );
}

// round to get at most 10-digits number
static Value RoundNumber(const Value& v)
{
  if(v.isNumber())
  {
    double d = v.asFloat();
    if(fabs(d) < DBL_EPSILON)
      d = 0.0;
    return Value( QString::number(d, 'g', 9) );
  }
  else
    return v;
}

Value TestDatetimeFunctions::evaluate(const QString& formula, Value& ex)
{
    Formula f;
    QString expr = formula;
    if ( expr[0] != '=' )
        expr.prepend( '=' );
    f.setExpression( expr );
    Value result = f.eval();

    if(result.isFloat() && ex.isInteger())
        ex = Value(ex.asFloat());
    if(result.isInteger() && ex.isFloat())
        result = Value(result.asFloat());

    return RoundNumber(result);
}

void TestDatetimeFunctions::testYEARFRAC()
{
  
  // basis 0 US
  CHECK_EVAL( "YEARFRAC( \"1999-01-01\" ; \"1999-06-30\" ; 0)", Value( 0.4972222222 ) );
  CHECK_EVAL( "YEARFRAC( \"1999-01-01\" ; \"1999-07-01\" ; 0)", Value( 0.5000000000 ) );
  CHECK_EVAL( "YEARFRAC( \"2000-01-01\" ; \"2000-06-30\" ; 0)", Value( 0.4972222222 ) );
  CHECK_EVAL( "YEARFRAC( \"2000-01-15\" ; \"2000-09-17\" ; 0)", Value( 0.6722222222 ) );
  CHECK_EVAL( "YEARFRAC( \"2000-01-01\" ; \"2001-01-01\" ; 0)", Value( 1.0000000000 ) );
  CHECK_EVAL( "YEARFRAC( \"2001-01-01\" ; \"2002-01-01\" ; 0)", Value( 1.0000000000 ) );
  CHECK_EVAL( "YEARFRAC( \"2001-12-05\" ; \"2001-12-30\" ; 0)", Value( 0.0694444444 ) );
  CHECK_EVAL( "YEARFRAC( \"2000-02-05\" ; \"2006-08-10\" ; 0)", Value( 6.5138888889 ) );
  
  // basis 1 Actual/actual
  CHECK_EVAL( "YEARFRAC( \"1999-01-01\" ; \"1999-06-30\" ; 1)", Value( 0.4931506849 ) ); 
  CHECK_EVAL( "YEARFRAC( \"1999-01-01\" ; \"1999-07-01\" ; 1)", Value( 0.4958904110 ) );
  CHECK_EVAL( "YEARFRAC( \"2000-01-01\" ; \"2000-06-30\" ; 1)", Value( 0.4945355191 ) );
  CHECK_EVAL( "YEARFRAC( \"2000-01-15\" ; \"2000-09-17\" ; 1)", Value( 0.6721311475 ) );
  CHECK_EVAL( "YEARFRAC( \"2000-01-01\" ; \"2001-01-01\" ; 1)", Value( 1.0000000000 ) );
  CHECK_EVAL( "YEARFRAC( \"2001-12-05\" ; \"2001-12-30\" ; 1)", Value( 0.0684931507 ) );
  CHECK_EVAL( "YEARFRAC( \"2000-02-05\" ; \"2006-08-10\" ; 1)", Value( 6.5099726242 ) );
  CHECK_EVAL( "YEARFRAC( \"2003-12-06\" ; \"2004-03-05\" ; 1)", Value( 0.2459016393 ) );
  CHECK_EVAL( "YEARFRAC( \"2003-12-31\" ; \"2004-03-31\" ; 1)", Value( 0.2486338798 ) );
  CHECK_EVAL( "YEARFRAC( \"2004-10-01\" ; \"2005-01-11\" ; 1)", Value( 0.2794520548 ) );
  CHECK_EVAL( "YEARFRAC( \"2004-10-26\" ; \"2005-02-06\" ; 1)", Value( 0.2821917808 ) );
  CHECK_EVAL( "YEARFRAC( \"2004-11-20\" ; \"2005-03-04\" ; 1)", Value( 0.2849315068 ) );
  CHECK_EVAL( "YEARFRAC( \"2004-12-15\" ; \"2005-03-30\" ; 1)", Value( 0.2876712329 ) );
  CHECK_EVAL( "YEARFRAC( \"2000-12-01\" ; \"2001-01-16\" ; 1)", Value( 0.1260273973 ) );
  CHECK_EVAL( "YEARFRAC( \"2000-12-26\" ; \"2001-02-11\" ; 1)", Value( 0.1287671233 ) );
  
  // basis 2 Actual/360
  CHECK_EVAL( "YEARFRAC( \"2000-01-01\" ; \"2000-06-30\" ; 2)", Value( 0.5027777778 ) );
  CHECK_EVAL( "YEARFRAC( \"1999-01-01\" ; \"1999-06-30\" ; 2)", Value( 0.5000000000 ) );
  CHECK_EVAL( "YEARFRAC( \"1999-01-01\" ; \"1999-07-01\" ; 2)", Value( 0.5027777778 ) );
  CHECK_EVAL( "YEARFRAC( \"2000-01-15\" ; \"2000-09-17\" ; 2)", Value( 0.6833333333 ) );
  CHECK_EVAL( "YEARFRAC( \"2000-01-01\" ; \"2001-01-01\" ; 2)", Value( 1.0166666667 ) );
  CHECK_EVAL( "YEARFRAC( \"2001-01-01\" ; \"2002-01-01\" ; 2)", Value( 1.0138888889 ) );
  CHECK_FAIL( "YEARFRAC( \"2001-12-15\" ; \"2001-12-30\" ; 2)", Value( 0.0694444444 ), "Known not to work -> Excel 0.0694444444" ); // 0.0416666667, but should be 0.0694444444
  CHECK_EVAL( "YEARFRAC( \"2000-02-05\" ; \"2006-08-10\" ; 2)", Value( 6.6055555556 ) );

  // basis 3 Actual/365
  CHECK_EVAL( "YEARFRAC( \"2000-01-01\" ; \"2001-01-01\" ; 3)", Value( 1.0027397260 ) );
  CHECK_EVAL( "YEARFRAC( \"2001-01-01\" ; \"2002-01-01\" ; 3)", Value( 1.0000000000 ) );
  CHECK_EVAL( "YEARFRAC( \"2001-12-05\" ; \"2001-12-30\" ; 3)", Value( 0.0684931507 ) );
  CHECK_EVAL( "YEARFRAC( \"2000-02-05\" ; \"2006-08-10\" ; 3)", Value( 6.5150684932 ) );

  // basis 4 European 30/360
  CHECK_EVAL( "YEARFRAC( \"1999-01-01\" ; \"1999-06-30\" ; 4)", Value( 0.4972222222 ) );
  CHECK_EVAL( "YEARFRAC( \"2000-01-01\" ; \"2000-06-30\" ; 4)", Value( 0.4972222222 ) );
  CHECK_EVAL( "YEARFRAC( \"2000-01-15\" ; \"2000-09-17\" ; 4)", Value( 0.6722222222 ) );
  CHECK_EVAL( "YEARFRAC( \"2000-01-01\" ; \"2001-01-01\" ; 4)", Value( 1.0000000000 ) );
  CHECK_EVAL( "YEARFRAC( \"2001-01-01\" ; \"2002-01-01\" ; 4)", Value( 1.0000000000 ) );
  CHECK_EVAL( "YEARFRAC( \"2001-12-05\" ; \"2001-12-30\" ; 4)", Value( 0.0694444444 ) );
  CHECK_EVAL( "YEARFRAC( \"2000-02-05\" ; \"2006-08-10\" ; 3)", Value( 6.5138888889 ) );
}

void TestDatetimeFunctions::testDATEDIF()
{
  // interval y  ( years )
  CHECK_EVAL( "DATEDIF(DATE(1990;2;15); DATE(1993;9;15); \"y\")", Value( 3 ) ); // TODO -  check value

  // interval m  ( Months. If there is not a complete month between the dates, 0 will be returned.)
  CHECK_EVAL( "DATEDIF(DATE(1990;2;15); DATE(1993;9;15); \"m\")", Value( 43 ) );

  // interval d  ( Days )
  CHECK_FAIL( "DATEDIF(DATE(1990;2;15); DATE(1993;9;15); \"d\")", Value( 0 ), "unknown value" ); // TODO

  // interval md ( Days, ignoring months and years )
  CHECK_EVAL( "DATEDIF(DATE(1990;2;15); DATE(1993;9;15); \"md\")", Value( 0 ) );

  // interval ym ( Months, ignoring years )
  CHECK_EVAL( "DATEDIF(DATE(1990;2;15); DATE(1993;9;15); \"ym\")", Value( 7 ) );

  // interval yd ( Days, ignoring years )
  CHECK_EVAL( "DATEDIF(DATE(1990;2;15); DATE(1993;9;15); \"yd\")", Value( 0 ) ); // TODO -  check value
}

void TestDatetimeFunctions::testWEEKNUM()
{
  // type default ( type 1 )
  CHECK_EVAL( "WEEKNUM(DATE(2000;05;21))", Value( 22 ) );
  CHECK_EVAL( "WEEKNUM(DATE(2005;01;01))", Value( 01 ) ); // TODO -  check value
#if 0 
  CHECK_EVAL( "WEEKNUM(DATE(2000;01;02))", Value( 22 ) ); // TODO -  check value
  CHECK_EVAL( "WEEKNUM(DATE(2000;01;03))", Value( 22 ) ); // TODO -  check value
  CHECK_EVAL( "WEEKNUM(DATE(2000;01;04))", Value( 22 ) ); // TODO -  check value
  CHECK_EVAL( "WEEKNUM(DATE(2006;01;01))", Value( 22 ) ); // TODO -  check value
#endif

  // type 1
  CHECK_EVAL( "WEEKNUM(DATE(2000;05;21);1)", Value( 22 ) );
  CHECK_EVAL( "WEEKNUM(DATE(2008;03;09);1)", Value( 11 ) );

  // type 2
  CHECK_EVAL( "WEEKNUM(DATE(2000;05;21);2)", Value( 21 ) );
#if 0
  CHECK_EVAL( "WEEKNUM(DATE(2005;01;01);2)", Value( 22 ) ); // TODO -  check value
  CHECK_EVAL( "WEEKNUM(DATE(2000;01;02);2)", Value( 22 ) ); // TODO -  check value
  CHECK_EVAL( "WEEKNUM(DATE(2000;01;03);2)", Value( 22 ) ); // TODO -  check value
  CHECK_EVAL( "WEEKNUM(DATE(2000;01;04);2)", Value( 22 ) ); // TODO -  check value
#endif
  CHECK_EVAL( "WEEKNUM(DATE(2008;03;09);2)", Value( 10 ) );
}

void TestDatetimeFunctions::testWORKDAY()
{
  // 2001 JAN 01 02 03 04 05 06 07 08
  //          MO TU WE TH FR SA SU MO
  //          01 02 -- --  
  CHECK_EVAL( "WORKDAY(DATE(2001;01;01);2;2)", Value( "Fri Jan 5 2001" ) ); // TODO change return value to 2001-01-05 in function
  CHECK_EVAL( "WORKDAY(DATE(2001;01;01);2;3)", Value( "Mon Jan 8 2001" ) );
}

void TestDatetimeFunctions::testNETWORKDAY()
{
  // 2001 JAN 01 02 03 04 05 06 07 08 09
  //          MO TU WE TH FR SA SU MO TU
  //             01 02 03 04 05 05 05 06 ... networkdays
  CHECK_EVAL( "NETWORKDAY(DATE(2001;01;01);DATE(2001;01;08))", Value( 5 ) );
  CHECK_EVAL( "NETWORKDAY(DATE(2001;01;01);DATE(2001;01;07))", Value( 5 ) );
  CHECK_EVAL( "NETWORKDAY(DATE(2001;01;01);DATE(2001;01;06))", Value( 5 ) );
  CHECK_EVAL( "NETWORKDAY(DATE(2001;01;01);DATE(2001;01;05))", Value( 4 ) );

  // 2008 FEB 25 26 27 28 29 01 02 03 04
  //          MO TU WE TH FR SA SU MO TU
  //             01 02 03 04 05 05 05 06 ... networkdays
  CHECK_EVAL( "NETWORKDAY(DATE(2008;02;25);DATE(2008;02;28))", Value( 3 ) );
  CHECK_EVAL( "NETWORKDAY(DATE(2008;02;25);DATE(2008;02;29))", Value( 4 ) );
  CHECK_EVAL( "NETWORKDAY(DATE(2008;02;25);DATE(2008;03;01))", Value( 5 ) );
  CHECK_EVAL( "NETWORKDAY(DATE(2008;02;25);DATE(2008;03;02))", Value( 5 ) );
  CHECK_EVAL( "NETWORKDAY(DATE(2008;02;25);DATE(2008;03;03))", Value( 5 ) );
  CHECK_EVAL( "NETWORKDAY(DATE(2008;02;25);DATE(2008;03;04))", Value( 6 ) );
}

#if 0
void TestDatetimeFunctions::testUNIX2DATE()
{
  // 01/01/2001 = 946681200
  CHECK_EVAL( "UNIX2DATE(DATE(2000;01;01))", Value( 946681200 ) ); // TODO
}

void TestDatetimeFunctions::testDATE2UNIX()
{
  //
  CHECK_EVAL( "DATE2UNIX(946681200)", Value( "01/01/2000" ) ); // TODO
}
#endif

void TestDatetimeFunctions::testDATE()
{
  //
  CHECK_EVAL( "DATE(2005;12;31)-DATE(1904;01;01)", Value( 37255 ) );
  CHECK_EVAL( "DATE(2004;02;29)=DATE(2004;02;28)+1", Value( TRUE ) );                           // leap year
  CHECK_EVAL( "DATE(2000;02;29)=DATE(2000;02;28)+1", Value( TRUE ) );                           // leap year
  CHECK_EVAL( "DATE(2005;03;01)=DATE(2005;02;28)+1", Value( TRUE ) );                           // no leap year
  CHECK_FAIL( "DATE(2017.5;05;29)=DATE(2017;01;02)", Value( TRUE ), "fraction not truncated" ); // fractional values for year are truncated
  CHECK_FAIL( "DATE(2006;02.5;03)=DATE(2006;02;03)", Value( TRUE ), "fraction not truncated" ); // fractional values for month are truncated
  CHECK_FAIL( "DATE(2006;01;03.5)=DATE(2006;01;03)", Value( TRUE ), "fraction not truncated" ); // fractional values for day are truncated
  CHECK_EVAL( "DATE(2006;13;03)=DATE(2007;01;03)", Value( TRUE ) );                             // months > 12 roll over to year
  CHECK_EVAL( "DATE(2006;01;32)=DATE(2006;02;01)", Value( TRUE ) );                             // days greater than month limit roll over to month
  CHECK_EVAL( "DATE(2006;25;34)=DATE(2008;02;03)", Value( TRUE ) );                             // days and months roll over transitively
  CHECK_EVAL( "DATE(2006;-01;01)=DATE(2005;11;01)", Value( TRUE ) );                            // negative months roll year backward
  CHECK_EVAL( "DATE(2006;04;-01)=DATE(2006;03;30)", Value( TRUE ) );                            // negative days roll month backward
  CHECK_EVAL( "DATE(2006;-04;-01)=DATE(2007;07;30)", Value( TRUE ) );                           // negative days and months roll backward transitively
  CHECK_EVAL( "DATE(2003;02;29)=DATE(2003;37;01)", Value( TRUE ) );                             // non-leap year rolls forward
}

void TestDatetimeFunctions::testDATEVALUE()
{
  //
  CHECK_EVAL( "DATEVALUE(\"2004-12-25\")=DATE(2004;12;25)", Value( TRUE ) );
}

void TestDatetimeFunctions::testDAY()
{
  //
  CHECK_EVAL( "DAY(DATE(2006;05;21))", Value( 21 ) );
  CHECK_EVAL( "DAY(\"2006-12-15\")", Value( 15 ) );
}

void TestDatetimeFunctions::testDAYS()
{
  //
  CHECK_EVAL( "DAYS(DATE(1993;4;16); DATE(1993;9;25))", Value( -162 ) ); // TODO - DAYS returns abs value
}

void TestDatetimeFunctions::testDAYS360()
{
  // TODO Note: Lotus 1-2-3v9.8 has a function named DAYS but with different semantics.  It supports an optional ��Basis�� parameter
  // with many different options.  Without the optional parameter, it defaults to a 30/360 basis, not calendar days; thus, in Lotus 1-2-3v9.8,
  // DAYS(DATE(1993;4;16);  DATE(1993;9;25)) computes -159, not -162. 

  //CHECK_EVAL( "DAYS360(DATE(1993;4;16);DATE(1993;9;25); FALSE)", Value( -162 ) ); 

  CHECK_EVAL( "DAYS360(\"2002-02-22\"; \"2002-04-21\"; FALSE)", Value( 59 ) ); // src docs
}

void TestDatetimeFunctions::testEDATE()
{
  //
  CHECK_EVAL( "EDATE(\"2006-01-01\";0)=DATE(2006;01;01)", Value( TRUE ) );      // If zero, unchanged.
  CHECK_EVAL( "EDATE(DATE(2006;01;01);0)=DATE(2006;01;01)", Value( TRUE ) );    // You can pass strings or serial numbers to EDATE
  CHECK_EVAL( "EDATE(\"2006-01-01\";2)=DATE(2006;03;01)", Value( TRUE ) );      //
  CHECK_EVAL( "EDATE(\"2006-01-01\";-2)=DATE(2005;11;01)", Value( TRUE ) );     // 2006 is not a leap year. Last day of March, going back to February
  //CHECK_EVAL( "EDATE(\"2000-04-30\";-2)=DATE(2006;2;29)", Value( TRUE ) );    // TODO 2000 was a leap year, so the end of February is the 29th
  CHECK_EVAL( "EDATE(\"2000-04-05\";24)=DATE(2002;04;12)", Value( TRUE ) );     // EDATE isn't limited to 12 months
}

void TestDatetimeFunctions::testEOMONTH()
{
  //
  CHECK_EVAL( "EOMONTH(\"2006-01-01\";0)=DATE(2006;01;31)", Value( TRUE ) );    // If zero, unchanged �V just returns end of that date's month. (January in this case)
  CHECK_EVAL( "EOMONTH(DATE(2006;01;01);0)=DATE(2006;01;31)", Value( TRUE ) );  // You can pass strings or serial numbers to EOMONTH
  CHECK_EVAL( "EOMONTH(\"2006-01-01\";2)=DATE(2006;03;31)", Value( TRUE ) );    // End of month of March is March 31.
  CHECK_EVAL( "EOMONTH(\"2006-01-01\";-2)=DATE(2005;11;30)", Value( TRUE ) );   // Nov. 30 is the last day of November
  CHECK_EVAL( "EOMONTH(\"2006-03-31\";-1)=DATE(2006;02;28)", Value( TRUE ) );   // 2006 is not a leap year. Last day of  February is Feb. 28.
  //CHECK_EVAL( "EOMONTH(\"2000-04-30\";-2)=DATE(2006;02;29)", Value( TRUE ) ); // TODO 2000 was a leap year, so the end of February is the 29th
  CHECK_EVAL( "EOMONTH(\"2000-04-05\";24)=DATE(2002;04;30)", Value( TRUE ) );   // Not limited to 12 months, and this tests April
  //CHECK_EVAL( "EOMONTH(\"2006-01-05\";4)=DATE(2002;05;31)", Value( TRUE ) );   // TODO
  //CHECK_EVAL( "EOMONTH(\"2006-01-05\";5)=DATE(2002;06;30)", Value( TRUE ) );   // TODO
  //CHECK_EVAL( "EOMONTH(\"2006-01-01\";0)=DATE(2006;1;31)", Value( TRUE ) );   // TODO
  //CHECK_EVAL( "EOMONTH(\"2006-01-01\";0)=DATE(2006;1;31)", Value( TRUE ) );   // TODO
  //CHECK_EVAL( "EOMONTH(\"2006-01-01\";0)=DATE(2006;1;31)", Value( TRUE ) );   // TODO
  //CHECK_EVAL( "EOMONTH(\"2006-01-01\";0)=DATE(2006;1;31)", Value( TRUE ) );   // TODO
  //CHECK_EVAL( "EOMONTH(\"2006-01-01\";0)=DATE(2006;1;31)", Value( TRUE ) );   // TODO
  //CHECK_EVAL( "EOMONTH(\"2006-01-01\";0)=DATE(2006;1;31)", Value( TRUE ) );   // TODO
}

void TestDatetimeFunctions::testHOUR()
{
  //
  CHECK_EVAL( "HOUR(5/24)", Value( 5 ) );              // 5/24ths of a day is 5 hours, aka 5AM.
  CHECK_EVAL( "HOUR(5/24-1/(24*60*60))", Value( 4 ) ); // A second before 5AM, it's 4AM.
  CHECK_EVAL( "HOUR(\"14:00\")", Value( 14 ) );        // TODO TimeParam accepts text
}

void TestDatetimeFunctions::testMINUTE()
{
  //
  CHECK_EVAL( "MINUTE(1/(24*60))", Value( 1 ) );         // 1 minute is 1/(24*60) of a day.
  CHECK_EVAL( "MINUTE(TODAY()+1/(24*60))", Value( 1 ) ); // If you start with today, and add a minute, you get a minute.
  CHECK_EVAL( "MINUTE(1/24)", Value( 0 ) );              // At the beginning of the hour, we have 0 minutes.
}

void TestDatetimeFunctions::testMONTH()
{
  //
  CHECK_EVAL( "MONTH(DATE(2006;5;21))", Value( 5 ) );    // Month extraction from DATE() value
}

void TestDatetimeFunctions::testNOW()
{
  //
  CHECK_EVAL( "NOW()>DATE(2006;1;3)", Value( TRUE ) );   // NOW constantly changes, but we know it's beyond this date.
  CHECK_EVAL( "INT(NOW())=TODAY()", Value( TRUE ) );     
}

void TestDatetimeFunctions::testSECOND()
{
  //
  CHECK_EVAL( "SECOND(1/(24*60*60))", Value( 1 ) );     // This is one second into today.
  CHECK_EVAL( "SECOND(1/(24*60*60*2))", Value( 1 ) );   // Rounds.
  CHECK_EVAL( "SECOND(1/(24*60*60*4))", Value( 0 ) );   // TODO Rounds.
}

void TestDatetimeFunctions::testTIME()
{
  //
  CHECK_EVAL( "TIME(0;0;0)", Value( 0 ) );                      // All zero arguments becomes midnight, 12:00:00 AM.
  CHECK_EVAL( "TIME(23;59;59)*60*60*24", Value( 86399 ) );      // This is 11:59:59 PM.
  CHECK_EVAL( "TIME(11;125;144)*60*60*24", Value( 47244 ) );    // Seconds and minutes roll over transitively; this is 1:07:24 PM.
  CHECK_EVAL( "TIME(11;0; -117)*60*60*24", Value( 39483 ) );    // Negative seconds roll minutes backwards, 10:58:03 AM
  CHECK_EVAL( "TIME(11;-117;0)*60*60*24", Value( 32580 ) );     // Negative minutes roll hours backwards, 9:03:00 AM
  CHECK_EVAL( "TIME(11;-125;-144)*60*60*24", Value( -31956 ) ); // TODO Negative seconds and minutes roll backwards transitively, 8:52:36 AM
}

void TestDatetimeFunctions::testTIMEVALUE()
{
  //
  //CHECK_EVAL( "TIMEVALUE(\"06:05\")=TIME(6;5;0)", Value( TRUE ) ); // TODO
  CHECK_EVAL( "TIMEVALUE(\"06:05:07\")=TIME(6;5;7)", Value( TRUE ) );
}

void TestDatetimeFunctions::testTODAY()
{
  //
  CHECK_EVAL( "TODAY()>DATE(2006;1;3)", Value( TRUE ) ); // Every date TODAY() changes, but we know it's beyond this date.
  CHECK_EVAL( "INT(TODAY())=TODAY()", Value( TRUE ) );   
}

void TestDatetimeFunctions::testWEEKDAY()
{
  //
  CHECK_EVAL( "WEEKDAY(DATE(2006;05;21))", Value( 1 ) );  // Year-month-date format

  // WARNING seems like methods are switched
  //CHECK_EVAL( "WEEKDAY(DATE(2005;01;01))", Value( 7 ) );   // TODO Saturday 
  //CHECK_EVAL( "WEEKDAY(DATE(2005;01;01);1)", Value( 7 ) ); // TODO Saturday
  //CHECK_EVAL( "WEEKDAY(DATE(2005;01;01);2)", Value( 7 ) ); // TODO Saturday
  //CHECK_EVAL( "WEEKDAY(DATE(2005;01;01);3)", Value( 7 ) ); // TODO Saturday
}

void TestDatetimeFunctions::testYEAR()
{
  //
  CHECK_EVAL( "YEAR(DATE(1904;1;1))", Value( 1904 ) );
}

QTEST_KDEMAIN(TestDatetimeFunctions, GUI)

#include "TestDatetimeFunctions.moc"
