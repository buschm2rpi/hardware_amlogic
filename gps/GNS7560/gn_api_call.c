
//****************************************************************************
// GPS IP Centre, ST-Ericsson (UK) Ltd.
// Copyright (c) 2009 ST-Ericsson (UK) Ltd.
// 15-16 Cottesbrooke Park, Heartlands Business Park, Daventry, NN11 8YL, UK.
// All rights reserved.
//
// Filename  GN_GPS_api_calls.c
//
// $Header: S:/GN_GPS_Nav_MKS/Arch/GNU/_GN_GPS/rcs/GN_GPS_api_calls.c 1.10 2009/04/23 15:23:46Z geraintf Exp $
// $Locker: $
//****************************************************************************


//****************************************************************************
//
//  This is example code to illustrate how the GN GPS High-Level software
//  can be integrated into the host platform. Note that, although this
//  constitutes a fully-working GPS receiver, it is simplified relative to
//  what would be expected in a real product. The emphasis is in trying to
//  provide clarity of understanding of what is going on in the software, and
//  how the various API function calls relate to each other, rather than in
//  providing a full optimised software implementation.
//
//  The functions in this file are those called by the GN_GPS_Library on the
//  host software to provide platform specific functionality that cannot be
//  achieved in a cross-platform library.
//
//  All these functions MUST be implemented in the host software to suit the
//  target platform and OS, even if only as a "{ return( 0 ); }" stub.
//
//****************************************************************************
#if HAVE_ANDROID_OS
//#include <linux/ioctl.h>
//#include <linux/rtc.h>
#include <utils/Atomic.h>
#include <linux/android_alarm.h>
#endif

#include "gps_api.h"

#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/times.h>
#include <sys/types.h>
#include <fcntl.h>

//#include <time.h>
#include <linux/rtc.h>
#include <sys/ioctl.h>
#include <errno.h>

#include <stdlib.h>

#include "GN_GPS_Example.h"

typedef struct RTC_Calib
{
   U4 CTime_Set;           // 'C' Time when the RTC was last Set / Calibrated
   I4 Offset_s;            // (UTC - RTC) calibration offset [s]
   I4 Offset_ms;           // (UTC - RTC) calibration offset [ms]
   U4 Acc_Est_Set;         // Time Accuracy Estimate when RTC was Set / Calibrated [ms]
   U4 checksum;            // RTC Calibration File 32-bit checksum
} s_RTC_Calib;

static s_RTC_Calib gn_RTC_Calib;          // RTC Calibration Data
static const U4 Diff_GPS_C_Time = ((365 * 10 + 2 + 5) * (24 * 60 *60));

//****************************************************************************

// Define the Non-Volatile data and RTC calibration file names
#define NONVOL_FNAME   "/data/gps_nonvol.txt"  // Non-volatile data file name
#define RTC_CALIB_FILE "/data/gps_rtc_cal.txt" // RTC calibration file name

// Define the difference (in seconds) between the GPS time reference and
// the C time reference.
#define      RTC_DRIFT       10  // RTC drift uncertainty = 10 ppm


//extern GpsState  g_gps_state[1];
//extern U4 gn_CLK_TCK ;
//extern int g_utc_diff;


static const U2 Days_to_Month[12] =
    { 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334 };

// Number of Days in a Month look up table.
static const U1 Days_in_Month[] =
{
    31,   // Jan
    28,   // Feb
    31,   // Mar
    30,   // Apr
    31,   // May
    30,   // Jun
    31,   // Jul
    31,   // Aug
    30,   // Sep
    31,   // Oct
    30,   // Nov
    31    // Dec
};



//*****************************************************************************
// Convert Time in YY-MM-DD & HH:MM:SS format to 'C' Time units [seconds].
// ('C' Time starts on the 1st Jan 1970.)

U4 GN_YMDHMS_To_CTime(
   U2 Year,                   // i  - Year         [eg 2006]
   U2 Month,                  // i  - Month        [range 1..12]
   U2 Day,                    // i  - Days         [range 1..31]
   U2 Hours,                  // i  - Hours        [range 0..23]
   U2 Minutes,                // i  - Minutes      [range 0..59]
   U2 Seconds )               // i  - Seconds      [range 0..59]
 {
   // Return Value Definition
   U4 CTime;                  // Time in 'C' Time units [seconds]
   // Local Definitions
   I4 GPS_secs;               // GPS Seconds since 5/6 Jan 1980.
   I4 dayOfYear;              // Completed days into current year
   I4 noYears;                // Completed years since 1900
   I4 noLeapYears;            // Number of leap years between 1900 and present
   I4 noDays;                 // Number of days since midnight, Dec 31/Jan 1, 1900.
   I4 noGPSdays;              // Number of days since start of GPS time

   // Compute the day number into the year
   dayOfYear = (I4)( Days_to_Month[Month-1] + Day );

   // Leap year check.  (Note that this algorithm fails at 2100!)
   if ( Month > 2  &&  (Year%4) == 0 )  dayOfYear++;

   // The number of days between midnight, Dec31/Jan 1, 1900 and
   // midnight, Jan5/6, 1980 (ie start of GPS) is 28860.
   noYears     = (I4)Year - 1901;
   noLeapYears = noYears / 4;
   noDays      = (noYears*365) + noLeapYears + dayOfYear;
   noGPSdays   = noDays - 28860;                    // Number of GPS days
   GPS_secs    = noGPSdays*86400 + Hours*3600 + Minutes*60  + Seconds;

   // Convert from GPS Time to C Time
   CTime = GPS_secs + Diff_GPS_C_Time;

   return( CTime );
}


//*****************************************************************************
// Convert Time in 'C' Time units [seconds] to a YY-MM-DD & HH:MM:SS format.
// ('C' Time starts on the 1st Jan 1970.)

