/* config.h.  Generated from config.h.in by configure.  */
/* config.h.in.  Generated from configure.ac by autoheader.  */

/* Absolute path of source tree */
#define ABS_TOP_SRCDIR "/home/craig/Projects/NuttX/apps/system/cfengine/core"

/* Define if building universal (internal helper macro) */
/* #undef AC_APPLE_UNIVERSAL_BUILD */

/* Special CFEngine symbol */
#define AUTOCONF_HOSTNAME "librem15"

/* Speial CFEngine symbol */
#define AUTOCONF_SYSNAME "linux-gnu"

/* binaries location */
#define BINDIR "/var/cfengine/bin"

/* "Software build year" */
#define BUILD_YEAR 2021

/* Define if you want builtin Enterprise extensions */
/* #undef BUILTIN_EXTENSIONS */

/* Datadir location */
#define CF_DATADIR "default"

/* Path to chpass tool */
/* #undef CHPASS */

/* Path to chpasswd tool */
#define CHPASSWD "/sbin/chpasswd"

/* Define to 1 if using `getloadavg.c'. */
/* #undef C_GETLOADAVG */

/* Define to 1 for DGUX with <sys/dg_sys_info.h>. */
/* #undef DGUX */

/* Whether endnetgrent returns int */
#define ENDNETGRENT_RETURNS_INT 0

/* Define to 1 if the `getloadavg' function needs to be run setuid or setgid.
   */
/* #undef GETLOADAVG_PRIVILEGED */

/* Define to 1 if you have the <acl.h> header file. */
/* #undef HAVE_ACL_H */

/* Define to 1 if you have the <acl/libacl.h> header file. */
#define HAVE_ACL_LIBACL_H 1

/* Define to 1 if you have the `asprintf' function. */
#define HAVE_ASPRINTF 1

/* Define to 1 if you have the <attr/xattr.h> header file. */
#define HAVE_ATTR_XATTR_H 1

/* Define to 1 if you have the <avahi-client/client.h> header file. */
/* #undef HAVE_AVAHI_CLIENT_CLIENT_H */

/* Define to 1 if you have the <avahi-common/address.h> header file. */
/* #undef HAVE_AVAHI_COMMON_ADDRESS_H */

/* Define to 1 if you have the `chflags' function. */
/* #undef HAVE_CHFLAGS */

/* Define if chpass tool is present */
/* #undef HAVE_CHPASS */

/* Define if chpasswd tool is present */
#define HAVE_CHPASSWD 1

/* Define to 1 if the system has the type `clockid_t'. */
#define HAVE_CLOCKID_T 1

/* Define to 1 if you have the `clock_gettime' function. */
#define HAVE_CLOCK_GETTIME 1

/* Define to 1 if you have the `closefrom' function. */
/* #undef HAVE_CLOSEFROM */

/* Define to 1 if you have the <curl/curl.h> header file. */
#define HAVE_CURL_CURL_H 1

/* Define to 1 if you have the declaration of `alarm', and to 0 if you don't.
   */
#define HAVE_DECL_ALARM 1

/* Define to 1 if you have the declaration of `chmod', and to 0 if you don't.
   */
#define HAVE_DECL_CHMOD 1

/* Define to 1 if you have the declaration of `chown', and to 0 if you don't.
   */
#define HAVE_DECL_CHOWN 1

/* Define to 1 if you have the declaration of `clock_gettime', and to 0 if you
   don't. */
#define HAVE_DECL_CLOCK_GETTIME 1

/* Define to 1 if you have the declaration of `closefrom', and to 0 if you
   don't. */
#define HAVE_DECL_CLOSEFROM 0

/* Define to 1 if you have the declaration of `dirfd', and to 0 if you don't.
   */
#define HAVE_DECL_DIRFD 1

/* Define to 1 if you have the declaration of `drand48', and to 0 if you
   don't. */
#define HAVE_DECL_DRAND48 1

/* Define to 1 if you have the declaration of `endnetgrent', and to 0 if you
   don't. */
#define HAVE_DECL_ENDNETGRENT 1

/* Define to 1 if you have the declaration of `fchmod', and to 0 if you don't.
   */
#define HAVE_DECL_FCHMOD 1

/* Define to 1 if you have the declaration of `fchmodat', and to 0 if you
   don't. */
#define HAVE_DECL_FCHMODAT 1

/* Define to 1 if you have the declaration of `fchownat', and to 0 if you
   don't. */
#define HAVE_DECL_FCHOWNAT 1

/* Define to 1 if you have the declaration of `fgetgrent', and to 0 if you
   don't. */
#define HAVE_DECL_FGETGRENT 1

/* Define to 1 if you have the declaration of `fstatat', and to 0 if you
   don't. */
#define HAVE_DECL_FSTATAT 1

/* Define to 1 if you have the declaration of `fsync', and to 0 if you don't.
   */
#define HAVE_DECL_FSYNC 1

/* Define to 1 if you have the declaration of `getaddrinfo', and to 0 if you
   don't. */
#define HAVE_DECL_GETADDRINFO 1

/* Define to 1 if you have the declaration of `getgid', and to 0 if you don't.
   */
#define HAVE_DECL_GETGID 1

/* Define to 1 if you have the declaration of `getline', and to 0 if you
   don't. */
#define HAVE_DECL_GETLINE 1

/* Define to 1 if you have the declaration of `getloadavg', and to 0 if you
   don't. */
#define HAVE_DECL_GETLOADAVG 1

