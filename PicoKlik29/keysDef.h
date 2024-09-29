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
 "C01", "C02", "C03", "Up", "C04", "C05", "C06", "Nxt", "C07", "C08", "C09", "Dwn", 
 "C10", "C11", "C12", "Up", "C13", "C14", "C15", "Nxt", "C16", "C17", "C18", "Dwn",
 "C19", "C20", "C21", "Up", "C22", "C23", "C24", "Nxt", "C25", "C26", "C27", "Dwn",
 "C28", "C29", "C30", "Up", "C31", "C32", "C33", "Nxt", "C34", "C35", "C36", "Dwn" };
