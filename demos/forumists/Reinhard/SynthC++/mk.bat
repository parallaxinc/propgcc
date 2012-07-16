propeller-elf-gcc -mlmm -o Synth.o  -c Synth.cpp


propeller-elf-gcc -mlmm -o FrequencySynth.elf  FrequencySynth.cpp  Synth.o  


propeller-load -r FrequencySynth.elf  
