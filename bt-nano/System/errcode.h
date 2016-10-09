
//sys err code

#define     RK_SUCCESS                       0
#define     RK_ERROR                         -1
#define     RK_PARA_ERR                      -2
#define     RK_TIMEOUT                       -3
#define     RK_EXIST                         -4
#define     RK_EMPTY                         -5

//file sytem err code
#define FDT_OVER -300
#define DISK_FULL -301
#define FILE_EXIST -302
#define ROOT_FULL -303 //FAT16 OR FAT12 ROOT

//fw update err code
#define DATA_LOSS -400
#define RESTART_SYSTEM -401

// linux err code
#define    EPERM         1001    /* Operation not permitted */
#define    ENOENT         1002    /* No such file or directory */
#define    ESRCH         1003    /* No such process */
#define    EINTR         1004    /* Interrupted system call */
#define    EIO             1005    /* I/O error */
#define    ENXIO         1006    /* No such device or address */
#define    E2BIG         1007    /* Argument list too long */
#define    ENOEXEC         1008    /* Exec format error */
#define    EBADF         1009    /* Bad file number */
#define    ECHILD        1010    /* No child processes */
#define    EAGAIN        1011    /* Try again */
#define    ENOMEM        1012    /* Out of memory */
#define    EACCES        1013    /* Permission denied */
#define    EFAULT        1014    /* Bad address */
#define    ENOTBLK        1015    /* Block device required */
#define    EBUSY        1016    /* Device or resource busy */
#define    EEXIST        1017    /* File exists */
#define    EXDEV        1018    /* Cross-device link */
#define    ENODEV        1019    /* No such device */
#define    ENOTDIR        1020    /* Not a directory */
#define    EISDIR        1021    /* Is a directory */
#define    EINVAL        1022    /* Invalid argument */
#define    ENFILE        1023    /* File table overflow */
#define    EMFILE        1024    /* Too many open files */
#define    ENOTTY        1025    /* Not a typewriter */
#define    ETXTBSY        1026    /* Text file busy */
#define    EFBIG        1027    /* File too large */
#define    ENOSPC        1028    /* No space left on device */
#define    ESPIPE        1029    /* Illegal seek */
#define    EROFS        1030    /* Read-only file system */
#define    EMLINK        1031    /* Too many links */
#define    EPIPE        1032    /* Broken pipe */
#define    EDOM        1033    /* Math argument out of domain of func */
#define    ERANGE        1034    /* Math result not representable */


//in file errno.h now add to one file.
#define    ENOMSG        1035    /* No message of desired type */
#define    EIDRM        1036    /* Identifier removed */
#define    ECHRNG        1037    /* Channel number out of range */
#define    EL2NSYNC    1038    /* Level 2 not synchronized */
#define    EL3HLT        1039    /* Level 3 halted */
#define    EL3RST        1040    /* Level 3 reset */
#define    ELNRNG        1041    /* Link number out of range */
#define    EUNATCH        1042    /* Protocol driver not attached */
#define    ENOCSI        1043    /* No CSI structure available */
#define    EL2HLT        1044    /* Level 2 halted */
#define    EDEADLK        1045    /* Resource deadlock would occur */
#define    EDEADLOCK    EDEADLK
#define    ENOLCK        1046    /* No record locks available */
#define    EILSEQ        1047    /* Illegal byte sequence */

#define    ENONET        1050    /* Machine is not on the network */
#define    ENODATA        1051    /* No data available */
#define    ETIME        1052    /* Timer expired */
#define    ENOSR        1053    /* Out of streams resources */
#define    ENOSTR        1054    /* Device not a stream */
#define    ENOPKG        1055    /* Package not installed */

#define    ENOLINK        1057    /* Link has been severed */
#define    EADV        1058    /* Advertise error */
#define    ESRMNT        1059    /* Srmount error */
#define    ECOMM        1060    /* Communication error on send */
#define    EPROTO        1061    /* Protocol error */

#define    EMULTIHOP    1064    /* Multihop attempted */

#define    EDOTDOT        1066    /* RFS specific error */
#define    EBADMSG        1067    /* Not a data message */
#define    EUSERS        1068    /* Too many users */
#define    EDQUOT        1069    /* Quota exceeded */
#define    ESTALE        1070    /* Stale NFS file handle */
#define    EREMOTE        1071    /* Object is remote */
#define    EOVERFLOW    1072    /* Value too large for defined data type */

/* these errnos are defined by Linux but not HPUX. */