void GN_CTime_To_YMDHMS(
   U4 C_Time,                 // i  - Time in 'C' Time units [seconds]
   U2 *Year,                  //  o - Year         [eg 2006]
   U2 *Month,                 //  o - Month        [range 1..12]
   U2 *Day,                   //  o - Days         [range 1..31]
   U2 *Hours,                 //  o - Hours        [range 0..23]
   U2 *Minutes,               //  o - Minutes      [range 0..59]
   U2 *Seconds )              //  o - Seconds      [range 0..59]
 {
   I4 GPS_secs;               // GPS Seconds since 5/6 Jan 1980.
   I2 gpsWeekNo;              // GPS Week Number
   I4 gpsTOW;                 // GPS Time of Week [seconds]
   I4 loSecOfD;               // Local Second of Day    (range 0-86399)
   I4 loYear;                 // Local Year             (range 1980-2100)
   I4 loDayOfW;               // Local Day of Week      (range 1-7)
   I4 loDayOfY;               // Local Day of Year      (range 1-366)
   I4 loSecOfH;               // Local Second of Hour   (range 0-3599)
   I4 i;                      // Loop index
   I4 tempI4;                 // Temporary I4 value

   // Convert from 'C' Time to GPS Time.
   GPS_secs = C_Time - Diff_GPS_C_Time;

   // Convert UTC Time of Week to Day of Week, Hours, Minutes and Seconds.
   gpsWeekNo = (I2)( GPS_secs /604800 );
   gpsTOW    = GPS_secs - 604800*gpsWeekNo;

   loDayOfW  = gpsTOW / 86400;            // Calculate completed Days into Week
   loSecOfD  = gpsTOW - 86400*loDayOfW;   // Calculate current Second of Day
   tempI4    = loSecOfD / 3600;           // Calculate current Hour of Day
   *Hours    = (U2)tempI4;                // Store current Hour of Day
   loSecOfH  = loSecOfD - 3600*tempI4;    // Calculate current Second of Hour
   tempI4    = loSecOfH / 60;             // Calculate current Minute of Hour
   *Minutes  = (U2)tempI4;                // Store current Minute of Hours
   *Seconds  = (U2)(loSecOfH - 60*tempI4);// Calc & Store current Minute of Second

   // Convert day of week and week number to day of year (tempI4) and year
   tempI4  = loDayOfW + (I4)gpsWeekNo*7;  // Calculate GPS day number
   tempI4  = tempI4 + 6;                  // Offset for start of GPS time 6/1/1980
   loYear  = 1980;

   // Advance completed 4 years periods,  which includes one leap year.
   // (Note that this algorithm fails at 2100, which is not a leap year.)
   while ( tempI4 > ((365*4) + 1) )
   {
      tempI4 = tempI4 - ((365*4) + 1);
      loYear = loYear + 4;
   };
   // Advance remaining completed years, which don't include any leap years.
   // (Note that this algorithm fails at 2100, which is not a leap year.)
   while ( tempI4 > 366 )
   {
      tempI4 = tempI4 - 365;
      if ( (loYear & 0x3) == 0 ) tempI4--;   // TRUE for leap years (fails at 2100)
      loYear++;
   };
   // Check for one too many days in a non-leap year.
   // (Note that this algorithm fails at 2100, which is not a leap year.)
   if ( tempI4 == 366  &&  (loYear & 0x3) != 0 )
   {
      loYear++;
      tempI4 = 1;
   }

   loDayOfY = tempI4;
   *Year    = (U2)loYear;

   // Convert Day of Year to Day of Month and Month of Year
   for ( i=0 ; i<12 ; i++ )
   {
      if ( loDayOfY <= Days_in_Month[i] )
      {
         *Day   = (U2)loDayOfY;
         *Month = (U2)i+1;
         break;
      }
       else
      {
         loDayOfY = loDayOfY - Days_in_Month[i];

         // Check for the extra day in February in Leap years
         if ( i == 1  &&  (loYear & 0x3) == 0 )   // Only Works up to 2100
         {
            if ( loDayOfY > (29-28) )  // After Feb 29th in a Leap year
            {
               loDayOfY--;                // Take off the 29th Feb
            }
            else                       // Must be the 29th of Feb on a Leap year
            {
               *Day     = (U2)29;
               *Month   = (U2)2;
               break;
            }
         }
      }
   }

   return;
}


U2 GN_GPS_Write_GNB_Patch(
   U2 ROM_version,                  // i - Current GN Baseband ROM version
   U2 Patch_CkSum )                 // i - Current GN Baseband Reported Patch
{
    char  temp_buffer[80];
    int   nWritten;
    U4    mstime;
    U2    ret_val = 0;

    RUNTO;
   // Record some details in the event log - for diagnostics purposes only

    mstime = GN_GPS_Get_OS_Time_ms();
    nWritten = sprintf(temp_buffer,
                       "\n\rGN_GPS_Write_GNB_Patch, V %3d, OS ms time %7d\n\r",
                       ROM_version,
                       mstime );

    GN_GPS_Write_Event_Log((U2)nWritten, (CH*)temp_buffer);
   // Setup the patch code upload process
   ret_val = GN_Setup_GNB_Patch( ROM_version, Patch_CkSum );

   return( ret_val );
}


//*****************************************************************************
// GN GPS Callback Function to Read back the GPS Non-Volatile Store Data from
// the Host's chosen Non-Volatile Store media.
// Returns the number of bytes actually read back.  If this is not equal to
// 'NV_size' then it is assumed that the Non-Volatile Store Data read back is
// invalid and Time-To_First_fix will be significantly degraded.

