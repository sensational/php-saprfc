/*
   +----------------------------------------------------------------------+
   | PHP version 4.0                                                      |
   +----------------------------------------------------------------------+
   | Copyright (c) 1997, 1998, 1999, 2000, 2001 The PHP Group             |
   +----------------------------------------------------------------------+
   | This source file is subject to version 2.02 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available at through the world-wide-web at                           |
   | http://www.php.net/license/2_02.txt.                                 |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
   | Author: Eduard Koucky <eduard.koucky@czech-tv.cz>                    |
   +----------------------------------------------------------------------+
   $Id: saprfc.c,v 1.43 2005/12/19 11:35:13 koucky Exp $   
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "php_globals.h"
#include "php_saprfc.h"
#include "ext/standard/info.h"

#define  SAPRFC_VERSION  "1.4.1"
#define  SAPRFC_RELEASE  "2005/12/19"

/* Compatibility with PHP 4.0.6 */
#if ZEND_MODULE_API_NO < 20010901
    #define TSRMLS_DC
    #define TSRMLS_CC
    #define TSRMLS_FETCH() ELS_FETCH()
#endif

ZEND_DECLARE_MODULE_GLOBALS(saprfc)

/* True global resources - no need for thread safety here */
static int le_rfc, le_function;

/* Every user visible function must have an entry in saprfc_functions[].
*/
function_entry saprfc_functions[] = {
    PHP_FE(saprfc_open,    NULL)
    PHP_FE(saprfc_function_discover,    NULL)
    PHP_FE(saprfc_function_define,    NULL)
    PHP_FE(saprfc_function_interface,    NULL)
    PHP_FE(saprfc_function_debug_info,    NULL)
    PHP_FE(saprfc_optional,    NULL)
    PHP_FE(saprfc_import,    NULL)
    PHP_FE(saprfc_export,    NULL) 
    PHP_FE(saprfc_table_init,    NULL)
    PHP_FE(saprfc_table_append,    NULL)
    PHP_FE(saprfc_table_insert,    NULL)
    PHP_FE(saprfc_table_modify,    NULL)
    PHP_FE(saprfc_table_remove,    NULL)
    PHP_FE(saprfc_table_read,    NULL)
    PHP_FE(saprfc_table_rows,    NULL)
    PHP_FE(saprfc_call_and_receive,    NULL)
    PHP_FE(saprfc_error,    NULL)
    PHP_FE(saprfc_function_free,    NULL)
    PHP_FE(saprfc_close,    NULL)
    PHP_FE(saprfc_set_code_page,    NULL)
    PHP_FE(saprfc_attributes,    NULL)
    PHP_FE(saprfc_server_accept,    NULL)
    PHP_FE(saprfc_server_import,    NULL)
    PHP_FE(saprfc_server_export,    NULL)
    PHP_FE(saprfc_server_dispatch,  NULL)
    PHP_FE(saprfc_trfc_install,  NULL)
    PHP_FALIAS(saprfc_trfc_dispatch, saprfc_server_dispatch,  NULL)
    PHP_FE(saprfc_trfc_call,  NULL)
    PHP_FE(saprfc_trfc_tid,  NULL)
    PHP_FE(saprfc_set_trace,  NULL)
    PHP_FE(saprfc_server_register_check,  NULL)
    PHP_FE(saprfc_server_register_cancel,  NULL)
    PHP_FE(saprfc_function_name,  NULL)
    PHP_FE(saprfc_exception,  NULL)
	  PHP_FE(saprfc_allow_start_program, NULL)
	  PHP_FE(saprfc_get_ticket, NULL)
    {NULL, NULL, NULL}    /* Must be the last line in saprfc_functions[] */
};

zend_module_entry saprfc_module_entry = {
#if ZEND_MODULE_API_NO >= 20010901
    STANDARD_MODULE_HEADER,
#endif
    "saprfc",
    saprfc_functions,
    PHP_MINIT(saprfc),
    PHP_MSHUTDOWN(saprfc),
    PHP_RINIT(saprfc),        /* Replace with NULL if there's nothing to do at request start */
    PHP_RSHUTDOWN(saprfc),    /* Replace with NULL if there's nothing to do at request end */
    PHP_MINFO(saprfc),
#if ZEND_MODULE_API_NO >= 20010901
    NO_VERSION_YET,          /* extension version number (string) */
#endif
    STANDARD_MODULE_PROPERTIES
};

#ifdef COMPILE_DL_SAPRFC
ZEND_GET_MODULE(saprfc)
#endif

static char *strtoupper (char *s)
{
    char *p;

    p=s;
    if (s)
      while (*p) { *p = toupper (*p); p++; }
    return (s); 
}


static char *strsafecpy (char *dest, char *src, int len)
{
    strncpy (dest,src,len);
    dest[len]=0;
    return (dest);
}


static void _free_resource_rfc(zend_rsrc_list_entry *rsrc TSRMLS_DC)
{
    RFC_RESOURCE *rfc_resource = (RFC_RESOURCE *)rsrc->ptr;

    CAL_CLOSE(rfc_resource->handle);
    efree (rfc_resource);    
}


static void _free_resource_function(zend_rsrc_list_entry *rsrc TSRMLS_DC)
{
    FCE_RESOURCE *fce_resource = (FCE_RESOURCE *)rsrc->ptr;

    CAL_DELETE(fce_resource->fce);
    efree (fce_resource);    
}

static void php_saprfc_init_globals (zend_saprfc_globals *saprfc_globals)
{
    saprfc_globals->trfc_install_flag = 0;
    saprfc_globals->trfc_tid_check = NULL;
    saprfc_globals->trfc_tid_commit = NULL;
    saprfc_globals->trfc_tid_rollback = NULL;
    saprfc_globals->trfc_tid_confirm = NULL;
    saprfc_globals->trfc_dispatcher = NULL;
}


/* callback functions for tRFC server */

/* dispatch tRFC function call */
static RFC_RC DLL_CALL_BACK_FUNCTION __callback_dispatch (RFC_HANDLE rfc_handle)
{
    RFC_RC rfc_rc;
    RFC_FUNCTIONNAME function_name;
    FCE_RESOURCE *fce_resource;
    zval *name, *retval;
    zval *callback_function, *callback_name, *callback_retval;
    zval **args[1];
    int type;
    CALD_FUNCTION_MODULE *function_module;
    char abort_text[1024];
    SAPRFCLS_FETCH();
    TSRMLS_FETCH();

    rfc_rc = SAL_GET_NAME (rfc_handle, function_name);
    if (rfc_rc != RFC_OK) return (rfc_rc);
     
    MAKE_STD_ZVAL(callback_function);
    MAKE_STD_ZVAL(callback_name);
    if ( SAPRFCG(trfc_dispatcher) == NULL )    
    {
        ZVAL_STRING(callback_name,"__saprfc_callback_dispatch",1);
    }
    else
    {
        ZVAL_STRING(callback_name,SAPRFCG(trfc_dispatcher),1);
    }
 
    ZVAL_STRING(callback_function,function_name,1);    
    args[0] = &callback_function;  		     

    if ( call_user_function_ex (EG(function_table), NULL, callback_name, &callback_retval,1,args,0,NULL TSRMLS_CC) == SUCCESS )
    {
         /* get function handle */
         fce_resource = (FCE_RESOURCE *) zend_list_find (Z_RESVAL_P(callback_retval),&type);
         if (fce_resource && type == le_function)
         {
              function_module = fce_resource->fce;
              /* retrieve import parameters and tables  */
              CAL_INIT_INTERFACE(function_module); 
              rfc_rc = SAL_GET_DATA (rfc_handle, function_module);
              if (rfc_rc != RFC_OK)
              {
                 if (rfc_rc == -1)
                    sprintf(abort_text,"Error %s in RfcGetData",CAL_DEBUG_MESSAGE());
                 else
                    sprintf(abort_text,"Error %s in RfcGetData",CAL_RFC_LAST_ERROR()); 
              }
              else
              {
                 /* call PHP function */
                 MAKE_STD_ZVAL(name);
                 ZVAL_STRING(name,function_name,1);
                 args[0] = &callback_retval;  		     
	 	     
                 if ( call_user_function_ex (EG(function_table), NULL, name, &retval,1,args,0,NULL TSRMLS_CC) == SUCCESS )
                 {
                    /* if return value is string, raise exception */
                    if (retval->type == IS_STRING && retval->value.str.len > 0 )
                    {
                       SAL_RAISE (rfc_handle, function_module, retval->value.str.val);     	
                    }
                    else
                    {
                       /* send export parameters and tables */
                       rfc_rc = SAL_SEND_DATA (rfc_handle, function_module);
                       if (rfc_rc != RFC_OK)
                       {
                           if (rfc_rc == -1)
                              sprintf(abort_text,"Error %s in RfcSendData",CAL_DEBUG_MESSAGE());
                           else
                              sprintf(abort_text,"Error %s in RfcSendData",CAL_RFC_LAST_ERROR()); 
                       }
                    }
                    zval_dtor(retval);
                    FREE_ZVAL(retval);
                 }
                 else
                 {
                    sprintf(abort_text,"PHP function %s can't be called",function_name);
                    rfc_rc = -1;
                 }
                 zval_dtor(name);
                 FREE_ZVAL(name);
			  }
              CAL_INIT_INTERFACE(function_module); 
   		  }
          else /* bad fce resource handle from __saprfc_callback_dispatch() */
          {
             sprintf(abort_text,"Invalid function handle returned by dispatcher()");
             rfc_rc = -1;
          }
    }
    else /*  __saprfc_callback_dispatch() is not implemented*/
    {
       sprintf(abort_text,"The function __saprfc_callback_dispatch() is not implemented");
       rfc_rc = -1;
    }
    zval_dtor(callback_name);
    FREE_ZVAL(callback_name);
    zval_dtor(callback_function);
    FREE_ZVAL(callback_function);

    if ( rfc_rc != RFC_OK )
    {
        php_error(E_WARNING, abort_text);
        SAL_ABORT (rfc_handle, abort_text);
    }

    return rfc_rc;
}


static int DLL_CALL_BACK_FUNCTION __callback_tid_check(RFC_TID tid)
{
    int rc;
    zval *callback_tid, *callback_name, *callback_retval;
    zval **args[1];
    SAPRFCLS_FETCH();
    TSRMLS_FETCH();
     
    rc = 0;
    if (SAPRFCG(trfc_tid_check) != NULL)
    {
        MAKE_STD_ZVAL(callback_tid);
        MAKE_STD_ZVAL(callback_name);
        ZVAL_STRING(callback_name,SAPRFCG(trfc_tid_check),1);
        ZVAL_STRING(callback_tid,tid,1);    
        args[0] = &callback_tid;  		     

        if ( call_user_function_ex (EG(function_table), NULL, callback_name, &callback_retval,1,args,0,NULL TSRMLS_CC) == SUCCESS )
        {
            rc =  Z_LVAL_P(callback_retval);
        }
     }
     return (rc);
}

static void DLL_CALL_BACK_FUNCTION __callback_tid_commit(RFC_TID tid)
{
    zval *callback_tid, *callback_name, *callback_retval;
    zval **args[1];
    SAPRFCLS_FETCH();
    TSRMLS_FETCH();

    if (SAPRFCG(trfc_tid_commit) != NULL)
    {
        MAKE_STD_ZVAL(callback_tid);
        MAKE_STD_ZVAL(callback_name);
        ZVAL_STRING(callback_name,SAPRFCG(trfc_tid_commit),1);
        ZVAL_STRING(callback_tid,tid,1);    
        args[0] = &callback_tid;  		     

        call_user_function_ex (EG(function_table), NULL, callback_name, &callback_retval,1,args,0,NULL TSRMLS_CC);
     }

}