/* Define to 1 if you have the declaration of `getnetgrent', and to 0 if you
   don't. */
#define HAVE_DECL_GETNETGRENT 1

/* Define to 1 if you have the declaration of `getuid', and to 0 if you don't.
   */
#define HAVE_DECL_GETUID 1

/* Define to 1 if you have the declaration of `glob', and to 0 if you don't.
   */
#define HAVE_DECL_GLOB 1

/* Define to 1 if you have the declaration of `gmtime_r', and to 0 if you
   don't. */
#define HAVE_DECL_GMTIME_R 1

/* Define to 1 if you have the declaration of `inet_ntop', and to 0 if you
   don't. */
#define HAVE_DECL_INET_NTOP 1

/* Define to 1 if you have the declaration of `inet_pton', and to 0 if you
   don't. */
#define HAVE_DECL_INET_PTON 1

/* Define to 1 if you have the declaration of `isfinite', and to 0 if you
   don't. */
#define HAVE_DECL_ISFINITE 1

/* Define to 1 if you have the declaration of `le32toh', and to 0 if you
   don't. */
#define HAVE_DECL_LE32TOH 1

/* Define to 1 if you have the declaration of `localtime_r', and to 0 if you
   don't. */
#define HAVE_DECL_LOCALTIME_R 1

/* Define to 1 if you have the declaration of `log2', and to 0 if you don't.
   */
#define HAVE_DECL_LOG2 1

/* Define to 1 if you have the declaration of `lstat', and to 0 if you don't.
   */
#define HAVE_DECL_LSTAT 1

/* Define to 1 if you have the declaration of `memdup', and to 0 if you don't.
   */
#define HAVE_DECL_MEMDUP 0

/* Define to 1 if you have the declaration of `memmem', and to 0 if you don't.
   */
#define HAVE_DECL_MEMMEM 0

/* Define to 1 if you have the declaration of `memrchr', and to 0 if you
   don't. */
#define HAVE_DECL_MEMRCHR 0

/* Define to 1 if you have the declaration of `mkdtemp', and to 0 if you
   don't. */
#define HAVE_DECL_MKDTEMP 1

/* Define to 1 if you have the declaration of `nanosleep', and to 0 if you
   don't. */
#define HAVE_DECL_NANOSLEEP 0

/* Define to 1 if you have the declaration of `openat', and to 0 if you don't.
   */
#define HAVE_DECL_OPENAT 1

/* Define to 1 if you have the declaration of `pthread_attr_setstacksize', and
   to 0 if you don't. */
#define HAVE_DECL_PTHREAD_ATTR_SETSTACKSIZE 1

/* Define to 1 if you have the declaration of `pthread_sigmask', and to 0 if
   you don't. */
#define HAVE_DECL_PTHREAD_SIGMASK 1

/* Define to 1 if you have the declaration of `readlinkat', and to 0 if you
   don't. */
#define HAVE_DECL_READLINKAT 1

/* Define to 1 if you have the declaration of `realpath', and to 0 if you
   don't. */
#define HAVE_DECL_REALPATH 1

/* Define to 1 if you have the declaration of `round', and to 0 if you don't.
   */
#define HAVE_DECL_ROUND 1

/* Define to 1 if you have the declaration of `sched_yield', and to 0 if you
   don't. */
#define HAVE_DECL_SCHED_YIELD 1

/* Define to 1 if you have the declaration of `seteuid', and to 0 if you
   don't. */
#define HAVE_DECL_SETEUID 1

/* Define to 1 if you have the declaration of `setlinebuf', and to 0 if you
   don't. */
#define HAVE_DECL_SETLINEBUF 1

/* Define to 1 if you have the declaration of `setnetgrent', and to 0 if you
   don't. */
#define HAVE_DECL_SETNETGRENT 1

/* Define to 1 if you have the declaration of `socketpair', and to 0 if you
   don't. */
#define HAVE_DECL_SOCKETPAIR 1

/* Define to 1 if you have the declaration of `srand48', and to 0 if you
   don't. */
#define HAVE_DECL_SRAND48 1

/* Define to 1 if you have the declaration of `SSL_CTX_clear_options', and to
   0 if you don't. */
#define HAVE_DECL_SSL_CTX_CLEAR_OPTIONS 1

/* Define to 1 if you have the declaration of `stpncpy', and to 0 if you
   don't. */
#define HAVE_DECL_STPNCPY 1

/* Define to 1 if you have the declaration of `strcasecmp', and to 0 if you
   don't. */
#define HAVE_DECL_STRCASECMP 1

/* Define to 1 if you have the declaration of `strcasestr', and to 0 if you
   don't. */
#define HAVE_DECL_STRCASESTR 0

/* Define to 1 if you have the declaration of `strchrnul', and to 0 if you
   don't. */
#define HAVE_DECL_STRCHRNUL 1

/* Define to 1 if you have the declaration of `strdup', and to 0 if you don't.
   */
#define HAVE_DECL_STRDUP 1

/* Define to 1 if you have the declaration of `strerror', and to 0 if you
   don't. */
#define HAVE_DECL_STRERROR 1

/* Define to 1 if you have the declaration of `strlcat', and to 0 if you
   don't. */
#define HAVE_DECL_STRLCAT 0

/* Define to 1 if you have the declaration of `strlcpy', and to 0 if you
   don't. */
#define HAVE_DECL_STRLCPY 0

