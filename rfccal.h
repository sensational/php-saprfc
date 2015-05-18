/* $Id: rfccal.h,v 1.15 2005/12/18 16:25:42 koucky Exp $
 * SAP RFC Client Abstraction Layer (CAL)
 * Author: Eduard Koucky <eduard.koucky@czech-tv.cz> (c) 2001
 *
 * Description:
 *
 *     The collection of routines for simplification the remote calling of ABAP function
 * modules in the SAP R/3 system from ANSI C client programs. Primary developed for SAPRFC
 * extension module for the PHP 4.0 (www.php.net).
 *
 * Routines use Remote Function Call API of the SAP RFCSDK library (www.sap.com).
 *
 * Properties:
 *     -  the conversion of RFC types to (from) string
 *     -  discovering interface of function module (export, import parameters and internal tables)
 *     -  the calling function module
 *
 * Example:
 *
 *    Call function RFC_GET_STRUCTURE_DEFINITION
 *       exporting
 *          TABNAME like X030L_TABNAME  type RFC_CHAR length 30
 *       importing
 *          TABLENGTH like RFC_FIELDS_INTLENGTH type RFC_INT length 4
 *       tables
 *          FIELDS structure RFC_FIELDS length 80 number of fields 7
 *       exceptions
 *          TABLE_NOT_ACTIVE
 *
 *
 *     rfc = CAL_OPEN(.....);     #open RFC connetion to R/3 system
 *     fce = CAL_NEW("RFC_GET_STRUCTURE_DEFINITION");
 *     CAL_DISCOVER_INTERFACE(fce,rfc);
 *     CAL_SET_IMPORT(fce,"TABNAME"."ZMYTABLE")  // set import parametr TABNAME to value "ZMYTABLE"
 *     CAL_CALL(fce,rfc);
 *     value = CAL_GET_EXPORT(fce,"TABLENGTH");
 *     .......
 *
 */


#include "saprfc.h"
#include "sapitab.h"
#include "rfcsi.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>


#ifndef STANDALONE
    /*  Memory allocation macros,
        If CAL is used with the SAPRFC PHP extension,
        use PHP memory management functions */

      #include "php.h"
      #include "php_ini.h"

      #define VMALLOC(size)         emalloc((size))       /* PHP module */
      #define VFREE(ptr)            efree((ptr))
      #define VCALLOC(size)         ecalloc((1),(size))
      #define VREALLOC(ptr,size)    erealloc((ptr),(size))
      #define VSTRDUP(s)            estrdup((s))
      #define VSTRNDUP(s,length)    estrndup((s),(length))
#else
      #define VMALLOC(size)         malloc((size))       /* C program */
      #define VFREE(ptr)            free((ptr))
      #define VCALLOC(size)         calloc((1),(size))
      #define VREALLOC(ptr,size)    realloc((ptr),(size))
      #define VSTRDUP(s)            strdup((s))
      #define VSTRNDUP(s,length)    strndup((s),(length))
#endif

#ifndef RFCCAL_H
	#define RFCCAL_H

/* Constants */

   #define CALC_UNDEF       0        /* interface type */
   #define CALC_IMPORT      1
   #define CALC_EXPORT      2
   #define CALC_TABLE       3

   #define CALC_READ        0        /* table operation */
   #define CALC_APPEND      1
   #define CALC_INSERT      2
   #define CALC_MODIFY      3
   #define CALC_REMOVE      4
   #define CALC_LENGTH      5
   #define CALC_INIT        6

   #define PHP_RFC_TIMEOUT_EXPIRED -1


/* Public interface:
 *
 *   CAL_NEW(name)                     create the new function module object
 */
   #define CAL_NEW(name)               __cal_fce_new ((name))