static void DLL_CALL_BACK_FUNCTION __callback_tid_rollback(RFC_TID tid)
{
    zval *callback_tid, *callback_name, *callback_retval;
    zval **args[1];
    SAPRFCLS_FETCH();
    TSRMLS_FETCH();
    
    if (SAPRFCG(trfc_tid_rollback) != NULL)
    {
        MAKE_STD_ZVAL(callback_tid);
        MAKE_STD_ZVAL(callback_name);
        ZVAL_STRING(callback_name,SAPRFCG(trfc_tid_rollback),1);
        ZVAL_STRING(callback_tid,tid,1);    
        args[0] = &callback_tid;  		     

        call_user_function_ex (EG(function_table), NULL, callback_name, &callback_retval,1,args,0,NULL TSRMLS_CC);
     }

}

static void DLL_CALL_BACK_FUNCTION __callback_tid_confirm(RFC_TID tid)
{
    zval *callback_tid, *callback_name, *callback_retval;
    zval **args[1];
    SAPRFCLS_FETCH();
    TSRMLS_FETCH();

    if (SAPRFCG(trfc_tid_confirm) != NULL)
    {
        MAKE_STD_ZVAL(callback_tid);
        MAKE_STD_ZVAL(callback_name);
        ZVAL_STRING(callback_name,SAPRFCG(trfc_tid_confirm),1);
        ZVAL_STRING(callback_tid,tid,1);    
        args[0] = &callback_tid;  		     

        call_user_function_ex (EG(function_table), NULL, callback_name, &callback_retval,1,args,0,NULL TSRMLS_CC);
     }

}


/* Remove comments and fill if you need to have entries in php.ini
PHP_INI_BEGIN()
PHP_INI_END()
*/

PHP_MINIT_FUNCTION(saprfc)
{
/* Remove comments if you have entries in php.ini
    REGISTER_INI_ENTRIES();
*/
    ZEND_INIT_MODULE_GLOBALS(saprfc,php_saprfc_init_globals,NULL);

    CAL_INIT(); 
    le_rfc = zend_register_list_destructors_ex(_free_resource_rfc, NULL, "saprfc handle", module_number);
    le_function = zend_register_list_destructors_ex(_free_resource_function, NULL, "saprfc function module", module_number);
     

    /* O.K. */ 
    REGISTER_LONG_CONSTANT("SAPRFC_OK",                  RFC_OK,                  CONST_CS | CONST_PERSISTENT);          
    /* Error occurred */ 
    REGISTER_LONG_CONSTANT("SAPRFC_FAILURE",             RFC_FAILURE,             CONST_CS | CONST_PERSISTENT);          
    /* Exception raised */ 
    REGISTER_LONG_CONSTANT("SAPRFC_EXCEPTION",           RFC_EXCEPTION,           CONST_CS | CONST_PERSISTENT);          
    /* System exception raised, connection closed */ 
    REGISTER_LONG_CONSTANT("SAPRFC_SYS_EXCEPTION",       RFC_SYS_EXCEPTION,       CONST_CS | CONST_PERSISTENT);          
    /* Call received */ 
    REGISTER_LONG_CONSTANT("SAPRFC_CALL",                RFC_CALL,                CONST_CS | CONST_PERSISTENT);          
    /* Internal communication, repeat (internal use only) */ 
    REGISTER_LONG_CONSTANT("SAPRFC_INTERNAL_COM",        RFC_INTERNAL_COM,        CONST_CS | CONST_PERSISTENT);          
    /* Connection closed by the other side. */ 
    REGISTER_LONG_CONSTANT("SAPRFC_CLOSED",              RFC_CLOSED,              CONST_CS | CONST_PERSISTENT);          
    /* No data yet (RfcListen or RfcWaitForRequest only) */ 
    REGISTER_LONG_CONSTANT("SAPRFC_RETRY",               RFC_RETRY,               CONST_CS | CONST_PERSISTENT);          
    /* No Transaction ID available */ 
    REGISTER_LONG_CONSTANT("SAPRFC_NO_TID",              RFC_NO_TID,              CONST_CS | CONST_PERSISTENT);          
    /* Function already executed */ 
    REGISTER_LONG_CONSTANT("SAPRFC_EXECUTED",            RFC_EXECUTED,            CONST_CS | CONST_PERSISTENT);          
    /* Synchronous Call in Progress (only for Windows) */ 
    REGISTER_LONG_CONSTANT("SAPRFC_SYNCHRONIZE",         RFC_SYNCHRONIZE,         CONST_CS | CONST_PERSISTENT);          
    /* Memory insufficient */ 
    REGISTER_LONG_CONSTANT("SAPRFC_MEMORY_INSUFFICIENT", RFC_MEMORY_INSUFFICIENT, CONST_CS | CONST_PERSISTENT);          
    /* Version mismatch */ 
    REGISTER_LONG_CONSTANT("SAPRFC_VERSION_MISMATCH",    RFC_VERSION_MISMATCH,    CONST_CS | CONST_PERSISTENT);          
    /* Function not found (internal use only) */ 
    REGISTER_LONG_CONSTANT("SAPRFC_NOT_FOUND",           RFC_NOT_FOUND,           CONST_CS | CONST_PERSISTENT);          
    /* This call is not supported */ 
    REGISTER_LONG_CONSTANT("SAPRFC_CALL_NOT_SUPPORTED",  RFC_CALL_NOT_SUPPORTED,  CONST_CS | CONST_PERSISTENT);          
    /* Caller does not own the specified handle */ 
    REGISTER_LONG_CONSTANT("SAPRFC_NOT_OWNER",           RFC_NOT_OWNER,           CONST_CS | CONST_PERSISTENT);          
    /* RFC not yet initialized. */ 
    REGISTER_LONG_CONSTANT("SAPRFC_NOT_INITIALIZED",     RFC_NOT_INITIALIZED,     CONST_CS | CONST_PERSISTENT);          
    /* A system call such as RFC_PING for connectiontest is executed. */ 
    REGISTER_LONG_CONSTANT("SAPRFC_SYSTEM_CALLED",       RFC_SYSTEM_CALLED,       CONST_CS | CONST_PERSISTENT);          
    /* Fix for missing constants in RFCSDK < 4.6D, 4.5b tested */
    /* An invalid handle was passed to an API call. */ 
    REGISTER_LONG_CONSTANT("SAPRFC_INVALID_HANDLE",      RFC_SYSTEM_CALLED+1,     CONST_CS | CONST_PERSISTENT);          
    /*An invalid parameter was passed to an API call. */ 
    REGISTER_LONG_CONSTANT("SAPRFC_INVALID_PARAMETER",   RFC_SYSTEM_CALLED+2,     CONST_CS | CONST_PERSISTENT);          
    /* Internal use only */ 
    REGISTER_LONG_CONSTANT("SAPRFC_CANCELED",            RFC_SYSTEM_CALLED+3,     CONST_CS | CONST_PERSISTENT);          
    return SUCCESS;
}

PHP_MSHUTDOWN_FUNCTION(saprfc)
{
/* Remove comments if you have entries in php.ini
    UNREGISTER_INI_ENTRIES();
*/
    CAL_DONE();
    return SUCCESS;
}

/* Remove if there's nothing to do at request start */
PHP_RINIT_FUNCTION(saprfc)
{
    return SUCCESS;
}

/* Remove if there's nothing to do at request end */
PHP_RSHUTDOWN_FUNCTION(saprfc)
{
    SAPRFCLS_FETCH();

    /* clean memory allocated fo global variables */
    
    SAPRFCG(trfc_install_flag)=0;
    if (SAPRFCG(trfc_tid_check))     efree (SAPRFCG(trfc_tid_check));
    if (SAPRFCG(trfc_tid_commit))    efree (SAPRFCG(trfc_tid_commit));
    if (SAPRFCG(trfc_tid_rollback))  efree (SAPRFCG(trfc_tid_rollback));
    if (SAPRFCG(trfc_tid_confirm))   efree (SAPRFCG(trfc_tid_confirm));
    if (SAPRFCG(trfc_dispatcher))    efree (SAPRFCG(trfc_dispatcher));
     
    return SUCCESS;
}

PHP_MINFO_FUNCTION(saprfc)
{
    php_info_print_table_start();
    php_info_print_table_header(2, "SAPRFC support", "enabled");
    php_info_print_table_row(2,"Version",SAPRFC_VERSION);
    php_info_print_table_row(2,"Release date",SAPRFC_RELEASE);
    php_info_print_table_row(2,"RFC Library",CAL_RFC_LIB_VERSION());
    php_info_print_table_end();

    /* Remove comments if you have entries in php.ini
    DISPLAY_INI_ENTRIES();
    */
}


/* {{{ proto int saprfc_open(array conn)
 */
PHP_FUNCTION(saprfc_open)
{
    zval **conn;
    RFC_HANDLE rfc;
    HashTable *hash;
    zval **value_ptr;
    char *buffer;
    int buflen;
    RFC_RESOURCE *rfc_resource;
    char *string_key;
    ulong num_key;

    if (ZEND_NUM_ARGS() != 1 || zend_get_parameters_ex(1, &conn) == FAILURE){
       WRONG_PARAM_COUNT;
    }

    convert_to_array_ex(conn);
    hash = HASH_OF(*conn);
    
    buflen = 0; 
    zend_hash_internal_pointer_reset(hash);
    while ( zend_hash_get_current_key(hash,&string_key, &num_key, 0) == HASH_KEY_IS_STRING)
    {
        buflen += strlen (string_key) + 2;
        zend_hash_get_current_data(hash,(void **)&value_ptr);
        convert_to_string_ex(value_ptr);
        buflen += Z_STRLEN_P(*value_ptr);
        zend_hash_move_forward(hash);
    }
    
    buffer = ecalloc(1,buflen+128);
    if ( buffer )
    {
        zend_hash_internal_pointer_reset(hash);
        while ( zend_hash_get_current_key(hash,&string_key, &num_key, 0) == HASH_KEY_IS_STRING)
        {
            strcat (buffer,string_key);
            strcat (buffer,"="); 
            zend_hash_get_current_data(hash,(void **)&value_ptr);
            convert_to_string_ex(value_ptr);
            strcat (buffer,Z_STRVAL_P(*value_ptr));
            strcat (buffer," "); 
            zend_hash_move_forward(hash);
        }
    }

    rfc = CAL_OPEN (buffer);
    if (buffer) efree (buffer);

    if ( rfc == RFC_HANDLE_NULL )
    {
        php_error(E_WARNING, CAL_DEBUG_MESSAGE());          
        RETURN_FALSE;
    }

    rfc_resource = (RFC_RESOURCE *) emalloc (sizeof(RFC_RESOURCE));
    if (rfc_resource) 
    { 
        rfc_resource->handle = rfc;
        rfc_resource->client = 1;
    }
    
    RETURN_RESOURCE(zend_list_insert(rfc_resource,le_rfc));
}
/* }}} */

/* {{{ proto int saprfc_function_discover(int rfc, string function_module, [bool not_trim])
 */