/* Define to 1 if you have the declaration of `strncasecmp', and to 0 if you
   don't. */
#define HAVE_DECL_STRNCASECMP 1

/* Define to 1 if you have the declaration of `strndup', and to 0 if you
   don't. */
#define HAVE_DECL_STRNDUP 1

/* Define to 1 if you have the declaration of `strnlen', and to 0 if you
   don't. */
#define HAVE_DECL_STRNLEN 1

/* Define to 1 if you have the declaration of `strrstr', and to 0 if you
   don't. */
#define HAVE_DECL_STRRSTR 0

/* Define to 1 if you have the declaration of `strsep', and to 0 if you don't.
   */
#define HAVE_DECL_STRSEP 1

/* Define to 1 if you have the declaration of `strsignal', and to 0 if you
   don't. */
#define HAVE_DECL_STRSIGNAL 1

/* Define to 1 if you have the declaration of `strstr', and to 0 if you don't.
   */
#define HAVE_DECL_STRSTR 1

/* Define to 1 if you have the declaration of `uname', and to 0 if you don't.
   */
#define HAVE_DECL_UNAME 0

/* Define to 1 if you have the declaration of `unsetenv', and to 0 if you
   don't. */
#define HAVE_DECL_UNSETENV 1

/* Define to 1 if you have the <depot.h> header file. */
/* #undef HAVE_DEPOT_H */

/* Define to 1 if you have the <dirent.h> header file, and it defines `DIR'.
   */
#define HAVE_DIRENT_H 1

/* Define to 1 if you have the `dirfd' function. */
#define HAVE_DIRFD 1

/* Define to 1 if you have the <dlfcn.h> header file. */
#define HAVE_DLFCN_H 1

/* Define to 1 if you have the `door' function. */
/* #undef HAVE_DOOR */

/* Define to 1 if you have the `drand48' function. */
#define HAVE_DRAND48 1

/* Define to 1 if you have the <dustat.h> header file. */
/* #undef HAVE_DUSTAT_H */

/* Define to 1 if you have the <endian.h> header file. */
#define HAVE_ENDIAN_H 1

/* Define to 1 if you have the `endnetgrent' function. */
#define HAVE_ENDNETGRENT 1

/* Define to 1 if you have the `endpwent' function. */
#define HAVE_ENDPWENT 1

/* Define to 1 if you have the `fchmod' function. */
#define HAVE_FCHMOD 1

/* Define to 1 if you have the `fchmodat' function. */
#define HAVE_FCHMODAT 1

/* Define to 1 if you have the `fchownat' function. */
#define HAVE_FCHOWNAT 1

/* Define to 1 if you have the <fcntl.h> header file. */
#define HAVE_FCNTL_H 1

/* Whether to use fexecve(3) to execute a new process */
#define HAVE_FEXECVE 1

/* Define to 1 if you have the `fgetgrent' function. */
#define HAVE_FGETGRENT 1

/* Define to 1 if you have the `fgetpwent' function. */
#define HAVE_FGETPWENT 1

/* Define if fgetpwent and fgetgrent functions are present */
#define HAVE_FGETPWENT_FGETGRENT 1

/* Define if fgetspent is available */
#define HAVE_FGETSPENT 1

/* Define to 1 if you have the `fpathconf' function. */
#define HAVE_FPATHCONF 1

/* Define to 1 if fseeko (and presumably ftello) exists and is declared. */
#define HAVE_FSEEKO 1

/* Define to 1 if you have the `fstatat' function. */
#define HAVE_FSTATAT 1

/* Define to 1 if you have the `gethostent' function. */
#define HAVE_GETHOSTENT 1

/* Define to 1 if you have the `gethostname' function. */
#define HAVE_GETHOSTNAME 1

/* Define to 1 if you have the `getifaddrs' function. */
#define HAVE_GETIFADDRS 1

/* Define to 1 if you have the `getline' function. */
#define HAVE_GETLINE 1

/* Define to 1 if you have the `getloadavg' function. */
#define HAVE_GETLOADAVG 1

/* Define to 1 if you have the `getnetgrent' function. */
#define HAVE_GETNETGRENT 1

/* Define to 1 if you have the `getprocs64' function. */
/* #undef HAVE_GETPROCS64 */

/* Define to 1 if you have the `getpwent' function. */
#define HAVE_GETPWENT 1

/* Define to 1 if you have the `getzoneid' function. */
/* #undef HAVE_GETZONEID */

/* Define to 1 if you have the `getzonenamebyid' function. */
/* #undef HAVE_GETZONENAMEBYID */

/* Define to 1 if you have the `gmtime_r' function. */
#define HAVE_GMTIME_R 1

/* Define to 1 if you have the <ieeefp.h> header file. */
/* #undef HAVE_IEEEFP_H */

/* Define to 1 if you have the `inet_ntop' function. */
#define HAVE_INET_NTOP 1

/* Define to 1 if you have the `inet_pton' function. */
#define HAVE_INET_PTON 1

/* Define to 1 if the system has the type `intmax_t'. */
#define HAVE_INTMAX_T 1

/* Define to 1 if you have the <inttypes.h> header file. */
#define HAVE_INTTYPES_H 1

/* Define to 1 if you have the `jail_get' function. */
/* #undef HAVE_JAIL_GET */

/* Define to 1 if you have the <kstat.h> header file. */
/* #undef HAVE_KSTAT_H */

/* Whether to use lchown(3) to change ownerships */
#define HAVE_LCHOWN 1