/*   CAL_DEF.....                        type definition macros
 */

   #define CAL_DEF_ELEMENT_CHAR(len)          "",'C',(len),0
   #define CAL_DEF_ELEMENT_DATE()             "",'D',8,0
   #define CAL_DEF_ELEMENT_TIME()             "",'T',6,0
   #define CAL_DEF_ELEMENT_RAW(len)           "",'X',(len),0
   #define CAL_DEF_ELEMENT_NUM(len)           "",'N',(len),0
   #define CAL_DEF_ELEMENT_BCD(len,dec)       "",'P',(len),(dec)
   #define CAL_DEF_ELEMENT_INT1()             "",'s',sizeof(RFC_INT1),0
   #define CAL_DEF_ELEMENT_INT2()             "",'b',sizeof(RFC_INT2),0
   #define CAL_DEF_ELEMENT_INT()              "",'I',sizeof(RFC_INT),0
   #define CAL_DEF_ELEMENT_FLOAT()            "",'F',sizeof(RFC_FLOAT),0

   #define CAL_DEF_STRUCTPART_CHAR(item,len)  (item),'C',(len),0
   #define CAL_DEF_STRUCTPART_DATE(item)      (item),'D',8,0
   #define CAL_DEF_STRUCTPART_TIME(item)      (item),'T',6,0
   #define CAL_DEF_STRUCTPART_RAW(item,len)   (item),'X',(len),0
   #define CAL_DEF_STRUCTPART_NUM(item,len)   (item),'N',(len),0
   #define CAL_DEF_STRUCTPART_BCD(item,len,dec)   (item),'P',(len),(dec)
   #define CAL_DEF_STRUCTPART_INT1(item)      (item),'s',sizeof(RFC_INT1),0
   #define CAL_DEF_STRUCTPART_INT2(item)      (item),'b',sizeof(RFC_INT2),0
   #define CAL_DEF_STRUCTPART_INT(item)       (item),'I',sizeof(RFC_INT),0
   #define CAL_DEF_STRUCTPART_FLOAT(item)     (item),'F',sizeof(RFC_FLOAT),0


/*   CAL_INTERFACE_IMPORT(fce,name,def)      definition of import parameter
     CAL_INTERFACE_EXPORT(fce,name,def)      definition of export parameter
     CAL_INTERFACE_TABLE(fce,name,def)       definition of internal table
                                                 def is type definition macro)
 */
   #define CAL_INTERFACE_IMPORT(fce,name,def)    __cal_fce_interface ((fce),CALC_IMPORT,(name),def)
   #define CAL_INTERFACE_EXPORT(fce,name,def)    __cal_fce_interface ((fce),CALC_EXPORT,(name),def)
   #define CAL_INTERFACE_TABLE(fce,name,def)     __cal_fce_interface ((fce),CALC_TABLE,(name),def)

   #define CAL_INTERFACE_IMPORT_RAW(fce,name,item,abap,len,dec)    __cal_fce_interface ((fce),CALC_IMPORT,(name),(item),(abap),(len),(dec))
   #define CAL_INTERFACE_EXPORT_RAW(fce,name,item,abap,len,dec)    __cal_fce_interface ((fce),CALC_EXPORT,(name),(item),(abap),(len),(dec))
   #define CAL_INTERFACE_TABLE_RAW(fce,name,item,abap,len,dec)     __cal_fce_interface ((fce),CALC_TABLE,(name),(item),(abap),(len),(dec))

   #define CAL_INTERFACE_IMPORT_OPT(fce,name,opt)    __cal_fce_optional ((fce),CALC_IMPORT,(name),(opt))
   #define CAL_INTERFACE_EXPORT_OPT(fce,name,opt)    __cal_fce_optional ((fce),CALC_EXPORT,(name),(opt))
   #define CAL_INTERFACE_TABLE_OPT(fce,name,opt)     __cal_fce_optional ((fce),CALC_TABLE,(name),(opt))

/*   CAL_INTERFACE_INFO(fce)
*/
   #define CAL_INTERFACE_INFO(fce)                    __cal_fce_getinterface ((fce),NULL,CALC_UNDEF)
   #define CAL_SINGLE_INTERFACE_INFO(fce,name,type)   __cal_fce_getinterface ((fce),(name),(type))