U2 GN_GPS_Read_NV_Store(
   U2 NV_size,                      // i - Size of the NV Store Data [bytes]
   U1 *p_NV_Store )                 // i - Pointer to the NV Store Data
{
   // In this example implementation the Non-Volatile Store data is stored in a
   // file on the local disk.
   // Note that, in a real product, it would probably not be desirable to
   // read data from a file at this point, because it would be relatively slow.
   // A better approach might be to read the file before the GPS was started,
   // and to store the data in memory.

   size_t num_read;                       // Number of bytes read

   // Firstly, this is a good time to recover the Real-Time Clock Calibration
   // Data from file to the global structure where it will be read from in
   // function GN_GPS_Read_UTC_Data().
   // Open the Real-Time Clock data file for reading, in binary mode.
   // Note that, if the file does not exist, it must not be created.
   {
      FILE *fp_RTC_Calib;           // RTC Calibration File Pointer

      fp_RTC_Calib = fopen( RTC_CALIB_FILE, "rb" );

      if ( fp_RTC_Calib == NULL )
      {
      int  n;
      char str[128];
      n = sprintf(str,"GN_GPS_Read_NV_Store: WARNING: Could not open file %s, errno %s.",
                 RTC_CALIB_FILE, strerror(errno));
      GN_GPS_Write_Event_Log( (U2)n, (CH*)str );

         memset( &gn_RTC_Calib, 0, sizeof(s_RTC_Calib) );   // Clear the RTC Calib data
         return( 0 );
      }

      // Read the RTC Calibration data
      num_read = fread( &gn_RTC_Calib, 1, sizeof(s_RTC_Calib), fp_RTC_Calib );

      // Close the RTC Calibration data file
      fclose( fp_RTC_Calib );

      // Check that the correct number of bytes were read
      if ( num_read != sizeof(s_RTC_Calib) )
      {
      int  n;
      char str[128];
      n = sprintf(str,"GN_GPS_Read_NV_Store: WARNING: Only read %d bytes, expected %d bytes in %s.",
                      num_read, sizeof(s_RTC_Calib), RTC_CALIB_FILE);
      GN_GPS_Write_Event_Log( (U2)n, (CH*)str );

         memset( &gn_RTC_Calib, 0, sizeof(s_RTC_Calib) );   // Clear the RTC Calib data
         num_read = 0;
      }
      else
      {
         LOGGPS( "GN_GPS_Read_NV_Store: RTC Calibration data read OK." ) ;
      }
   }

   // Open the Non-Volatile Store data file for reading, in binary mode.
   // Note that, if the file does not exist, it must not be created.
   {
      FILE *fp_NVol;                // GPS Non-Volatile data File pointer

      fp_NVol = fopen( NONVOL_FNAME, "rb" );

      if ( fp_NVol == NULL )
      {
        int  n;
       char str[128];
       n = sprintf(str,"GN_GPS_Read_NV_Store: WARNING: Could not open file %s, errno %s.",
                NONVOL_FNAME, strerror(errno));
       GN_GPS_Write_Event_Log( (U2)n, (CH*)str );

         memset( p_NV_Store, 0, NV_size );            // Clear NV Store data
         return( 0 );
      }

      // Read the NV Store data
      num_read = fread( p_NV_Store, 1, NV_size, fp_NVol );

      // Close the NV Store file
      fclose( fp_NVol );

      // Check that the correct number of bytes were read
      if ( num_read != NV_size )
      {
         LOGGPS( "GN_GPS_Read_NV_Store: WARNING: Only read %d bytes, expected %d bytes in %s",
                 num_read, NV_size, NONVOL_FNAME );
         memset( p_NV_Store, 0, NV_size );            // Clear NV Store data
         num_read = 0;
      }
      else
      {
         LOGGPS( "GN_GPS_Read_NV_Store: NV Store data read OK, %d.", num_read ) ;
      }
   }

   // Return the size of NV Store data read back.
   return( (U2)num_read );
}


//*****************************************************************************
// GN GPS Callback Function to Write the GPS Non-Volatile Store Data to the
// Host's chosen Non-Volatile Store media.
// Returns the number of bytes actually written.

void GN_GPS_Write_NV_Store(
   U2 NV_size,                      // i - Size of the NV Store Data [bytes]
   U1 *p_NV_Store )                 // i - Pointer to the NV Store Data
{
   // In this example implementation, the non-volatile data is stored in a
   // file on the local disk.
   // Note that, in a real product, it would probably not be desirable to
   // write data to a file at this point, because it would be relatively slow.
   // A better approach might be to write the data to memory, and to output
   // this to the file after the GPS has been stopped.

   size_t  num_write;         // Number of bytes written

   // Open the non-volatile data file for writing, in binary mode.
   // If the file already exists it will be over-written, otherwise
   // it will be created.
   {
      FILE*   fp_NVol;          // GPS Non-Volatile data File pointer

      fp_NVol = fopen( NONVOL_FNAME, "wb" );
      if ( fp_NVol == NULL )
      {
         LOGGPS( "GN_GPS_Write_NV_Store: WARNING: Could not open file %s, errno %s.",
                 NONVOL_FNAME,strerror( errno) );
         return;
      }

      // Write the NV Store data to the NV Store file
      num_write = fwrite( p_NV_Store, 1, (size_t)NV_size, fp_NVol );

      // Flush & Close the NV Store file
      fflush( fp_NVol );
      fclose( fp_NVol );

      // Check that the correct number of bytes were written
      if ( num_write != (size_t)NV_size )
      {
         LOGGPS( "GN_GPS_Write_NV_Store: WARNING: Only wrote %d of %d bytes!",
                num_write, NV_size );
      }
      else
      {
         LOGGPS( "GN_GPS_Write_NV_Store: Data written OK, %d.", num_write );
      }
   }

   // Finally, save this is a good time to save the RTC Calibration Data from
   // the structure it was written to in GN_GPS_Write_UTC_Data() to file.
   // Open the non-volatile data file for writing, in binary mode.
   // If the file already exists it will be over-written, otherwise
   // it will be created.
   {
      FILE *fp_RTC_Calib;           // RTC Calibration File Pointer

      fp_RTC_Calib = fopen( RTC_CALIB_FILE, "wb" );
      if ( fp_RTC_Calib == NULL )
      {
         return;
      }

      // Write the RTC Calibration Data to the RTC Calibration file
      num_write = fwrite( &gn_RTC_Calib, 1, sizeof(s_RTC_Calib), fp_RTC_Calib );

      // Flush & Close the RTC Calibration file
      fflush( fp_RTC_Calib );
      fclose( fp_RTC_Calib );

      // Check that the correct number of bytes were written
      if ( num_write != sizeof(s_RTC_Calib) )
      {
         LOGGPS( "GN_GPS_Write_NV_Store: WARNING: Only wrote %d of %d bytes in %s",
                num_write, sizeof(s_RTC_Calib), RTC_CALIB_FILE );
      }
      else
      {
        LOGGPS( "GN_GPS_Write_NV_Store: RTC Calibration Data written OK." );
      }
   }

   return;
}


//*****************************************************************************

// GN GPS Callback Function to Get the current OS Time tick in integer
// millisecond units.  The returned 'OS_Time_ms' must be capable of going all
// the way up to and correctly wrapping a 32-bit unsigned integer boundary.