/* Define to 1 if you have the `lckpwdf' function. */
#define HAVE_LCKPWDF 1

/* Define to 1 if you have the `acl' library (-lacl). */
#define HAVE_LIBACL 1

/* Define to 1 if you have the `crypto' library (-lcrypto). */
#define HAVE_LIBCRYPTO 1

/* Define to 1 if you have the `curl' library (-lcurl). */
#define HAVE_LIBCURL 1

/* Define to 1 if you have the `dgc' library (-ldgc). */
/* #undef HAVE_LIBDGC */

/* Define to 1 if you have the `dl' library (-ldl). */
#define HAVE_LIBDL 1

/* Define to 1 if you have the `kstat' library (-lkstat). */
/* #undef HAVE_LIBKSTAT */

/* Define to 1 if you have the `lmdb' library (-llmdb). */
#define HAVE_LIBLMDB 1

/* Define to 1 if you have the `m' library (-lm). */
#define HAVE_LIBM 1

/* Define to 1 if you have the `mysqlclient' library (-lmysqlclient). */
/* #undef HAVE_LIBMYSQLCLIENT */

/* Define to 1 if you have the `nss_nis' library (-lnss_nis). */
/* #undef HAVE_LIBNSS_NIS */

/* Define to 1 if you have the `pam' library (-lpam). */
#define HAVE_LIBPAM 1

/* Define to 1 if you have the `pcre' library (-lpcre). */
#define HAVE_LIBPCRE 1

/* Define to 1 if you have the `pq' library (-lpq). */
#define HAVE_LIBPQ 1

/* Define to 1 if you have the <libpq-fe.h> header file. */
#define HAVE_LIBPQ_FE_H 1

/* Define to 1 if you have the `qdbm' library (-lqdbm). */
/* #undef HAVE_LIBQDBM */

/* Define to 1 if you have the `rt' library (-lrt). */
#define HAVE_LIBRT 1

/* Define if -lsec is available */
/* #undef HAVE_LIBSEC */

/* Define to 1 if you have the `ssl' library (-lssl). */
#define HAVE_LIBSSL 1

/* Define to 1 if you have the `systemd' library (-lsystemd). */
/* #undef HAVE_LIBSYSTEMD */

/* Define to 1 if you have the `tokyocabinet' library (-ltokyocabinet). */
/* #undef HAVE_LIBTOKYOCABINET */

/* Define to 1 if you have the `virt' library (-lvirt). */
#define HAVE_LIBVIRT 1

/* Define to 1 if you have the <libvirt/libvirt.h> header file. */
#define HAVE_LIBVIRT_LIBVIRT_H 1

/* Define to 1 if you have the `xml2' library (-lxml2). */
#define HAVE_LIBXML2 1

/* Define to 1 if you have the <libxml/xmlwriter.h> header file. */
#define HAVE_LIBXML_XMLWRITER_H 1

/* Define to 1 if you have the `yaml' library (-lyaml). */
#define HAVE_LIBYAML 1

/* Define to 1 if you have the `llistxattr' function. */
#define HAVE_LLISTXATTR 1

/* Define to 1 if you have the <lmdb.h> header file. */
#define HAVE_LMDB_H 1

/* Define to 1 if you have the `localeconv' function. */
#define HAVE_LOCALECONV 1

/* Define to 1 if you have the <locale.h> header file. */
#define HAVE_LOCALE_H 1

/* Define to 1 if you have the `localtime_r' function. */
#define HAVE_LOCALTIME_R 1

/* Define to 1 if you have the `log2' function. */
#define HAVE_LOG2 1

/* Define to 1 if the system has the type `long double'. */
#define HAVE_LONG_DOUBLE 1

/* Define to 1 if the system has the type `long long int'. */
#define HAVE_LONG_LONG_INT 1

/* Define to 1 if you have the <mach/mach.h> header file. */
/* #undef HAVE_MACH_MACH_H */

/* Define to 1 if you have the <malloc.h> header file. */
#define HAVE_MALLOC_H 1

/* Define to 1 if you have the `memdup' function. */
/* #undef HAVE_MEMDUP */

/* Define to 1 if you have the `memmem' function. */
#define HAVE_MEMMEM 1

/* Define to 1 if you have the <memory.h> header file. */
#define HAVE_MEMORY_H 1

/* Define to 1 if you have the `memrchr' function. */
#define HAVE_MEMRCHR 1

/* Define to 1 if you have the `mkdtemp' function. */
#define HAVE_MKDTEMP 1

/* Define to 1 if you have the `mkfifo' function. */
#define HAVE_MKFIFO 1

/* Define to 1 if BSD .msg_accrights supported */
/* #undef HAVE_MSGHDR_ACCRIGHTS */

/* Define to 1 if SCM_RIGHTS supported */
#define HAVE_MSGHDR_MSG_CONTROL 1

/* Define to 1 if you have the <mysql.h> header file. */
/* #undef HAVE_MYSQL_H */

/* Define to 1 if you have the `nanosleep' function. */
#define HAVE_NANOSLEEP 1

/* Define to 1 if you have the <ndir.h> header file, and it defines `DIR'. */
/* #undef HAVE_NDIR_H */

/* Define to 1 if you have the <netinet/in.h> header file. */
#define HAVE_NETINET_IN_H 1

/* Define to 1 if you have the <netinet/ip.h> header file. */
#define HAVE_NETINET_IP_H 1

