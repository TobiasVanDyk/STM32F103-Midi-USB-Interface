// pipesDef.h

// Used as pipeIDSet[pipeSet][n];   // Preset list of PipeID + 4 parameters 

byte pipeIDSet[9][5] = { 0, 0, 1, 8, 0,    // Exclude SysEx
                         0, 0, 1, 4, 0,    // Exclude Realtime
                         0, 2, 0, 0, 7,    // Midi Channel filter include Channels 0-7 only
                         0, 2, 1, 0, 7,    // Midi Channel filter exclude Channels 0-7 only
                         1, 0, 1, 0, 0,    // Transpose Up   1 semitone 
                         1, 1, 1, 0, 0,    // Transpose Down 1 semitone 
                         2, 0, 1, 7, 0,    // Channel Map: Map Ch1 to Ch7 
                         4, 0, 1, 7, 0,    // Control Change Map: Map CC1 to CC7 
                         0, 1, 14,8, 0 };  // Exclude  pitchBendStatus