PHP_FUNCTION(saprfc_function_discover)
{
    zval **rfc, **function_module,**not_trim;
    RFC_RESOURCE *rfc_resource;
    FCE_RESOURCE *fce_resource;
    CALD_FUNCTION_MODULE *fce;
    int retval;
    int type;
    int not_trim_flag=0;

    if (ZEND_NUM_ARGS() < 2 || ZEND_NUM_ARGS() > 3 ) {
        WRONG_PARAM_COUNT;
    }
    if (ZEND_NUM_ARGS() == 2) {
        if (zend_get_parameters_ex(2, &rfc, &function_module) == FAILURE){
           WRONG_PARAM_COUNT;
        }
    } else {
        if (zend_get_parameters_ex(3, &rfc, &function_module, &not_trim) == FAILURE){
           WRONG_PARAM_COUNT;
        }
        convert_to_boolean_ex(not_trim);
        not_trim_flag = (Z_LVAL_P(*not_trim)) ? 1 : 0; 
    }

    convert_to_string_ex(function_module);

    rfc_resource = (RFC_RESOURCE *) zend_list_find (Z_RESVAL_P(*rfc),&type);
    if (rfc_resource && type == le_rfc)
    {
         fce = CAL_NEW(Z_STRVAL_P(*function_module));
         if ( fce == NULL )
         {
             php_error(E_WARNING, "fail allocate internal memory for function %s",Z_STRVAL_P(*function_module));
             RETURN_FALSE;
         }
         retval = CAL_DISCOVER_INTERFACE(fce,rfc_resource->handle);
         if ( retval != 0 )
         {
             php_error(E_WARNING, CAL_DEBUG_MESSAGE());
             CAL_DELETE(fce);
             RETURN_FALSE;
         }

         fce_resource = (FCE_RESOURCE *) emalloc (sizeof(FCE_RESOURCE));
         if (fce_resource) 
         {
             fce_resource->handle = rfc_resource->handle;
             if (not_trim_flag == 1) CAL_SET_RAWSTR(fce);
             fce_resource->fce = fce;
         }
         else
         {
             php_error(E_WARNING, "fail allocate internal memory for function %s",Z_STRVAL_P(*function_module));
             CAL_DELETE(fce);
             RETURN_FALSE;
         }
         RETURN_RESOURCE(zend_list_insert(fce_resource,le_function));
    }
    else
    {
        php_error(E_WARNING, "Invalid resource resource for RFC connection");
        RETURN_FALSE;
    }
}
/* }}} */

/* {{{ proto int saprfc_function_define(int rfc, string function_module, array def,[bool not_trim])
 */
PHP_FUNCTION(saprfc_function_define)
{
    zval **rfc, **function_module, **iface, **not_trim;
    RFC_RESOURCE *rfc_resource;
    FCE_RESOURCE *fce_resource;
    CALD_FUNCTION_MODULE *fce;
    int type;
    zval **param, **def, **item;
    int if_max, def_max;
    int i,j;
    zval **tmp;
    char *iname, *itype, *iitem, *iabap;
    int ilen, idec, ioptional, iofs;
    int rfc_undef = 0;
    int not_trim_flag=0;
    
    
    if (ZEND_NUM_ARGS() < 3 || ZEND_NUM_ARGS() > 4 ) {
        WRONG_PARAM_COUNT;
    }
    if (ZEND_NUM_ARGS() == 3) {
        if (zend_get_parameters_ex(3, &rfc, &function_module, &iface) == FAILURE){
           WRONG_PARAM_COUNT;
        }
    } else {
        if (zend_get_parameters_ex(4, &rfc, &function_module, &iface, &not_trim) == FAILURE){
           WRONG_PARAM_COUNT;
        }
        convert_to_boolean_ex(not_trim);
        not_trim_flag = (Z_LVAL_P(*not_trim)) ? 1 : 0; 
    }

    convert_to_long_ex(rfc);
    convert_to_string_ex(function_module);
    convert_to_array_ex(iface);

    if (Z_LVAL_P(*rfc) == 0) 
    {
        rfc_resource = NULL;
        rfc_undef = 1;
    }
    else
        rfc_resource = (RFC_RESOURCE *) zend_list_find (Z_RESVAL_P(*rfc),&type);
    if ( ( rfc_resource && type == le_rfc ) || rfc_undef )
    {
         fce = CAL_NEW(Z_STRVAL_P(*function_module));
         if ( fce == NULL )
         {
             php_error(E_WARNING, "fail allocate internal memory for function %s",Z_STRVAL_P(*function_module));
             RETURN_FALSE;
         }
         if_max = zend_hash_num_elements (HASH_OF(*iface)); 
         for (i=0; i<if_max; i++)
         {
            
            if ( zend_hash_index_find(HASH_OF(*iface),i,(void **)&param) ==  SUCCESS )
            {            
                iname = iitem = iabap = "";
                ilen = idec = ioptional = 0;
                convert_to_array_ex(param);
                if ( zend_hash_find(Z_ARRVAL_PP(param),"name",sizeof ("name"), (void **)&tmp ) == SUCCESS )
                 {
                    convert_to_string_ex(tmp);
                    iname = Z_STRVAL_P(*tmp);
                    strtoupper(iitem);
                }
                if ( zend_hash_find(HASH_OF(*param),"type", sizeof ("type"), (void **)&tmp ) == SUCCESS )
                {
                    convert_to_string_ex(tmp);
                    itype = Z_STRVAL_P(*tmp);
                    strtoupper(itype);
                }
                if ( zend_hash_find(HASH_OF(*param),"type", sizeof ("optional"), (void **)&tmp ) == SUCCESS )
                {
                    convert_to_boolean_ex(tmp);
                    ioptional = Z_LVAL_P(*tmp);                   
                }
                if ( zend_hash_find(HASH_OF(*param),"def", sizeof  ("def"), (void **)&def ) == SUCCESS )
                {
                    convert_to_array_ex(def);
                    def_max = zend_hash_num_elements (HASH_OF(*def));
                    for (j=0; j<def_max; j++)
                    {
                        if ( zend_hash_index_find(HASH_OF(*def),j,(void **)&item) == SUCCESS )
                        {
                            convert_to_array_ex(item);
                            if ( zend_hash_find(HASH_OF(*item),"name",sizeof ("name"), (void **)&tmp ) == SUCCESS )
                            {
                                convert_to_string_ex(tmp);
                                iitem = Z_STRVAL_P(*tmp);
                                strtoupper(iitem);
                            }  
                            if ( zend_hash_find(HASH_OF(*item),"abap",sizeof ("abap"), (void **)&tmp ) == SUCCESS )
                            {
                                convert_to_string_ex(tmp);
                                iabap = Z_STRVAL_P(*tmp);                            
                            }  
                            if ( zend_hash_find(HASH_OF(*item),"len",sizeof ("len"), (void **)&tmp ) == SUCCESS )
                            {
                                convert_to_long_ex(tmp);
                                ilen = (int) Z_LVAL_P(*tmp);                            
                            }  
                            if ( zend_hash_find(HASH_OF(*item),"dec",sizeof ("dec"), (void **)&tmp ) == SUCCESS )
                            {
                                convert_to_long_ex(tmp);
                                idec = (int) Z_LVAL_P(*tmp);                            
                            }  
                            if ( zend_hash_find(HASH_OF(*item),"offset",sizeof ("offset"), (void **)&tmp ) == SUCCESS )
                            {
                                convert_to_long_ex(tmp);
                                iofs = (int) Z_LVAL_P(*tmp);                            
                            }
                            else
                                iofs = -1;
                        }
                        
                        fce->par_offset = iofs;
                        if ( strcmp (itype,"IMPORT") == 0 )
                        {
                            if (def_max == 1) iitem="";
                            CAL_INTERFACE_IMPORT_RAW(fce,iname,iitem,*iabap,ilen,idec);
                            CAL_INTERFACE_IMPORT_OPT(fce,iname,ioptional);
                        } 
                        else if ( strcmp (itype,"EXPORT") == 0 )
                        {
                            if (def_max == 1) iitem="";
                            CAL_INTERFACE_EXPORT_RAW(fce,iname,iitem,*iabap,ilen,idec);
                            CAL_INTERFACE_EXPORT_OPT(fce,iname,ioptional);
                        }
                        else if ( strcmp (itype,"TABLE") == 0 )
                        {
                            CAL_INTERFACE_TABLE_RAW(fce,iname,iitem,*iabap,ilen,idec);
                            CAL_INTERFACE_TABLE_OPT(fce,iname,ioptional);
                        }
                        fce->par_offset = -1;
                    }
                }
            } 
         }
         CAL_INIT_INTERFACE(fce);
         fce_resource = (FCE_RESOURCE *) emalloc (sizeof(FCE_RESOURCE));
         
         if (fce_resource) 
         {
             if (rfc_undef)
                 fce_resource->handle = 0;
             else
                 fce_resource->handle = rfc_resource->handle;
             if (not_trim_flag == 1) CAL_SET_RAWSTR(fce);
             fce_resource->fce = fce;
         }
         else
         {
             php_error(E_WARNING, "fail allocate internal memory for function %s",Z_STRVAL_P(*function_module));
             CAL_DELETE(fce);
             RETURN_FALSE;
         }
         RETURN_RESOURCE(zend_list_insert(fce_resource,le_function));
    }
    else
    {
        php_error(E_WARNING, "Invalid resource for RFC connection");
        RETURN_FALSE;
    }
}
/* }}} */

/* {{{ proto array saprfc_function_interface(int fce)
 */
PHP_FUNCTION(saprfc_function_interface)
{
    zval **fce;
    FCE_RESOURCE *fce_resource;
    CALD_FUNCTION_MODULE *f;
    CALD_INTERFACE_INFO *iinfo;
    int type;
    int i,j;
    char abap_tmp[2];
    zval *param, *def, *item;
    
    if (ZEND_NUM_ARGS() != 1 || zend_get_parameters_ex(1, &fce) == FAILURE){
    WRONG_PARAM_COUNT;
    }

    fce_resource = (FCE_RESOURCE *) zend_list_find (Z_RESVAL_P(*fce),&type);
    if (fce_resource && type == le_function)
    {
      f = fce_resource->fce;
      iinfo = CAL_INTERFACE_INFO(f);
      if ( iinfo == NULL )
      {
         php_error(E_WARNING, CAL_DEBUG_MESSAGE());
         RETURN_FALSE;
      }
      if (array_init(return_value) == FAILURE)
      {
         php_error (E_WARNING,"array_init failed");
         CAL_DEL_INTERFACE(iinfo);
         RETURN_FALSE;
      }
      i=0;
      while ( iinfo[i].name != NULL )
      {
         MAKE_STD_ZVAL(param);
         if (array_init(param) == FAILURE)
         {
             php_error (E_WARNING,"array_init failed");
             CAL_DEL_INTERFACE(iinfo);
             RETURN_FALSE;
         }
         add_assoc_string (param,"name",iinfo[i].name,1);
         switch (iinfo[i].type) {
           case CALC_IMPORT : add_assoc_string (param,"type","IMPORT",1); break;
           case CALC_EXPORT : add_assoc_string (param,"type","EXPORT",1); break;
           case CALC_TABLE  : add_assoc_string (param,"type","TABLE",1); break;
           default          : add_assoc_string (param,"type","UNDEF",1); break;
         }
		 add_assoc_long (param,"optional",iinfo[i].is_optional);
         MAKE_STD_ZVAL(def);
         if (array_init(def) == FAILURE)
         {
            php_error (E_WARNING,"array_init failed");
            CAL_DEL_INTERFACE(iinfo);
            RETURN_FALSE;
         }
         for (j=0; j<iinfo[i].size; j++)
         {
            MAKE_STD_ZVAL(item);
            if (array_init(item) == FAILURE)
            {
                php_error (E_WARNING,"array_init failed");
                CAL_DEL_INTERFACE(iinfo);
                RETURN_FALSE;
            }
            add_assoc_string(item,"name",iinfo[i].typeinfo[j].name,1);
            abap_tmp[0]=iinfo[i].typeinfo[j].abap;
            abap_tmp[1]=0;
            add_assoc_string(item,"abap",abap_tmp,1);
            add_assoc_long(item,"len",iinfo[i].typeinfo[j].length);
            add_assoc_long(item,"dec",iinfo[i].typeinfo[j].decimals);
            add_assoc_long(item,"offset",iinfo[i].typeinfo[j].offset);
            zend_hash_next_index_insert (HASH_OF(def), (void *) &item, sizeof (zval *), NULL);
          }
          zend_hash_update(HASH_OF(param), "def", strlen("def") + 1, (void *)&def, sizeof(zval *), NULL);
          zend_hash_next_index_insert (HASH_OF(return_value), (void *) &param, sizeof (zval *), NULL);
          i++;
      }
    }
    else
    {
          php_error(E_WARNING, "Invalid resource for function module");
          RETURN_FALSE;
    }

    CAL_DEL_INTERFACE(iinfo); 
    return;     
}
/* }}} */

/* {{{ proto void saprfc_function_debug_info(int fce, [bool only_values])
 */
