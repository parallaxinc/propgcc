## Introduction ##

If your goal is to run your C++ program on a classic Propeller-with-32K-EEPROM system, you need to use the PropGCC LMM memory model (and thus avoid the XMMC and XMM).  This article gives hints and tricks to help you make this possible.  The hints are listed in decreasing order by how much memory can be saved.

## Background ##

The C++ language originally was a very simple front end to C, and writing a program using C++ for its ability to group code into classes incurred very little CPU and storage overhead over writing an equivalent program in C.  If you use the subset of the C++ language that existed when C++ was first introduced, this is still true today.  However, over the years the C++ language evolved to add features that, when used, can add significant overhead to a C++ program.  This article describes these features, quantifies the overhead, and gives hints on how to avoid them.  In addition it describes general techniques to minimize the footprint of PropGCC programs written in both C and C++.

## Quick Summary ##

Don't use the -lstdc++.a -lsupc++.a or -lm libraries.  If you use the C++ heap, define your own operator new/delete.

Compile C++ using the following command:

```
  propeller-elf-gcc -mlmm -Os -mfcache -fno-exceptions -fno-rtti -Dprintf=__simple_printf -m32bit-doubles -c [your-file].cpp
```

Compile C using the following command:

```
  propeller-elf-gcc -mlmm -Os -mfcache -fno-exceptions -Dprintf=__simple_printf -m32bit-doubles -c [your-file].cpp
```

Link both C and C++ using the following command:

```
  propeller-elf-gcc -mlmm -m32bit-doubles -o [your-file].elf [your-objects]
```


## Technique 1: Use the C++ Library Cautiously ##

libstdc++ is not friendly to embedded systems; many of the classes in it are very large.  For example, consider the standard "Hello World" program found at the start of many C++ manuals:

```
  main.cpp:

  #include <iostream>
  main()
  {
      std::cout << "Hello World\n";
  }
```

Notice what happens when you compile and link it:

```
  > propeller-elf-g++ -mlmm -o main.elf main.cpp
  propeller-elf/bin/ld.exe: main.elf section `.text' will not fit in region `hub'
  propeller-elf/bin/ld.exe: region `hub' overflowed by 501608 bytes
  collect2: ld returned 1 exit status
```

The `<`iostream`>` library will just not fit into the LMM memory space.

There are a few classes in the standard library which you may find useful that will typically fit into a LMM program; for example, the _`<`complex`>`_ complex number class.

However, if you want to play it safe and avoid the Standard C++ Library a general rule is to be wary of all libraries that use system header files with no ".h" extension, such as _`<`hash\_set`>`_.

In addition, you can compile with gcc instead of g++.  Here's a quick explanation about that:

> The bin/ directory of the PropGCC distribution includes a command named _propeller-elf-gcc_, hereafter called gcc.  It also contains the _propeller-elf-g++_ and _propeller-elf-c++_ commands, which behave identically and are hereafter called g++.  There are three differences between these commands:

  1. The gcc command can compile both C and C++ source, and treats files with the .c, .h, and (preprocessed) .i extension as C source while files, while it treats the .C, .cc. .cpp. .CPP, .c++, .cp, .cxx, .hh, .H, .hp, .hxx, .hpp, .HPP, .h++ and (preprocessed) .ii extensions as C++ source.  The g++ treats all files (including those with the .c, .h, and .i) extensions as C++ source.
  1. The gcc command does not link with the standard C++ libraries by default while the g++ command does.  The g++ command also seems to link with the math library (-lm), which is big.
  1. This is a minor point, but the g++ command also enables some language-specific code-generation conventions that affect linking; for example: -fcommon.

> By compiling with gcc, you completely avoid the libstdc++.a and libsupc++.a libraries.  Any use of these libraries will result in "undefined reference" errors during linking, so you can then identify the routines being used and decide what to do about them.


## Technique 2: Avoid exceptions, compile with the _-fno-exceptions_ flag ##

The use of exceptions in C++ is somewhat controversial; some people think they're great, others think they're the exact opposite.  No matter which opinion you hold, the library support for PropGCC's exception handling will not fit in LMM.  Consider this example:

