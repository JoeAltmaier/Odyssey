#
#  Interpolate.awk
#
#     This file contains a little Awk program which expands the .csv
#     form of CurveZ.xls from a 5 degree resolution (as given in the
#     Quality Thermistor "curve Z" data) into a one degree resolution
#     map from detected thermistor voltage to inferred temperature.
#     See readme.txt in this directory for the particulars.
#
#     If you re-export the CSV file from CurveZ.xls, be sure to strip
#     header lines from the CSV file before running this awk script
#     on it.
#
#
#  $Log: $
#

#  since we're scanning a CSV file, make field separator be a comma:
BEGIN  {
         FS = ",";

#  do a few more useful things:
         FALSE = (1 == 0);
         TRUE = (! FALSE);
         RowSeen = FALSE;

#  scale constant used to integer-ize Vad (A/D voltage input, from divider)
         VadScale = 1000000;

#  what our output row format looks like (printf-style):
#  we can change this to generate nice human-readable output,
#  or something a bit more suited to C++ compilers.
         DisplayHuman = "%d\t%u\n";
         DisplayCpp   = "{  %3d, %7u  },\n";

##         RowFormat = DisplayHuman;
         RowFormat = DisplayCpp;
       }

#  now process each record:
 {

#  read current row data
   CurTemp = $1;
   CurVadScaled = $8 * VadScale;

#  decide whether we're doing our first-ever row, or some subsequent one:
   if (! RowSeen)  {
#     first-ever row, and we don't have anything to do, except save it in
#     our standard place at the bottom of this action.
   }  else  {
#     got a previous row, so start filling in the gaps

#     interpolate between previous & current rows
#     (note that our loop structure also outputs the raw previous row)

      DeltaTemp = CurTemp - PrevTemp;
      DeltaVadIncr = (CurVadScaled - PrevVadScaled) / DeltaTemp;

      for (i = 0;  i < DeltaTemp;  i ++)  {
         printf(RowFormat, PrevTemp + i, PrevVadScaled + i * DeltaVadIncr);
      }
   }

#  done with current row, save it as previous for next row
   PrevTemp = CurTemp;
   PrevVadScaled = CurVadScaled;

   RowSeen = TRUE;
}


#  finally, dump out most recently-read row's own data:
END  {
      printf(RowFormat, PrevTemp, PrevVadScaled);
   }

