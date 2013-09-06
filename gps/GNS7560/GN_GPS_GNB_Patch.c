
//*****************************************************************************
// GPS IP Centre, ST-Ericsson (UK) Ltd.
// Copyright (c) 2009 ST-Ericsson (UK) Ltd.
// 15-16 Cottesbrooke Park, Heartlands Business Park, Daventry, NN11 8YL, UK.
// All rights reserved.
//
// Filename  GN_GPS_GNB_Patch.c
//
// $Header: S:/GN_GPS_Nav_MKS/Arch/GNU/_GN_GPS/rcs/GN_GPS_GNB_Patch.c 1.6 2009/04/23 15:02:35Z geraintf Exp $
// $Locker: $
//*****************************************************************************


//*****************************************************************************
//
// Example utility functions relating to Uploading the GN Baseband Patch code.
//
//*****************************************************************************


#include "gps_api.h"



//*****************************************************************************
// Define the Patch Upload function pointer. This will point to the correct
// upload function for the appropriate ROM version
static void (*ptr_GN_Upload_GNB_Patch)( U4 Max_Num_Patch_Mess ) = NULL;


//*****************************************************************************
// The following are parameters relating to uploading a patch file
// to the baseband chip.

U1  gn_Patch_Status;                // Status of GPS baseband patch transmission
//U1  gn_Patch_Progress;              // % progress of patch upload for each stage
U2  gn_Cur_Mess[5];                 // Current messages to send, of each 'stage'


//*****************************************************************************
// Prototypes for external GNS???? chip & ROM specific patch upload functions.

#ifdef GN_GPS_GNB_PATCH_205         // For GNS4540 ROM2 v205
   U2 GN_GetPatchCheckSum_205( void );
   void GN_Upload_GNB_Patch_205(
      U4 Max_Num_Patch_Mess );     // i  - Maximum Number of Patch Messages to Upload
#endif

#ifdef GN_GPS_GNB_PATCH_301         // For GNS4540 ROM3 v301
   U2 GN_GetPatchCheckSum_301( void );
   void GN_Upload_GNB_Patch_301(
      U4 Max_Num_Patch_Mess );     // i  - Maximum Number of Patch Messages to Upload
#endif

#ifdef GN_GPS_GNB_PATCH_502         // For GNS7560 ROM5 v502
   U2 GN_GetPatchCheckSum_502( void );
   void GN_Upload_GNB_Patch_502(
      U4 Max_Num_Patch_Mess );     // i  - Maximum Number of Patch Messages to Upload
#endif

#ifdef GN_GPS_GNB_PATCH_506         // For GNS7560 ROM5 v506
   U2 GN_GetPatchCheckSum_506( void );
   void GN_Upload_GNB_Patch_506(
      U4 Max_Num_Patch_Mess );     // i  - Maximum Number of Patch Messages to Upload
#endif

#ifdef GN_GPS_GNB_PATCH_510         // For GNS7560 ROM5 v510
   U2 GN_GetPatchCheckSum_510( void );
   void GN_Upload_GNB_Patch_510(
      U4 Max_Num_Patch_Mess );     // i  - Maximum Number of Patch Messages to Upload
#endif

//*****************************************************************************
// Upload the next set GNS Baseband Patch messages

void GN_Upload_GNB_Patch(
   U4 Max_Num_Patch_Mess )    // i  - Maximum Number of Patch Messages to Upload
{
   // Call the correct Upload function via the Patch Upload function pointer
   (*ptr_GN_Upload_GNB_Patch)(Max_Num_Patch_Mess);

   return;
}


//*****************************************************************************
// This function is called from the GN GPS Library 'callback' function
// GN_GPS_Write_GNB_Patch() to start / set-up the GN Baseband Patch upload.

U2 GN_Setup_GNB_Patch( U2 ROM_version, U2 Patch_CkSum )
{
   U2 ret_val; // Return value

   ret_val = 0;

   // Get the patch checksum for the appropriate patch, and assign the address
   // of the correct Upload function to the Patch Upload function pointer

#ifdef GN_GPS_GNB_PATCH_205
   if ( ROM_version == 205 )                    // GNS4540 ROM2 v205
   {
      ret_val = GN_GetPatchCheckSum_205();
      ptr_GN_Upload_GNB_Patch = &GN_Upload_GNB_Patch_205;
   }
   else
#endif
#ifdef GN_GPS_GNB_PATCH_301
   if ( ROM_version == 301 )               // GNS4540 ROM3 v301
   {
      ret_val = GN_GetPatchCheckSum_301();
      ptr_GN_Upload_GNB_Patch = &GN_Upload_GNB_Patch_301;
   }
   else
#endif
#ifdef GN_GPS_GNB_PATCH_502
   if ( ROM_version == 502 )               // GNS7560 ROM5 v502
   {
      ret_val = GN_GetPatchCheckSum_502();
      ptr_GN_Upload_GNB_Patch = &GN_Upload_GNB_Patch_502;
   }
   else
#endif
#ifdef GN_GPS_GNB_PATCH_506
   if ( ROM_version == 506 )               // GNS7560 ROM5 v506
   {
      ret_val = GN_GetPatchCheckSum_506();
      ptr_GN_Upload_GNB_Patch = &GN_Upload_GNB_Patch_506;
   }
   else
#endif
#ifdef GN_GPS_GNB_PATCH_510
   if ( ROM_version == 510 )               // GNS7560 ROM5 v510
   {
      ret_val = GN_GetPatchCheckSum_510();
      ptr_GN_Upload_GNB_Patch = &GN_Upload_GNB_Patch_510;
   }
   else
#endif
   {
      LOGGPS( "GN_Setup_GNB_Patch: No patch available for ROM %3d.", ROM_version );
      return( 0 );
   }

   // Set the patch status to indicate that the upload should start.
   gn_Patch_Status   = 1;
//   gn_Patch_Progress = 0;
   gn_Cur_Mess[0]    = 0;
   gn_Cur_Mess[1]    = 0;
   gn_Cur_Mess[2]    = 0;
   gn_Cur_Mess[3]    = 0;
   gn_Cur_Mess[4]    = 0;

   // Check that the patch available is not already uploaded.
   if ( ret_val == Patch_CkSum )
   {
      LOGGPS( "GN_Setup_GNB_Patch: Patch available already in use." );
      gn_Patch_Status   = 0;              // Do not start upload again
   }

   return( ret_val );
}

//*****************************************************************************
