# Check for sig_procmask vs. sig_rt_procmask vs. no sig procmask (which won't
# work for us.

# NOTE: this is not tested, so we won't include it yet.
#

#AC_DEFUN([AC_HAVE_PROCMASK],
#[AC_MSG_CHECKING(for sys_sigprocmask)
# AC_LANG_SAVE
# AC_LANG_C
# ac_sys_sigprocmask="no"
# HAVE_SIGPROCMASK=0
# HAVE_RT_SIGPROCMASK=0
# #try for sig_procmask
# AC_TRY_LINK([#include <signal.h>],
#             [ sigset_t set, oset; long x = sys_sigprocmask(&set, &oset, sizeof(sigset_t));],
#             [ac_sys_sigprocmask=yes])
# if test "x$ac_sys_sigprocmask" != "xyes"; then
#   AC_TRY_LINK([#include <signal.h>],
#               [ sigset_t set, oset; long x = sys_rt_sigprocmask(&set, &oset, sizeof(sigset_t),sizeof(struct kernel_sigaction));],
#               [ac_sys_sigprocmask=yes])
#   if test "x$ac_sys_sigprocmask" != "xyes"; then
#      AC_MSG_ERROR([cannot find sys_sigprocmask or sys_rt_sigprocmask])
#   else
#      HAVE_RT_SIGPROCMASK=1
#      HAVE_SIGPROCMASK=1
#   fi
# else
#   HAVE_SIGPROCMASK=1
# fi
# AC_MSG_RESULT(${HAVE_SIGPROCMASK:-no})
# AC_LANG_RESTORE
#])
