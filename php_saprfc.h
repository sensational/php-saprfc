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

#if PHP_VERSION_ID >= 80000
  #define FREE_ZVAL(arg)
  #define MAKE_STD_ZVAL(arg)

  #define TSRMLS_DC
  #define TSRMLS_CC
  #define TSRMLS_FETCH()
#endif

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

#if PHP_VERSION_ID >= 80000
#include "saprfc_arginfo.h"
#else
#include "saprfc_legacy_arginfo.h"
#endif

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
   char *connection_string;
} RFC_RESOURCE;
#define PHP_RFC_RES_NAME "saprfc handle"

typedef struct {
   RFC_HANDLE handle;
   RFC_RESOURCE *rfc_resource;
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
