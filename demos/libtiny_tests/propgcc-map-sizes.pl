#!/usr/bin/perl
#
# Perl filter to transform a PropGCC linker map to a more readable format
#
# Copyright (c) 2012, Ted Stefanik
# See end of file for terms of use.

#
# The .map file output by the linker is difficult to read for several reasons:
# (1) it contains these really long, really noisy file names with longer-than-full
#     paths, because the paths include backtracking in the form of ../../..
# (2) the lengths of objects are in hex
# (3) individual routines have an address but no calculated lengths
# (4) nothing rolls up the individual object files costs in each section to give you
#     a total cost for a each object file.
# (4) C++ routine names are mangled.
#
# This script fixes the first four problems, using it in conjunction with
# propeller-elf-c++filt fixes the last.  For example:
#
# propeller-elf-g++ -Os -mfcache -mlmm -fno-exceptions -fno-rtti -Xlinker -Map=main.rawmap -o main.elf main.cpp
# propeller-elf-c++filt --strip-underscore < main.rawmap | perl propgcc-map-sizes.pl > main.map
#
# Note that this script also calculates the total size for each section.  You
# can get this using the "propeller-elf-size -A" command.  However, that
# command always includes the debugging information in the totals, which makes
# it difficult to use.
#
# This script always prints the section totals on STDERR at the end of the
# script.  It does this because you if you're compiling using make, you
# probably want to include this script in the commands you use to do a link,
# and it's very convenient if you see a "current size" report every time you
# link.
#
# This script is written using an "old school" Perl style.  Apologies in
# advance for the quick-and-dirty approach.
#
# This script has been tested with all four memory models (COG, LMM, XMM, XMMC)
# on both Linux and Windows.  Report any problems to the forum.
#

sub max ($$) { return $_[$_[0] < $_[1]]; }

#
# Handle the "Archive member included because of file(symbol)" section of
# the map by making the file names less noisy.
#
$_ = "";
do
{
    # Collapse things like c:/users/ted/documents/propgcc/propgcc-0.2.2/bin/../lib/gcc/propeller-elf/4.6.1/_crt0.o and
    # Collapse things like c:/users/ted/documents/propgcc/propgcc-0.2.2/bin/../lib/gcc/propeller-elf/4.6.1/../../../../propeller-elf/lib\libstdc++.a
    # down to _crt0.0 and libstdc++a.
    s/([a-z]:)?\/.*[\\\/]([^.]*\.[oa])/\2/;

    print;
} while(($_ = <>) !~ /(Allocating common symbols|Memory Configuration)/i);

#
# Handle the "Allocating common symbols" section of the map by making the
# file names less noisy and also by displaying sizes in both hex and decimal.
#
if ($_ !~ /Memory Configuration/i)
{
    $total = 0;
    $blankLineCount = 0;
    do
    {
        # See above for documentation on what this does.
        s/([a-z]:)?\/.*[\\\/]([^.]*\.[oa])/\2/;

        # The "Allocating common symbols" section ends after the second blank line.
        $blankLineCount++
            if (/^\s*$/);

        # Realign the column header (because we're inserting decimal sizes)
        s/    size/               size/;

        # Insert a decimal conversion of each hex size
        if (/ 0x/)
        {
            $hex = substr($_, 20, 10);
            $dec = hex($hex);
            substr($_, 20, 10) = sprintf("%10d ", $dec) . $hex;
            $total += $dec;
        }

        print;
    } while(($_ = <>) && $blankLineCount < 2);

    # Add a running total of the common symbols
    printf("%20s%10d 0x%x\n\n", "Total:", $total, $total);
    
}

#
# Handle the "Memory Configuration" and the top part of the "Linker script and
# memory map" sections by making the file names less noisy; these sections end
# at the line starting with ".boot".
#
do
{
    # See above for documentation on what this does.
    # Note that we add the "(library)" markup for library files to make it a
    # bit more obvious where the files came from.
    s/([a-z]:)?\/.*[\\\/]([^.]*\.[oa])/(library) \2/;

    print;
} while(($_ = <>) !~ /^\.boot/);


