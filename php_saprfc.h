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
   $Id: php_saprfc.h,v 1.10 2005/12/18 16:25:42 koucky Exp $
 */

#ifndef PHP_SAPRFC_H
#define PHP_SAPRFC_H

#include "rfccal.h"

extern zend_module_entry saprfc_module_entry;
#define phpext_saprfc_ptr &saprfc_module_entry

#ifdef PHP_WIN32
#define PHP_SAPRFC_API __declspec(dllexport)
#else
#define PHP_SAPRFC_API
#endif

#ifdef ZTS
#include "TSRM.h"
#endif

PHP_MINIT_FUNCTION(saprfc);
PHP_MSHUTDOWN_FUNCTION(saprfc);
PHP_RINIT_FUNCTION(saprfc);
PHP_RSHUTDOWN_FUNCTION(saprfc);
PHP_MINFO_FUNCTION(saprfc);

PHP_FUNCTION(saprfc_open);
PHP_FUNCTION(saprfc_function_discover);
PHP_FUNCTION(saprfc_function_define);
PHP_FUNCTION(saprfc_function_interface);
PHP_FUNCTION(saprfc_function_debug_info);
PHP_FUNCTION(saprfc_optional);
PHP_FUNCTION(saprfc_import);
PHP_FUNCTION(saprfc_export);
PHP_FUNCTION(saprfc_table_init);
PHP_FUNCTION(saprfc_table_append);
PHP_FUNCTION(saprfc_table_insert);
PHP_FUNCTION(saprfc_table_modify);
PHP_FUNCTION(saprfc_table_remove);
PHP_FUNCTION(saprfc_table_read);
PHP_FUNCTION(saprfc_table_rows);
PHP_FUNCTION(saprfc_call_and_receive);
PHP_FUNCTION(saprfc_exception);
PHP_FUNCTION(saprfc_error);
PHP_FUNCTION(saprfc_function_free);
PHP_FUNCTION(saprfc_close);
PHP_FUNCTION(saprfc_set_code_page);
PHP_FUNCTION(saprfc_attributes);
PHP_FUNCTION(saprfc_server_accept);
PHP_FUNCTION(saprfc_server_import);
PHP_FUNCTION(saprfc_server_export);
PHP_FUNCTION(saprfc_server_dispatch);
PHP_FUNCTION(saprfc_trfc_install);
PHP_FUNCTION(saprfc_trfc_dispatch);
PHP_FUNCTION(saprfc_trfc_call);
PHP_FUNCTION(saprfc_trfc_tid);
PHP_FUNCTION(saprfc_set_trace);
PHP_FUNCTION(saprfc_server_register_check);
PHP_FUNCTION(saprfc_server_register_cancel);
PHP_FUNCTION(saprfc_function_name);
PHP_FUNCTION(saprfc_allow_start_program);
PHP_FUNCTION(saprfc_get_ticket);


/*
      Declare any global variables you may need between the BEGIN
    and END macros here:
*/

ZEND_BEGIN_MODULE_GLOBALS(saprfc)
    int trfc_install_flag;
    char *trfc_tid_check;
    char *trfc_tid_commit;
    char *trfc_tid_rollback;
    char *trfc_tid_confirm;
    char *trfc_dispatcher;
ZEND_END_MODULE_GLOBALS(saprfc)


/* In every function that needs to use variables in php_saprfc_globals,
   do call SAPRFCLS_FETCH(); after declaring other variables used by
   that function, and always refer to them as SAPRFCG(variable).
   You are encouraged to rename these macros something shorter, see
   examples in any other php module directory.
*/

#ifdef ZTS
#define SAPRFCG(v) (saprfc_globals->v)
#define SAPRFCLS_FETCH() zend_saprfc_globals *saprfc_globals = ts_resource(saprfc_globals_id)
#else
#define SAPRFCG(v) (saprfc_globals.v)
#define SAPRFCLS_FETCH()
#endif

typedef struct {
   RFC_HANDLE handle;
   int client;
} RFC_RESOURCE;
#define PHP_RFC_RES_NAME "saprfc handle"

typedef struct {
   RFC_HANDLE handle;
   CALD_FUNCTION_MODULE *fce;
} FCE_RESOURCE;
#define PHP_RFC_FUNC_RES_NAME "saprfc function module"

#endif    /* PHP_SAPRFC_H */


/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 */