/*  CAL_DEF_....                             get item type and length
*/

   #define CAL_DEF_IMPORT_TYPE(fce,name)                    __cal_def_type (fce,CALC_IMPORT,(name),"")
   #define CAL_DEF_IMPORT_STRUCT_TYPE(fce,name,item)        __cal_def_type (fce,CALC_IMPORT,(name),(item))
   #define CAL_DEF_EXPORT_TYPE(fce,name)                    __cal_def_type (fce,CALC_EXPORT,(name),"")
   #define CAL_DEF_EXPORT_STRUCT_TYPE(fce,name,item)        __cal_def_type (fce,CALC_EXPORT,(name),(item))
   #define CAL_DEF_TABLE_TYPE(fce,name,item)                __cal_def_type (fce,CALC_TABLE,(name),(item))

   #define CAL_DEF_IMPORT_LENGTH(fce,name)                  __cal_def_length (fce,CALC_IMPORT,(name),"")
   #define CAL_DEF_IMPORT_STRUCT_LENGTH(fce,name,item)      __cal_def_length (fce,CALC_IMPORT,(name),(item))
   #define CAL_DEF_EXPORT_LENGTH(fce,name)                  __cal_def_length (fce,CALC_EXPORT,(name),"")
   #define CAL_DEF_EXPORT_STRUCT_LENGTH(fce,name,item)      __cal_def_length (fce,CALC_EXPORT,(name),(item))
   #define CAL_DEF_TABLE_LENGTH(fce,name,item)              __cal_def_length (fce,CALC_TABLE,(name),(item))

/* CAL_SET...                                  set import/export/table
   CAL_GET...                                  get import/export/table
*/
   #define CAL_SET_IMPORT(fce,name,value)              __cal_set (fce,CALC_IMPORT,(name),"",(value))
   #define CAL_SET_IMPORT_STRUCT(fce,name,item,value)  __cal_set (fce,CALC_IMPORT,(name),(item),(value))
   #define CAL_SET_EXPORT(fce,name,value)              __cal_set (fce,CALC_EXPORT,(name),"",(value))
   #define CAL_SET_EXPORT_STRUCT(fce,name,item,value)  __cal_set (fce,CALC_EXPORT,(name),(item),(value))
   #define CAL_SET_TABLE(fce,name,item,value)          __cal_set (fce,CALC_TABLE,(name),(item),(value))
   #define CAL_GET_IMPORT(fce,name)                    __cal_get (fce,CALC_IMPORT,(name),"")
   #define CAL_GET_IMPORT_STRUCT(fce,name,item)        __cal_get (fce,CALC_IMPORT,(name),(item))
   #define CAL_GET_EXPORT(fce,name)                    __cal_get (fce,CALC_EXPORT,(name),"")
   #define CAL_GET_EXPORT_STRUCT(fce,name,item)        __cal_get (fce,CALC_EXPORT,(name),(item))
   #define CAL_GET_TABLE(fce,name,item)                __cal_get (fce,CALC_TABLE,(name),(item))

/* CAL_TBL....                                 operation with internal table
*/
   #define CAL_TBL_READ(fce,name,index)      __cal_table ((fce),CALC_READ,(name),(index))
   #define CAL_TBL_APPEND(fce,name)          __cal_table ((fce),CALC_APPEND,(name),0)
   #define CAL_TBL_MODIFY(fce,name,index)    __cal_table ((fce),CALC_MODIFY,(name),(index))
   #define CAL_TBL_REMOVE(fce,name,index)    __cal_table ((fce),CALC_REMOVE,(name),(index))
   #define CAL_TBL_INSERT(fce,name,index)    __cal_table ((fce),CALC_INSERT,(name),(index))
   #define CAL_TBL_LENGTH(fce,name)          __cal_table ((fce),CALC_LENGTH,(name),0)
   #define CAL_TBL_INIT(fce,name)            __cal_table ((fce),CALC_INIT,(name),0)