U4 GN_GPS_Get_OS_Time_ms( void )
{
   struct tms  process_times;
   clock_t     pr_time;

   // Get the current process time.
   // Note that times() is used because clock() does not appear to work properly.
   pr_time = times( &process_times );
   //printf("GN_GPS_Get_OS_Time_ms  pr_time=%d\n",pr_time);

   // Otherwise, if the process time count 100 times per second, ie 10ms for 1 tick.
   if ( gn_CLK_TCK == 100 )
   {
      static BL tick_count_set = FALSE;
      static U4 last_clk       = 0;
      static U4 OS_Time_ms     = 0;

      if ( ! tick_count_set )
      {
         last_clk       = (U4)pr_time;
         OS_Time_ms     = last_clk * 10;
         tick_count_set = TRUE;
      }
      else
      {
         U4 this_clk = (U4)pr_time;
         if ( this_clk >= last_clk )
         {
            OS_Time_ms = OS_Time_ms  +  ( this_clk - last_clk ) * 10;
         }
         else
         {
            OS_Time_ms = OS_Time_ms  + ( (0xFFFFFFFF - last_clk) + this_clk + 1 ) * 10;
         }
         last_clk = this_clk;
      }
      //printf("GN_GPS_Get_OS_Time_ms  OS_Time_ms = %d\n",OS_Time_ms);
      return( OS_Time_ms );
   }
   // Otherwise, the process time counts in as yet unsupported units, so return 0.
   else
   {
      return( 0 );
   }
}


//*****************************************************************************
// GN GPS Callback Function to Get the OS Time tick, in integer millisecond
// units, corresponding to when a burst of Measurement data received from
// GPS Baseband started.  This helps in determining the system latency.
// If it is not possible to determine this value then, return( 0 );
// The returned 'OS_Time_ms' must be capable of going all the way up to and
// correctly wrapping a 32-bit unsigned integer boundary.

U4 GN_GPS_Get_Meas_OS_Time_ms( void )
{
   // This is not implemented.
   return( 0 );
}


//*****************************************************************************
// GN GPS Callback Function to Read back the current UTC Date & Time data
// (eg from the Host's Real-Time Clock).
// Returns TRUE if successful.