PHP_FUNCTION(saprfc_function_debug_info)
{
    zval **fce, **only_values;
    FCE_RESOURCE *fce_resource;
    CALD_FUNCTION_MODULE *f;
    CALD_INTERFACE_INFO *iinfo, *p;
	char *optstr;
    int type;
    int i,j,k;
    int flag;
    int size;
	int print_only_values=0;
    
    if ( ZEND_NUM_ARGS() < 1 || ZEND_NUM_ARGS() > 2  || zend_get_parameters_ex(1, &fce) == FAILURE){
       WRONG_PARAM_COUNT;
    }

	if ( zend_get_parameters_ex(2, &only_values) == SUCCESS ) print_only_values = 1;

    fce_resource = (FCE_RESOURCE *) zend_list_find (Z_RESVAL_P(*fce),&type);
    if (fce_resource && type == le_function)
    {
        f = fce_resource->fce;
        iinfo = CAL_INTERFACE_INFO(f);
        if (f->unicode == 0)
            zend_printf ("<h3>Function module: %s (remote SAP R/3: %s)</h3>\n",f->name,f->rfcsaprl);
        else       
            zend_printf ("<h3>Function module: %s (remote SAP R/3: %s Unicode)</h3>\n",f->name,f->rfcsaprl); 
        if (print_only_values == 0)
        {
          for (j=CALC_IMPORT; j<=CALC_TABLE; j++)        /* show interface definition */
		  {
            switch (j) {
               case CALC_IMPORT: zend_printf ("<h3>IMPORT</h3>\n<table>"); break;
               case CALC_EXPORT: zend_printf ("<h3>EXPORT</h3>\n<table>"); break;
               case CALC_TABLE: zend_printf ("<h3>TABLE</h3>\n<table>"); break;       
               default: zend_printf ("<h3>UNDEF</h3>\n<table>");        
            }
            p = iinfo;
            while (p->name)
            {
               if (p->type == j) 
               { 
                 flag = 1;
                 for (i=0;i<p->size;i++)
                 {
                   zend_printf ("<tr>");
                   if ( flag )
                   {
                      if (p->is_optional)
                        zend_printf ("<td><b>%s /o/</b></td>",p->name);  
                      else    
                        zend_printf ("<td><b>%s</b></td>",p->name);
                   }
                   else
                       zend_printf ("<td></td>");
                   flag = 0;
                   zend_printf ("<td>%s</td><td>%c</td><td>%d</td><td>%d</td><td>offset = %d</td></tr>\n",
                                 p->typeinfo[i].name,p->typeinfo[i].abap,p->typeinfo[i].length,
                                 p->typeinfo[i].decimals,p->typeinfo[i].offset);
                 }
               }
               p++;
            }
            zend_printf ("</table>");         
        }
     }
    
     for (j=CALC_IMPORT; j<=CALC_TABLE; j++)        /* show internal buffer values */
     {
            p = iinfo;
            while (p->name)
            {
               if (p->type == j) 
               { 
                 if (p->is_optional) 
                    optstr = " (optional)";
                 else
                    optstr = "";
                 switch (j) {
                    case CALC_IMPORT: zend_printf ("<h3>Value of import (input) parameter <b>%s</b>%s (memory = %d):</h3>",p->name,optstr, p->buflen); break;
                    case CALC_EXPORT: zend_printf ("<h3>Value of export (output) parameter <b>%s</b>%s (memory = %d):</h3>",p->name,optstr,p->buflen); break;
                    case CALC_TABLE: zend_printf ("<h3>Internal table <b>%s</b>%s (memory = %d):</h3>",p->name,optstr,p->buflen); break;       
                 }

                 /* header */
                 if ( strcmp (p->typeinfo[0].name,"") != 0 )
                 {
                     zend_printf ("<table><tr>");
                     for (i=0;i<p->size;i++) 
                         zend_printf ("<td><b>%s</b></td>",p->typeinfo[i].name);
                     zend_printf ("</tr>");
                 }
                 /* content */
                 if ( strcmp (p->typeinfo[0].name,"") == 0 ) /* single value */
                 {
                     if (j==CALC_IMPORT) zend_printf ("\"%s\"",CAL_GET_IMPORT(f,p->name));
                       else zend_printf ("\"%s\"",CAL_GET_EXPORT(f,p->name));
                 }
                 else if ( j!= CALC_TABLE )                  /* export or import structure */
                 {
                     zend_printf ("<tr>");
                     for (i=0;i<p->size;i++) 
                        if (j==CALC_IMPORT) 
                           zend_printf ("<td>\"%s\"</td>",CAL_GET_IMPORT_STRUCT(f,p->name,p->typeinfo[i].name));
                        else
                           zend_printf ("<td>\"%s\"</td>",CAL_GET_EXPORT_STRUCT(f,p->name,p->typeinfo[i].name));
                     zend_printf ("</tr></table>");
                 }
                 else /* table */                        
                 {
                     size = CAL_TBL_LENGTH(f,p->name);
                     for (k=1;k<=size;k++)
                     {
                         zend_printf ("<tr>");
                         CAL_TBL_READ (f,p->name,k);
                         for (i=0;i<p->size;i++) 
                             zend_printf ("<td>\"%s\"</td>",CAL_GET_TABLE(f,p->name,p->typeinfo[i].name));
                         zend_printf ("</tr>");
                     }
                     zend_printf ("</table><p>%d rows",size);
                 }
               }
               p++;
            }
        }
    }
    else
    {
          php_error(E_WARNING, "Invalid resource for function module");
          RETURN_FALSE;
    }

    CAL_DEL_INTERFACE(iinfo); 
    return;     
}
/* }}} */


/* {{{ proto bool saprfc_optional (int fce, string name, bool value)
 */
PHP_FUNCTION(saprfc_optional)
{
    zval **fce, **name, **value;
    FCE_RESOURCE *fce_resource;
    int retval, type, is_optional;
    
    if (ZEND_NUM_ARGS() != 3 || zend_get_parameters_ex(3, &fce, &name, &value) == FAILURE){
        WRONG_PARAM_COUNT;
    }

    convert_to_boolean_ex(value)
    convert_to_string_ex(name);
    strtoupper(Z_STRVAL_P(*name));

    fce_resource = (FCE_RESOURCE *) zend_list_find (Z_RESVAL_P(*fce),&type);
    if (fce_resource && type == le_function)
    {
        is_optional = (Z_LVAL_P(*value)) ? 1 : 0;
		retval = CAL_INTERFACE_IMPORT_OPT(fce_resource->fce,Z_STRVAL_P(*name),is_optional);
        if ( retval != 0 )
        {
            php_error(E_WARNING, CAL_DEBUG_MESSAGE());
            RETURN_FALSE;
        }
    }
    else
    {
          php_error(E_WARNING, "Invalid resource for function module");
          RETURN_FALSE;
    }
    RETURN_TRUE;
}
/* }}} */


/* {{{ proto bool saprfc_import(int fce, string name, mixed value)
 */
PHP_FUNCTION(saprfc_import)
{
    zval **fce, **name, **value;
    FCE_RESOURCE *fce_resource;
    zval **tmp;
    char *string_key;
    ulong num_key;
    char *val;
    char *buffer;
    int len, retval, type;
    
    if (ZEND_NUM_ARGS() != 3 || zend_get_parameters_ex(3, &fce, &name, &value) == FAILURE){
        WRONG_PARAM_COUNT;
    }

    convert_to_string_ex(name);
    strtoupper(Z_STRVAL_P(*name));

    fce_resource = (FCE_RESOURCE *) zend_list_find (Z_RESVAL_P(*fce),&type);
    if (fce_resource && type == le_function)
    {
        if ((*value)->type == IS_ARRAY)
        {
            zend_hash_internal_pointer_reset(HASH_OF(*value));
            while ( zend_hash_get_current_key(HASH_OF(*value),&string_key, &num_key, 0) == HASH_KEY_IS_STRING)
            {
                strtoupper (string_key);
                zend_hash_get_current_data(HASH_OF(*value),(void **)&tmp);
                convert_to_string_ex(tmp);
                buffer = NULL; 
                if ( CAL_DEF_IMPORT_STRUCT_TYPE(fce_resource->fce,Z_STRVAL_P(*name),string_key) == TYPX )
                {
                    len = CAL_DEF_IMPORT_STRUCT_LENGTH(fce_resource->fce,Z_STRVAL_P(*name),string_key);    
                    buffer = ecalloc (1,len);
                    memcpy (buffer,Z_STRVAL_P(*tmp),Z_STRLEN_P(*tmp));
                }
                if (buffer == NULL)
                    val = Z_STRVAL_P(*tmp);
                else
                    val = buffer;
                retval = CAL_SET_IMPORT_STRUCT(fce_resource->fce,Z_STRVAL_P(*name),string_key,val); 
                if (buffer) efree (buffer);
                if ( retval != 0 )
                {
                    php_error(E_WARNING, CAL_DEBUG_MESSAGE());
                    RETURN_FALSE;
                }
                zend_hash_move_forward(HASH_OF(*value));
            }
        }
        else
        {
            convert_to_string_ex(value);   
            buffer = NULL; 
            if ( CAL_DEF_IMPORT_TYPE(fce_resource->fce,Z_STRVAL_P(*name)) == TYPX )
            {
                len = CAL_DEF_IMPORT_LENGTH(fce_resource->fce,Z_STRVAL_P(*name));    
                buffer = ecalloc (1,len);
                memcpy (buffer,Z_STRVAL_P(*value),Z_STRLEN_P(*value));
            }
            if (buffer == NULL)
                val = Z_STRVAL_P(*value);
            else
                val = buffer;
            retval = CAL_SET_IMPORT(fce_resource->fce,Z_STRVAL_P(*name),val); 
            if (buffer) efree (buffer);
            if ( retval != 0 )
            {
                php_error(E_WARNING, CAL_DEBUG_MESSAGE());
                RETURN_FALSE;
            }
        }
    }
    else
    {
          php_error(E_WARNING, "Invalid resource for function module");
          RETURN_FALSE;
    }
    RETURN_TRUE;
}
/* }}} */

/* {{{ proto mixed saprfc_export(int fce, string name)
 */
PHP_FUNCTION(saprfc_export)
{
    zval **fce, **name;
    int i;
    int type;
    FCE_RESOURCE *fce_resource;
    CALD_INTERFACE_INFO *iinfo;

    if (ZEND_NUM_ARGS() != 2 || zend_get_parameters_ex(2, &fce, &name) == FAILURE){
        WRONG_PARAM_COUNT;
    }

    convert_to_string_ex(name);
    strtoupper(Z_STRVAL_P(*name));

    fce_resource = (FCE_RESOURCE *) zend_list_find (Z_RESVAL_P(*fce),&type);
    if (fce_resource && type == le_function)
    {
        iinfo = CAL_SINGLE_INTERFACE_INFO(fce_resource->fce,Z_STRVAL_P(*name),CALC_EXPORT);
        if ( iinfo == NULL )
        {
            php_error(E_WARNING, CAL_DEBUG_MESSAGE());
            RETURN_FALSE;
        }
        if ( strcmp (iinfo->typeinfo[0].name,"") != 0 ) /* structure */
        {
            if (array_init(return_value) == FAILURE)
            {
                php_error (E_WARNING,"array_init failed");
                CAL_DEL_INTERFACE(iinfo);
                RETURN_FALSE;
            }
            for (i=0; i<iinfo->size; i++)
            {
                 if ( iinfo->typeinfo[i].abap == 'X' ) /* special handling for RAW type */
                    add_assoc_stringl(return_value,
                                      iinfo->typeinfo[i].name, 
                                      CAL_GET_EXPORT_STRUCT(fce_resource->fce,Z_STRVAL_P(*name),iinfo->typeinfo[i].name),
                                      iinfo->typeinfo[i].length,
                                      1);
                 else    
                    add_assoc_string (return_value,
                                      iinfo->typeinfo[i].name, 
                                      CAL_GET_EXPORT_STRUCT(fce_resource->fce,Z_STRVAL_P(*name),iinfo->typeinfo[i].name),
                                      1);
            }
        }
        else
        {
             if ( iinfo->typeinfo[0].abap == 'X' ) /* special handling for RAW type */
             {
                    RETVAL_STRINGL(CAL_GET_EXPORT(fce_resource->fce,Z_STRVAL_P(*name)),
                                iinfo->typeinfo[0].length,
                                1);
             }
             else 
             {     
                 RETVAL_STRING(CAL_GET_EXPORT(fce_resource->fce,Z_STRVAL_P(*name)),1);
             }
        }
    }
    else
    {
          php_error(E_WARNING, "Invalid resource for function module");
          RETURN_FALSE;
    }
    CAL_DEL_INTERFACE(iinfo);
    return;
}
/* }}} */