/* Define to 1 if you have the <net/if_arp.h> header file. */
#define HAVE_NET_IF_ARP_H 1

/* Define to 1 if you have the <net/if_dl.h> header file. */
/* #undef HAVE_NET_IF_DL_H */

/* Define to 1 if you have the <net/route.h> header file. */
#define HAVE_NET_ROUTE_H 1

/* Define to 1 if you have the <nlist.h> header file. */
/* #undef HAVE_NLIST_H */

/* Define to 1 if no BSD .msg_accrights support */
#define HAVE_NO_MSGHDR_ACCRIGHTS 1

/* Define to 1 if SCM_RIGHTS support */
/* #undef HAVE_NO_MSGHDR_MSG_CONTROL */

/* Define to 1 if you have the `openat' function. */
#define HAVE_OPENAT 1

/* Define to 1 if you have the <openssl/opensslv.h> header file. */
#define HAVE_OPENSSL_OPENSSLV_H 1

/* The old route entry structure in newer BSDs */
/* #undef HAVE_ORTENTRY */

/* Define to 1 if you have the <pcre.h> header file. */
#define HAVE_PCRE_H 1

/* Define to 1 if you have the <pcre/pcre.h> header file. */
/* #undef HAVE_PCRE_PCRE_H */

/* Define to 1 if you have the `pstat_getdynamic' function. */
/* #undef HAVE_PSTAT_GETDYNAMIC */

/* Define to 1 if you have the `pstat_getfile2' function. */
/* #undef HAVE_PSTAT_GETFILE2 */

/* Define if you have POSIX threads libraries and header files. */
#define HAVE_PTHREAD 1

/* Define to 1 if you have the `pthread_attr_setstacksize' function. */
#define HAVE_PTHREAD_ATTR_SETSTACKSIZE 1

/* Define to 1 if you have the `pthread_sigmask' function. */
#define HAVE_PTHREAD_SIGMASK 1

/* Define to 1 if the system has the type `ptrdiff_t'. */
#define HAVE_PTRDIFF_T 1

/* Define if pw tool is present */
/* #undef HAVE_PW */

/* Define if pwdadm tool is present */
/* #undef HAVE_PWDADM */

/* Define to 1 if you have the `readlinkat' function. */
#define HAVE_READLINKAT 1

/* Define to 1 if you have the `round' function. */
#define HAVE_ROUND 1

/* Do we have any route entry structure? */
#define HAVE_RTENTRY 1

/* Define to 1 if you have the `sched_yield' function. */
#define HAVE_SCHED_YIELD 1

/* sd_notify_barrier on recent systemd */
/* #undef HAVE_SD_NOTIFY_BARRIER */

/* Define to 1 if you have the <security/pam_appl.h> header file. */
#define HAVE_SECURITY_PAM_APPL_H 1

/* Define to 1 if you have the `sendto' function. */
#define HAVE_SENDTO 1

/* Define to 1 if you have the `setegid' function. */
#define HAVE_SETEGID 1

/* Define to 1 if you have the `seteuid' function. */
#define HAVE_SETEUID 1

/* Define to 1 if you have the `setlinebuf' function. */
#define HAVE_SETLINEBUF 1

/* Define to 1 if you have the `setlocale' function. */
/* #undef HAVE_SETLOCALE */

/* Define to 1 if you have the `setnetgrent' function. */
#define HAVE_SETNETGRENT 1

/* Define to 1 if you have the `setpwent' function. */
#define HAVE_SETPWENT 1

/* Define to 1 if you have the `setregid' function. */
#define HAVE_SETREGID 1

/* Define to 1 if you have the `setreuid' function. */
#define HAVE_SETREUID 1

/* Define to 1 if you have the `setsid' function. */
#define HAVE_SETSID 1

/* Define to 1 if you have the `setsockopt' function. */
#define HAVE_SETSOCKOPT 1

/* Define to 1 if you have the <shadow.h> header file. */
#define HAVE_SHADOW_H 1

/* Define to 1 if you have the `sleep' function. */
#define HAVE_SLEEP 1

/* Define to 1 if you have a C99 compliant `snprintf' function. */
#define HAVE_SNPRINTF 1

/* Define to 1 if you have the `socket' function. */
#define HAVE_SOCKET 1

/* Define to 1 if the system has the type `socklen_t'. */
#define HAVE_SOCKLEN_T 1

/* Define to 1 if you have the `srand48' function. */
#define HAVE_SRAND48 1

/* Define to 1 if you have the `statfs' function. */
#define HAVE_STATFS 1

/* Define to 1 if you have the `statvfs' function. */
#define HAVE_STATVFS 1

/* Define to 1 if you have the <stdarg.h> header file. */
#define HAVE_STDARG_H 1

/* Define to 1 if stdbool.h conforms to C99. */
#define HAVE_STDBOOL_H 1

/* Define to 1 if you have the <stddef.h> header file. */
#define HAVE_STDDEF_H 1

/* Define to 1 if you have the <stdint.h> header file. */
#define HAVE_STDINT_H 1

/* Define to 1 if you have the <stdlib.h> header file. */
#define HAVE_STDLIB_H 1

/* Define to 1 if you have the `stpncpy' function. */
#define HAVE_STPNCPY 1

/* Define to 1 if you have the `strcasecmp' function. */
#define HAVE_STRCASECMP 1

/* Define to 1 if you have the `strcasestr' function. */
#define HAVE_STRCASESTR 1