/*   CAL_DISCOVER_INTERFACE(fce,rfc)             discover and setup interface for function module
     CAL_CALL(fce,rfc)                           call function modul
     CAL_DELETE(fce)                             deallocate memory for function module
     CAL_DEL_INTERFACE(iface)                    deallocate memory for interface definition
*/

   #define CAL_DISCOVER_INTERFACE(fce,rfc)       __cal_fce_discover_interface((fce),(rfc))
   #define CAL_INIT_INTERFACE(fce)               __cal_fce_init_interface((fce),CALC_UNDEF)
   #define CAL_INIT_INTERFACE_IMPORT(fce)        __cal_fce_init_interface((fce),CALC_IMPORT)
   #define CAL_INIT_INTERFACE_EXPORT(fce)        __cal_fce_init_interface((fce),CALC_EXPORT)
   #define CAL_INIT_INTERFACE_TABLE(fce)         __cal_fce_init_interface((fce),CALC_TABLE)
   #define CAL_REFRESH_INTERNAL_BUFFER(fce,name,type) __cal_fce_refresh_internal_buffer((fce),(name),(type))
   #define CAL_CALL(fce,rfc)					           __cal_fce_call((fce),(rfc),-1)
   #define CAL_CALL_TIMEOUT(fce,rfc,timeout)     __cal_fce_call((fce),(rfc),(timeout))
   #define CAL_INDIRECT_CALL(fce,rfc,tid)		 __cal_fce_indirect_call((fce),(rfc),(tid))
   #define CAL_DELETE(fce)                       __cal_del_fce((fce))
   #define CAL_DEL_INTERFACE(iface)              __cal_del_interface((iface))
   #define CAL_DEBUG_MESSAGE()                   __cal_get_internal_error_msg()
   #define CAL_RFC_LAST_ERROR()                  __cal_last_error()
   #define CAL_RFC_LIB_VERSION()                 __cal_lib_version ()

/* Others */

   #define CAL_INIT()   __cal_install_enviroment()
   #define CAL_DONE()   __cal_deinstall_enviroment()
   #define CAL_OPEN(connect_param)  _cal_open((connect_param))
   #define CAL_CLOSE(rfc)  RfcClose((rfc))
   #define CAL_SET_CODE_PAGE(rfc,codepage)      RfcSetCodePage((rfc),(codepage))
   #define CAL_SET_SYSTEM_CODE_PAGE(codepage)   RfcSetSystemCodePage((codepage))
   #define CAL_ATTRIBUTES(rfc,attributes)       RfcGetAttributes((rfc),(attributes))
   #define CAL_CREATE_TID(rfc,tid)              RfcCreateTransID((rfc),(tid))
   #define CAL_SET_TRACE(rfc,level)             RfcSetTrace((rfc),(level))
   #define CAL_SET_RAWSTR(fce)                  __cal_set_rawstr ((fce))
   #define CAL_ALLOW_START_PROGRAM(progs)	    RfcAllowStartProgram((progs))
   #define CAL_GET_TICKET(rfc,ticket)           RfcGetTicket((rfc),(ticket))