/* {{{ proto bool saprfc_table_init (int fce, string name)
 */
PHP_FUNCTION(saprfc_table_init)
{
    zval **fce, **name;
    int retval;
    int type;
    FCE_RESOURCE *fce_resource;
    
    
    if (ZEND_NUM_ARGS() != 2 || zend_get_parameters_ex(2, &fce, &name) == FAILURE){
        WRONG_PARAM_COUNT;
    }

    convert_to_string_ex(name);
    strtoupper(Z_STRVAL_P(*name));

    fce_resource = (FCE_RESOURCE *) zend_list_find (Z_RESVAL_P(*fce),&type);
    if (fce_resource && type == le_function)
    {
        retval = CAL_REFRESH_INTERNAL_BUFFER(fce_resource->fce,Z_STRVAL_P(*name),CALC_TABLE);
        if ( retval != 0 )
        {
            php_error(E_WARNING, CAL_DEBUG_MESSAGE());
            RETURN_FALSE;
        }
        retval = CAL_TBL_INIT(fce_resource->fce,Z_STRVAL_P(*name));
        if ( retval != 0 )
        {
            php_error(E_WARNING, CAL_DEBUG_MESSAGE());
            RETURN_FALSE;
        }
    }
    else
    {
          php_error(E_WARNING, "Invalid resource for function module");
          RETURN_FALSE;
    }
    RETURN_TRUE;
}
/* }}} */


/* {{{ proto bool saprfc_table_append (int fce, string name, array value)
 */
PHP_FUNCTION(saprfc_table_append)
{
    zval **fce, **name, **value;
    int retval;
    int type;
    char *string_key;
    ulong num_key;
    zval **tmp;
    char *buffer, *val;
    int len;
    FCE_RESOURCE *fce_resource;

    if (ZEND_NUM_ARGS() != 3 || zend_get_parameters_ex(3, &fce, &name, &value) == FAILURE){
        WRONG_PARAM_COUNT;
    }

    convert_to_string_ex(name);
    convert_to_array_ex(value);
    strtoupper(Z_STRVAL_P(*name));

    fce_resource = (FCE_RESOURCE *) zend_list_find (Z_RESVAL_P(*fce),&type);
    if (fce_resource && type == le_function)
    {
        retval = CAL_REFRESH_INTERNAL_BUFFER(fce_resource->fce,Z_STRVAL_P(*name),CALC_TABLE);
        if ( retval != 0 )
        {
            php_error(E_WARNING, CAL_DEBUG_MESSAGE());
            RETURN_FALSE;
        }
        zend_hash_internal_pointer_reset(HASH_OF(*value));
        while ( zend_hash_get_current_key(HASH_OF(*value),&string_key, &num_key, 0) == HASH_KEY_IS_STRING)
        {
            strtoupper (string_key);
            zend_hash_get_current_data(HASH_OF(*value),(void **)&tmp);
            convert_to_string_ex(tmp);
            buffer = NULL; 
            if ( CAL_DEF_TABLE_TYPE(fce_resource->fce,Z_STRVAL_P(*name),string_key) == TYPX )
            {
               len = CAL_DEF_TABLE_LENGTH(fce_resource->fce,Z_STRVAL_P(*name),string_key);    
               buffer = ecalloc (1,len);
               memcpy (buffer,Z_STRVAL_P(*tmp),Z_STRLEN_P(*tmp));
            }
            if (buffer == NULL)
                val = Z_STRVAL_P(*tmp);
            else
                val = buffer;
            retval = CAL_SET_TABLE(fce_resource->fce,Z_STRVAL_P(*name),string_key,val); 
            if (buffer) efree (buffer);
            if ( retval != 0 )
            {
                 php_error(E_WARNING, CAL_DEBUG_MESSAGE());
                 RETURN_FALSE;
            }
            zend_hash_move_forward(HASH_OF(*value));
        }
        retval = CAL_TBL_APPEND(fce_resource->fce,Z_STRVAL_P(*name));
        if ( retval != 0 )
        {
            php_error(E_WARNING, CAL_DEBUG_MESSAGE());
            RETURN_FALSE;
        }    
    }
    else
    {
          php_error(E_WARNING, "Invalid resource for function module");
          RETURN_FALSE;
    }

    RETURN_TRUE;   
}
/* }}} */

/* {{{ proto bool saprfc_table_insert (int fce, string name, array value, int index)
 */
PHP_FUNCTION(saprfc_table_insert)
{
    zval **fce, **name, **value, **index;
    int retval;
    int type;
    char *string_key;
    ulong num_key;
    zval **tmp;
    char *buffer, *val;
    int len;
    FCE_RESOURCE *fce_resource;

    
    if (ZEND_NUM_ARGS() != 4 || zend_get_parameters_ex(4, &fce, &name, &value, &index) == FAILURE){
        WRONG_PARAM_COUNT;
    }

    convert_to_string_ex(name);
    convert_to_array_ex(value);
    convert_to_long_ex(index);
    strtoupper(Z_STRVAL_P(*name));

    fce_resource = (FCE_RESOURCE *) zend_list_find (Z_RESVAL_P(*fce),&type);
    if (fce_resource && type == le_function)
    {
        retval = CAL_REFRESH_INTERNAL_BUFFER(fce_resource->fce,Z_STRVAL_P(*name),CALC_TABLE);
        if ( retval != 0 )
        {
            php_error(E_WARNING, CAL_DEBUG_MESSAGE());
            RETURN_FALSE;
        }
        zend_hash_internal_pointer_reset(HASH_OF(*value));
        while ( zend_hash_get_current_key(HASH_OF(*value),&string_key, &num_key, 0) == HASH_KEY_IS_STRING)
        {
            strtoupper (string_key);
            zend_hash_get_current_data(HASH_OF(*value),(void **)&tmp);
            convert_to_string_ex(tmp);
            buffer = NULL; 
            if ( CAL_DEF_TABLE_TYPE(fce_resource->fce,Z_STRVAL_P(*name),string_key) == TYPX )
            {
               len = CAL_DEF_TABLE_LENGTH(fce_resource->fce,Z_STRVAL_P(*name),string_key);    
               buffer = ecalloc (1,len);
               memcpy (buffer,Z_STRVAL_P(*tmp),Z_STRLEN_P(*tmp));
            }
            if (buffer == NULL)
                val = Z_STRVAL_P(*tmp);
            else
                val = buffer;
            retval = CAL_SET_TABLE(fce_resource->fce,Z_STRVAL_P(*name),string_key,val); 
            if (buffer) efree (buffer);
            if ( retval != 0 )
            {
                 php_error(E_WARNING, CAL_DEBUG_MESSAGE());
                 RETURN_FALSE;
            }
            zend_hash_move_forward(HASH_OF(*value));
        }
        retval = CAL_TBL_INSERT(fce_resource->fce,Z_STRVAL_P(*name),Z_LVAL_P(*index));
        if ( retval != 0 )
        {
            php_error(E_WARNING, CAL_DEBUG_MESSAGE());
            RETURN_FALSE;
        }    
    }
    else
    {
          php_error(E_WARNING, "Invalid resource for function module");
          RETURN_FALSE;
    }

    RETURN_TRUE;   
}
/* }}} */


/* {{{ proto bool saprfc_table_modify (int fce, string name, array value, int index)
 */
PHP_FUNCTION(saprfc_table_modify)
{
    zval **fce, **name, **value, **index;
    int retval;
    int type;
    char *string_key;
    ulong num_key;
    zval **tmp;
    char *buffer, *val;
    int len;
    FCE_RESOURCE *fce_resource;

    if (ZEND_NUM_ARGS() != 4 || zend_get_parameters_ex(4, &fce, &name, &value, &index) == FAILURE){
        WRONG_PARAM_COUNT;
    }

    convert_to_string_ex(name);
    convert_to_array_ex(value);
    convert_to_long_ex(index);
    strtoupper(Z_STRVAL_P(*name));

    fce_resource = (FCE_RESOURCE *) zend_list_find (Z_RESVAL_P(*fce),&type);
    if (fce_resource && type == le_function)
    {
        retval = CAL_REFRESH_INTERNAL_BUFFER(fce_resource->fce,Z_STRVAL_P(*name),CALC_TABLE);
        if ( retval != 0 )
        {
            php_error(E_WARNING, CAL_DEBUG_MESSAGE());
            RETURN_FALSE;
        }
        zend_hash_internal_pointer_reset(HASH_OF(*value));
        while ( zend_hash_get_current_key(HASH_OF(*value),&string_key, &num_key, 0) == HASH_KEY_IS_STRING)
        {
            strtoupper (string_key);
            zend_hash_get_current_data(HASH_OF(*value),(void **)&tmp);
            convert_to_string_ex(tmp);
            buffer = NULL; 
            if ( CAL_DEF_TABLE_TYPE(fce_resource->fce,Z_STRVAL_P(*name),string_key) == TYPX )
            {
               len = CAL_DEF_TABLE_LENGTH(fce_resource->fce,Z_STRVAL_P(*name),string_key);    
               buffer = ecalloc (1,len);
               memcpy (buffer,Z_STRVAL_P(*tmp),Z_STRLEN_P(*tmp));
            }
            if (buffer == NULL)
                val = Z_STRVAL_P(*tmp);
            else
                val = buffer;
            retval = CAL_SET_TABLE(fce_resource->fce,Z_STRVAL_P(*name),string_key,val); 
            if (buffer) efree (buffer);
            if ( retval != 0 )
            {
                 php_error(E_WARNING, CAL_DEBUG_MESSAGE());
                 RETURN_FALSE;
            }
            zend_hash_move_forward(HASH_OF(*value));
        }
        retval = CAL_TBL_MODIFY(fce_resource->fce,Z_STRVAL_P(*name),Z_LVAL_P(*index));
        if ( retval != 0 )
        {
            php_error(E_WARNING, CAL_DEBUG_MESSAGE());
            RETURN_FALSE;
        }    
    }
    else
    {
          php_error(E_WARNING, "Invalid resource for function module");
          RETURN_FALSE;
    }

    RETURN_TRUE;   
}
/* }}} */


/* {{{ proto bool saprfc_table_remove (int fce, string name, int index)
 */
PHP_FUNCTION(saprfc_table_remove)
{
    zval **fce, **name, **index;
    int retval;
    int type;
    FCE_RESOURCE *fce_resource;

    if (ZEND_NUM_ARGS() != 3 || zend_get_parameters_ex(3, &fce, &name, &index) == FAILURE){
        WRONG_PARAM_COUNT;
    }

    convert_to_string_ex(name);
    convert_to_long_ex(index);
    strtoupper(Z_STRVAL_P(*name));

    fce_resource = (FCE_RESOURCE *) zend_list_find (Z_RESVAL_P(*fce),&type);
    if (fce_resource && type == le_function)
    {
        retval = CAL_TBL_REMOVE(fce_resource->fce,Z_STRVAL_P(*name),Z_LVAL_P(*index));
        if ( retval != 0 )
        {
            php_error(E_WARNING, CAL_DEBUG_MESSAGE());
            RETURN_FALSE;
        }    
    }
    else
    {
          php_error(E_WARNING, "Invalid resource for function module");
          RETURN_FALSE;
    }
}
/* }}} */