```
  main.cpp:

  #include <stdio.h>
  main()
  {
      try
      {
          puts("a");
      }
      catch (...)
      {
          puts("b");
      }
  }
```

If you compile and link this under LMM mode using, you get an overflow:

```
  > propeller-elf-g++ -mlmm -o main.elf main.cpp
  propeller-elf/bin/ld.exe: main.elf section `.text' will not fit in region `hub'
  propeller-elf/bin/ld.exe: region `hub' overflowed by 74384 bytes
  collect2: ld returned 1 exit status
```

PropGCC will generate exceptions handling code in unexpected ways.  Consider the following two program files:

```
  main.cpp:

  class MyClass
  {
  public:
     ~MyClass();
  };
  extern void foo();

  int main()
  {
      MyClass bar;
      foo();
  }
```

```
  myclass.cpp:

  class MyClass
  {
  public:
     ~MyClass();
  };

  MyClass::~MyClass() { }

  void foo()
  {
  }
```

Watch what happens when you compile them:

```
  >propeller-elf-g++ -mlmm -o main.elf main.cpp myclass.cpp
  propeller-elf/bin/ld.exe: main.elf section `.text' will not fit in region `hub'
  propeller-elf/bin/ld.exe: region `hub' overflowed by 74288 bytes
  collect2: ld returned 1 exit status

  or

  >propeller-elf-gcc -mlmm -o main.elf main.cpp myclass.cpp
  ccFP5E1g.o: In function `_main':
  (.text+0x1c): undefined reference to `___gxx_personality_sj0'
  collect2: ld returned 1 exit status
```

The reason is: PropGCC is unsure if the subroutine _foo()_ is going to call an exception, so it generates exception handling and stack unwinding code to make sure that the _MyClass bar_ object's destructor is always called in the _main()_ routine, even if an exception occurs.  Note that the reason this example was split into two files is that if you place them into one file then PropGCC is smart enough during optimization to know that _foo()_ won't throw an exception.  However, this optimization won't help you if you structure your code "correctly" by placing all your classes into separate files and breaking up the rest your code into reasonable functional units.

You could solve this problem by decorating all your routine declarations with a _throw()_ modifier, indicating that your routines do not throw exceptions.  This is messy.  A better way is to add the _-fno-exceptions_ optimization option to all your compilation lines.

If you include the _-fno-exceptions_ option to your compile lines, then PropGCC will do two things:

  1. It will generate errors when it encounters the _try_, _catch_, and _throw_ statements in a program
  1. It will ignore _throw_ modifiers in function declarations and assume your functions do not throw exceptions.

Here is an example of compiling the above program with -fno-exceptions:
```
  > make all
  propeller-elf-gcc -mlmm -fno-exceptions -o main.o -c main.cpp
  propeller-elf-gcc -mlmm -fno-exceptions -o myclass.o -c myclass.cpp
  propeller-elf-gcc -mlmm -o main.elf main.o myclass.o 
  > ...notice that no errors occur!...
```

There is no need to add -fno-exceptions it to the command that links your executables, because it will do nothing.  In other words, it will not prevent the linker from grabbing the exception handling code for any file that already has exception code (because it was not compiled with -fno-exceptions).  Note that once again the Standard C++ Library is problematic because it was not compiled with -fno-exceptions, and also has explicit use of try/catch/throw.  Please see next technique for why this matters...


## Technique 3: Override _operator new()_ and _operator delete()_  When Using the _new_ and _delete_ Keywords ##

Embedded programs need to use dynamic memory allocation very cautiously because of the obvious concerns:

  * An extremely limited heap size
  * Without extreme care, allocating and deleting a lot of objects can easily lead to a fragmented (and therefore useless) heap

Therefore, many high-reliability embedded systems avoid stack- and dynmically-allocated objects and only use statically-allocated objects.  Nonetheless, there are many instances where dynamically-allocated objects are useful (when programmed with care).

In C++, you use the _new_ and _delete_ operators to allocate/free storage instead of _malloc()/free()_.  And, you can also create/destroy a dynamically-allocated instance of a class using the _new_ and _delete_ operators.  For example:

```
  main.cpp:

  class MyClass
  {
  private:
      int x;

  public:
      MyClass() { x = 0; }
      ~MyClass() {}
  };

  int main()
  {
      MyClass* m1 = new MyClass();
      delete m1;
      char* p = new char[1000];
      delete p;
  }
```

When you compile and link this program, you get the following errors:

```
  >>propeller-elf-g++ -mlmm -fno-exceptions -o main.elf main.cpp
  propeller-elf/bin/ld.exe: main.elf section `.text' will not fit in region `hub'
  propeller-elf/bin/ld.exe: region `hub' overflowed by 75180 bytes
  collect2: ld returned 1 exit status

  >propeller-elf-gcc -mlmm -fno-exceptions -o main.elf main.cpp
  ccYaL1Dx.o: In function `_main':
  (.text+0x84): undefined reference to `operator new(unsigned int)'
  ccYaL1Dx.o: In function `_main':
  (.text+0xcc): undefined reference to `operator delete(void*)'
  ccYaL1Dx.o: In function `_main':
  (.text+0xe4): undefined reference to `operator new[](unsigned int)'
  ccYaL1Dx.o: In function `_main':
  (.text+0x100): undefined reference to `operator delete(void*)'
  collect2: ld returned 1 exit status
```

The overflow occurs because the standard _operator new()_ routines in _libstdc++.a_ operate with exceptions, and (as mentioned in the previous section) exception handling code does not fit in the LMM memory model.  Note that even the "nothrow" version of _operator new()_ still contains exception handling code.

The fix is to define your own _operator new()_ and _operator delete()_ in your code:

```
  c++-alloc.cpp:

  #include <cstdlib>
  #include <new>

  using std::new_handler;
  new_handler __new_handler;

  void *
  operator new (std::size_t sz)
  {
    void *p;

    /* malloc (0) is unpredictable; avoid it.  */
    if (sz == 0)
      sz = 1;
    p = (void *) malloc (sz);
    while (p == 0)
      {
        new_handler handler = __new_handler;
        if (! handler)
          std::abort();
        handler ();
        p = (void *) std::malloc (sz);
      }

    return p;
  }

  void*
  operator new[] (std::size_t sz)
  {
    return ::operator new(sz);
  }

  void
  operator delete(void* ptr)
  {
    if (ptr)
      std::free(ptr);
  }

  new_handler
  std::set_new_handler (new_handler handler)
  {
    new_handler prev_handler = __new_handler;
    __new_handler = handler;
    return prev_handler;
  }
```

Compiling both ways now works:

```
  >propeller-elf-gcc -mlmm -fno-exceptions -o main.elf main.cpp c++-alloc.cpp
  >propeller-elf-g++ -mlmm -fno-exceptions -o main.elf main.cpp c++-alloc.cpp
```


## Technique 4: Don't use _printf_; Instead Use _`__`simple`_`printf'_. ##