BL GN_GPS_Read_UTC(
   s_GN_GPS_UTC_Data *p_UTC )          // i - Pointer to the UTC Date & Time
{
   // In this PC Example implementation, RTC is calibrated by the GPS
   // That is, the offset between the GPS time and the PC's RTC time is
   // computed and stored in a structure (checksumed) which is then in-turn
   // stored in a file on the local disk when we save the NV_Store data.
   // When the GPS Library makes this function call (i.e. calls GN_GPS_Read_UTC),
   // we check the content of the calibration data against its expected checksum
   // and if it passes we apply the calibration.
   // If we find anything wrong with the calibration data we assume that
   // the RTC Time cannot be trusted and we return( FALSE ) to say that there
   // is no starting UTC Time available.
   //
   // Note that, if the error in the time returned by this function is
   // significantly greater than is indicated by the associated
   // accuracy estimate (i.e. 'Acc_Est' in the s_GN_GPS_UTC_Data structure)
   // then the performance of the GPS might be degraded..

    RUNTO;
   struct timeval tv_RTC;              // Local OS Real-Time-Clock Time in timeval format
   struct tm      tm_RTC;              // Local OS Real-Time-Clock Time in tm format
   U4             Curr_CTime;          // Current Time in 'C' Time units [seconds]
   I4             dT_s;                // A Time difference [seconds]
   I4             milliseconds;        // The milliseconds part as a signed value
   I4             acc_est_deg;         // Time Accuracy Estimate degredation [ms]
   I4             acc_est_now;         // Time Accuracy Estimate now [ms]
   I4             age;                 // Age since RTC Last Set/Calibrated
   U4             checksum;            // RTC Calibration File 32-bit checksum

   // Clear the return UTC time to a NULL state with the Estimated Accuracy
   // set very high (ie a NOT SET state).
   memset( p_UTC, 0, sizeof(s_GN_GPS_UTC_Data) );
   p_UTC->Acc_Est  =  0x7FFFFF00;

   // Get the current OS time [ms]
   p_UTC->OS_Time_ms = GN_GPS_Get_OS_Time_ms();

    LOGGPS( "GN_GPS_Read_UTC: gettimeofday() beginning" );
   // Get the current RTC Time via the OS System Time.
   // (This RTC time must always be UTC (ie no time-zone adjustments etc).
   // Read the actual host platform current SystemTime.
   if ( gettimeofday( &tv_RTC, NULL ) < 0 )
   {
      LOGGPS( "GN_GPS_Read_UTC: gettimeofday() failed" );
      return( FALSE );
   }

   LOGGPS( "GN_GPS_Read_UTC: gmtime_r() beginning" );
   if ( gmtime_r( &tv_RTC.tv_sec, &tm_RTC ) == NULL )
   {
      LOGGPS( "GN_GPS_Read_UTC: gmtime_r() failed." );
      return( FALSE );
   }

   LOGGPS( "GN_GPS_Read_UTC: gmtime_r:%d-%d-%d, %d:%02d:%02d", tm_RTC.tm_year, tm_RTC.tm_mon
                ,tm_RTC.tm_mday,tm_RTC.tm_hour,tm_RTC.tm_min,tm_RTC.tm_sec);

   p_UTC->Year         = (U2)tm_RTC.tm_year + 1900;
   p_UTC->Month        = (U2)tm_RTC.tm_mon + 1;
   p_UTC->Day          = (U2)tm_RTC.tm_mday;
   p_UTC->Hours        = (U2)tm_RTC.tm_hour;
   p_UTC->Minutes      = (U2)tm_RTC.tm_min;
   p_UTC->Seconds      = (U2)tm_RTC.tm_sec;
   p_UTC->Milliseconds = (U2)tv_RTC.tv_usec / 1000;

   // Most embedded RTC's do not have a millisecond resolution, therefore,
   // Set the Milliseconds to 500 so that the error is -499 to +499 ms.
// p_UTC->Milliseconds = 500;

   // Sensibility check the RTC Time, and QUIT now if it contains a year older
   // than 2001 (we must allow a few years margin because of old GPS Simulations).
   if ( p_UTC->Year < 2001 )
   {
      // Real-Time Clock Time invlaid,  quit now without a UTC time.
      LOGGPS( "GN_GPS_Read_UTC: Date before 2001 (%d-%d-%d), Assuming its invalid.",
              p_UTC->Day, p_UTC->Month, p_UTC->Year );
      return( FALSE );
   }

   // If there is no calibration data available, quit now without a valid starting UTC Time.
   if ( gn_RTC_Calib.CTime_Set == 0 )
   {
      LOGGPS( "GN_GPS_Read_UTC: No calibration data available" );
      return( FALSE );
   }

   // Convert to 'C' Time units [seconds]
   Curr_CTime = GN_YMDHMS_To_CTime( p_UTC->Year,
                                    p_UTC->Month,
                                    p_UTC->Day,
                                    p_UTC->Hours,
                                    p_UTC->Minutes,
                                    p_UTC->Seconds );


   // Check the RTC Calibrtation Data checksum to see if it the data is OK.
   checksum = (U4)( (U4)0x55555555              +
                    (U4)gn_RTC_Calib.CTime_Set  +
                    (U4)gn_RTC_Calib.Offset_s   +
                    (U4)gn_RTC_Calib.Offset_ms  +
                    (U4)gn_RTC_Calib.Acc_Est_Set );

   // If the checksum is NOT correct then Time cannot be trusted, so QUIT now
   // without a valid starting UTC Time.
   if ( checksum != gn_RTC_Calib.checksum  )
   {
      LOGGPS( "GN_GPS_Read_UTC: WARNING: RTC Calibration Checksum fail, 0x%08X != 0x%08X.",
              checksum, gn_RTC_Calib.checksum );
      return( FALSE );
   }

   // Apply the integer second part of the (UTC - RTC) calibration offset here.
   Curr_CTime = Curr_CTime  -  gn_RTC_Calib.Offset_s;                // {sec]

   // Apply the millisecond part of the calibration offset into the Milliseconds.
   // Use the signed local variable "milliseconds" because it can go negative.
   // If it does go negative, make it postive by moveing in 1 second from the integer part.
   milliseconds = (I4)p_UTC->Milliseconds  -  gn_RTC_Calib.Offset_ms;
   while ( milliseconds < 0 )
   {
      milliseconds = milliseconds + 1000;       // add 1 second to make positive
      Curr_CTime   = Curr_CTime   - 1;          // subtract 1 second to balance
   }
   p_UTC->Milliseconds  = (U2)milliseconds;

   // If the Milliseconds have gone over a second boundary, then move the integer
   // seconds part of it into the Current 'C' Time.
   if ( p_UTC->Milliseconds >= 1000 )
   {
      dT_s                 = p_UTC->Milliseconds / 1000;             // [sec]
      p_UTC->Milliseconds  = p_UTC->Milliseconds  - dT_s*1000;       // [ms]
      Curr_CTime           = Curr_CTime + dT_s;                      // {sec]
   }

   // Compute the age since the RTC was last calibrated.
   age = (I4)( Curr_CTime - gn_RTC_Calib.CTime_Set );             // seconds

   // If the age is slightly negative (eg -10 sec) then set it to zero in case
   // it was caused by rounding etc.
   if ( age < 0  &&  age > -10 )   age = 0;

   LOGGPS("GN_GPS_Read_UTC: age = %d", age);

   // If the age is still negative,  ie Time has gone backwards!
   // This probably means that the RTC has been reset, and therefore can't be
   // trusted, so QUIT now without a valid starting UTC Time.
   // This can also occur when working with GPS Simulators!
   if ( age < 0 )
   {
      LOGGPS( "GN_GPS_Read_UTC: ERROR: RTC time has gone backwards!, age = %d = (%d -%d)",
              age, Curr_CTime, gn_RTC_Calib.CTime_Set );
      return( FALSE );
   }

   // Compute the expected degradation in the accuracy of this RTC time,
   // allowing for degredations due to.
   // a) 170ms RMS for the +/- 0.5 sec error in reading the RTC.
   // b) 10 ppm RMS for the RTC crystal stability over the age DT.
   // Note that 'age' is in seconds, but RTC_acc and acc are ms.
   acc_est_deg =  150  +  age / (1000000/(10*1000));

   // Compute the Estimated time Accuracy now [RMS ms] , using the Gaussian
   // Propogation of Error Law to combine the estimated accuracy when set
   // and the estimated degredation since then.
   {
      R4 tempR4 =  (R4)gn_RTC_Calib.Acc_Est_Set * (R4)gn_RTC_Calib.Acc_Est_Set
                   +  (R4)acc_est_deg * (R4)acc_est_deg;
      acc_est_now  =  (U4)sqrt( (R8)tempR4 );
   }
   p_UTC->Acc_Est  =  acc_est_now;

   // Limit the accuracy to 300 ms RMS (i.e. a worst-case error of about 1 sec).
   // We do this because the resolution of a Typical OS appears to be 1 seconds.
   if ( p_UTC->Acc_Est < 300 )  p_UTC->Acc_Est = 300;                // [ms]

   // Convert the Current 'C' Time back to a YY-MM-DD hh:mm:ss date & time format.
   GN_CTime_To_YMDHMS( Curr_CTime,
                       &p_UTC->Year,  &p_UTC->Month,   &p_UTC->Day,
                       &p_UTC->Hours, &p_UTC->Minutes, &p_UTC->Seconds );

      LOGGPS( "GN_GPS_Read_UTC: %d  %d-%d-%d  %d:%02d:%02d.%03d  RMS=%d  (%d  %d  %d  %d)  %d  %d  %d",
               p_UTC->OS_Time_ms, p_UTC->Day, p_UTC->Month, p_UTC->Year,
               p_UTC->Hours, p_UTC->Minutes, p_UTC->Seconds, p_UTC->Milliseconds,
               p_UTC->Acc_Est,  gn_RTC_Calib.CTime_Set, gn_RTC_Calib.Offset_s,
               gn_RTC_Calib.Offset_ms, gn_RTC_Calib.Acc_Est_Set,
               age, acc_est_deg, acc_est_now );

   return( TRUE );
}


//*****************************************************************************
// GN GPS Callback Function to Write UTC Date & Time data to the Host platform
// software area,  which can be used by the Host to update its Real-Time Clock.