/* {{{ proto array rfc_table_read (int fce, string name, int index)
 */
PHP_FUNCTION(saprfc_table_read)
{
    zval **fce, **name, **index;
    int retval;
    int type;
    int i;
    FCE_RESOURCE *fce_resource;
    CALD_INTERFACE_INFO *iinfo;

    if (ZEND_NUM_ARGS() != 3 || zend_get_parameters_ex(3, &fce, &name, &index) == FAILURE){
        WRONG_PARAM_COUNT;
    }

    convert_to_string_ex(name);
    convert_to_long_ex(index);
    strtoupper(Z_STRVAL_P(*name));

    fce_resource = (FCE_RESOURCE *) zend_list_find (Z_RESVAL_P(*fce),&type);
    if (fce_resource && type == le_function)
    {
	    retval = CAL_TBL_READ(fce_resource->fce,Z_STRVAL_P(*name),Z_LVAL_P(*index));
        if ( retval != 0 )
        {
            php_error(E_WARNING, CAL_DEBUG_MESSAGE());
            RETURN_FALSE;
        }    
        iinfo = CAL_SINGLE_INTERFACE_INFO(fce_resource->fce,Z_STRVAL_P(*name),CALC_TABLE);
        if ( iinfo == NULL )
        {
            php_error(E_WARNING, CAL_DEBUG_MESSAGE());
            RETURN_FALSE;
        }
        if (array_init(return_value) == FAILURE)
        {
            php_error (E_WARNING,"array_init failed");
            CAL_DEL_INTERFACE(iinfo);
            RETURN_FALSE;
        }
        for (i=0; i<iinfo->size; i++)
        {
            if ( iinfo->typeinfo[i].abap == 'X' ) /* special handling for RAW type */
             add_assoc_stringl(return_value,
                               iinfo->typeinfo[i].name, 
                                 CAL_GET_TABLE(fce_resource->fce,Z_STRVAL_P(*name),iinfo->typeinfo[i].name),
                               iinfo->typeinfo[i].length,
                               1);
           else    
             add_assoc_string (return_value,
                               iinfo->typeinfo[i].name, 
                               CAL_GET_TABLE(fce_resource->fce,Z_STRVAL_P(*name),iinfo->typeinfo[i].name),
                               1);
           }
    }
    else
    {
          php_error(E_WARNING, "Invalid resource for function module");
          RETURN_FALSE;
    }
    CAL_DEL_INTERFACE(iinfo);
    return;
}
/* }}} */


/* {{{ proto int saprfc_table_rows (int fce, string name)
 */
PHP_FUNCTION(saprfc_table_rows)
{
    zval **fce, **name;
    FCE_RESOURCE *fce_resource;
    int type;
    
    if (ZEND_NUM_ARGS() != 2 || zend_get_parameters_ex(2, &fce, &name) == FAILURE){
        WRONG_PARAM_COUNT;
    }

    convert_to_string_ex(name);
    strtoupper(Z_STRVAL_P(*name));

    fce_resource = (FCE_RESOURCE *) zend_list_find (Z_RESVAL_P(*fce),&type);
    if (fce_resource && type == le_function)
    {
          RETURN_LONG(CAL_TBL_LENGTH(fce_resource->fce,Z_STRVAL_P(*name)));
    }
    else
    {
          php_error(E_WARNING, "Invalid resource for function module");
          RETURN_FALSE;
    }
}
/* }}} */


/* {{{ proto int saprfc_call_and_receive(int fce)
 */
PHP_FUNCTION(saprfc_call_and_receive)
{
    zval **fce;
    FCE_RESOURCE *fce_resource;
    int type;
	RFC_RC rfc_rc;

    if (ZEND_NUM_ARGS() != 1 || zend_get_parameters_ex(1, &fce) == FAILURE){
        WRONG_PARAM_COUNT;
    }

    fce_resource = (FCE_RESOURCE *) zend_list_find (Z_RESVAL_P(*fce),&type);
    if (fce_resource && type == le_function)
    {
       CAL_INIT_INTERFACE_EXPORT(fce_resource->fce);
       rfc_rc = CAL_CALL(fce_resource->fce,fce_resource->handle);
       if ( rfc_rc != 0 )
       {
          php_error(E_WARNING, CAL_DEBUG_MESSAGE());
       }    
       CAL_INIT_INTERFACE_IMPORT(fce_resource->fce); 
    }
    else
    {
       php_error(E_WARNING, "Invalid resource for function module");
       RETURN_LONG(-1);
    }
    RETURN_LONG (rfc_rc);   
}
/* }}} */

/* {{{ proto string saprfc_error()
 */
PHP_FUNCTION(saprfc_error)
{
    RETURN_STRING(CAL_RFC_LAST_ERROR(),1);
}
/* }}} */


/* {{{ proto string saprfc_exception(int fce)
 */
PHP_FUNCTION(saprfc_exception)
{
    zval **fce;
    FCE_RESOURCE *fce_resource;
    int type;
    
    if (ZEND_NUM_ARGS() != 1 || zend_get_parameters_ex(1, &fce) == FAILURE){
        WRONG_PARAM_COUNT;
    }

    fce_resource = (FCE_RESOURCE *) zend_list_find (Z_RESVAL_P(*fce),&type);
    if (fce_resource && type == le_function)
    {
      if (fce_resource->fce->exception)
      { 
          RETURN_STRING(fce_resource->fce->exception,1);
      }
      else
      {
          RETURN_FALSE; 
      }
    }
    else
    {
		php_error(E_WARNING, "Invalid resource for function module");
        RETURN_FALSE;
    }    
}
/* }}} */


/* {{{ proto bool saprfc_function_free(int fce)
 */
PHP_FUNCTION(saprfc_function_free)
{
    zval **fce;
    FCE_RESOURCE *fce_resource;
    int type;
    if (ZEND_NUM_ARGS() != 1 || zend_get_parameters_ex(1, &fce) == FAILURE){
        WRONG_PARAM_COUNT;
    }

    fce_resource = (FCE_RESOURCE *) zend_list_find (Z_RESVAL_P(*fce),&type);
    if (fce_resource && type == le_function)
        zend_list_delete (Z_RESVAL_P(*fce));
    else
    {
        php_error(E_WARNING, "Invalid resource for function module");
        RETURN_FALSE;
    }
    
    RETURN_TRUE;
}
/* }}} */

/* {{{ proto bool saprfc_close(int rfc)
 */
PHP_FUNCTION(saprfc_close)
{
    zval **rfc;
    RFC_RESOURCE *rfc_resource;
    int type;

    if (ZEND_NUM_ARGS() != 1 || zend_get_parameters_ex(1, &rfc) == FAILURE){
        WRONG_PARAM_COUNT;
    }

    rfc_resource = (RFC_RESOURCE *) zend_list_find (Z_RESVAL_P(*rfc),&type);
    if (rfc_resource && type == le_rfc)
        zend_list_delete (Z_RESVAL_P(*rfc));
    else
    {
        php_error(E_WARNING, "Invalid resource for RFC connection");
        RETURN_FALSE;
    }
    
    RETURN_TRUE;
}
/* }}} */

/* {{{ proto int saprfc_set_code_page(int rfc, string codepage)
 */
PHP_FUNCTION(saprfc_set_code_page)
{
    zval **rfc, **codepage;
    RFC_RESOURCE *rfc_resource;
    int type;
	int retval;

    if (ZEND_NUM_ARGS() != 2 || zend_get_parameters_ex(2, &rfc, &codepage) == FAILURE){
        WRONG_PARAM_COUNT;
    }

    convert_to_string_ex(codepage);

    rfc_resource = (RFC_RESOURCE *) zend_list_find (Z_RESVAL_P(*rfc),&type);
    if (rfc_resource && type == le_rfc)
    {   
       retval = CAL_SET_CODE_PAGE(rfc_resource->handle,Z_STRVAL_P(*codepage));
       if ( retval != 0 )
       {
          php_error (E_WARNING, "Cannot set codepage %s",Z_STRVAL_P(*codepage));
          RETURN_FALSE;
       }
    }
    else
    {
        php_error(E_WARNING, "Invalid resource for RFC connection");
        RETURN_FALSE;
    }
    
    RETURN_TRUE;
}

/* {{{ proto array saprfc_attributes(int rfc)
 */
PHP_FUNCTION(saprfc_attributes)
{
    zval **rfc;
    RFC_RESOURCE *rfc_resource;   
    int type;
	RFC_ATTRIBUTES attributes;
	int retval;
    
    if (ZEND_NUM_ARGS() != 1 || zend_get_parameters_ex(1, &rfc) == FAILURE){
    WRONG_PARAM_COUNT;
    }

    rfc_resource = (RFC_RESOURCE *) zend_list_find (Z_RESVAL_P(*rfc),&type);
    if (rfc_resource && type == le_rfc)
    {
      retval = CAL_ATTRIBUTES (rfc_resource->handle,&attributes);
      if ( retval != 0 )
      {
         php_error (E_WARNING,"RfcGetAttributes error");
         RETURN_FALSE;
      }
		
      if (array_init(return_value) == FAILURE)
      {
         php_error (E_WARNING,"array_init failed");
         RETURN_FALSE;
      }

      add_assoc_string(return_value,"dest",attributes.dest,1);
      add_assoc_string(return_value,"own_host",attributes.own_host,1);
      add_assoc_string(return_value,"partner_host",attributes.partner_host,1);
      add_assoc_string(return_value,"systnr",attributes.systnr,1);
      add_assoc_string(return_value,"sysid",attributes.sysid,1);
      add_assoc_string(return_value,"client",attributes.client,1);
      add_assoc_string(return_value,"user",attributes.user,1);
      add_assoc_string(return_value,"language",attributes.language,1);
      add_assoc_stringl(return_value,"trace", &attributes.trace,1,1);
      add_assoc_string(return_value,"ISO_language",attributes.ISO_language,1);
      add_assoc_string(return_value,"own_codepage",attributes.own_codepage,1);
      add_assoc_string(return_value,"partner_codepage",attributes.partner_codepage,1);
      add_assoc_stringl(return_value,"rfc_role", &attributes.rfc_role,1,1);
      add_assoc_stringl(return_value,"own_type", &attributes.own_type,1,1);
      add_assoc_string(return_value,"own_rel",attributes.own_rel,1);
      add_assoc_stringl(return_value,"partner_type", &attributes.partner_type,1,1);
      add_assoc_string(return_value,"partner_rel",attributes.partner_rel,1);
      add_assoc_string(return_value,"kernel_rel",attributes.kernel_rel,1);
      add_assoc_string(return_value,"CPIC_convid",attributes.CPIC_convid,1);
      /*  unsupported by rfcsdk < 4.6D, maybe better fix in future  
          add_assoc_stringl(return_value,"password_sate", &attributes.password_sate,1,1);
       */
    }
    else
    {
        php_error(E_WARNING, "Invalid resource for RFC connection");
        RETURN_FALSE;			
    }

    return;     
}
/* }}} */


/* {{{ proto int saprfc_server_accept(mixed args)
 */
PHP_FUNCTION(saprfc_server_accept)
{
    zval **args;
    RFC_HANDLE rfc;
    RFC_RESOURCE *rfc_resource;
    char **argv;
    int argc;
	zval **param;
    int i;


    argv = NULL; 
    if (ZEND_NUM_ARGS() != 1 || zend_get_parameters_ex(1, &args) == FAILURE){
       WRONG_PARAM_COUNT;
    }
    if ( (*args)->type == IS_ARRAY )
    {
        argc = zend_hash_num_elements (HASH_OF(*args)); 
        argv = ecalloc (1,(argc+1) * sizeof (char *));
        if (argv)
        {
		  for (i=0; i<argc; i++)
            if ( zend_hash_index_find(HASH_OF(*args),i,(void **)&param) ==  SUCCESS )
                argv[i] = Z_STRVAL_P(*param);  
        }
    }
    else
        convert_to_string_ex(args);

    if (argv) 
    {
        rfc = SAL_ACCEPT(argv); 
        efree (argv);  
    }
    else
        rfc = SAL_ACCEPT_EXT(Z_STRVAL_P(*args));

    if ( rfc == RFC_HANDLE_NULL )
    {
        php_error(E_WARNING, CAL_RFC_LAST_ERROR());          
        RETURN_FALSE;
    }

    rfc_resource = (RFC_RESOURCE *) emalloc (sizeof(RFC_RESOURCE));
    if (rfc_resource) 
    { 
        rfc_resource->handle = rfc;
        rfc_resource->client = 0;
    }
    
    RETURN_RESOURCE(zend_list_insert(rfc_resource,le_rfc));
}
/* }}} */

