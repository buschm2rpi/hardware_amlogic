
//****************************************************************************
// GPS IP Centre, ST-Ericsson (UK) Ltd.
// Copyright (c) 2009 ST-Ericsson (UK) Ltd.
// 15-16 Cottesbrooke Park, Heartlands Business Park, Daventry, NN11 8YL, UK.
// All rights reserved.
//
// Filename  GN_GPS_api.h
//
// $Header: S:/GN_GPS_Nav_MKS/Inc_API/rcs/GN_GPS_api.h 1.37 2009/04/30 13:26:06Z geraintf Rel $
// $Locker: $
//****************************************************************************

#ifndef GN_GPS_API_H
#define GN_GPS_API_H

#ifdef __cplusplus
   extern "C" {
#endif

//****************************************************************************
//
//   GloNav GPS External Interface API Definitions
//
//*****************************************************************************

#include "gps_ptypes.h"

#define SIZE_NV_STORE   (5120)

//*****************************************************************************
//
//  GN GPS External Interface API/Callback function related enumerated data
//  type and structure definitions.
//
//*****************************************************************************

// Status of the patch upload process
typedef enum GN_Patch_Status      // GN GPS Patch Upload Status
{
   GN_HL_SLEEP,          // The High-Level software has sent a Sleep command
                         // to the baseband
   GN_HL_WAKING,         // The High-Level software has sent a WakeUp command
                         // to the baseband, and is now waiting to receive data
   GN_PATCH_IN_PROCESS,  // The Patch upload is in process
   GN_PATCH_COMPLETE     // The Patch upload has been completed

}  e_GN_Patch_Status;            // GN GPS Patch Upload Status


 // PGNV Command type enum

typedef enum GN_PGNV_Cmd      // GN GPS PGNV Command Type Received
{
   GN_PGNV_NONE,              // No Command received
   GN_PGNV_GSLP,              // GSLP (Go To Sleep) Command
   GN_PGNV_WAKE,              // Wakeup
   GN_PGNV_SDWN,              // Shutdown
   GN_PGNV_RSET               // RSET (Reset) Command

}  e_GN_PGNV_Cmd;            // GN GPS PGNV Command Type Received


 // GN GPS Navigation Library Version Details

typedef struct GN_GPS_Version  // GN_GPS Configuration Data Structure
{
   CH Name[32];               // Pointer to a GN GPS Nav Lib Version name string
   U2 MajorVersNo;            // Major Version Number (ie y in yy.yy.xx)
   U2 MinorVersNo;            // Minor Version Number (ie y in xx.xx.yy)
   CH VerDate[8];             // GN GPS Lib Library "Version" YYMMDD Date string
   CH Date[16];               // GN GPS Lib Library "compiled on" Date string
   CH Time[16];               // GN GPS Lib Library "compiled at" Time string
   U2 BB_SW_Ver_ROM;          // GN GPS Baseband SW Version in ROM (0=Unknown)
   U2 BB_SW_Patch_Cs;         // GN_GPS Baseband SW Patch Checksum (0=Unknown)
   CH SysVersion[32];         // GN GPS System Version string which is a combination
                              //    of the Nav SW version, date, & GNB SW version:
                              //    MajorVersNo.ManorVersNo-BB_SW_Ver_ROM-BB_SW_Patch_Cs-VerDate
}  s_GN_GPS_Version;


 // User application specific Configuration Data Structure

typedef enum GN_GPS_Sens_Mode // GN GPS Sensitivity Mode states
{
   GN_GPS_SENS_MODE_HIGH,     // High Sensitivity Mode, optimised for mainly
                              //    indoor / handset usage.
   GN_GPS_SENS_MODE_NORMAL,   // Normal Sensitivity Mode, optimised for mainly
                              //    automotive and outdoor usage.
   GN_GPS_SENS_MODE_DYNAMIC   // Dynamic Sensitivity Mode, optimially switching
                              //    between the above High & Noraml modes.

}  e_GN_GPS_Sens_Mode;        // GN GPS Sensitivity Mode states

// Balance between potentially faster/slower cold start TTFF and lower/higher
// cold start sensitivity

typedef enum GN_GPS_Cold_TTFF  // GN GPS Cold Start TTFF Mode states
{
   GN_GPS_COLD_TTFF_BALANCE,   // = Balance between speed and sensitivity (= library default setting)
   GN_GPS_COLD_TTFF_FASTER,    // = Potentially faster cold start TTFF but lower cold start sensitivity
   GN_GPS_COLD_TTFF_SENSITIVE  // = More sensitive cold start but potentially slower cold start TTFF

} e_GN_GPS_Cold_TTFF;          // GN GPS Cold Start TTFF Mode states


// Power versus Performance Mode
typedef enum e_GN_GPS_PowPerf  // GN GPS Power versus Performance Mode states
{
   GN_GPS_POW_PERF_DEFAULT,    // = Baseband default setting (= library default setting)
   GN_GPS_POW_PERF_HI_PERF,    // = High performance
   GN_GPS_POW_PERF_LO_POW      // = Low power

} e_GN_GPS_PowPerf;            // GN GPS Power versus Performance Mode states



typedef struct GN_GPS_Config  // GN_GPS Configuration Data Structure
{
   e_GN_GPS_Sens_Mode SensMode;  // User's preferred Sensitivity Mode

   e_GN_GPS_Cold_TTFF ColdTTFF;  // Cold Start TTFF mode - see e_Cold_TTFF enum, above
                                 //    GN_GPS_COLD_TTFF_BALANCE   = Balance between speed and sensitivity
                                 //                                 (library default setting)
                                 //    GN_GPS_COLD_TTFF_FASTER    = Potentially faster cold start TTFF but
                                 //                                 lower cold start sensitivity
                                 //    GN_GPS_COLD_TTFF_SENSITIVE = More sensitive cold start but
                                 //                                 potentially slower cold start TTFF

   e_GN_GPS_PowPerf  PowerPerf;  // Power vs. performance mode - see e_GN_GPS_PowPerf enum, above
                                 //    GN_GPS_POW_PERF_DEFAULT,   = Baseband default setting (= library default setting)
                                 //    GN_GPS_POW_PERF_HI_PERF,   = High performance
                                 //    GN_GPS_POW_PERF_LO_POW     = Low power

   BL BGA_Chip;                  // The Baseband chip is BGA (rather than CSP)
                                 //   TRUE  = the chip is BGA (= library default setting)
                                 //   FALSE = the chip is CSP

   BL c32KHz_NotFitted;          //  The 32KHz clock is not fitted
                                 //   TRUE  = the 32KHz clock is definitely not fitted
                                 //   FALSE = the baseband will auto detect whether the 32KHz
                                 //           clock is present or not (= library default setting)

   BL SBAS_Enabled;              // Enable tracking and use of SBAS satellites (PRN 120..138)
                                 // Currently only GNS7560 ROM 510 (patched) is SBAS capable.
                                 // Enabling SBAS reduces the number of tracking channels
                                 // available for GPS satellites.  Note that the EGNOS SBAS satellites
                                 // over Europe are currently not enabled for ranging and, therefore,
                                 // cannot be used for Navigation, but may consume tracking channels.

   U1 PosFiltMode;            // Navigation Output Positon Filtering Mode
                              //    0 : Raw Navigation Output
                              //  >=1 : Navigation Output Filtering Mode Enabled. (Default is Mode 4)
                              //  >=5 : Reserved for future enhancements.

   U2 FixInterval;            // Desired interval [milliseconds] between adjacent
                              //     position fixes [0 = As fast as is possible]

   U2 H_AccEst_Mask[2];       // Horizontal position RMS Accuracy Estimate and,
   U2 V_AccEst_Mask[2];       // Vertical position RMS Accuracy Estimate
                              //    masks [m] at which point a suitable fix is
                              //    deemed to have been obtained for the current
                              //    Operating Mode of :
                              //    [0] : Single "Push To Fix" required
                              //    [1] : "Continuous Navigation" required

   U2 SleepModeTimeOut;       // Sleep Mode Time-Out [seconds],  ie when an
                              //    automatic transition is made to Coma Mode.
   U4 ComaModeTimeOut;        // Coma Mode Time-Out [seconds],  ie when an
                              //    automatic transition is made to Deep-Coma Mode.

                              // NMEA Sentence output rate as a function
                              // of the above defined "Fix_Interval".
                              //    0 = No output
                              //    N = Once every 'N' Navigation Fix updates
   U1 GPGLL_Rate;             // $GPGLL output rate - Geographic Position - Lat, Long
   U1 GPGGA_Rate;             // $GPGGA output rate - GPS Fix Data
   U1 GPGSA_Rate;             // $GPGSA output rate - GNSS DOPS and Active Satellites
   U1 GPGST_Rate;             // $GPGST output rate - GNSS Pseudorange Error Statistics
   U1 GPGSV_Rate;             // $GPGSV output rate - GNSS Satellites in View
   U1 GPRMC_Rate;             // $GPRMC output rate - Recommended Minimum GNSS Sentence
   U1 GPVTG_Rate;             // $GPVTG output rate - Course Over Ground and Ground Speed
   U1 GPZCD_Rate;             // $GPZCD output rate - OS Timestamp for the Fix [seconds]
   U1 GPZDA_Rate;             // $GPZDA output rate - Time & Date
   U1 PGNVD_Rate;             // $PGNVD output rate - GloNav proprietary Diagnostics data

   U1 ForceCold_Timeout;      // Force a cold start if a valid fix has not been obtained after
                              // the defined period [minutes]
                              //     0 : Use library default values
                              // 1..254: Force a cold-start after defined period (minutes)
                              //         without a fix after power-on
                              //    255: Invalid

   U2 NV_Write_Interval;      // Interval to make periodic calls to GN_GPS_Write_NV_Store()
                              //    while running [minutes].      0 = Never.
                              //    (Eg, In-case power is lost before Sleep or Shutdown.)

   U2 Enable_Nav_Debug;       // GN Navigation Solution debug Enabled level    (>0 = yes)
   U2 Enable_GNB_Debug;       // GN Baseband I/O Comms debug Enabled level     (>0 = yes)
   U2 Enable_Event_Log;       // GN Navigation Library Event Log Enabled level (>0 = yes)

   R8 Reference_LLH[3];       // A Reference WGS84 Latitude, Longitude [degrees] and
                              //    Ellipsoidal Height [metres] for debug outputs

   void *Reserved;            // Reserved for future used.

} s_GN_GPS_Config;            // GN_GPS Configuration Data Structure


//----------------------------------------------------------------------------
//  GN GPS UTC Date & Time data.
//  A local time zone adjustments have been removed.

typedef struct GN_GPS_UTC_Data   // GN GPS UTC Date & Time Data
{
   U2 Year;                   // UTC Year A.D.                    [eg 2006]
   U2 Month;                  // UTC Month into year              [range 1..12]
   U2 Day;                    // UTC Days into month              [range 1..31]
   U2 Hours;                  // UTC Hours into day               [range 0..23]
   U2 Minutes;                // UTC Minutes into the hour        [range 0..59]
   U2 Seconds;                // UTC Seconds into minute          [range 0..59]
   U2 Milliseconds;           // UTC Milliseconds into the second [range 0..999]
   U4 Acc_Est;                // RTC Time Accuracy RMS Estimate   [millieconds]
   U4 OS_Time_ms;             // Corresponding OS Time tick       [millieconds]

}  s_GN_GPS_UTC_Data;            // GN GPS UTC Date & Time Data


//----------------------------------------------------------------------------
//  GN GPS Navigation Solution Data.
//  This data is sufficient to generate all the commonly used NMEA GPS
//  sentences of $GPGLL, $GPGGA, $GPGSA, $GPGST, $GPGSV, $GPRMC, $GPVTG & $GPZDA.

#define NMEA_SV  16           // Number of entries in the satellite arrays

typedef enum GN_GPS_FIX_TYPE  // GN GPS Navigation Position Fix Type
{
   GN_GPS_FIX_NONE,           // No Position Fix
   GN_GPS_FIX_ESTIMATED,      // Estimated (ie forward predicted) Position Fix
   GN_GPS_FIX_2D,             // Autonomous   2-Dimensional Position Fix
   GN_GPS_FIX_DIFF_2D,        // Differential 3-Dimensional Position Fix
   GN_GPS_FIX_3D,             // Autonomous   2-Dimensional Position Fix
   GN_GPS_FIX_DIFF_3D         // Differential 3-Dimensional Position Fix

}  e_GN_GPS_FIX_TYPE;         // GN GPS Navigation Position Fix Type

typedef struct GN_GPS_Nav_Data   // GN GPS Navigation solution data
{
   U4 Local_TTag;             // Local receiver time-tag since start-up [msec]
   U4 OS_Time_ms;             // Local Operating System Time [msec]
   U2 Year;                   // UTC Year A.D.                     [eg 2006]
   U2 Month;                  // UTC Month into the year           [range 1..12]
   U2 Day;                    // UTC Days into the month           [range 1..31]
   U2 Hours;                  // UTC Hours into the day            [range 0..23]
   U2 Minutes;                // UTC Minutes into the hour         [range 0..59]
   U2 Seconds;                // UTC Seconds into the hour         [range 0..59]
   U2 Milliseconds;           // UTC Milliseconds into the second  [range 0..999]
   I2 Gps_WeekNo;             // GPS Week Number
   R8 Gps_TOW;                // Corrected GPS Time of Week [seconds]
   R8 UTC_Correction;         // Current (GPS-UTC) time difference [seconds]
   R8 X;                      // WGS84 ECEF X Cartesian coordinate [m]
   R8 Y;                      // WGS84 ECEF Y Cartesian coordinate [m]
   R8 Z;                      // WGS84 ECEF Z Cartesian coordinate [m]
   R8 Latitude;               // WGS84 Latitude  [degrees, positive North]
   R8 Longitude;              // WGS84 Longitude [degrees, positive East]
   R4 Altitude_Ell;           // Altitude above WGS84 Ellipsoid [m]
   R4 Altitude_MSL;           // Altitude above Mean Sea Level [m]
   R4 SpeedOverGround;        // 2-dimensional Speed Over Ground [m/s]
   R4 CourseOverGround;       // 2-dimensional Course Over Ground [degrees]
   R4 VerticalVelocity;       // Vertical velocity [m/s]
   R4 N_AccEst;               // Northing 1-sigma (67%) Accuracy estimate [m]
   R4 E_AccEst;               // Easting  1-sigma (67%) Accuracy estimate [m]
   R4 V_AccEst;               // Vertical 1-sigma (67%) Accuracy estimate [m]
   R4 H_AccMaj;               // Horizontal error ellipse semi-major axis [m]
   R4 H_AccMin;               // Horizontal error ellipse semi-minor axis [m]
   R4 H_AccMajBrg;            // Bearing of the semi-major axis [degrees]
   R4 HVel_AccEst;            // Horizontal Velocity 1-sigma (67%) Accuracy estimate [m/s]
   R4 VVel_AccEst;            // Vertical Velocity 1-sigma (67%) Accuracy estimate [m/s]
   R4 PR_ResRMS;              // Standard dev of the PR a posteriori residuals [m]
   R4 H_DOP;                  // Horizontal Dilution of Precision
   R4 V_DOP;                  // Vertical Dilution of Precision
   R4 P_DOP;                  // 3-D Position Dilution of Precision

   e_GN_GPS_FIX_TYPE FixType; // Position fix type
   BL Valid_2D_Fix;           // Is the published 2D position fix "valid"
                              //    relative to the required Horizontal
                              //    accuracy masks ?
   BL Valid_3D_Fix;           // Is the published 3D position fix "valid"
                              //    relative to both the required Horizontal
                              //    and Vertical accuracy masks ?
   U1 FixMode;                // Solution Fixing Mode
                              //    1 = Forced 2-D at MSL,
                              //    2 = 3-D, with automatic fall back to 2-D

   U1 SatsInView;             // Satellites in View count
   U1 SatsUsed;               // Satellites in Used for Navigation count

   U1 SatsInViewSVid[NMEA_SV];// Satellites in View SV id number [PRN]
   U1 SatsInViewSNR[NMEA_SV]; // Satellites in Signal To Noise Ratio [dBHz]
   U2 SatsInViewAzim[NMEA_SV];// Satellites in View Azimuth [degrees]
   I1 SatsInViewElev[NMEA_SV];// Satellites in View Elevation [degrees]
                              //    if = -99 then Azimuth & Elevation angles
                              //         are currently unknown
   BL SatsInViewUsed[NMEA_SV];// Satellites in View Used for Navigation ?

} s_GN_GPS_Nav_Data;          // GN GPS Navigation solution data


//----------------------------------------------------------------------------
//  GN GPS Time Solution Data.
//  This data is primarily inteded for Time & Frequency applications

typedef struct GN_GPS_Time_Data   // GN GPS Navigation solution data
{
   U4 Local_TTag;             // Local receiver time-tag since start-up [msec]
   U4 OS_Time_ms;             // Local Operating System Time [msec]
   U2 Year;                   // UTC Year A.D.                     [eg 2006]
   U2 Month;                  // UTC Month into the year           [range 1..12]
   U2 Day;                    // UTC Days into the month           [range 1..31]
   U2 Hours;                  // UTC Hours into the day            [range 0..23]
   U2 Minutes;                // UTC Minutes into the hour         [range 0..59]
   U2 Seconds;                // UTC Seconds into the hour         [range 0..59]
   U2 Milliseconds;           // UTC Milliseconds into the second  [range 0..999]
   I2 Gps_WeekNo;             // GPS Week Number
   R8 Gps_TOW;                // Corrected GPS Time of Week [seconds]
   R8 Loc_TOW;                // Local GPS Time of Week [seconds]
   R8 Clock_Bias;             // Receiver Clock Bias [seconds]
   R8 Clock_Drift;            // Receiver Clock Drift [seconds/second]
   R8 UTC_Correction;         // Current (GPS-UTC) time difference [seconds]
   R4 T_AccEst;               // Time        1-sigma (67%) Accuracy estimate [s]
   R4 B_AccEst;               // Clock Bias  1-sigma (67%) Accuracy estimate [s]
   R4 F_AccEst;               // Frequency   1-sigma (67%) Accuracy estimate [s/s]
   R4 T_DOP;                  // Time Dilution of Precision

   e_GN_GPS_FIX_TYPE FixType; // Position fix type

} s_GN_GPS_Time_Data;         // GN GPS Time solution data


//----------------------------------------------------------------------------
//  GN GPS Power Usage Allowed States

typedef enum GN_GPS_Power_Mode   // Power Usage Allowed Mode states
{
   GN_GPS_POWER_ALLOWED_HIGH,       // High / unlimited level of Power use is Allowed
   GN_GPS_POWER_ALLOWED_MEDIUM,     // Only a Medium level of Power use is Allowed
   GN_GPS_POWER_ALLOWED_LOW         // Only a Low level of Power use is Allowed
}  e_GN_GPS_Power_Mode;


//*****************************************************************************
//
// GN GPS External Interface API/Callback related function prototypes
//
//*****************************************************************************

//*****************************************************************************
// API Functions that may be called by the Host platform software.

// GN GPS API Function to return the current status of the patch upload process.
e_GN_Patch_Status GN_GPS_Get_Patch_Status( void );

// GN GPS API Function to Get the Version Details of the GN GPS Library.
// Returns TRUE if successful.
BL GN_GPS_Get_Version(
   s_GN_GPS_Version *p_Version );   // i - Pointer to where the GPS Core software
                                    //        should write the Version Details to

// GN GPS API Function to Set the Version Details of the GN GPS Software based
// on more up to date information from the host system wrapper software.
// Only non-zero or non-NULL inputs are applied.
// This feature is provided so that the host application version details will
// can be made to appear in the NMEA $PGNVR,VERS message and the debug outputs.
// Returns TRUE if successful.
BL GN_GPS_Set_Version(
   s_GN_GPS_Version *p_Version );   // i - Pointer to where the GPS Core software
                                    //        should get the Version Details from

// GN GPS Function that performs all the GN GPS Software Initialisation at
// system start-up.
// Any target specific GPS Baseband & RF realted hardware initialisations must
// have been completed before this call.  See the sample source code provided.
void GN_GPS_Initialise( void );

// GN GPS Function that performs all the GPS Position Fix related Update
// activities.   This is the main body of the GN GPS software run on the host.
// It is this function that makes all the API Callbacks on the host software
// as and when it needs any further interaction.
void GN_GPS_Update( void );

// GN GPS Function that parses a $PGNV GloNav proprietary NMEA Input Stream
// for configuration messages.  This function only need be called on host
// platforms that require the ability to trigger some GN GPS API functionality
// via a in input message rather than by directly calling the API function.
// This function may make call backs on the host application.
// Returns with the type of command seen (either GN_PGNV_GSLP, GN_PGNV_RSET,
// or GN_PGNV_NONE).
e_GN_PGNV_Cmd GN_GPS_Parse_PGNV( void );

// GN GPS Function that performs all the GN GPS Shutdown activities at
// system shutdown.
// Some target specific GPS Baseband & RF related hardware shutdown activities
// may be required after this call.  See the sample source code provided.
void GN_GPS_Shutdown( void );

// GN GPS API Function to Wake Up the GPS to start a period of Navigation.
// This assumes that the GPS has already been initialised and is waiting in a
// minimum power Sleep, Coma or DeepComa state.
void GN_GPS_WakeUp( void );

// GN GPS API Function to stop a period of GPS Navigation and put it into a
// low power Sleep, Coma or DeepComa state pending the next period of navigation.
void GN_GPS_Sleep(
   U2 SleepModeTimeOut,             // i - Sleep Mode Time-Out [seconds],
                                    //        ie when an automatic transition
                                    //        is made to Coma Mode.
   U4 ComaModeTimeOut );            // i - Coma Mode Time-Out [seconds],
                                    //        ie when an automatic transition
                                    //        is made to a Deep-Coma Mode.

// GN GPS API Function to Get the user Configuration data items currently being
// used.  If a 'GN_GPS_Get_Config' call is made before a valid call to
// 'GN_GPS_Set_Config', then the GPS Core Library default settings will be
// offered.
// Returns TRUE if the configuration settings are valid.
BL GN_GPS_Get_Config(
   s_GN_GPS_Config *p_Config );     // i - Pointer to where the host software
                                    //        can get the Configuration data from

// GN GPS API Function to Set or change the user Configuration data items.
// Returns TRUE if the configuration settings were accepted.
BL GN_GPS_Set_Config(
   s_GN_GPS_Config *p_Config );     // i - Pointer to where the GPS Core software
                                    //        can get the Configuration data from

// GN GPS API Function to Set the current Power Usage Allowed state
// Returns TRUE if the configuration settings were accepted.
BL GN_GPS_Set_Power_Mode(
   e_GN_GPS_Power_Mode Power_Mode );// i - Current Power Usage Allowed Mode State

// GN GPS API Function to Initialise the GPS High Level Software's Non-Volatile
// Store data from Host's chosen Non-Volatile Store media at system start-up.
// Returns TRUE if successful.
BL GN_GPS_Init_NV_Store(
   U1 *p_NV_Ram );                  // i - Pointer to where the GPS Core software
                                    //        can get the NV Store Data from

// GN GPS API Function to delete the position information from the GPS High
// Level Software's Non-Volatile Store data. This function should be called
// if the host knows that the receiver has been moved many hundreds or
// thousands of km since it was last used. The function should not be called
// more than once. If called, it is preferable to call it soon after the
// GN_GPS_Initialise function call.
void GN_GPS_Clear_Pos_NV_Store( void );

// GN GPS API Function to delete data items from a Non-Volatile Store RAM
// image located at the given pointer.  The character string describing the
// items to be Cleared matches the one used in the $PGNVS,RSET, input command.
// "WARM" or "COLD" will delete all the data required prior to a Warm or Cold
// Start test.  Any other input string will be read a character at a time for
// a data category to be deleted as follows:
//    P = Position,       T = Time,        F = Frequency calibration,
//    E = Ephemeredes for all satellites,  A = Almanacs for all satellites,
//    I = Ionospheric Model Parameters,    U = UTC Model Correction Parameters,
//    H = Satellite Health data.
// If Time is to be cleared then GN_GPS_Write_UTC_Data() will be called to
// clear the host platform Real-Time Clock information.
// Returns TRUE  if the specified Non-Volatile Data items were cleared.
// Returns FALSE if there was an error in the input.
BL GN_GPS_Clear_NV_Data(
   U1 *p_NV_Ram,                    // i - Pointer to the NV Store Data
   CH Items[16] );                  // i - String describing the Data items
                                    //        to clear

// GN GPS API Function to Get the GPS High Level Software's Non-Volatile Store
// data so that it can be written to the Host's chosen Non-Volatile Store media
// at system shutdown.
// Returns the size of the Non-Volatile Store image to save.
U2 GN_GPS_Get_NV_Store(
   U1 **p_NV_Store );               // o - Pointer to where the host software
                                    //        can get the NV Store Data from

// GN GPS API Function to request the latest GPS Navigation solution data.
// Returns TRUE if there is valid GPS Navigation solution data.
BL GN_GPS_Get_Nav_Data(
   s_GN_GPS_Nav_Data *p_Nav_Data ); // i - Pointer to where the GPS Core software
                                    //        should write the Nav Data to

// GN GPS API Function to request the latest GPS Time solution data.
// Returns TRUE if there is valid GPS Time solution data.
BL GN_GPS_Get_Time_Data(
   s_GN_GPS_Time_Data *p_Time_Data );// i - Pointer to where the GPS Core software
                                     //        should write the Time Data to

// GN GPS API Function to request NMEA compatible GPS sentences.
// Returns the number of bytes of NMEA data presented.
U2 GN_GPS_Get_NMEA(
   U2 max_size,                     // i - Maximum number of bytes to return
                                    //        in a single function call
   CH *p_NMEA );                    // i - Pointer to where GPS Core software
                                    //        should write the NMEA Data to


// GN GPS Utility Function to Re-Qualify the contents of a GPS Navigation solution
// Data structure against input Horizontal and Vertical Accuracy Requirements.
// The "Valid_2D_Fix" and "Valid_3D_Fix" flags are modified accordingly.
// Returns TRUE if the GPS Navigation solution Data is Qualified to at least a
// "Valid_2D_Fix" state.
BL GN_GPS_ReQual_Nav_Data(
   s_GN_GPS_Nav_Data *p_Nav_Data,   // io - Pointer to where the Nav Data to Re-Qualify
                                    //         should write the Nav Data to
   U2 H_AccReq,                     // i  - Horizontal position RMS Accuracy Requirement
                                    //          [metres],  0 = Use the Library default.
   U2 V_AccReq );                   // i  - Vertical position RMS Accuracy Requirement
                                    //          [metres],  0 = Use the Library default.


// GN GPS Utility Function to Encode NMEA 0183 compatible GPS sentences to a
// given string buffer based on a given GPS Navigation Solution data structure.
// This function may be used to provide different combinations of NMEA sentences
// to multiple end users.  Adjusting the "Valid_2D_Fix" and "Valid_3D_Fix" flags
// in the input GPS Navigation Solution data structure will correspondingly
// adjust the GPS Fix status flags in the NMEA sentence outputs.
// Returns the number of bytes of NMEA data written to the string buffer.
U2 GN_GPS_Encode_NMEA(
   U2 max_size,                     // i  - Maximum number of NMEA bytes to write
   CH *p_NMEA,                      //  o - Pointer to where the NMEA sentences
                                    //         should be written to
   U2 Encode_Mask,                  // i  - Mask stating which NMEA messages
                                    //         are to be Encoded (see below)
   s_GN_GPS_Nav_Data *p_Nav_Data ); // i  - Pointer to where the Nav Solution
                                    //         Data should be read from

#define GN_GPGLL_ENC_MASK  (1<<0)   // NMEA $GPGLL sentence Encode Mask
#define GN_GPGGA_ENC_MASK  (1<<1)   // NMEA $GPGGA sentence Encode Mask
#define GN_GPGSA_ENC_MASK  (1<<2)   // NMEA $GPGSA sentence Encode Mask
#define GN_GPGST_ENC_MASK  (1<<3)   // NMEA $GPGST sentence Encode Mask
#define GN_GPGSV_ENC_MASK  (1<<4)   // NMEA $GPGSV sentence Encode Mask
#define GN_GPRMC_ENC_MASK  (1<<5)   // NMEA $GPRMC sentence Encode Mask
#define GN_GPVTG_ENC_MASK  (1<<6)   // NMEA $GPVTG sentence Encode Mask
#define GN_GPZCD_ENC_MASK  (1<<7)   // NMEA $GPZDA sentence Encode Mask
#define GN_GPZDA_ENC_MASK  (1<<8)   // NMEA $GPZDA sentence Encode Mask
#define GN_PGNVD_ENC_MASK  (1<<9)   // NMEA $PGNVD sentence Encode Mask


//*****************************************************************************
// Callback Functions called by the GN GPS High Level Core Software library
// that need to be implemented by the Host platform software.

// GN GPS Callback Function to request that the host start uploading a Patch
// File to the GN GPS Baseband for the given ROM code version. The host must
// ensure that if the data in this file is split amongst several writes then
// this is done at a <CR><LF> boundary so as to avoid message corruption by the
// Ctrl Data also being sent to the Baseband.
// Returns the Patch Checksum, or zero if there is no Patch.
U2 GN_GPS_Write_GNB_Patch(
   U2 ROM_version,                  // i - Current GN Baseband ROM version
   U2 Patch_CkSum );                // i - Current GN Baseband Reported Patch
                                    //        Checksum

// GN GPS Callback Function to Read back the GPS Non-Volatile Store Data from
// the Host's chosen Non-Volatile Store media.
// Returns the number of bytes actually read back.  If this is not equal to
// 'NV_size' then it is assumed that the Non-Volatile Store Data read back is
// invalid and Time-To_First_fix will be significantly degraded.
U2 GN_GPS_Read_NV_Store(
   U2 NV_size,                      // i - Size of the NV Store Data [bytes]
   U1 *p_NV_Store );                // i - Pointer to where the host software
                                    //        should put the NV Store Data

// GN GPS Callback Function to Write the GPS Non-Volatile Store Data to the
// Host's chosen Non-Volatile Store media.
// Returns the number of bytes actually written.
void GN_GPS_Write_NV_Store(
   U2 NV_size,                      // i - Size of the NV Store Data [bytes]
   U1 *p_NV_Store );                // i - Pointer to where the host software
                                    //        can get the NV Store Data from

// GN GPS Callback Function to Get the current OS Time tick in integer
// millisecond units.  The returned 'OS_Time_ms' must be capable of going all
// the way up to and correctly wrapping a 32-bit unsigned integer boundary.
U4 GN_GPS_Get_OS_Time_ms( void );

// GN GPS Callback Function to Get the OS Time tick, in integer millisecond
// units, corresponding to when a burst of Measurement data received from
// GPS Baseband started.  This helps in determining the system latency.
// If it is not possible to determine this value then, return( 0 );
// The returned 'OS_Time_ms' must be capable of going all the way up to and
// correctly wrapping a 32-bit unsigned integer boundary.
U4 GN_GPS_Get_Meas_OS_Time_ms( void );

// GN GPS Callback Function to Read back the current UTC Date & Time data
// (eg from the Host's Real-Time Clock).
// Returns TRUE if successful.
BL GN_GPS_Read_UTC(
   s_GN_GPS_UTC_Data *p_UTC );      // i - Pointer to where the host software
                                    //        should put the UTC Date & Time

// GN GPS Callback Function to Write UTC Date & Time data to the Host platform
// software area,  which can be used by the Host to update its Real-Time Clock.
void GN_GPS_Write_UTC(
   s_GN_GPS_UTC_Data *p_UTC );      // i - Pointer to where the host software
                                    //        can get the UTC Date & Time from

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
   CH *p_GNB_Meas );                // i - Pointer to where the host software
                                    //        should put the GPS Baseband Raw
                                    //        Measurement data.

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
   CH *p_GNB_Ctrl );                // i - Pointer to where the host software
                                    //        can get the GPS Baseband Raw
                                    //        Measurement data from.