/* Server extension */

   #define SAL_ACCEPT(argv)                  RfcAccept((argv))
   #define SAL_ACCEPT_EXT(cmdline)           RfcAcceptExt((cmdline))
   #define SAL_WAIT_FOR_REQUEST(rfc,wtime)   RfcWaitForRequest((rfc),(wtime))
   #define SAL_GET_NAME(rfc,function_name)   RfcGetNameEx((rfc),(function_name))
   #define SAL_ABORT(rfc,text)               RfcAbort((rfc),(text))
   #define SAL_DISPATCH(rfc)                 RfcDispatch((rfc))

   #define SAL_GET_DATA(rfc,fce)             __sal_get_data ((rfc),(fce))
   #define SAL_SEND_DATA(rfc,fce)            __sal_send_data ((rfc),(fce))
   #define SAL_RAISE(rfc,fce,exception)      __sal_raise ((rfc),(fce),(exception))
   #define SAL_INSTALL_TRANSACTION_CONTROL(TID_check,TID_commit,TID_rollback,TID_confirm)     RfcInstallTransactionControl((RFC_ON_CHECK_TID) (TID_check),(RFC_ON_COMMIT) (TID_commit),(RFC_ON_ROLLBACK) (TID_rollback), (RFC_ON_CONFIRM_TID) (TID_confirm))
   #define SAL_INSTALL_FUNCTION(function_name,f_ptr,docu) RfcInstallFunction((function_name),(RFC_ONCALL)(f_ptr),(docu))
   #define SAL_CHECK_REGISTER(tpid,gwhost,gwserver,ntotal,ninit,nready,nbusy,error_info) RfcCheckRegisterServer((tpid),(gwhost),(gwserver),(ntotal),(ninit),(nready),(nbusy),(error_info))
   #define SAL_CANCEL_REGISTER(tpid,gwhost,gwserver,ntotal,ncancel,error_info) RfcCancelRegisterServer((tpid),(gwhost),(gwserver),(ntotal),(ncancel),(error_info))

/* Public structures */

/* Type definition */


#define CALC_DEFINITION_SZ		30
typedef struct CALD_DEFINITION CALD_DEFINITION;
typedef struct CALD_DEFINITION {
    char             item[CALC_DEFINITION_SZ+1];
    RFC_TYPEHANDLE   type;
    unsigned         len;
    unsigned         decimals;
    unsigned         _not_used;
    char             *retbuf;
    CALD_DEFINITION  *next;
    int              is_rawstr;                       /* 1 if string TYPC shouldn't trimmed in conversion RFC->PHP string */
    int              offset;                          /* offset position in the structure */
} __CALD_DEFINITION;


/* Interface item: export/import parameter or internal table */

#define CALC_INTERFACE_SZ		30
typedef struct CALD_INTERFACE CALD_INTERFACE;
typedef struct CALD_INTERFACE {
    char             name[CALC_INTERFACE_SZ+1];         /* name of interface item */
    int              type;                              /* CALC_UNDEF, CALC_IMPORT, CALC_EXPORT, CALC_TABLE  */
    CALD_DEFINITION  *def;                              /* type definition list */
    RFC_TYPE_ELEMENT *defarray;
    int              num;                               /* number of definition items */
    char             *buffer;                           /* buffer for value */
    int              buflen;                            /* size of buffer (in bytes) */
    RFC_TYPEHANDLE   typehandle;                        /* type handle for structure: RfcInstallStructure */
    ITAB_H           handle;                            /* handle for internal table */
    int              is_optional;                       /* 1 if interface is optional */
    int              is_set;                            /* 1 if parameter was set */
    int              is_rfcgetdata_table;               /* 1 if internal table is created by RfcGetData */
    CALD_INTERFACE   *next;
} __CALD_INTERFACE;

/* Function module */

#define CALC_FUNCNAME_SZ		30
typedef struct {
    char             name[CALC_FUNCNAME_SZ+1];      /* name of function module */
    CALD_INTERFACE   *iface;                        /* interface definition */
    char             *exception;                    /* exception */
    char             rfcsaprl[5];                   /* remote SAP R/3 release */
    int              unicode;                       /* 1 if target SAP system UNICODE */
    int              par_offset;                    /* parameter for offset setting when define interface */
                                                    /* before 1.1 was offset computed, after detected from RFC_GET_STRUCTURE_DEFINITION_P */
} CALD_FUNCTION_MODULE;


/* Type info */

typedef struct {
    char            *name;
    char            abap;
    unsigned        length;
    unsigned        decimals;
    int             offset;
} CALD_INTERFACE_TYPE_INFO;

/* Interface info */

typedef struct {
    char                      *name;
    int                       size;
    int                       type;
    int                       is_optional;
    int                       buflen;
    CALD_INTERFACE_TYPE_INFO  *typeinfo;
}  CALD_INTERFACE_INFO;