/* {{{ proto mixed saprfc_server_import(int fce, string name)
 */
PHP_FUNCTION(saprfc_server_import)
{    
    zval **fce, **name;
    int i;
    int type;
    FCE_RESOURCE *fce_resource;
    CALD_INTERFACE_INFO *iinfo;

    if (ZEND_NUM_ARGS() != 2 || zend_get_parameters_ex(2, &fce, &name) == FAILURE){
        WRONG_PARAM_COUNT;
    }

    convert_to_string_ex(name);
    strtoupper(Z_STRVAL_P(*name));

    fce_resource = (FCE_RESOURCE *) zend_list_find (Z_RESVAL_P(*fce),&type);
    if (fce_resource && type == le_function)
    {
        iinfo = CAL_SINGLE_INTERFACE_INFO(fce_resource->fce,Z_STRVAL_P(*name),CALC_IMPORT);
        if ( iinfo == NULL )
        {
            php_error(E_WARNING, CAL_DEBUG_MESSAGE());
            RETURN_FALSE;
        }
        if ( strcmp (iinfo->typeinfo[0].name,"") != 0 ) /* structure */
        {
            if (array_init(return_value) == FAILURE)
            {
                php_error (E_WARNING,"array_init failed");
                CAL_DEL_INTERFACE(iinfo);
                RETURN_FALSE;
            }
            for (i=0; i<iinfo->size; i++)
            {
                 if ( iinfo->typeinfo[i].abap == 'X' ) /* special handling for RAW type */
                    add_assoc_stringl(return_value,
                                      iinfo->typeinfo[i].name, 
                                      CAL_GET_IMPORT_STRUCT(fce_resource->fce,Z_STRVAL_P(*name),iinfo->typeinfo[i].name),
                                      iinfo->typeinfo[i].length,
                                      1);
                 else    
                    add_assoc_string (return_value,
                                      iinfo->typeinfo[i].name, 
                                      CAL_GET_IMPORT_STRUCT(fce_resource->fce,Z_STRVAL_P(*name),iinfo->typeinfo[i].name),
                                      1);
            }
        }
        else
        {
             if ( iinfo->typeinfo[0].abap == 'X' ) /* special handling for RAW type */
             {
                    RETVAL_STRINGL(CAL_GET_IMPORT(fce_resource->fce,Z_STRVAL_P(*name)),
                                   iinfo->typeinfo[0].length,
                                   1);
             }
             else 
             {     
                 RETVAL_STRING(CAL_GET_IMPORT(fce_resource->fce,Z_STRVAL_P(*name)),1);
             }
        }
    }
    else
    {
          php_error(E_WARNING, "Invalid resource for function module");
          RETURN_FALSE;
    }
    CAL_DEL_INTERFACE(iinfo);
    return;
}
/* }}} */


/* {{{ proto bool saprfc_server_export(int fce, string name, mixed value)
 */
PHP_FUNCTION(saprfc_server_export)
{
    zval **fce, **name, **value;
    FCE_RESOURCE *fce_resource;
    zval **tmp;
    char *string_key;
    ulong num_key;
    char *val;
    char *buffer;
    int len, retval, type;
    
    if (ZEND_NUM_ARGS() != 3 || zend_get_parameters_ex(3, &fce, &name, &value) == FAILURE){
        WRONG_PARAM_COUNT;
    }

    convert_to_string_ex(name);
    strtoupper(Z_STRVAL_P(*name));

    fce_resource = (FCE_RESOURCE *) zend_list_find (Z_RESVAL_P(*fce),&type);
    if (fce_resource && type == le_function)
    {
        if ((*value)->type == IS_ARRAY)
        {
            zend_hash_internal_pointer_reset(HASH_OF(*value));
            while ( zend_hash_get_current_key(HASH_OF(*value),&string_key, &num_key, 0) == HASH_KEY_IS_STRING)
            {
                strtoupper (string_key);
                zend_hash_get_current_data(HASH_OF(*value),(void **)&tmp);
                convert_to_string_ex(tmp);
                buffer = NULL; 
                if ( CAL_DEF_EXPORT_STRUCT_TYPE(fce_resource->fce,Z_STRVAL_P(*name),string_key) == TYPX )
                {
                    len = CAL_DEF_EXPORT_STRUCT_LENGTH(fce_resource->fce,Z_STRVAL_P(*name),string_key);    
                    buffer = ecalloc (1,len);
                    memcpy (buffer,Z_STRVAL_P(*tmp),Z_STRLEN_P(*tmp));
                }
                if (buffer == NULL)
                    val = Z_STRVAL_P(*tmp);
                else
                    val = buffer;
                retval = CAL_SET_EXPORT_STRUCT(fce_resource->fce,Z_STRVAL_P(*name),string_key,val); 
                if (buffer) efree (buffer);
                if ( retval != 0 )
                {
                    php_error(E_WARNING, CAL_DEBUG_MESSAGE());
                    RETURN_FALSE;
                }
                zend_hash_move_forward(HASH_OF(*value));
            }
        }
        else
        {
            convert_to_string_ex(value);   
            buffer = NULL; 
            if ( CAL_DEF_EXPORT_TYPE(fce_resource->fce,Z_STRVAL_P(*name)) == TYPX )
            {
                len = CAL_DEF_EXPORT_LENGTH(fce_resource->fce,Z_STRVAL_P(*name));    
                buffer = ecalloc (1,len);
                memcpy (buffer,Z_STRVAL_P(*value),Z_STRLEN_P(*value));
            }
            if (buffer == NULL)
                val = Z_STRVAL_P(*value);
            else
                val = buffer;
            retval = CAL_SET_EXPORT(fce_resource->fce,Z_STRVAL_P(*name),val); 
            if (buffer) efree (buffer);
            if ( retval != 0 )
            {
                php_error(E_WARNING, CAL_DEBUG_MESSAGE());
                RETURN_FALSE;
            }
        }
    }
    else
    {
          php_error(E_WARNING, "Invalid resource for function module");
          RETURN_FALSE;
    }
    RETURN_TRUE;
}
/* }}} */


/* {{{ proto int saprfc_server_dispatch (int rfc, array list, [int timeout])
 */
PHP_FUNCTION(saprfc_server_dispatch)
{
    zval **rfc, **list, **timeout;
    RFC_RESOURCE *rfc_resource;
    FCE_RESOURCE *fce_resource;
    int type;
    int timeout_val;
    int num;
    CALD_FUNCTION_MODULE *function_module;
    zval **fce;
    RFC_RC rfc_rc;
    RFC_FUNCTIONNAME function_name;
    char abort_text[1024];
    zval *name, *retval;
    zval **args[1];
	
    timeout_val = 0;
    num = 0;
    rfc_rc = RFC_OK;

    if (ZEND_NUM_ARGS() < 2 || ZEND_NUM_ARGS() > 3 || zend_get_parameters_ex(2, &rfc, &list) == FAILURE){
        WRONG_PARAM_COUNT;
    }

    convert_to_array_ex(list);

    if ( zend_get_parameters_ex(3, &timeout) == SUCCESS ) 
    {
       convert_to_long_ex (timeout);
       timeout_val = Z_LVAL_P(*timeout);
    }

    rfc_resource = (RFC_RESOURCE *) zend_list_find (Z_RESVAL_P(*rfc),&type);
    if (rfc_resource && type == le_rfc)
    {

       /* wait given timeout for incoming rfc call */
       if ( timeout_val != 0 )
       {
          rfc_rc = SAL_WAIT_FOR_REQUEST (rfc_resource->handle, timeout_val);
          if ( rfc_rc == RFC_RETRY )
              RETURN_LONG(rfc_rc); /* no rfc request */
       }

       /* get function name */
       rfc_rc = SAL_GET_NAME (rfc_resource->handle, function_name);
       if ( rfc_rc == RFC_SYSTEM_CALLED ) /* handle system call,  RFC_PING */
          RETURN_LONG(RFC_OK);
            /* and tRFC call */     
       if (rfc_rc != RFC_OK)
          RETURN_LONG(rfc_rc);

       /* get function resource */
       strtoupper (function_name);
       if ( zend_hash_find(HASH_OF(*list),function_name,strlen(function_name)+1, (void **)&fce ) != SUCCESS )		
       {
          sprintf(abort_text,"RFC function %s is not implemented in this server program",function_name);
          rfc_rc = -1;
       }
       else
       {
          /* get function handle */
          fce_resource = (FCE_RESOURCE *) zend_list_find (Z_RESVAL_P(*fce),&type);
          if (fce_resource && type == le_function)
          {
              function_module = fce_resource->fce;
              /* retrieve import parameters and tables  */
              CAL_INIT_INTERFACE(function_module); 
              /* CAL_INIT_INTERFACE_EXPORT(function_module);*/
              rfc_rc = SAL_GET_DATA (rfc_resource->handle, function_module);
              if (rfc_rc != RFC_OK)
              {
                 if (rfc_rc == -1)
                    sprintf(abort_text,"Error %s in RfcGetData",CAL_DEBUG_MESSAGE());
                 else
                    sprintf(abort_text,"Error %s in RfcGetData",CAL_RFC_LAST_ERROR()); 
              }
              else
              {
                 /* call PHP function */
                 MAKE_STD_ZVAL(name);
                 ZVAL_STRING(name,function_name,1);
                 args[0] = fce;  		     
	 	     
                 if ( call_user_function_ex (EG(function_table), NULL, name, &retval,1,args,0,NULL TSRMLS_CC) == SUCCESS )
                 {
                    /* if return value is string, raise exception */
                    if (retval->type == IS_STRING && retval->value.str.len > 0 )
                    {
                       SAL_RAISE (rfc_resource->handle, function_module, retval->value.str.val);     	
                    }
                    else
                    {
                       /* send export parameters and tables */
                       rfc_rc = SAL_SEND_DATA (rfc_resource->handle, function_module);
                       if (rfc_rc != RFC_OK)
                       {
                           if (rfc_rc == -1)
                              sprintf(abort_text,"Error %s in RfcSendData",CAL_DEBUG_MESSAGE());
                           else
                              sprintf(abort_text,"Error %s in RfcSendData",CAL_RFC_LAST_ERROR()); 
                       }
                    }
                    zval_dtor(retval);
                    FREE_ZVAL(retval);
                 }
                 else
                 {
                    sprintf(abort_text,"PHP function %s can't be called",function_name);
                    rfc_rc = -1;
                 }
                 zval_dtor(name);
                 FREE_ZVAL(name);
			  }
              CAL_INIT_INTERFACE(function_module); 
          }
          else
          {      
              sprintf(abort_text,"RFC function %s is not implemented in this server program",function_name);
              rfc_rc = -1;
          }
		}
	}
	else
    {
        php_error(E_WARNING, "Invalid resource for RFC connection");
        RETURN_LONG(-1);			
    }
   
    if ( rfc_rc != RFC_OK )
    {
        SAL_ABORT (rfc_resource->handle, abort_text);
        zend_list_delete (Z_RESVAL_P(*rfc));
        php_error(E_WARNING, abort_text);
    }

    RETURN_LONG(rfc_rc);
}
/* }}} */
/* }}} */


