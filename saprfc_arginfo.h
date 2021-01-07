/* This is a generated file, edit the .stub.php file instead.
 * Stub hash: 0775340295884687ac8848bc850adab886fdd9a3 */

ZEND_BEGIN_ARG_INFO_EX(arginfo_saprfc_open, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, conn, IS_ARRAY, 0)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, timeout, IS_LONG, 0, "-1")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_saprfc_function_discover, 0, 0, 2)
	ZEND_ARG_INFO(0, rfc)
	ZEND_ARG_TYPE_INFO(0, function_module, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, not_trim, _IS_BOOL, 0, "false")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_saprfc_function_define, 0, 0, 3)
	ZEND_ARG_INFO(0, rfc)
	ZEND_ARG_TYPE_INFO(0, function_module, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, def, IS_ARRAY, 0)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, not_trim, _IS_BOOL, 0, "false")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_saprfc_function_interface, 0, 1, MAY_BE_ARRAY|MAY_BE_FALSE)
	ZEND_ARG_INFO(0, fce)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_saprfc_function_debug_info, 0, 1, IS_VOID, 0)
	ZEND_ARG_INFO(0, fce)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, only_value, _IS_BOOL, 0, "false")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_saprfc_optional, 0, 3, _IS_BOOL, 0)
	ZEND_ARG_INFO(0, fce)
	ZEND_ARG_TYPE_INFO(0, name, IS_STRING, 0)
	ZEND_ARG_OBJ_INFO(0, value, boolean, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_saprfc_import, 0, 3, _IS_BOOL, 0)
	ZEND_ARG_INFO(0, fce)
	ZEND_ARG_TYPE_INFO(0, name, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, value, IS_MIXED, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_saprfc_export, 0, 2, IS_MIXED, 0)
	ZEND_ARG_INFO(0, fce)
	ZEND_ARG_TYPE_INFO(0, name, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_saprfc_table_init, 0, 2, _IS_BOOL, 0)
	ZEND_ARG_INFO(0, fce)
	ZEND_ARG_TYPE_INFO(0, name, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_saprfc_table_append, 0, 3, _IS_BOOL, 0)
	ZEND_ARG_INFO(0, fce)
	ZEND_ARG_TYPE_INFO(0, name, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, value, IS_ARRAY, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_saprfc_table_insert, 0, 4, _IS_BOOL, 0)
	ZEND_ARG_INFO(0, fce)
	ZEND_ARG_TYPE_INFO(0, name, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, value, IS_ARRAY, 0)
	ZEND_ARG_TYPE_INFO(0, index, IS_LONG, 0)
ZEND_END_ARG_INFO()

#define arginfo_saprfc_table_modify arginfo_saprfc_table_insert

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_saprfc_table_remove, 0, 3, _IS_BOOL, 0)
	ZEND_ARG_INFO(0, fce)
	ZEND_ARG_TYPE_INFO(0, name, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, index, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_saprfc_table_read, 0, 3, MAY_BE_ARRAY|MAY_BE_FALSE)
	ZEND_ARG_INFO(0, fce)
	ZEND_ARG_TYPE_INFO(0, name, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, index, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_saprfc_table_rows, 0, 2, IS_LONG, 0)
	ZEND_ARG_INFO(0, fce)
	ZEND_ARG_TYPE_INFO(0, name, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_saprfc_call_and_receive, 0, 1, IS_LONG, 0)
	ZEND_ARG_INFO(0, fce)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, timeout, IS_LONG, 1, "null")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_saprfc_error, 0, 0, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_saprfc_exception, 0, 1, IS_STRING, 0)
	ZEND_ARG_INFO(0, fce)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_saprfc_function_free, 0, 1, _IS_BOOL, 0)
	ZEND_ARG_INFO(0, fce)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_saprfc_close, 0, 1, _IS_BOOL, 0)
	ZEND_ARG_INFO(0, rfc)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_saprfc_set_code_page, 0, 2, _IS_BOOL, 0)
	ZEND_ARG_INFO(0, rfc)
	ZEND_ARG_TYPE_INFO(0, codepage, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_saprfc_attributes, 0, 1, MAY_BE_ARRAY|MAY_BE_FALSE)
	ZEND_ARG_INFO(0, rfc)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_saprfc_server_accept, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, args, IS_ARRAY, 0)
ZEND_END_ARG_INFO()

#define arginfo_saprfc_server_import arginfo_saprfc_export

#define arginfo_saprfc_server_export arginfo_saprfc_import

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_saprfc_trfc_dispatch, 0, 2, IS_LONG, 0)
	ZEND_ARG_INFO(0, rfc)
	ZEND_ARG_TYPE_INFO(0, list, IS_ARRAY, 0)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, timeout, IS_LONG, 0, "0")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_saprfc_trfc_install, 0, 5, _IS_BOOL, 0)
	ZEND_ARG_TYPE_INFO(0, tid_check, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, tid_commit, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, tid_rollback, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, tid_confirm, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, dispatcher, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_saprfc_trfc_call, 0, 2, IS_LONG, 0)
	ZEND_ARG_INFO(0, fce)
	ZEND_ARG_TYPE_INFO(0, tid, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_saprfc_trfc_tid, 0, 1, MAY_BE_STRING|MAY_BE_FALSE)
	ZEND_ARG_INFO(0, rfc)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_saprfc_set_trace, 0, 2, IS_VOID, 0)
	ZEND_ARG_INFO(0, rfc)
	ZEND_ARG_OBJ_INFO(0, level, boolean, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_saprfc_server_register_check, 0, 3, MAY_BE_ARRAY|MAY_BE_LONG)
	ZEND_ARG_TYPE_INFO(0, tpid, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, gwhost, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, gwserver, IS_STRING, 0)
