<?php

/** @generate-legacy-arginfo */

/** @return resource|int|false */
function saprfc_open(array $conn, int $timeout = -1) {}

/**
 * @param resource $rfc
 * @return resource|false
 */
function saprfc_function_discover($rfc, string $function_module, bool $not_trim = false) {}

/**
 * @param resource $rfc
 * @return resource
 */
function saprfc_function_define($rfc, string $function_module, array $def, bool $not_trim = false) {}

/**
 * @param resource $fce
 */
function saprfc_function_interface($fce): array|false {}

/**
 * @param resource $fce
 */
function saprfc_function_debug_info($fce, bool $only_value = false): void {}

/**
 * @param resource $fce
 */
function saprfc_optional($fce, string $name, boolean $value): bool {}

/**
 * @param resource $fce
 */
function saprfc_import($fce, string $name, mixed $value): bool {}

/**
 * @param resource $fce
 */
function saprfc_export($fce, string $name): mixed {}

/**
 * @param resource $fce
 */
function saprfc_table_init($fce, string $name): bool {}

/**
 * @param resource $fce
 */
function saprfc_table_append($fce, string $name, array $value): bool {}

/**
 * @param resource $fce
 */
function saprfc_table_insert($fce, string $name, array $value, int $index): bool {}

/**
 * @param resource $fce
 */
function saprfc_table_modify($fce, string $name, array $value, int $index): bool {}

/**
 * @param resource $fce
 */
function saprfc_table_remove($fce, string $name, int $index): bool {}

/**
 * @param resource $fce
 */
function saprfc_table_read($fce, string $name, int $index): array|false {}

/**
 * @param resource $fce
 */
function saprfc_table_rows($fce, string $name): int {}

/**
 * @param resource $fce
 */
function saprfc_call_and_receive($fce, ?int $timeout = null): int{}

/**
 * Get a last RFC error message
 * @return string
 */
function saprfc_error(): string {}

/**
 * @param resource $fce
 */
function saprfc_exception($fce): string {}

/**
 * @param resource $fce
 */
function saprfc_function_free($fce): bool {}

/**
 *
 * @param resource $rfc
 */
function saprfc_close($rfc): bool{}

/**
 * @param resource $rfc
 */
function saprfc_set_code_page($rfc, string $codepage): bool {}

/**
 * @param resource $rfc
 */
function saprfc_attributes($rfc): array|false {}

/**
 *
 * @return resource|false
 */
function saprfc_server_accept(array $args) {}

/**
 * @param resource $fce
 */
function saprfc_server_import($fce, string $name): mixed {}

/**
 * @param resource $fce
 */
function saprfc_server_export($fce, string $name, mixed $value): bool {}

/**
 * @param resource $rfc
 */
function saprfc_server_dispatch($rfc, array $list, int $timeout = 0): int {}

function saprfc_trfc_install(string $tid_check, string $tid_commit, string $tid_rollback, string $tid_confirm, string $dispatcher): bool {}

/**
 * @param resource $fce
 */
function saprfc_trfc_call($fce, string $tid): int {}

/**
 * @param resource $rfc
 */
function saprfc_trfc_tid($rfc): string|false {}

/**
 * @param resource $rfc
 */
function saprfc_set_trace($rfc, boolean $level): void {}

function saprfc_server_register_check(string $tpid, string $gwhost, string $gwserver): array|int {}

function saprfc_server_register_cancel(string $tpid, string $gwhost, string $gwserver): array|int {}

/**
 * @param resource $fce
 */
function saprfc_function_name($fce): string|false {}

function saprfc_allow_start_program(string $program_list = ''): bool {}

/**
 * @param resource $rfc
 */
function saprfc_get_ticket($rfc): string|false {}