/* {{{ proto bool saprfc_trfc_install (string tid_check, string tid_commit, string tid_rollback, string tid_confirm, string dispatcher)
 */
PHP_FUNCTION(saprfc_trfc_install)
{
    zval **tid_check, **tid_commit, **tid_rollback, **tid_confirm, **dispatcher;
    RFC_RC rfc_rc;
    SAPRFCLS_FETCH();
	
    if (ZEND_NUM_ARGS() != 5 || zend_get_parameters_ex(5, &tid_check,&tid_commit,&tid_rollback,&tid_confirm,&dispatcher) == FAILURE){
        WRONG_PARAM_COUNT;
    }

    convert_to_string_ex(tid_check);
    convert_to_string_ex(tid_commit);
    convert_to_string_ex(tid_rollback);
    convert_to_string_ex(tid_confirm);
    convert_to_string_ex(dispatcher);

    if ( SAPRFCG(trfc_install_flag) == 1 )
    {
        php_error(E_WARNING, "tRFC managment is already installed.");
        RETURN_FALSE;
    }

    rfc_rc = SAL_INSTALL_FUNCTION("%%USER_GLOBAL_SERVER",__callback_dispatch,NULL);
    if (rfc_rc != RFC_OK)
    {
        php_error(E_WARNING, "RfcInstallFunction() failed with code %d",rfc_rc);
        RETURN_FALSE;
    }

    SAL_INSTALL_TRANSACTION_CONTROL (__callback_tid_check,__callback_tid_commit,__callback_tid_rollback,__callback_tid_confirm);
    SAPRFCG(trfc_install_flag)=1;
    SAPRFCG(trfc_tid_check) = ecalloc (1,Z_STRLEN_P(*tid_check)+1);
    SAPRFCG(trfc_tid_commit) = ecalloc (1,Z_STRLEN_P(*tid_commit)+1);
    SAPRFCG(trfc_tid_rollback) = ecalloc (1,Z_STRLEN_P(*tid_rollback)+1);
    SAPRFCG(trfc_tid_confirm) = ecalloc (1,Z_STRLEN_P(*tid_confirm)+1);
    SAPRFCG(trfc_dispatcher) = ecalloc (1,Z_STRLEN_P(*dispatcher)+1);
    strsafecpy (SAPRFCG(trfc_tid_check),Z_STRVAL_P(*tid_check),Z_STRLEN_P(*tid_check));
    strsafecpy (SAPRFCG(trfc_tid_commit),Z_STRVAL_P(*tid_commit),Z_STRLEN_P(*tid_commit));
    strsafecpy (SAPRFCG(trfc_tid_rollback),Z_STRVAL_P(*tid_rollback),Z_STRLEN_P(*tid_rollback));
    strsafecpy (SAPRFCG(trfc_tid_confirm),Z_STRVAL_P(*tid_confirm),Z_STRLEN_P(*tid_confirm));
    strsafecpy (SAPRFCG(trfc_dispatcher),Z_STRVAL_P(*dispatcher),Z_STRLEN_P(*dispatcher));
    RETURN_TRUE;
}
/* }}} */

/* {{{ proto int saprfc_trfc_call(int fce, string tid)
 */
PHP_FUNCTION(saprfc_trfc_call)
{
    zval **fce, **tid;
    FCE_RESOURCE *fce_resource;
    int type;
	RFC_RC rfc_rc;

    if (ZEND_NUM_ARGS() != 2 || zend_get_parameters_ex(2, &fce, &tid) == FAILURE){
        WRONG_PARAM_COUNT;
    }

    convert_to_string_ex(tid);

    fce_resource = (FCE_RESOURCE *) zend_list_find (Z_RESVAL_P(*fce),&type);
    if (fce_resource && type == le_function)
    {
       CAL_INIT_INTERFACE_EXPORT(fce_resource->fce);
       rfc_rc = CAL_INDIRECT_CALL(fce_resource->fce,fce_resource->handle,Z_STRVAL_P(*tid));
       if ( rfc_rc != 0 )
       {
          php_error(E_WARNING, CAL_DEBUG_MESSAGE());
       }    
       CAL_INIT_INTERFACE_IMPORT(fce_resource->fce); 
    }
    else
    {
       php_error(E_WARNING, "Invalid resource for function module");
       RETURN_LONG(-1);
    }
    RETURN_LONG (rfc_rc);   
}
/* }}} */


/* {{{ proto string saprfc_trfc_tid(int rfc)
 */
PHP_FUNCTION(saprfc_trfc_tid)
{
    zval **rfc;
    RFC_RESOURCE *rfc_resource;
    int type;
    RFC_TID tid;
    RFC_RC rfc_rc;

    if (ZEND_NUM_ARGS() != 1 || zend_get_parameters_ex(1, &rfc) == FAILURE){
        WRONG_PARAM_COUNT;
    }

    *tid = 0; /* empty string */

    rfc_resource = (RFC_RESOURCE *) zend_list_find (Z_RESVAL_P(*rfc),&type);
    if (rfc_resource && type == le_rfc)
        rfc_rc = CAL_CREATE_TID(rfc_resource->handle,tid);
    else
    {
        php_error(E_WARNING, "Invalid resource for RFC connection");
        RETURN_FALSE;			
    }
    if (rfc_rc == RFC_OK)
    {
         tid[RFC_TID_LN]=0;
         RETURN_STRING(tid,1);
    }
    else
    {
         RETURN_FALSE;			
    }

}
/* }}} */


/* {{{ proto void saprfc_set_trace(int rfc, bool level)
 */
PHP_FUNCTION(saprfc_set_trace)
{
    zval **rfc, **level;
    RFC_RESOURCE *rfc_resource;
    int type;
    int lv;

    if (ZEND_NUM_ARGS() != 2 || zend_get_parameters_ex(2, &rfc, &level) == FAILURE){
        WRONG_PARAM_COUNT;
    }

    lv = Z_BVAL_P(*level) == 0 ? 0 : 1;

    rfc_resource = (RFC_RESOURCE *) zend_list_find (Z_RESVAL_P(*rfc),&type);
    if (rfc_resource && type == le_rfc)
    {
       CAL_SET_TRACE(rfc_resource->handle,lv);
    }
	else
    {
       CAL_SET_TRACE(RFC_HANDLE_NULL,lv);
        
    }
    return;

}
/* }}} */


/* {{{ proto array saprfc_server_register_check (string tpid, string gwhost, string gwserver)
 */
PHP_FUNCTION(saprfc_server_register_check)
{
    zval **tpid, **gwhost, **gwserver;
	RFC_RC rfc_rc;
    int ntotal, ninit, nready, nbusy;
    RFC_ERROR_INFO_EX error_info;

    if (ZEND_NUM_ARGS() != 3 || zend_get_parameters_ex(3, &tpid, &gwhost, &gwserver) == FAILURE){
        WRONG_PARAM_COUNT;
    }

    convert_to_string_ex(tpid);
    convert_to_string_ex(gwhost);
    convert_to_string_ex(gwserver);
   
    rfc_rc = SAL_CHECK_REGISTER(Z_STRVAL_P(*tpid),Z_STRVAL_P(*gwhost),Z_STRVAL_P(*gwserver),&ntotal,&ninit,&nready,&nbusy,&error_info);
    if ( rfc_rc != RFC_OK )
    {
        RETURN_LONG (rfc_rc);
    }

    if (array_init(return_value) == FAILURE)
    {
         php_error (E_WARNING,"array_init failed");
         RETURN_LONG (-1);
    }

    add_assoc_long(return_value,"ntotal",ntotal);
    add_assoc_long(return_value,"ninit",ninit);
    add_assoc_long(return_value,"nready",nready);
    add_assoc_long(return_value,"nbusy",nbusy);
    return;

}
/* }}} */


/* {{{ proto array saprfc_server_register_cancel (string tpid, string gwhost, string gwserver)
 */
PHP_FUNCTION(saprfc_server_register_cancel)
{
    zval **tpid, **gwhost, **gwserver;
	RFC_RC rfc_rc;
    int ntotal, ncancel;
    RFC_ERROR_INFO_EX error_info;

    if (ZEND_NUM_ARGS() != 3 || zend_get_parameters_ex(3, &tpid, &gwhost, &gwserver) == FAILURE){
        WRONG_PARAM_COUNT;
    }

    convert_to_string_ex(tpid);
    convert_to_string_ex(gwhost);
    convert_to_string_ex(gwserver);
   
    rfc_rc = SAL_CANCEL_REGISTER(Z_STRVAL_P(*tpid),Z_STRVAL_P(*gwhost),Z_STRVAL_P(*gwserver),&ntotal,&ncancel,&error_info);
    if ( rfc_rc != RFC_OK )
    {
        RETURN_LONG (rfc_rc);
    }

    if (array_init(return_value) == FAILURE)
    {
         php_error (E_WARNING,"array_init failed");
         RETURN_LONG (-1);
    }

    add_assoc_long(return_value,"ntotal",ntotal);
    add_assoc_long(return_value,"ncancel",ncancel);
    return;

}
/* }}} */


/* {{{ proto string saprfc_function_name(int fce)
 */
PHP_FUNCTION(saprfc_function_name)
{
    zval **fce;
    FCE_RESOURCE *fce_resource;
    int type;
    
    if (ZEND_NUM_ARGS() != 1 || zend_get_parameters_ex(1, &fce) == FAILURE){
    WRONG_PARAM_COUNT;
    }

    fce_resource = (FCE_RESOURCE *) zend_list_find (Z_RESVAL_P(*fce),&type);
    if (fce_resource && type == le_function)
    {
          RETURN_STRING (fce_resource->fce->name,1);     
   
    }
    else
    {
          php_error(E_WARNING, "Invalid resource for function module");
          RETURN_FALSE;
    }

}
/* {{{ proto bool saprfc_allow_start_program([string program_list])
 */
PHP_FUNCTION(saprfc_allow_start_program)
{
	zval  **program_list;
	int noparam;
	RFC_RC rfc_rc;

	noparam = 0;
	if (ZEND_NUM_ARGS() > 2) {
        WRONG_PARAM_COUNT;
	} else if (ZEND_NUM_ARGS() == 0) {
		noparam = 1;
	}
    else if  ( zend_get_parameters_ex(1, &program_list) == FAILURE){
        WRONG_PARAM_COUNT;
    }

    convert_to_string_ex(program_list);

	if (noparam == 1) {
		rfc_rc = CAL_ALLOW_START_PROGRAM(NULL);
	}
	else {
		rfc_rc = CAL_ALLOW_START_PROGRAM(Z_STRVAL_P(*program_list));
	}

	if (rfc_rc == RFC_OK)
    {
         RETURN_TRUE;
    }
    else
    {
        php_error(E_WARNING, "RfcAllowStartProgram failed with code = %d",rfc_rc);
        RETURN_FALSE;			
    }
}
 
/* }}} */

/* {{{ proto string saprfc_get_ticket(int rfc)
 */
PHP_FUNCTION(saprfc_get_ticket)
{
    zval **rfc;
    RFC_RESOURCE *rfc_resource;   
    RFC_CHAR ticket[1024];
    RFC_RC rfc_rc;
    int type;
    
    if (ZEND_NUM_ARGS() != 1 || zend_get_parameters_ex(1, &rfc) == FAILURE){
    WRONG_PARAM_COUNT;
    }

    *ticket = 0;
    rfc_resource = (RFC_RESOURCE *) zend_list_find (Z_RESVAL_P(*rfc),&type);
    if (rfc_resource && type == le_rfc)
    {
         rfc_rc = CAL_GET_TICKET(rfc_resource->handle,ticket);
         if (rfc_rc == RFC_OK) {
            RETURN_STRING(ticket,1);
         }
         else {
            php_error(E_WARNING, "Unable get SSO ticket");
            RETURN_FALSE;			
         }
    }
    else
    {
        php_error(E_WARNING, "Invalid resource for RFC connection");
        RETURN_FALSE;			
    }

    return;     
}
/* }}} */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 */
