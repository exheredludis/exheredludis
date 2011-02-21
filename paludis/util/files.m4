dnl vim: set ft=m4 et :
dnl This file is used by Makefile.am.m4. You should use the provided
dnl autogen.bash script to do all the hard work.
dnl
dnl This file is used to avoid having to make lots of repetitive changes in
dnl Makefile.am every time we add a source or test file. The first parameter is
dnl the base filename with no extension; later parameters can be `hh', `cc',
dnl `test', `impl', `testscript'. Note that there isn't much error checking done
dnl on this file at present...

add(`accept_visitor',                    `hh')
add(`action_queue',                      `hh', `cc', `test')
add(`active_object_ptr',                 `hh', `cc', `fwd', `test')
add(`attributes',                        `hh')
add(`buffer_output_stream',              `hh', `cc', `fwd', `test')
add(`byte_swap',                         `hh', `test')
add(`channel',                           `hh', `cc')
add(`checked_delete',                    `hh')
add(`clone',                             `hh', `impl')
add(`config_file',                       `hh', `cc', `fwd', `se', `test', `testscript')
add(`condition_variable',                `hh', `cc', `test')
add(`cookie',                            `hh', `cc')
add(`create_iterator',                   `hh', `fwd', `impl', `test')
add(`damerau_levenshtein',               `hh', `cc', `test')
add(`destringify',                       `hh', `cc', `test')
add(`deferred_construction_ptr',         `hh', `cc', `fwd', `test')
add(`discard_output_stream',             `hh', `cc')
add(`elf',                               `hh', `cc')
add(`elf_dynamic_section',               `hh', `cc')
add(`elf_relocation_section',            `hh', `cc')
add(`elf_sections',                      `hh', `cc')
add(`elf_symbol_section',                `hh', `cc')
add(`elf_types',                         `hh')
add(`enum_iterator',                     `hh', `cc', `fwd', `test')
add(`exception',                         `hh', `cc')
add(`executor',                          `hh', `cc', `fwd')
add(`extract_host_from_url',             `hh', `cc', `fwd', `test')
add(`fd_holder',                         `hh')
add(`fs_iterator',                       `hh', `cc', `fwd', `se', `test', `testscript')
add(`fs_error',                          `hh', `cc')
add(`fs_path',                           `hh', `cc', `fwd', `se', `test', `testscript')
add(`fs_stat',                           `hh', `cc', `fwd', `test', `testscript')
add(`graph',                             `hh', `cc', `fwd', `impl', `test')
add(`hashes',                            `hh', `cc', `test')
add(`iterator_funcs',                    `hh', `test')
add(`indirect_iterator',                 `hh', `fwd', `impl', `test')
add(`is_file_with_extension',            `hh', `cc', `se', `test', `testscript')
add(`join',                              `hh', `test')
add(`log',                               `hh', `cc', `se', `test')
add(`make_named_values',                 `hh', `cc')
add(`make_null_shared_ptr',              `hh')
add(`make_shared_copy',                  `hh', `fwd')
add(`map',                               `hh', `fwd', `impl', `cc')
add(`member_iterator',                   `hh', `fwd', `impl', `test')
add(`md5',                               `hh', `cc', `test')
add(`mutex',                             `hh', `cc', `fwd', `test')
add(`named_value',                       `hh', `cc', `fwd')
add(`no_type',                           `hh')
add(`operators',                         `hh')
add(`options',                           `hh', `fwd', `cc', `test')
add(`pimp',                              `hh', `impl')
add(`pipe',                              `hh', `cc')
add(`pretty_print',                      `hh', `cc', `test')
add(`process',                           `hh', `cc', `fwd', `test', `testscript')
add(`pty',                               `hh', `cc', `test')
add(`random',                            `hh', `cc', `test')
add(`realpath',                          `hh', `cc', `test', `testscript')
add(`remove_shared_ptr',                 `hh')
add(`return_literal_function',           `hh', `cc', `fwd', `test')
add(`rmd160',                            `hh', `cc', `test')
add(`safe_ifstream',                     `hh', `cc', `fwd', `test', `testscript')
add(`safe_ofstream',                     `hh', `cc', `test', `testscript')
add(`save',                              `hh', `test')
add(`sequence',                          `hh', `fwd', `impl', `cc')
add(`set',                               `hh', `fwd', `impl', `cc')
add(`sha1',                              `hh', `cc', `test')
add(`sha256',                            `hh', `cc', `test')
add(`simple_parser',                     `hh', `cc', `test', `fwd')
add(`singleton',                         `hh', `impl', `test')
add(`stringify',                         `hh', `test')
add(`string_list_stream',                `hh', `cc', `fwd', `test')
add(`strip',                             `hh', `cc', `test')
add(`system',                            `hh', `cc', `test')
add(`tail_output_stream',                `hh', `cc', `fwd', `test')
add(`tee_output_stream',                 `hh', `cc', `fwd')
add(`thread',                            `hh', `cc', `test')
add(`thread_pool',                       `hh', `cc', `test')
add(`timestamp',                         `hh', `cc', `fwd')
add(`tokeniser',                         `hh', `cc', `test')
add(`tribool',                           `hh', `cc', `fwd', `test')
add(`type_list',                         `hh', `cc', `fwd')
add(`util',                              `hh')
add(`visitor',                           `hh', `cc', `fwd', `impl')
add(`visitor_cast',                      `hh', `cc', `fwd')
add(`wildcard_expander',                 `hh', `cc', `test', `testscript')
add(`wrapped_forward_iterator',          `hh', `fwd', `impl', `test')
add(`wrapped_output_iterator',           `hh', `fwd', `impl')
add(`wrapped_value',                     `hh', `fwd', `impl', `test')