// GN GPS Callback Function to request that if possible the host should
// perform a Hard Power-Down Reset of the GN Baseband Chips.
// Returns TRUE is this is possible.
BL GN_GPS_Hard_Reset_GNB( void );


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
   CH *p_NMEA );                    // i - Pointer to where the host software
                                    //        can get the GPS NMEA data from.


// GN GPS Callback Function to Read $PGNV GloNav Propriatary NMEA Input
// Messages from the Host's chosen $PGNV communications interface.
// Internally a circular buffer to store this data.  Therefore, 'max_bytes'
// is dynamically set to prevent a single Read operation from straddling the
// internal circular buffer's end wrap point, or from over writing data that
// has not been processed yet.
// Returns the number of bytes actually read.  If this is equal to 'max_bytes'
// then this callback function may be called again if 'max_bytes' was limited
// due to the circular buffer's end wrap point.
U2 GN_GPS_Read_PGNV(
   U2  max_bytes,                   // i - Maximum number of bytes to read
   CH *p_PGNV );                    // i - Pointer to where the host software
                                    //        should put the $PGNV data.


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
   CH *p_GNB_Debug );               // i - Pointer to where the host software
                                    //        can get the GNB Debug data from.

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
   CH *p_Nav_Debug );               // i - Pointer to where the host software
                                    //        can get the Nav Debug data from.


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
   CH *p_Event_Log );               // i - Pointer to where the host software
                                    //        can get the Event Log data from.

//*****************************************************************************

#ifdef __cplusplus
   }     // extern "C"
#endif

#endif   // GN_GPS_API_H