void GN_GPS_Write_UTC(
   s_GN_GPS_UTC_Data *p_UTC )       // i - Pointer to the UTC Date & Time
{
   static U4 counter = 0;

   // Compute the difference between the time from the PC's RTC and that
   // provided by the GPS.  This difference will be saved as a Calibration
   // Offset to a structure (checksumed), which is then written to a data file
   // on the local disk when we next save the NV Store data.
   // On the next start-up this calibration data will be read back and applied
   // to the Time read back from the RTC.
   //
   // When the GPS is generating position fixes, this function will be called
   // once per update.
   //
   // It may also be necessary to actually set the RTC if the Calibration Offset
   // is too big in order to prevent other applications on the Host system
   // from doing so and hence causing our calibration offset to be wrong.
   // We do not want to do this in PC Example because an incorrect time generated
   // when using a GPS Simulator will clash with the local time calibrated by a
   // Network Time Server, cause source file time-tags to be set in-correctly,
   // thus upsetting make systems, etc, etc

   struct timeval tv_RTC;              // Local OS Real-Time-Clock Time in timeval format
   struct tm      tm_RTC;              // Local OS Real-Time-Clock Time in tm format
   U4             RTC_CTime;           // Real-Time Clock Time, in 'C' Time units [seconds]
   U4             RTC_OS_Time_ms;      // OS Time [ms] corresponding to the RTC Time
   I4             dT_s;                // A Time difference [seconds]
   U4             New_CTime;           // New UTC Time, in 'C' Time units [seconds]

#if HAVE_ANDROID_OS
    struct timespec ts;
    int fd;
    int res;
#endif

   // Check for the case of being given time of all zero's with a very large
   // Accuracy Estimate and treat this as a request to Clear Time, which is
   // achieved simply by deleting the UTC data file.
   if ( p_UTC->Acc_Est > 0x7FFF0000  &&
        p_UTC->Year  == 0  &&  p_UTC->Month   == 0  &&  p_UTC->Day     == 0  &&
        p_UTC->Hours == 0  &&  p_UTC->Minutes == 0  &&  p_UTC->Seconds == 0    )
   {
      LOGGPS( "GN_GPS_Write_UTC: Request to delete UTC Time" );
      memset( &gn_RTC_Calib, 0, sizeof(gn_RTC_Calib) );
      remove( RTC_CALIB_FILE );
      return;
   }
    RUNTO;
   // If the new time is worse than the currently saved then consider ignoring it.
   // The worse it is the longer it is ignored in the hope something better comes along.

   if ( p_UTC->Acc_Est >= gn_RTC_Calib.Acc_Est_Set )
   {
      counter++;
      if ( p_UTC->Acc_Est > 1000  &&  counter < ( 20 * 60 ) )  return;
      if ( p_UTC->Acc_Est > 300   &&  counter < ( 10 * 60 ) )  return;
      if ( p_UTC->Acc_Est > 100   &&  counter < (  5 * 60 ) )  return;
      if ( p_UTC->Acc_Est > 30    &&  counter < (  3 * 60 ) )  return;
      if ( p_UTC->Acc_Est > 10    &&  counter < (  2 * 60 ) )  return;
      if (                            counter < (  1 * 60 ) )  return;
   }
   counter = 0;

   // Only Calibrate the RTC if we have been given time to better than
   // 300ms RMS, ie < 1 sec MAX,  ie it is good enough to be reliable.
   if ( p_UTC->Acc_Est < 300 )
   {
      // Convert the New UTC Time to 'C' Time units [seconds]
      New_CTime = GN_YMDHMS_To_CTime( p_UTC->Year,
                                      p_UTC->Month,
                                      p_UTC->Day,
                                      p_UTC->Hours,
                                      p_UTC->Minutes,
                                      p_UTC->Seconds );



         // Get the current RTC Time via the Win OS SYSTEMTIME time,
         // (This RTC time must always be UTC (ie no time-zone adjustments etc).
         // Also get the equivalent OS Time.
         // GetSystemTime( &RTC );
         RTC_OS_Time_ms = GN_GPS_Get_OS_Time_ms();
         //gettimeofday( &tv_RTC, NULL );
         if ( gettimeofday( &tv_RTC, NULL ) < 0 )
         {
            LOGGPS( "GN_GPS_Write_UTC:  gettimeofday() failed." );
            return;
         }
         if ( gmtime_r( &tv_RTC.tv_sec, &tm_RTC ) == NULL )
         {
            LOGGPS( "GN_GPS_Write_UTC:  gmtime_r() failed." );
            return;
         }

        LOGGPS("GN_GPS_Write_UTC: gmtime_r():%d-%d-%d, %d:%02d:%02d", tm_RTC.tm_year, tm_RTC.tm_mon
                ,tm_RTC.tm_mday,tm_RTC.tm_hour,tm_RTC.tm_min,tm_RTC.tm_sec);

         // Convert the current RTC Time to 'C' Time units.
         RTC_CTime = GN_YMDHMS_To_CTime( (U2)tm_RTC.tm_year + 1900,
                                         (U2)tm_RTC.tm_mon + 1,
                                         (U2)tm_RTC.tm_mday,
                                         (U2)tm_RTC.tm_hour,
                                         (U2)tm_RTC.tm_min,
                                         (U2)tm_RTC.tm_sec );

         // Compute the difference between the Current RTC Time the New UTC Time
         dT_s = (I4)( RTC_CTime - New_CTime );        // [seconds]

         LOGGPS("GN_GPS_Write_UTC: RTC difference is : %d", dT_s);

         // If the difference is too much for the target system, then update the RTC.
         // TODO:  Adjust this test limit to suit the target platform & environment.
         if ( abs(dT_s) > 10 )                        // 10 seconds
         {
            // Big offset, so set the RTC to integer second resolution
            tm_RTC.tm_mday        =  (int)p_UTC->Day;
            tm_RTC.tm_mon         =  (int)p_UTC->Month - 1;
            tm_RTC.tm_year        =  (int)p_UTC->Year - 1900;
            tm_RTC.tm_hour        =  (int)p_UTC->Hours;
            tm_RTC.tm_min         =  (int)p_UTC->Minutes ;
            tm_RTC.tm_sec         =  (int)p_UTC->Seconds;
            //tv_RTC.tv_sec  = timegm( &tm_RTC );
            tv_RTC.tv_sec  = mktime( &tm_RTC ) - g_utc_diff;
            tv_RTC.tv_usec = p_UTC->Milliseconds * 1000;

            LOGGPS("GN_GPS_Write_UTC: Setting time of day to sec=%d\n", (int) tv_RTC.tv_sec);

#if HAVE_ANDROID_OS
            LOGGPS("GN_GPS_Write_UTC  -------------1111111111");
            fd = open("/dev/alarm", O_RDWR);
            if(fd < 0) {
                LOGGPS("GN_GPS_Write_UTC:Unable to open alarm driver: %s\n", strerror(errno));
                return ;
            }
            ts.tv_sec = tv_RTC.tv_sec;
            ts.tv_nsec = tv_RTC.tv_usec * 1000;
            res = ioctl(fd, ANDROID_ALARM_SET_RTC, &ts);
            if(res < 0) {
                LOGGPS("GN_GPS_Write_UTC:Unable to set rtc to %ld: %s\n", tv_RTC.tv_sec, strerror(errno));
            }
            close(fd);
#else
            LOGGPS("GN_GPS_Write_UTC  -------------222222222");
            if ( settimeofday( &tv_RTC, NULL ) == 0 )
            {
               LOGGPS( "GN_GPS_Write_UTC:  RTC Set: %d-%d-%d  %d:%02d:%02d",
                       p_UTC->Day, p_UTC->Month, p_UTC->Year,
                       p_UTC->Hours, p_UTC->Minutes, p_UTC->Seconds );
            }
            else
            {
                LOGGPS("GN_GPS_Write_UTC: settimeofday has error, because : %s", strerror(errno));
            }
#endif
         }


      // Save the RTC Calibration data to the global structure

      // a) When this Calibrtation was done
      gn_RTC_Calib.CTime_Set = New_CTime;

      // b) The bigger integer second part of the Calibration Offset
      gn_RTC_Calib.Offset_s  = dT_s;

      // c) The smaller millisecond part of the Calibration Offset based on:
      //    1) The +/- 0.5sec rounding in reading the RTC
      //    2) Milliseconds bit of the input UTC Time which was not used
      //    3) The latency between when the UTC Time was computed for and when
      //       we read-back the RTC to compute the calibration.
      gn_RTC_Calib.Offset_ms = ( 500 - p_UTC->Milliseconds )  +
                               ( RTC_OS_Time_ms - p_UTC->OS_Time_ms );

      // d) The Estimated accuracy of the RTC Calibration.  Assume that we can:
      //    1) Keep the GN_GPS_Update loop latency to within 100ms, say 30ms RMS,
      //    2) Calibration is good to +/- 0.5 second,  ie 170ms RMS
      gn_RTC_Calib.Acc_Est_Set =  p_UTC->Acc_Est + 30 + 150;

      // e) Compute the new checksum for the Calibration data structure
      gn_RTC_Calib.checksum =  (U4)( (U4)0x55555555              +
                                     (U4)gn_RTC_Calib.CTime_Set  +
                                     (U4)gn_RTC_Calib.Offset_s   +
                                     (U4)gn_RTC_Calib.Offset_ms  +
                                     (U4)gn_RTC_Calib.Acc_Est_Set  );

    LOGGPS("GN_GPS_Write_UTC:  ------------------ ");

   }

   return;
}