#define    EBADE        1160    /* Invalid exchange */
#define    EBADR        1161    /* Invalid request descriptor */
#define    EXFULL        1162    /* Exchange full */
#define    ENOANO        1163    /* No anode */
#define    EBADRQC        1164    /* Invalid request code */
#define    EBADSLT        1165    /* Invalid slot */
#define    EBFONT        1166    /* Bad font file format */
#define    ENOTUNIQ    1167    /* Name not unique on network */
#define    EBADFD        1168    /* File descriptor in bad state */
#define    EREMCHG        1169    /* Remote address changed */
#define    ELIBACC        1170    /* Can not access a needed shared library */
#define    ELIBBAD        1171    /* Accessing a corrupted shared library */
#define    ELIBSCN        1172    /* .lib section in a.out corrupted */
#define    ELIBMAX        1173    /* Attempting to link in too many shared libraries */
#define    ELIBEXEC    1174    /* Cannot exec a shared library directly */
#define    ERESTART    1175    /* Interrupted system call should be restarted */
#define    ESTRPIPE    1176    /* Streams pipe error */
#define    EUCLEAN        1177    /* Structure needs cleaning */
#define    ENOTNAM        1178    /* Not a XENIX named type file */
#define    ENAVAIL        1179    /* No XENIX semaphores available */
#define    EISNAM        1180    /* Is a named type file */
#define    EREMOTEIO    1181    /* Remote I/O error */
#define    ENOMEDIUM    1182    /* No medium found */
#define    EMEDIUMTYPE    1183    /* Wrong medium type */
#define    ENOKEY        1184    /* Required key not available */
#define    EKEYEXPIRED    1185    /* Key has expired */
#define    EKEYREVOKED    1186    /* Key has been revoked */
#define    EKEYREJECTED    1187    /* Key was rejected by service */

/* We now return you to your regularly scheduled HPUX. */

#define ENOSYM        1215    /* symbol does not exist in executable */
#define    ENOTSOCK    1216    /* Socket operation on non-socket */
#define    EDESTADDRREQ    1217    /* Destination address required */
#define    EMSGSIZE    1218    /* Message too long */
#define    EPROTOTYPE    1219    /* Protocol wrong type for socket */
#define    ENOPROTOOPT    1220    /* Protocol not available */
#define    EPROTONOSUPPORT    1221    /* Protocol not supported */
#define    ESOCKTNOSUPPORT    1222    /* Socket type not supported */
#define    EOPNOTSUPP    1223    /* Operation not supported on transport endpoint */
#define    EPFNOSUPPORT    1224    /* Protocol family not supported */
#define    EAFNOSUPPORT    1225    /* Address family not supported by protocol */
#define    EADDRINUSE    1226    /* Address already in use */
#define    EADDRNOTAVAIL    1227    /* Cannot assign requested address */
#define    ENETDOWN    1228    /* Network is down */
#define    ENETUNREACH    1229    /* Network is unreachable */
#define    ENETRESET    1230    /* Network dropped connection because of reset */
#define    ECONNABORTED    1231    /* Software caused connection abort */
#define    ECONNRESET    1232    /* Connection reset by peer */
#define    ENOBUFS        1233    /* No buffer space available */
#define    EISCONN        1234    /* Transport endpoint is already connected */
#define    ENOTCONN    1235    /* Transport endpoint is not connected */
#define    ESHUTDOWN    1236    /* Cannot send after transport endpoint shutdown */
#define    ETOOMANYREFS    1237    /* Too many references: cannot splice */
#define EREFUSED    ECONNREFUSED    /* for HP's NFS apparently */
#define    ETIMEDOUT    1238    /* Connection timed out */
#define    ECONNREFUSED    1239    /* Connection refused */
#define EREMOTERELEASE    1240    /* Remote peer released connection */
#define    EHOSTDOWN    1241    /* Host is down */
#define    EHOSTUNREACH    1242    /* No route to host */

#define    EALREADY    1244    /* Operation already in progress */
#define    EINPROGRESS    1245    /* Operation now in progress */
#define    EWOULDBLOCK    1246    /* Operation would block (Linux returns EAGAIN) */
#define    ENOTEMPTY    1247    /* Directory not empty */
#define    ENAMETOOLONG    1248    /* File name too long */
#define    ELOOP        1249    /* Too many symbolic links encountered */
#define    ENOSYS        1251    /* Function not implemented */

#define ENOTSUP        1252    /* Function not implemented (POSIX.4 / HPUX) */
#define ECANCELLED    1253    /* aio request was canceled before complete (POSIX.4 / HPUX) */
#define ECANCELED    ECANCELLED    /* SuSv3 and Solaris wants one 'L' */

/* for robust mutexes */
#define EOWNERDEAD    1254    /* Owner died */
#define ENOTRECOVERABLE    1255    /* State not recoverable */

#define    ERFKILL        1256    /* Operation not possible due to RF-kill */

#define EHWPOISON    1257    /* Memory page has hardware error */

