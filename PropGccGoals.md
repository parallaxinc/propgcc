# Propeller GCC Goals and Status #

### Project Goals ###
  * GCC will compile/link C and/or C++ programs
  * GCC will produce PASM-GAS COG code
  * Device drivers may also be written in PASM and combined with C/C++ programs
  * GCC will produce LMM (Large Memory Model) code programs
  * GCC will allow running programs on external memory platforms XMM/XMMC
  * GCC will allow debugging via GDB over the console serial port
  * GCC will generate optimized code based on size/speed
  * A flexible loader application will be used to program Propeller
  * GCC will allow users to specify loader parameters in a simple way
  * GCC will allow users to define their own drivers in a simple way
  * GCC will allow users to specify linker controlled memory areas
  * GCC will run on popular operating systems - Mac, Linux, and Windows
  * GCC should work with Eclipse, Net Beans, Code::Blocks, and other GUI programs
  * There will be limited versions of some C libraries because of Propeller constraints
  * GCC will be free and open source according to GNU licensing terms
  * MIT licensed demonstration programs will be provided
  * Volunteer tester application efforts will be supported and highlighted for achievement

### Project Status ###

  * GCC can compile/link C and/or C++ programs
  * GAS can produce PASM-GAS COG code
  * GCC allows in-line GAS code
  * Device drivers can be written in GAS/PASM and combined with C/C++ programs
  * GCC can generate COG C programs
  * GCC can generate LMM C programs
  * GCC can generate CMM C programs (Compact Memory Model)
  * GCC can generate XMMC C programs with code external memory
  * GCC can generate XMM C programs with code and data in external memory
  * GCC can generate All Model C programs that can start COGC COGS
  * GCC can generate All Model C programs that can start GAS COGS
  * GCC can generate All Model C programs that can start PASM COGS
  * GCC can generate optimized code based on size/speed
  * GCC lets users define their own device drivers in a simple way
  * Programs can be loaded and run on Propeller hardware with propeller-load
  * Predefined linker scripts are available; custom scripts can be used
  * Limited versions of some C libraries are provided
  * GCC meets all license requirements
  * GCC works with Eclipse, Netbeans, Codeblocks, and Geany IDEs
  * GCC works with SimpleIDE - available as an installer for Linux, MAC, and Windows

  * GDB debugging has been deferred by Parallax to the Propeller II project
