/* This is a generated file, edit the .stub.php file instead.
 * Stub hash: 41513f2d638b5fd09dd017a1f1761deed61d2b60 */

ZEND_BEGIN_ARG_INFO_EX(arginfo_saprfc_open, 0, 0, 1)
	ZEND_ARG_INFO(0, conn)
	ZEND_ARG_INFO(0, timeout)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_saprfc_function_discover, 0, 0, 2)
	ZEND_ARG_INFO(0, rfc)
	ZEND_ARG_INFO(0, function_module)
	ZEND_ARG_INFO(0, not_trim)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_saprfc_function_define, 0, 0, 3)
	ZEND_ARG_INFO(0, rfc)
	ZEND_ARG_INFO(0, function_module)
	ZEND_ARG_INFO(0, def)
	ZEND_ARG_INFO(0, not_trim)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_saprfc_function_interface, 0, 0, 1)
	ZEND_ARG_INFO(0, fce)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_saprfc_function_debug_info, 0, 0, 1)
	ZEND_ARG_INFO(0, fce)
	ZEND_ARG_INFO(0, only_value)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_saprfc_optional, 0, 0, 3)
	ZEND_ARG_INFO(0, fce)
	ZEND_ARG_INFO(0, name)
	ZEND_ARG_INFO(0, value)
ZEND_END_ARG_INFO()

#define arginfo_saprfc_import arginfo_saprfc_optional

ZEND_BEGIN_ARG_INFO_EX(arginfo_saprfc_export, 0, 0, 2)
	ZEND_ARG_INFO(0, fce)
	ZEND_ARG_INFO(0, name)
ZEND_END_ARG_INFO()

#define arginfo_saprfc_table_init arginfo_saprfc_export

#define arginfo_saprfc_table_append arginfo_saprfc_optional

ZEND_BEGIN_ARG_INFO_EX(arginfo_saprfc_table_insert, 0, 0, 4)
	ZEND_ARG_INFO(0, fce)
	ZEND_ARG_INFO(0, name)
	ZEND_ARG_INFO(0, value)
	ZEND_ARG_INFO(0, index)
ZEND_END_ARG_INFO()

#define arginfo_saprfc_table_modify arginfo_saprfc_table_insert

ZEND_BEGIN_ARG_INFO_EX(arginfo_saprfc_table_remove, 0, 0, 3)
	ZEND_ARG_INFO(0, fce)
	ZEND_ARG_INFO(0, name)
	ZEND_ARG_INFO(0, index)
ZEND_END_ARG_INFO()

#define arginfo_saprfc_table_read arginfo_saprfc_table_remove

#define arginfo_saprfc_table_rows arginfo_saprfc_export

ZEND_BEGIN_ARG_INFO_EX(arginfo_saprfc_call_and_receive, 0, 0, 1)
	ZEND_ARG_INFO(0, fce)
	ZEND_ARG_INFO(0, timeout)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_saprfc_error, 0, 0, 0)
ZEND_END_ARG_INFO()

#define arginfo_saprfc_exception arginfo_saprfc_function_interface

#define arginfo_saprfc_function_free arginfo_saprfc_function_interface

ZEND_BEGIN_ARG_INFO_EX(arginfo_saprfc_close, 0, 0, 1)
	ZEND_ARG_INFO(0, rfc)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_saprfc_set_code_page, 0, 0, 2)
	ZEND_ARG_INFO(0, rfc)
	ZEND_ARG_INFO(0, codepage)
ZEND_END_ARG_INFO()

#define arginfo_saprfc_attributes arginfo_saprfc_close

ZEND_BEGIN_ARG_INFO_EX(arginfo_saprfc_server_accept, 0, 0, 1)
	ZEND_ARG_INFO(0, args)
ZEND_END_ARG_INFO()

#define arginfo_saprfc_server_import arginfo_saprfc_export

#define arginfo_saprfc_server_export arginfo_saprfc_optional

ZEND_BEGIN_ARG_INFO_EX(arginfo_saprfc_server_dispatch, 0, 0, 2)
	ZEND_ARG_INFO(0, rfc)
	ZEND_ARG_INFO(0, list)
	ZEND_ARG_INFO(0, timeout)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_saprfc_trfc_install, 0, 0, 5)
	ZEND_ARG_INFO(0, tid_check)
	ZEND_ARG_INFO(0, tid_commit)
	ZEND_ARG_INFO(0, tid_rollback)
	ZEND_ARG_INFO(0, tid_confirm)
	ZEND_ARG_INFO(0, dispatcher)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_saprfc_trfc_call, 0, 0, 2)
	ZEND_ARG_INFO(0, fce)
	ZEND_ARG_INFO(0, tid)
ZEND_END_ARG_INFO()

#define arginfo_saprfc_trfc_tid arginfo_saprfc_close

ZEND_BEGIN_ARG_INFO_EX(arginfo_saprfc_set_trace, 0, 0, 2)
	ZEND_ARG_INFO(0, rfc)
	ZEND_ARG_INFO(0, level)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_saprfc_server_register_check, 0, 0, 3)
	ZEND_ARG_INFO(0, tpid)
	ZEND_ARG_INFO(0, gwhost)
	ZEND_ARG_INFO(0, gwserver)
ZEND_END_ARG_INFO()

#define arginfo_saprfc_server_register_cancel arginfo_saprfc_server_register_check

#define arginfo_saprfc_function_name arginfo_saprfc_function_interface

ZEND_BEGIN_ARG_INFO_EX(arginfo_saprfc_allow_start_program, 0, 0, 0)
	ZEND_ARG_INFO(0, program_list)
ZEND_END_ARG_INFO()

#define arginfo_saprfc_get_ticket arginfo_saprfc_close
