#if defined(__GNUC__)
 #define LD_LINUX_SO   "ld-linux.so.2"
 #define LD_SO         "ld-linux.so.2"
 #define LIBANL_SO     "libanl.so.1"
 #define LIBBROKENLOCALE_SO  "libBrokenLocale.so.1"
 #define LIBCIDN_SO          "libcidn.so.1"
 #define LIBCRYPT_SO         "libcrypt.so.1"
 #define LIBC_SO             "libc.so.6"
 #define LIBDL_SO            "libdl.so.2"
 #define LIBM_SO             "libm.so.6"
 #define LIBNSL_SO           "libnsl.so.1"
 #define LIBNSS_COMPAT_SO    "libnss_compat.so.2"
 #define LIBNSS_DNS_SO       "libnss_dns.so.2"
 #define LIBNSS_FILES_SO     "libnss_files.so.2"
 #define LIBNSS_HESIOD_SO    "libnss_hesiod.so.2"
 #define LIBNSS_LDAP_SO      "libnss_ldap.so.2"
 #define LIBNSS_NISPLUS_SO   "libnss_nisplus.so.2"
 #define LIBNSS_NIS_SO       "libnss_nis.so.2"
 #define LIBPTHREAD_SO       "libpthread.so.0"
 #define LIBRESOLV_SO        "libresolv.so.2"
 #define LIBRT_SO            "librt.so.1"
 #define LIBTHREAD_DB_SO     "libthread_db.so.1"
 #define LIBUTIL_SO          "libutil.so.1"
#endif

#if defined(__WINC__) /* support for Windows */
 #define LIB_BGI "libbgi.a"
 #define LIB_KERNEL "kernel32.dll"
 #define LIB_GDI "gdi32.dll"
 #define LIB_USER "user32.dll"
 #define LIB_OLE "ole32.dll"
 #define LIB_MSVCRT "msvcrt.dll"
 #define LIB_NET "netapi32.dll"
 #define LIB_INET "libwininet.a"
 #define LIB_HAL  "hal.dll"
#endif

#if defined(__JAVA__)
 #define LIB_SPRING "spring.jar"
 #define LIB_AWT "awt.jar"
 #define LIB_JCORE "jcore.jar" /* the System class */
 #define LIB_MAVEN "maven.jar"
 #define LIB_APACHE "apache.jar"
#endif

#if defined(__ANDROIDC__)
 #define LIB_ASTDC "libstdc++.so"
#endif

#if defined(__MACC__) /* support for MacOS/iOS */
 #define LIB_SYS "libsystem_c.dylib"
#endif
#define FULL_LIB(x) "/home/lib/"x

void importlib(char* lib){
  char* fname = FULL_LIB(lib);
}