/* Define to 1 if you have the `strchrnul' function. */
#define HAVE_STRCHRNUL 1

/* Define to 1 if you have the `strdup' function. */
#define HAVE_STRDUP 1

/* Define to 1 if you have the `strerror' function. */
#define HAVE_STRERROR 1

/* Define to 1 if you have the <strings.h> header file. */
#define HAVE_STRINGS_H 1

/* Define to 1 if you have the <string.h> header file. */
#define HAVE_STRING_H 1

/* Define to 1 if you have the `strlcat' function. */
/* #undef HAVE_STRLCAT */

/* Define to 1 if you have the `strlcpy' function. */
/* #undef HAVE_STRLCPY */

/* Define to 1 if you have the `strncasecmp' function. */
#define HAVE_STRNCASECMP 1

/* Define to 1 if you have the `strndup' function. */
#define HAVE_STRNDUP 1

/* Define to 1 if you have the `strnlen' function. */
#define HAVE_STRNLEN 1

/* Define to 1 if you have the `strrstr' function. */
/* #undef HAVE_STRRSTR */

/* Define to 1 if you have the `strsep' function. */
#define HAVE_STRSEP 1

/* Define to 1 if you have the `strsignal' function. */
#define HAVE_STRSIGNAL 1

/* Define to 1 if you have the `strstr' function. */
#define HAVE_STRSTR 1

/* Define to 1 if `ifr_hwaddr' is a member of `struct ifreq'. */
#define HAVE_STRUCT_IFREQ_IFR_HWADDR 1

/* Define to 1 if `decimal_point' is a member of `struct lconv'. */
#define HAVE_STRUCT_LCONV_DECIMAL_POINT 1

/* Define to 1 if `thousands_sep' is a member of `struct lconv'. */
#define HAVE_STRUCT_LCONV_THOUSANDS_SEP 1

/* Define to 1 if `n_un.n_name' is a member of `struct nlist'. */
/* #undef HAVE_STRUCT_NLIST_N_UN_N_NAME */

/* Define to 1 if `sa_len' is a member of `struct sockaddr'. */
/* #undef HAVE_STRUCT_SOCKADDR_SA_LEN */

/* Define to 1 if the system has the type `struct sockaddr_storage'. */
#define HAVE_STRUCT_SOCKADDR_STORAGE 1

/* Define to 1 if `st_blocks' is a member of `struct stat'. */
#define HAVE_STRUCT_STAT_ST_BLOCKS 1

/* Define to 1 if `st_mtim' is a member of `struct stat'. */
#define HAVE_STRUCT_STAT_ST_MTIM 1

/* Define to 1 if `st_mtimespec' is a member of `struct stat'. */
/* #undef HAVE_STRUCT_STAT_ST_MTIMESPEC */

/* Define to 1 if `uptime' is a member of `struct sysinfo'. */
#define HAVE_STRUCT_SYSINFO_UPTIME 1

/* Define to 1 if you have the `sysconf' function. */
#define HAVE_SYSCONF 1

/* Define to 1 if you have the `sysinfo' function. */
#define HAVE_SYSINFO 1

/* Define to 1 if you have the <systemd/sd-daemon.h> header file. */
/* #undef HAVE_SYSTEMD_SD_DAEMON_H */

/* Define to 1 if you have the <sys/acl.h> header file. */
#define HAVE_SYS_ACL_H 1

/* Define to 1 if you have the <sys/dir.h> header file, and it defines `DIR'.
   */
/* #undef HAVE_SYS_DIR_H */

/* Define to 1 if you have the <sys/filesys.h> header file. */
/* #undef HAVE_SYS_FILESYS_H */

/* Define to 1 if you have the <sys/jail.h> header file. */
/* #undef HAVE_SYS_JAIL_H */

/* Define to 1 if you have the <sys/loadavg.h> header file. */
/* #undef HAVE_SYS_LOADAVG_H */

/* Define to 1 if you have the <sys/malloc.h> header file. */
/* #undef HAVE_SYS_MALLOC_H */

/* Define to 1 if you have the <sys/mount.h> header file. */
#define HAVE_SYS_MOUNT_H 1

/* Define to 1 if you have the <sys/mpctl.h> header file. */
/* #undef HAVE_SYS_MPCTL_H */

/* Define to 1 if you have the <sys/ndir.h> header file, and it defines `DIR'.
   */
/* #undef HAVE_SYS_NDIR_H */

/* Define to 1 if you have the <sys/param.h> header file. */
#define HAVE_SYS_PARAM_H 1

/* Define to 1 if you have the <sys/pstat.h> header file. */
/* #undef HAVE_SYS_PSTAT_H */

/* Define to 1 if you have the <sys/resource.h> header file. */
#define HAVE_SYS_RESOURCE_H 1

/* Define to 1 if you have the <sys/sockio.h> header file. */
/* #undef HAVE_SYS_SOCKIO_H */

/* Define to 1 if you have the <sys/statfs.h> header file. */
#define HAVE_SYS_STATFS_H 1

/* Define to 1 if you have the <sys/statvfs.h> header file. */
#define HAVE_SYS_STATVFS_H 1

/* Define to 1 if you have the <sys/stat.h> header file. */
#define HAVE_SYS_STAT_H 1

/* Define to 1 if you have the <sys/sysctl.h> header file. */
#define HAVE_SYS_SYSCTL_H 1

/* Define to 1 if you have the <sys/sysmacros.h> header file. */
#define HAVE_SYS_SYSMACROS_H 1