The printf() routine takes 14K; enough said.  If you need to use printf, and can afford certain limitations (see [the library documentation](http://propgcc.googlecode.com/hg/doc/Library.html#printf)); the `__`simple`_`printf routine takes only 7K.

The easist way to use `__`simple`_`printf is to include the following compile flag:  -Dprintf=`__`simple`_`printf


## Technique 5: Don't Use Floating Point, or If You Use Floating Point, Use 32-bit Instead of 64-bit ##

The PropGCC support for floating point takes a lot of space, particularly for 64-bit floating point numbers.  It is much better to avoid floating point.  If you need to use floating point, then use the 32-bit _float_ keyword instead of the 64-bit _double_ keyword.  The problem isn't the fact that the numbers themselves take 64-bits (8 bytes) intead of 32-bits (4 bytes).  The problem is that the Propeller doesn't directly support floating point mathematical operations and therefore all the math operations are performed using library routines; these routines are large:

| **Routine** | **Additional** | **Additional** |
|:------------|:---------------|:---------------|
|  | **32-bit Size** | **64-bit Size** |
| Math (add, subtract, multiply, divide) | 5.5K | 11K |
| printf (via the -lm linker flag) | 5K | 10K |

Note that if you need printf() with floating point support, you must include the _-lm_ flag in the link command.  However, for 32-bit-only floating point support in printf(), it's not enough to just use _float_ intead of _double_.  Instead, compile _and_ link using the _-m32bit-doubles_ flag:

```
  propeller-elf-gcc -mlmm -m32bit-doubles -c float.c
  propeller-elf-gcc -mlmm -m32bit-doubles -o float.elf float.o -lm
```

The -m32bit-doubles flag tells the compiler to make the _double_ keyword 32-bits long, just like the _float_ keyword.  The -m32bit-doubles flag tells the linker to link with 32-bit-only libraries found in the _short-doubles_ directories of your PropGCC distribution.


## Technique 6: Use Spin Objects ##

Compiled C/C++ code is often less "dense" than compiled Spin code.  For example, the files fsrw.c and fsrw.spin from SD Card Routines in the [Parallax Object Exchange](http://obex.parallax.com/) contain identical functionality.  Compiling them yields the following approximate sizes:

| Language | Code Bytes | Data Bytes |
|:---------|:-----------|:-----------|
| Spin | 2600 | 1100 |
| C | 6700 | 1100 |

Notice that the data size for each is the same (as would be expected), but the C code compiles to 2.5 times the size of the Spin code.  (Paradoxically, C code typically runs 2.5 or more times faster than Spin code.)

If an object is already available in Spin and you can afford to give up an extra cog, you can save memory by running the Spin object instead of translating it to C.  Note that the Spin interpreter is already in the Propeller's internal ROM, so there's no overhead for using it.  However, you have to do a bit of work to interface your C and Spin code.

The fsrw SD card routines are a great example of this.  PropGCC's built-in SD card code is about the same size as the fsrw C code - around 7K.  Using the fsrw Spin object saves over 3K.

... Details TBD ...


## Technique 7: Use the _-Os_ Optimization Flag ##

When a computer's execution system includes caching, to get the fastest code it is often more desirable to optimize programs for size rather than speed.  This is true for PropGCC.  To see why, consider that a XMM program can run anywhere from 1 to 70 times slower than an equivalent LMM program (depending on how much of your program runs in tight loops cached by fcache).  If you care about speed, it's typically in your interest to have your program fit in LMM.  Therefore, use the "-Os" option when compiling your code.

Note: After getting your program to fit into LMM, if you need extra speed, you may want to compile individual functions with -O2 or -O3, or else decorate individual functions with something like the _`__`attribute`__`((optimize("-O2")))_ attribute to enable local optimization.

Additionally, you may wish to play around with other compiler flags that deal with code generation.  Here are some ideas of flags that may affect code size:

| -O1 |
|:----|
| -O3 |
| -fshort-enums |
| -fno-default-inline |
| -fno-implicit-inline-templates |
| -fno-implement-inline |
| -fno-threadsafe-statics |


## Technique 8: Use _-fno-rtti_ to Disable Run Time Type Information (RTTI) ##

PropGCC supports a _Run Time Type Information_ (RTTI) system that supports two features in the C++ language: _dynamic\_cast<>_ and _typeid_.  If you have any classes that include one or more virtual methods, then PropGCC will include extra information to support this.

If you compile by adding the -fno-rtti flag, then PropGCC will not add this type information, and therefore the _dynamic\_cast<>_ and _typeid_ features will not function.  This saves you a small amount of space: (10 + the length of the class name) bytes for each class with any virtual method.


## Technique 9: Use _-mno-fcache_ Instead of _-mfcache_ ##

This is a last-resort.  The _-mfcache_ flag allows the PropGCC runtime system to copy the code for loops to a cache and execute it from there.  Using the _-mno-fcache_ flag will make your code smaller by about 12 bytes per loop.  However, your tight loops will run several times slower because they will not be cached.

## What's Left ##

Given all this, it may seem unwise to use any C++ functionality.  However, an LMM program can easily use C++ features such as inheritance, virtual functions, and the heap as long as the program uses the features carefully and you keep an eye on memory usage.  See the PropGccMaps page for information on how to easily keep track of memory usage.