ZEND_END_ARG_INFO()

#define arginfo_saprfc_server_register_cancel arginfo_saprfc_server_register_check

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_saprfc_function_name, 0, 1, MAY_BE_STRING|MAY_BE_FALSE)
	ZEND_ARG_INFO(0, fce)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_saprfc_allow_start_program, 0, 0, _IS_BOOL, 0)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, program_list, IS_STRING, 0, "\'\'")
ZEND_END_ARG_INFO()

#define arginfo_saprfc_get_ticket arginfo_saprfc_trfc_tid


ZEND_FUNCTION(saprfc_open);
ZEND_FUNCTION(saprfc_function_discover);
ZEND_FUNCTION(saprfc_function_define);
ZEND_FUNCTION(saprfc_function_interface);
ZEND_FUNCTION(saprfc_function_debug_info);
ZEND_FUNCTION(saprfc_optional);
ZEND_FUNCTION(saprfc_import);
ZEND_FUNCTION(saprfc_export);
ZEND_FUNCTION(saprfc_table_init);
ZEND_FUNCTION(saprfc_table_append);
ZEND_FUNCTION(saprfc_table_insert);
ZEND_FUNCTION(saprfc_table_modify);
ZEND_FUNCTION(saprfc_table_remove);
ZEND_FUNCTION(saprfc_table_read);
ZEND_FUNCTION(saprfc_table_rows);
ZEND_FUNCTION(saprfc_call_and_receive);
ZEND_FUNCTION(saprfc_error);
ZEND_FUNCTION(saprfc_exception);
ZEND_FUNCTION(saprfc_function_free);
ZEND_FUNCTION(saprfc_close);
ZEND_FUNCTION(saprfc_set_code_page);
ZEND_FUNCTION(saprfc_attributes);
ZEND_FUNCTION(saprfc_server_accept);
ZEND_FUNCTION(saprfc_server_import);
ZEND_FUNCTION(saprfc_server_export);
ZEND_FUNCTION(saprfc_server_dispatch);
ZEND_FUNCTION(saprfc_trfc_install);
ZEND_FUNCTION(saprfc_trfc_call);
ZEND_FUNCTION(saprfc_trfc_tid);
ZEND_FUNCTION(saprfc_set_trace);
ZEND_FUNCTION(saprfc_server_register_check);
ZEND_FUNCTION(saprfc_server_register_cancel);
ZEND_FUNCTION(saprfc_function_name);
ZEND_FUNCTION(saprfc_allow_start_program);
ZEND_FUNCTION(saprfc_get_ticket);


static const zend_function_entry ext_functions[] = {
	ZEND_FE(saprfc_open, arginfo_saprfc_open)
	ZEND_FE(saprfc_function_discover, arginfo_saprfc_function_discover)
	ZEND_FE(saprfc_function_define, arginfo_saprfc_function_define)
	ZEND_FE(saprfc_function_interface, arginfo_saprfc_function_interface)
	ZEND_FE(saprfc_function_debug_info, arginfo_saprfc_function_debug_info)
	ZEND_FE(saprfc_optional, arginfo_saprfc_optional)
	ZEND_FE(saprfc_import, arginfo_saprfc_import)
	ZEND_FE(saprfc_export, arginfo_saprfc_export)
	ZEND_FE(saprfc_table_init, arginfo_saprfc_table_init)
	ZEND_FE(saprfc_table_append, arginfo_saprfc_table_append)
	ZEND_FE(saprfc_table_insert, arginfo_saprfc_table_insert)
	ZEND_FE(saprfc_table_modify, arginfo_saprfc_table_modify)
	ZEND_FE(saprfc_table_remove, arginfo_saprfc_table_remove)
	ZEND_FE(saprfc_table_read, arginfo_saprfc_table_read)
	ZEND_FE(saprfc_table_rows, arginfo_saprfc_table_rows)
	ZEND_FE(saprfc_call_and_receive, arginfo_saprfc_call_and_receive)
	ZEND_FE(saprfc_error, arginfo_saprfc_error)
	ZEND_FE(saprfc_exception, arginfo_saprfc_exception)
	ZEND_FE(saprfc_function_free, arginfo_saprfc_function_free)
	ZEND_FE(saprfc_close, arginfo_saprfc_close)
	ZEND_FE(saprfc_set_code_page, arginfo_saprfc_set_code_page)
	ZEND_FE(saprfc_attributes, arginfo_saprfc_attributes)
	ZEND_FE(saprfc_server_accept, arginfo_saprfc_server_accept)
	ZEND_FE(saprfc_server_import, arginfo_saprfc_server_import)
	ZEND_FE(saprfc_server_export, arginfo_saprfc_server_export)
	ZEND_FALIAS(saprfc_trfc_dispatch, saprfc_server_dispatch, arginfo_saprfc_trfc_dispatch)
	ZEND_FE(saprfc_trfc_install, arginfo_saprfc_trfc_install)
	ZEND_FE(saprfc_trfc_call, arginfo_saprfc_trfc_call)
	ZEND_FE(saprfc_trfc_tid, arginfo_saprfc_trfc_tid)
	ZEND_FE(saprfc_set_trace, arginfo_saprfc_set_trace)
	ZEND_FE(saprfc_server_register_check, arginfo_saprfc_server_register_check)
	ZEND_FE(saprfc_server_register_cancel, arginfo_saprfc_server_register_cancel)
	ZEND_FE(saprfc_function_name, arginfo_saprfc_function_name)
	ZEND_FE(saprfc_allow_start_program, arginfo_saprfc_allow_start_program)
	ZEND_FE(saprfc_get_ticket, arginfo_saprfc_get_ticket)
	ZEND_FE_END
};
