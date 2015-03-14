# Introduction #

The Propeller port of GCC supports a number of memory models, or ways of storing the program in memory. Basically these models provide a trade off of speed for code space. By far the fastest model is the native “cog” model (-mcog) in which machine instructions are executed directly. However, in that model only the 2K of internal memory (actually slightly less) is available for code. In the other models (LMM, XMMC, and XMM) code is stored in RAM external to the cog and is loaded in by a small kernel. This makes more space available for the code (and, in the XMM case, the data) but at the cost of having kernel overhead on each instruction executed.

# Details #

Please see: [Propeller-GCC Memory Models](http://propgcc.googlecode.com/hg/doc/Memory.html)