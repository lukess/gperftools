TCMalloc for Mips32/Mips64
--------------------------

Platform Port Authors:
 * Ray Balogh   <Ray.Balogh@sycamorenet.com>
 * Aaron Conole <apconole@yahoo.com>

CAVEATS
-------
Testing has only been done extensively with 32-bit MIPS. Some minimal testing
has been done on 64-bit MIPS. Additionally, this has only been tested on the
Cavium Octeon platform. YMMV.

BUILDING
--------
Building TCMalloc for the MIPS platform may require some additional work, apart
from simply running ./configure --host=xxxxx. Ideally, one could follow the 
instructions in the INSTALL and README files to use this for your mips project,
however, we have run into some building issues with various versions of
compilers that we'll document here.

The following new #define directives are known to the system:

- __MIPS64__

 Set __MIPS64__=1 when you are building the library for a 64-bit MIPS target. 
If you don't, you'll get many compilation errors casting AtomicWord* to 
Atomic32*. Some compilers automatically set this, while others 

- HAVE_RT_SIGPROCMASK
 
 Ideally, we would have written a .m4 macro to detect whether or not the target
platform uses the old or new sigprocmask API (ie: sig_rt_sigprocmask vs.
sig_sigprocmask). Since the issue hasn't seemed to crop up for others, we felt
it niche enough not to warrant a separate test, but that is certainly one 
enhancement to be added to this patch.

 A sample is included in the m4 directory (m4/sigprocmask.m4) but is not 
currently tested - uncomment it to attempt to test it.

MMAP
----
 On many linux MIPS compilers, the mmap/mremap detection routine fails. If you
truly have MMAP on your platform (and linux users typically do) simply add the
following environment variable while compiling:

ac_cv_func_mmap_fixed_mapped=yes

 This will give you access to mmap/mremap and avoid compilation errors cropping
up.


STACK UNWINDING
---------------
 This feature is not currently tested/supported properly. Use it at your own 
risk.
