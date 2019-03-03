## Copyright(c) 2016-2018 Myriota Pty Ltd
##
## All Rights Reserved.
##
## TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED “AS IS” AND
## MYRIOTA DISCLAIMS ALL WARRANTIES WITH RESPECT TO THE SOFTWARE INCLUDING ANY
## IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR PURPOSE, ACCURACY OR SECURITY
## AND EXCLUDES ALL LIABILITY FOR PERSONAL INJURY OR ANY INCIDENTAL, SPECIAL,
## INDIRECT OR CONSEQUENTIAL DAMAGES (INCLUDING LOSS OF PROFITS, LOSS OF DATA OR
## BUSINESS INTERRUPTION) ARISING OUT OF USE OF OR INABILITY TO USE THE SOFTWARE.

ROOTDIR?=$(abspath ../)

# Source extensions
CPPEXT := cpp
CEXT := c
ASMEXT := s

include $(ROOTDIR)/math/flags.mk

## Build tools for the satellite simulator dongle
satellite_simulator: resampler convert_type

## Type conversion for a sequence, e.g. int8 to double
convert_type: convert_type.o myriotamath.a
	$(CXX) -o $@ $^ $(LDFLAGS)

## Changes rate of a complex valued sequence
resampler: resampler.o myriotamath.a
	$(CXX) -o $@ $^ $(LDFLAGS)

include $(ROOTDIR)/math/build.mk