#if 0
char UART_Input_Task(void)
{
	int bytes_read;
	int max_size = 0;
	int read_more;
	char input_status=0;

	do
	{
		read_more = 0;
		// Determine the amount of space to the wrap-point of the buffer
		max_size = UARTrx_cb.end_buf -  UARTrx_cb.write;
		if ( max_size > 256 )
			max_size = 256;//每次最多读256个字节

		bytes_read = read(g_gps_state->fd, UARTrx_cb.write, max_size);
		LOGD("####  bytes_read = %d   max_size = %d ###", bytes_read, max_size);
		if ( bytes_read > 0 )
		{
			input_status=1;
			UARTrx_cb.write = UARTrx_cb.write + bytes_read;
			// Are we at the end of the buffer?
			if (UARTrx_cb.write >= UARTrx_cb.end_buf)
			{
				UARTrx_cb.write = UARTrx_cb.start_buf;
			}
			// If we read the maximum amount, there might be more
			// available, so try again
			if ( bytes_read == max_size )
			{
				read_more = 1;
			}
		}

	} while ( read_more != 0 );
	// 读完之后，清除接收Buffer

	return input_status;
}
#endif

//*****************************************************************************

// GN GPS Callback Function to Read GPS Measurement Data from the Host's
// chosen GPS Baseband communications interface.
// Internally the GN GPS Core library uses a circular buffer to store this
// data.  Therefore, 'max_bytes' is dynamically set to prevent a single Read
// operation from straddling the internal circular buffer's end wrap point, or
// from over writing data that has not been processed yet.
// Returns the number of bytes actually read.  If this is equal to 'max_bytes'
// then this callback function may be called again if 'max_bytes' was limited
// due to the circular buffer's end wrap point.

U2 GN_GPS_Read_GNB_Meas(
   U2  max_bytes,                   // i - Maximum number of bytes to read
   CH *p_GNB_Meas )                 // i - Pointer to the Measurement data.
{
   int bytes_read;                  // Number of bytes actually read
   U2 num;

    RUNTO;

#if 1
    bytes_read = read(g_gps_state->fd, p_GNB_Meas, max_bytes);
	//LOGD("-------------- GN_GPS_Read_GNB_Meas end read time %d  bytes_read = %d   max_bytes = %d ----------------------", GN_GPS_Get_OS_Time_ms(), bytes_read, max_bytes);

    if (bytes_read < 0 ) {
        bytes_read = 0;
    }
#if WRITE_LOG
    Write_Data_To_Log( TEST_LOG, bytes_read, p_GNB_Meas );
#endif
    return ((U2)bytes_read);
#endif
#if 0
	num = 0;

	// Copy the data from the input circular buffer to the
	// Measurement Data
	while ( (UARTrx_cb.write != UARTrx_cb.read) && (num < max_bytes) )
	{
		p_GNB_Meas[num] = *UARTrx_cb.read;
		UARTrx_cb.read++;
		num++;
		if ( UARTrx_cb.read >= UARTrx_cb.end_buf)
		{
			UARTrx_cb.read = UARTrx_cb.start_buf;
		}
	}
	return (num);
#endif
}