/* Public routines - don't use them. Use public interface macros only. */

extern RFC_HANDLE  _cal_open (char *connect_param);
extern CALD_FUNCTION_MODULE *__cal_fce_new (char *name);
extern int __cal_fce_interface (CALD_FUNCTION_MODULE *fce, int type, char *name, char *item, char abap, unsigned len, unsigned decimals);
extern CALD_INTERFACE_INFO *__cal_fce_getinterface (CALD_FUNCTION_MODULE *fce, char *name, int type);
extern int __cal_set (CALD_FUNCTION_MODULE *fce, int type, char *name, char *item, char *value);
extern char *__cal_get (CALD_FUNCTION_MODULE *fce, int type, char *name, char *item);
extern int __cal_table (CALD_FUNCTION_MODULE *fce, int oper, char *name, int index);
extern int __cal_fce_discover_interface (CALD_FUNCTION_MODULE *fce, RFC_HANDLE rfc);
extern void __cal_fce_init_interface (CALD_FUNCTION_MODULE *fce, int type);
extern int __cal_fce_refresh_internal_buffer (CALD_FUNCTION_MODULE *fce, char *name, int type);
extern int __cal_fce_optional (CALD_FUNCTION_MODULE *fce, int type, char *name, int opt);
extern int __cal_fce_call (CALD_FUNCTION_MODULE *fce, RFC_HANDLE rfc, long timeout);
extern int __cal_fce_indirect_call (CALD_FUNCTION_MODULE *fce, RFC_HANDLE rfc, char *tid);
extern void __cal_del_fce (CALD_FUNCTION_MODULE *fce);
extern void __cal_del_interface (CALD_INTERFACE_INFO *iinfo);
extern char __cal_def_type (CALD_FUNCTION_MODULE *fce, int type, char *name, char *item);
extern int __cal_def_length (CALD_FUNCTION_MODULE *fce, int type, char *name, char *item);
extern char *__cal_get_internal_error_msg();
extern char *__cal_last_error ();
extern char *__cal_lib_version ();
extern void __cal_install_enviroment (void);
extern void __cal_deinstall_enviroment (void);
extern int __sal_get_data (RFC_HANDLE rfc, CALD_FUNCTION_MODULE *fce);
extern int __sal_send_data (RFC_HANDLE rfc, CALD_FUNCTION_MODULE *fce);
extern int __sal_raise (RFC_HANDLE rfc, CALD_FUNCTION_MODULE *fce, char *exception);
extern void __cal_set_rawstr (CALD_FUNCTION_MODULE *fce);



/* -------------------- Private macros and functions ------------------ */
    #define __MIN(a,b)     (a)<(b) ? (a) : (b)


/* Supported SAP RFC elementary data types (defined in saprfc.h):
 *
 *  C type     ABAP  handle   description
 *  -----------------------------------------------------------------------------------------
 *      RFC_CHAR        "C", TYPC     blank padded character string of fixed length /unsigned char/
 *	RFC_NUM		"N", TYPNUM   character string of fixed length containing only digits, '0' padded./unsigned char/
 *	RFC_BYTE	"X", TYPX     raw data. fixed length, zero padded. /unsigned char/
 *	RFC_BCD		"P", TYPP     packed number in BCD format (binary coded decimal) any length between 1 and 16 bytes/unsigned char/
 *	RFC_INT1	"b", TYPINT1  1-byte unsigned integer./unsigned char/
 *	RFC_INT2	"s", TYPINT2  2-byte signed integer. /short/
 *	RFC_INT		"I", TYPINT   4-byte signed integer /long int/.
 *      RFC_FLOAT       "F", TYPFLOAT 8-byte IEEE floating point number /double/.
 *	RFC_DATE	"D", TYPDATE  8-byte date (YYYYMMDD) /unsigned char[8]/
 *	RFC_TIME	"T", TYPTIME  6-byte time (HHMMSS) /unsigned char[6]/
 */
#endif