/* Define to 1 if you have the <sys/systeminfo.h> header file. */
/* #undef HAVE_SYS_SYSTEMINFO_H */

/* Define to 1 if you have the <sys/time.h> header file. */
#define HAVE_SYS_TIME_H 1

/* Define to 1 if you have the <sys/types.h> header file. */
#define HAVE_SYS_TYPES_H 1

/* Define to 1 if you have the <sys/uio.h> header file. */
#define HAVE_SYS_UIO_H 1

/* Define to 1 if you have the <sys/vfs.h> header file. */
#define HAVE_SYS_VFS_H 1

/* Define to 1 if you have <sys/wait.h> that is POSIX.1 compatible. */
#define HAVE_SYS_WAIT_H 1

/* Define to 1 if you have the <sys/xattr.h> header file. */
#define HAVE_SYS_XATTR_H 1

/* Define to 1 if you have the <tchdb.h> header file. */
/* #undef HAVE_TCHDB_H */

/* Define to 1 if you have the <tcutil.h> header file. */
/* #undef HAVE_TCUTIL_H */

/* Define to 1 if you have the <time.h> header file. */
#define HAVE_TIME_H 1

/* Define if TLS 1.1 is supported by OpenSSL */
#define HAVE_TLS_1_1 1

/* Define if TLS 1.2 is supported by OpenSSL */
#define HAVE_TLS_1_2 1

/* Define if TLS 1.3 is supported by OpenSSL */
#define HAVE_TLS_1_3 1

/* Define to 1 if the system has the type `uintmax_t'. */
#define HAVE_UINTMAX_T 1

/* Define to 1 if the system has the type `uintptr_t'. */
#define HAVE_UINTPTR_T 1

/* Define to 1 if you have the `ulckpwdf' function. */
#define HAVE_ULCKPWDF 1

/* Define to 1 if you have the `uname' function. */
#define HAVE_UNAME 1

/* Define to 1 if you have the <unistd.h> header file. */
#define HAVE_UNISTD_H 1

/* Define to 1 if you have the `unsetenv' function. */
#define HAVE_UNSETENV 1

/* Define to 1 if the system has the type `unsigned long long int'. */
#define HAVE_UNSIGNED_LONG_LONG_INT 1

/* Define if useradd tool is present */
#define HAVE_USERADD 1

/* Define if userdel tool is present */
#define HAVE_USERDEL 1

/* Define if usermod tool is present */
#define HAVE_USERMOD 1

/* Define to 1 if you have the <utime.h> header file. */
#define HAVE_UTIME_H 1

/* Define to 1 if you have the <utmpx.h> header file. */
#define HAVE_UTMPX_H 1

/* Define to 1 if you have the <utmp.h> header file. */
#define HAVE_UTMP_H 1

/* Define to 1 if you have the <varargs.h> header file. */
/* #undef HAVE_VARARGS_H */

/* Define to 1 if you have the `vasprintf' function. */
#define HAVE_VASPRINTF 1

/* Define to 1 if you have the `va_copy' function or macro. */
/* #undef HAVE_VA_COPY */

/* Define to 1 if you have the <vfs.h> header file. */
/* #undef HAVE_VFS_H */

/* Define to 1 if you have a C99 compliant `vsnprintf' function. */
#define HAVE_VSNPRINTF 1

/* Define to 1 if you have the <winsock2.h> header file. */
/* #undef HAVE_WINSOCK2_H */

/* Define to 1 if you have the <ws2tcpip.h> header file. */
/* #undef HAVE_WS2TCPIP_H */

/* Define to 1 if you have the <yaml.h> header file. */
#define HAVE_YAML_H 1

/* Define to 1 if you have the <zone.h> header file. */
/* #undef HAVE_ZONE_H */

/* Define to 1 if the system has the type `_Bool'. */
#define HAVE__BOOL 1

/* Define to 1 if you have the `__va_copy' function or macro. */
/* #undef HAVE___VA_COPY */

/* Define to 1 if you have properly defined `ctime' function */
/* #undef HAVE_ctime_PROPER */

/* Define to 1 if you have properly defined `mkdir' function */
#define HAVE_mkdir_PROPER 1

/* Define to 1 if you have properly defined `rename' function */
#define HAVE_rename_PROPER 1

/* Define to 1 if you have properly defined `stat' function */
#define HAVE_stat_PROPER 1

/* Inputs directory location */
#define INPUTDIR "default"

/* Define if Lightning MDB is available */
#define LMDB 1

/* Logdir location */
#define LOGDIR "/var/cfengine"

/* Define to the sub-directory where libtool stores uninstalled libraries. */
#define LT_OBJDIR ".libs/"

/* Masterfiles directory location */
#define MASTERDIR "default"

/* Define to 1 if your `struct nlist' has an `n_un' member. Obsolete, depend
   on `HAVE_STRUCT_NLIST_N_UN_N_NAME */
/* #undef NLIST_NAME_UNION */

/* Define if non deprecated API is available. */
#define OPENSSL_NO_DEPRECATED 1

/* Name of package */
#define PACKAGE "cfengine"

/* Define to the address where bug reports for this package should be sent. */
#define PACKAGE_BUGREPORT ""

/* Define to the full name of this package. */
#define PACKAGE_NAME "cfengine"

/* Define to the full name and version of this package. */
#define PACKAGE_STRING "cfengine 3.18.0a.33156e1dc"