//*****************************************************************************
// GN GPS Callback Function to Write GPS Control Data to the Host's chosen
// GPS Baseband communications interface.
// Internally the GN GPS Core library uses a circular buffer to store this
// data.  Therefore, this callback function may be called twice if the data to
// be written straddles the internal circular buffer's end wrap point.
// Returns the number of bytes actually written.  If this is less than the
// number of bytes requested to be written, then it is assumed that this is
// because the host side cannot physically handle any more data at this time.

U2 GN_GPS_Write_GNB_Ctrl(
   U2  num_bytes,                   // i - Available number of bytes to Write
   CH *p_GNB_Ctrl )                 // i - Pointer to the Ctrl data.
{
   int bytes_written;               // Number of bytes written

    RUNTO;

    bytes_written = write(g_gps_state->fd, p_GNB_Ctrl, num_bytes);
   if ( bytes_written < 0 )   bytes_written = 0;

#if WRITE_LOG
    Write_Data_To_Log( TEST_LOG, bytes_written, p_GNB_Ctrl );
#endif
    return ((U2)bytes_written) ;
}

//*****************************************************************************
// GN GPS Callback Function to request that if possible the host should
// perform a Hard Power-Down Reset of the GN Baseband Chips.
// Returns TRUE is this is possible.

BL GN_GPS_Hard_Reset_GNB( void )
{
   // Call the GN GNS???? Chip/Module Hardware Abstraction Layer Function,
   // which will return TRUE if "Reset" Control is implemented.

   //return( GN_GNS_HAL_Reset() );
   return (FALSE);
}


//*****************************************************************************
// GN GPS Callback Function to Write GPS NMEA 183 Output Sentences to the
// the Host's chosen NMEA interface.
// Internally the GN GPS Core library uses a circular buffer to store this
// data.  Therefore, this callback function may be called twice if the data to
// be written straddles the internal circular buffer's end wrap point.
// Returns the number of bytes actually written.  If this is less than the
// number of bytes requested to be written, then it is assumed that this is
// because the host side cannot physically handle any more data at this time.

U2 GN_GPS_Write_NMEA(
   U2  num_bytes,                   // i - Available number of bytes to Write
   CH *p_NMEA )                     // i - Pointer to the NMEA data.
{
   // RUNTO;
#if WRITE_LOG
    Write_Data_To_Log( NMEA_LOG, num_bytes, p_NMEA );
#endif
    return(num_bytes);
}


//*****************************************************************************
// GN GPS Callback Function to Read $PGNV GN GPS Propriatary NMEA Input
// Messages from the Host's chosen $PGNV communications interface.
// Internally the GN GPS Core library a circular buffer to store this data.
// Therefore, 'max_bytes' is dynamically set to prevent a single Read operation
// from straddling the internal circular buffer's end wrap point, or from
// over-writing data that has not yet been processed.
// Returns the number of bytes actually read.  If this is equal to 'max_bytes'
// then this callback function may be called again if 'max_bytes' was limited
// due to the circular buffer's end wrap point.

U2 GN_GPS_Read_PGNV(
   U2  max_bytes,                   // i - Maximum number of bytes to read
   CH *p_PGNV )                     // i - Pointer to the $PGNV data.
{
   // This function will be called by the GPS Core only if the host has
   // called the GN_GPS_Parse_PGNV() function.
   // This is only necessary on host platforms that require the ability to
   // trigger some GN GPS API functionality via an input NMEA message rather
   // than by directly calling the API function.  Usually, this would only be
   // necessary or desirable to test purposes.


    // This is not implemented.
    return(0);
}

//*****************************************************************************
// Debug Callback Functions called by the GN GPS High Level Software library
// that need to be implemented by the Host platform software to capture debug
// data to an appropriate interface (eg UART, File, both etc).
// GN GPS Callback Function to Write GPS Baseband I/O communications Debug data
// to the the Host's chosen debug interface.
// Internally the GN GPS Core library uses a circular buffer to store this
// data.  Therefore, this callback function may be called twice if the data to
// be written straddles the internal circular buffer's end wrap point.
// Returns the number of bytes actually written.  If this is less than the
// number of bytes requested to be written, then it is assumed that this is
// because the host side cannot physically handle any more data at this time.

U2 GN_GPS_Write_GNB_Debug(
   U2  num_bytes,                   // i - Available number of bytes to Write
   CH *p_GNB_Debug )                // i - Pointer to the GNB Debug data.
{
   // Write the data to the GNB Debug log file
   // RUNTO;
#if WRITE_LOG
    Write_Data_To_Log( ME_LOG, num_bytes, p_GNB_Debug );
#endif
    return(num_bytes);
}





//*****************************************************************************
// GN GPS Callback Function to Write GPS Navigation Solution Debug data to the
// Host's chosen debug interface.
// Internally the GN GPS Core library uses a circular buffer to store this
// data.  Therefore, this callback function may be called twice if the data to
// be written straddles the internal circular buffer's end wrap point.
// Returns the number of bytes actually written.  If this is less than the
// number of bytes requested to be written, then it is assumed that this is
// because the host side cannot physically handle any more data at this time.

U2 GN_GPS_Write_Nav_Debug(
   U2  num_bytes,                   // i - Available number of bytes to Write
   CH *p_Nav_Debug )                // i - Pointer to the Nav Debug data.
{
   // RUNTO;
#if WRITE_LOG
    // Write the data to the log file
    Write_Data_To_Log( NAV_LOG, num_bytes, p_Nav_Debug );
#endif
    return(num_bytes);
}

//*****************************************************************************
// GN GPS Callback Function to Write GPS Navigation Library Event Log data
// to the Host's chosen debug interface.
// Internally the GN GPS Core library uses a circular buffer to store this
// data.  Therefore, this callback function may be called twice if the data to
// be written straddles the internal circular buffer's end wrap point.
// Returns the number of bytes actually written.  If this is less than the
// number of bytes requested to be written, then it is assumed that this is
// because the host side cannot physically handle any more data at this time.

U2 GN_GPS_Write_Event_Log(
   U2  num_bytes,                   // i - Available number of bytes to Write
   CH *p_Event_Log )                // i - Pointer to the Event Log data.
{
   // RUNTO;
#if WRITE_LOG
    // Write the data to the log file
    Write_Data_To_Log( EVENT_LOG, num_bytes, p_Event_Log );
#endif
    return(num_bytes);
}

