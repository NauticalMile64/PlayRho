# The Physical Units Interface

The *Physical Units Interface* for
[PlayRho](https://github.com/louis-langholtz/PlayRho), is an interface
that ties physical values to their required types.

The base implementation for the *physical units interface* simply uses
the `Real` type. To get the interface to enforce strong typing, an
implementation that enforces it has to be used &mdash; otherwise the
interface is just syntactic sugar that states only to the programmer
what the unit is supposed to be.

To date, a modified version of the Boost Units library has been used as an
implementation that leverages the compiler to enforce strong typing. The
source code for the modified Boost Units library is available from GitHub
from https://github.com/louis-langholtz/units.

To build with this Boost units support:
  1. Download the latest Boost sources from boost.org. At the time this
     document was prepared, the latest was version 1.63.0. This can be
     downloaded from: https://sourceforge.net/projects/boost/files/boost/1.63.0/boost_1_63_0.tar.bz2
     The SHA256 hash for the boost 1.63.0 bzipped tarball is:
       beae2529f759f6b3bf3f4969a19c2e9d6f0c503edcb2de4a61d1428519fcb3b0
  2. Extract these sources into a directory/folder of your choice.
  3. As only headers are needed for the physical units support, simply
     recursively copy the files from the include headers directory to a
     target destination directory.
  4. From within the target destination directory, find the directory named
     "units" and move it out of the way. One way to do this is to rename the
     directory to "units.old".
  5. Download the modified units library from:
     https://github.com/louis-langholtz/units.
  6. Make the directory containing the include headers from the modified units
     library accessible from the target destination directory as the directory
     named "units".
  7. Go back to the PlayRho project setup and add "USE_BOOST_UNITS" as a
     C pre-processor define so that the file
     [`PlayRho/Common/Units.hpp`](../PlayRho/Common/Units.hpp) can see that it's defined.
  8. Now rebuild the PlayRho library and any applications. If you've used the
     physical units interface correctly everywhere, you should get no units
     related warnings or errors.