/* Define to the one symbol short name of this package. */
#define PACKAGE_TARNAME "cfengine"

/* Define to the home page for this package. */
#define PACKAGE_URL ""

/* Define to the version of this package. */
#define PACKAGE_VERSION "3.18.0a.33156e1dc"

/* piddir location */
#define PIDDIR "/var/cfengine"

/* Define to necessary symbol if this constant uses a non-standard name on
   your system. */
/* #undef PTHREAD_CREATE_JOINABLE */

/* Path to pw tool */
/* #undef PW */

/* Path to pwdadm tool */
/* #undef PWDADM */

/* Define if QDBM is available */
/* #undef QDB */

/* Release number */
#define RELEASE "1"

/* Whether sendto does not returns ssize_t */
#define SENDTO_RETURNS_SSIZE_T 1

/* Whether setnetgrent returns int */
#define SETNETGRENT_RETURNS_INT 1

/* Path to the POSIX-compatible shell */
#define SHELL_PATH "/bin/sh"

/* State directory location */
#define STATEDIR "default"

/* Define to 1 if you have the ANSI C header files. */
#define STDC_HEADERS 1

/* Define to 1 on System V Release 4. */
/* #undef SVR4 */

/* Define if Tokyo Cabinet is available. */
/* #undef TCDB */

/* Define to 1 if you can safely include both <sys/time.h> and <time.h>. */
#define TIME_WITH_SYS_TIME 1

/* Define to 1 for Encore UMAX. */
/* #undef UMAX */

/* Define to 1 for Encore UMAX 4.3 that has <inq_status/cpustats.h> instead of
   <sys/cpustats.h>. */
/* #undef UMAX4_3 */

/* Path to useradd tool */
#define USERADD "/sbin/useradd"

/* Path to userdel tool */
#define USERDEL "/sbin/userdel"

/* Path to usermod tool */
#define USERMOD "/sbin/usermod"

/* Version number of package */
#define VERSION "3.18.0a.33156e1dc"

/* Define if you have a libc that supports extended attributes */
#define WITH_XATTR 1

/* Define if your xattr implementation has extra arguments */
/* #undef WITH_XATTR_EXTRA_ARGS */

/* Define WORDS_BIGENDIAN to 1 if your processor stores words with the most
   significant byte first (like Motorola and SPARC, unlike Intel). */
#if defined AC_APPLE_UNIVERSAL_BUILD
# if defined __BIG_ENDIAN__
#  define WORDS_BIGENDIAN 1
# endif
#else
# ifndef WORDS_BIGENDIAN
/* #  undef WORDS_BIGENDIAN */
# endif
#endif

/* Workdir location */
#define WORKDIR "/var/cfengine"

/* Define if XEN cpuid-based HVM detection is available. */
#define XEN_CPUID_SUPPORT 1

/* Define to 1 if `lex' declares `yytext' as a `char *' by default, not a
   `char[]'. */
#define YYTEXT_POINTER 1

/* Enable large inode numbers on Mac OS X 10.5.  */
#ifndef _DARWIN_USE_64_BIT_INODE
# define _DARWIN_USE_64_BIT_INODE 1
#endif

/* Number of bits in a file offset, on hosts where this is settable. */
/* #undef _FILE_OFFSET_BITS */

/* Define to 1 to make fseeko visible on some hosts (e.g. glibc 2.2). */
/* #undef _LARGEFILE_SOURCE */

/* Define for large files, on AIX-style hosts. */
/* #undef _LARGE_FILES */

/* Enable wide data structures everywhere */
/* #undef _PSTAT64 */

/* SUSv3 */
/* #undef _XOPEN_SOURCE */

/* Extended UNIX 98 interfaces */
/* #undef __EXTENSIONS__ */

/* Define to rpl_asprintf if the replacement function should be used. */
/* #undef asprintf */

/* Define to empty if `const' does not conform to ANSI C. */
/* #undef const */

/* Define to rpl_fprintf if the replacement function should be used. */
/* #undef fprintf */

/* Define to `int' if <sys/types.h> doesn't define. */
/* #undef gid_t */

/* Define to the widest signed integer type if <stdint.h> and <inttypes.h> do
   not define. */
/* #undef intmax_t */

/* Define to `int' if <sys/types.h> does not define. */
/* #undef mode_t */

/* Define to `long int' if <sys/types.h> does not define. */
/* #undef off_t */

/* Define to `int' if <sys/types.h> does not define. */
/* #undef pid_t */

/* Define to rpl_printf if the replacement function should be used. */
/* #undef printf */

/* Define to `unsigned int' if <sys/types.h> does not define. */
/* #undef size_t */

/* Define to rpl_snprintf if the replacement function should be used. */
/* #undef snprintf */

/* Define to `int' if <sys/types.h> doesn't define. */
/* #undef uid_t */

/* Define to the widest unsigned integer type if <stdint.h> and <inttypes.h>
   do not define. */
/* #undef uintmax_t */

/* Define to the type of an unsigned integer type wide enough to hold a
   pointer, if such a type exists, and if the system does not define it. */
/* #undef uintptr_t */

/* Define to rpl_vasprintf if the replacement function should be used. */
/* #undef vasprintf */

/* Define to rpl_vfprintf if the replacement function should be used. */
/* #undef vfprintf */

/* Define to rpl_vprintf if the replacement function should be used. */
/* #undef vprintf */

/* Define to rpl_vsnprintf if the replacement function should be used. */
/* #undef vsnprintf */