#
# Handle the memory map part of the "Linker script and memory map" section of
# the map by making the file names less noisy and also by displaying sizes in
# both hex and decimal.  The section ends at the line starting with ".hash".
#
@lines = ();
$lastAddr = 0;
$lastAddrLine = -1;
# Print a suitable header
print "Symbol          Address    Length   File-Length   Symbol-or-file\n";
do
{
    # See above for documentation on what this does.
    s/([a-z]:)?\/.*[\\\/]([^.]*\.[oa])/\2/;

    # When the line contains two hex numbers, the second is the total
    # length of a linked-in object, so we convert it to decimal and insert it.
    if (/0x.*0x/)
    {
        $hexLen = substr($_, 27);
        $hexLen =~ s/\s+//;
        $decLen = hex($hexLen);
        substr($_, 37, 0) = sprintf(" (%d) ", $decLen);

        # This next block of code saves section and object length
        # information for the summary at the end.  Note that we
        # also keep "maxLength" information so we can print pretty
        # titles.
        if (/^\.(\w+)/ || $previousSection ne '')     # We found a section head
        {
            $currentSection = ($1 ne '') ? $1 : $previousSection;
            $previousSection = '';
            push(@sections, $currentSection);
            $sectionLengths{$currentSection} = $decLen;
            $maxSectionNameLen = max($maxSectionNameLen, length($currentSection));
        }
        elsif (!/ALIGN \(0x[0-9a-z]+\)/i && /\s+(LONG\s+0x[0-9a-z]+(\s+\S+)?|\S+)$/i) # We found an object file
        {
            $objectName = $1;
            $objectName = "*fill*"
                if (/^\s+\*fill\*/i);

            $maxObjectNameLen = max($maxObjectNameLen, length($objectName));

            if (!defined($objectLengths{$objectName}))
            {
                push(@objects, $objectName);
                $objectLengths{$objectName} = {};
            }
            $objectLengths{$objectName}{$currentSection} += $decLen;
            $maxLengthLen = max($maxLengthLen, length("$objectLengths{$objectName}{$currentSection}"));
        }
    }
    elsif (/^\.(\w+)/)
    {
        $previousSection = $1;
    }

    # Tuck the line into an array of lines we're saving
    $thisLine = $#lines + 1;
    $lines[$thisLine] = $_;

    # If the line has any hex address, then it's a function or other
    # symbol that has an intrinsic length.
    if (/ 0x/)
    {
        # Calculate the current address (in decimal)
        $hexAddr = substr($_, 16, 10);
        $decAddr = hex($hexAddr);

        # Insert space for a length
        substr($lines[$thisLine], 26, 0) = ' ' x 8;

        # We now have enough information to calculate the length of the symbol
        # at the previous address.  After calculating the length, we insert it
        # (in decimal form) into the line that that had the previous address.
        # Note: If this line is a new section, we really can't make a calculation
        # for the last line because the new section could be at a wildly different
        # address.
        $diff = $decAddr - $lastAddr;
        substr($lines[$lastAddrLine], 26, 8) = sprintf("%7d ", $diff)
            if ($lastAddrLine != -1 && !/^\.\w+/);

        # Record this line's address for the above calculation (when the next
        # symbol comes along).
        $lastAddrLine = $thisLine;
        $lastAddr = $decAddr;
    }

} while(($_ = <>) !~ /__heap_start/);

# Print out the stored lines (now including the calculated lengths)
$" = '';
print @lines;
print $_;


#
# Print the rest of the file, but shorten file names.
#
while(<>)
{
    # See above for documentation on what this does.
    s/([a-z]:)?\/.*[\\\/]([^.]*\.[oa])/\2/;

    print;
}

#
# Print out the object summary.
#
print "\nObject Summary\n\n";
$format = sprintf("%%-%ds ", $maxObjectNameLen);

# Print out the section header line.
# The object name column is variable-width, based on the longest object name.
$header = sprintf($format, "Object");

# The length columns are variable-width, based on the longest length name;
# the minimum is 6 columns/places (decimal).
$maxLengthLen = max($maxLengthLen, 6);
$headerFormat = sprintf("  %%%d.%ds", $maxLengthLen, $maxLengthLen);

# Now print the header line showing each section
for ($j = 0;   $j <= $#sections;  $j++)
{
    $section = $sections[$j];

    # Shorten certain long section names.
    $section =~ s/[lx]mm.?(kernel)/\1/i;
    $section =~ s/gcc_(except)_table/\1/i;

    $header .= sprintf($headerFormat, $section);
}
$header .= sprintf($headerFormat, 'Total');
print "$header\n";

# Print out the object lengths in for each section, and the total.
$lenFormat = sprintf("  %%%dd", $maxLengthLen);
$zeroFormat = ' ' x ($maxLengthLen + 1) . '-';
for ($i = 0;   $i <= $#objects;  $i++)
{
    printf($format, $objects[$i]);

    $total = 0;
    for ($j = 0;   $j <= $#sections;  $j++)
    {
        $len = $objectLengths{$objects[$i]}{$sections[$j]};
        if ($len)
        {
            printf($lenFormat, $len);
        }
        else
        {
            print $zeroFormat;  # Don't make the table noisy with zeros.
        }
        $totals[$j] += $len;
        $total += $len;
    }

    $totals[$#sections + 1] += $total;
    printf($lenFormat . "\n", $total);
}

# Print out the grand totals of each section length
printf($format, "Total:");
for ($j = 0;   $j <= $#sections + 1;  $j++)
{
    printf($lenFormat, $totals[$j]);
}
print "\n";

#
# Print out the section summary.  This is basically the same information
# listed in "section totals" above, but with two differences:
#
# 1) It uses information directly taken from the map, whereas the "section
#    totals" uses data calculated from individual object lengths in the map.
#    Thus, if there are discrepencies between the two, then this program likely
#    has an error (or, less likely, the gcc ld program has an error).
# 2) This summary is in vertical format where the "section totals" is in
#    horizontal format.
#
# We print this information to both stdout and stderr, because it's very nice
# to see this information during a compile.
#

print "\nSection Summary\n\n";

# Print out the header line.
# The section name column is variable-width, based on the longest section name (min 7 chars).
$maxSectionNameLen = max($maxSectionNameLen, 7);
$headerFormat = sprintf("%%-%ds      %8s  %10s\n", $maxSectionNameLen, "hex", "decimal");
push(@summary, sprintf($headerFormat, "section"));

$total = 0;
$format = sprintf("%%-%ds    0x%%08x  %%10d\n", $maxSectionNameLen);
for ($i = 0;   $i <= $#sections;  $i++)
{
    push(@summary, sprintf($format, $sections[$i], $sectionLengths{$sections[$i]}, $sectionLengths{$sections[$i]}));
    $total += $sectionLengths{$sections[$i]};
}
$sepFormat = sprintf("%%-%ds    ----------  ----------\n", $maxSectionNameLen);
push(@summary, sprintf($sepFormat, "------", $total, $total));
push(@summary, sprintf($format, "Total:", $total, $total));

$" = '';
print @summary;
print stderr @summary;

# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.
