Hi..

This directory contains the working files used to build the map
which lets us convert from battery thermistor voltage readings
to battery temperature, the desired reading.

To understand how this works, a little background helps:  each
Odyssey battery pack has a single thermistor temperature probe,
probably glued onto a single cell in the pack.

Thermistor Info
---------------

A thermistor is a resistive device whose resistance varies with
applied temperature in a repeatable way.  The part we're using
is made by a company called Quality Thermistor (QT).  It is one
of QT's "negative temperature coefficient" models, and conforms
to their "curve Z" characteristics.  Its nominal resistance at
25 degrees C is 10000 ohms.

"curve Z" describes the mapping from temperature to thermistor
resistance.  This mapping is rather non-linear, which is why this
directory exists.  The raw "curve Z" data describes the expected
resistance for temperatures at 5 degree (C) intervals.  Note that
the resistance in the "curve Z" data is actually expressed as the
ratio of the part's true resistance at a given temperature ("RT")
to its nominal resistance at 25 degrees C (R25).  QT does this
because they have thermistors with different R25 values, but whose
temperature responses conform to the same relative curve.

Thermistor Use
--------------

What the EVC actually measures when reading the thermistor value
is a voltage.  In the Odyssey, the EVC's A/D voltage converter
is reading the voltage across the thermistor itself.  However, the
thermistor is placed in series with a 10000 (10k) ohm resistor ("RD"),
to form a voltage divider.  A reasonably accurate reference voltage
("Vref") of 2.45 volts is applied to this divider.

Thus, the actual voltage read by the EVC from across the thermistor
is described by this equation:

        Vad = Vref * RT / (RT + RD)

where RT is derived from Quality Thermistor's "curve Z" [RT/R25]
data by

        RT = [RT/R25] * R25

Recall that R25 is 10000 ohms for the part used in the Odyssey batteries.

The two equations above are computed on the raw QT "curve Z" data by the
Excel spreadsheet CurveZ.xls.  But recall that QT provide their data in
five degree increments.  So the results of the spreadsheet are saved as
CurveZ.csv ("comma-separated value" format), and then run through an Awk
script which performs a simple linear interpolation to obtain a table of
temperature and Vad pairs with one degree increments.  Note that after
exporting the CSV file from Excel, it is necessary to hand-edit it to
remove the header lines.  The awk script only wants to see numeric data.
After editing the CSV, run MakeCpp.bat, or the following command:

        gawk -f Interpolate.awk CurveZ.csv >table.cpp

To keep things simple and integer-ish, the awk script scales the Vad
values from the CSV value by multiplying them by 1,000,000.  Vad in the
final table is thus in units of microvolts (uV).

The resulting table output is integrated as aThermistorMap[] in
..\CtlPollEnvironment.cpp.

