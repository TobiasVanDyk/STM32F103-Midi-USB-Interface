// keysDef.h
// Two sets of 5x5 25 Switches for Midi Xpoint switch + 5 Option keys
const static char Labels12[2][30][4] = 
{"a1", "a2", "a3", "a4", "a5", "Rt",  
 "b1", "b2", "b3", "b4", "b5", "Sl", 
 "c1", "c2", "c3", "c4", "c5", ">",
 "d1", "d2", "d3", "d4", "d5", "Pp",
 "e1", "e2", "e3", "e4", "e5", "Op", 
 "A1", "A2", "A3", "A4", "A5", "Rt",  
 "B1", "B2", "B3", "B4", "B5", "Sl", 
 "C1", "C2", "C3", "C4", "C5", ">",
 "D1", "D2", "D3", "D4", "D5", "Pp",
 "E1", "E2", "E3", "E4", "E5", "Op" }; 
 
char keyLabel12[30][4] = {};  // Layout 1 + 2: 25 5x5 XPoint switch + 4 Options keys and 1 Nav key
char keyLabel34[12][4] = {};  // Layout 3 + 4: 12 keys

// Two layout sets of 12 button labels <name>[<number-of-lables>][<number-of-chars-per-label]
// The number of chars per label include the termination char \0 => 3 character labels here
const static char Labels34[2][48][4] = 
{"S01", "S02", "S03", "Up", "S04", "S05", "S06", "Nxt", "S07", "S08", "S09", "Dwn",
 "S10", "S11", "S12", "Up", "S13", "S14", "S15", "Nxt", "S16", "S17", "S18", "Dwn",
 "S19", "S20", "S21", "Up", "S22", "S23", "S24", "Nxt", "S25", "S26", "S27", "Dwn",
 "S28", "S29", "S30", "Up", "S31", "S32", "S33", "Nxt", "S34", "S35", "S36", "Dwn",
 "T01", "T02", "T03", "Up", "T04", "T05", "T06", "Nxt", "T07", "T08", "T09", "Dwn", 
 "T10", "T11", "T12", "Up", "T13", "T14", "T15", "Nxt", "T16", "T17", "T18", "Dwn",
 "T19", "T20", "T21", "Up", "T22", "T23", "T24", "Nxt", "T25", "T26", "T27", "Dwn",
 "T28", "T29", "T30", "Up", "T31", "T32", "T33", "Nxt", "T34", "T35", "T36", "Dwn" };
