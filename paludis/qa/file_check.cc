/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007 Ciaran McCreesh <ciaranm@ciaranm.org>
 *
 * This file is part of the Paludis package manager. Paludis is free software;
 * you can redistribute it and/or modify it under the terms of the GNU General
 * Public License version 2, as published by the Free Software Foundation.
 *
 * Paludis is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 59 Temple
 * Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <paludis/qa/file_check.hh>
#include <paludis/qa/broken_global_variables_check.hh>
#include <paludis/qa/changelog_check.hh>
#include <paludis/qa/defaults_check.hh>
#include <paludis/qa/deprecated_functions_check.hh>
#include <paludis/qa/ebuild_name_check.hh>
#include <paludis/qa/filename_check.hh>
#include <paludis/qa/file_permissions_check.hh>
#include <paludis/qa/function_check.hh>
#include <paludis/qa/glep_31_check.hh>
#include <paludis/qa/header_check.hh>
#include <paludis/qa/metadata_check.hh>
#include <paludis/qa/subshell_die_check.hh>
#include <paludis/qa/root_check.hh>
#include <paludis/qa/variable_assigns_check.hh>
#include <paludis/qa/whitespace_check.hh>

#include <paludis/util/virtual_constructor-impl.hh>

using namespace paludis;
using namespace paludis::qa;

template class VirtualConstructor<std::string, tr1::shared_ptr<FileCheck> (*) (),
         virtual_constructor_not_found::ThrowException<NoSuchFileCheckTypeError> >;

FileCheck::FileCheck()
{
}

NoSuchFileCheckTypeError::NoSuchFileCheckTypeError(const std::string & s) throw () :
    Exception("No such file check type: '" + s + "'")
{
}

FileCheckMaker::FileCheckMaker()
{
    register_maker(BrokenGlobalVariablesCheck::identifier(), &MakeFileCheck<BrokenGlobalVariablesCheck>::make_file_check);
    register_maker(ChangeLogCheck::identifier(), &MakeFileCheck<ChangeLogCheck>::make_file_check);
    register_maker(DefaultsCheck::identifier(), &MakeFileCheck<DefaultsCheck>::make_file_check);
    register_maker(DeprecatedFunctionsCheck::identifier(), &MakeFileCheck<DeprecatedFunctionsCheck>::make_file_check);
    register_maker(EbuildNameCheck::identifier(), &MakeFileCheck<EbuildNameCheck>::make_file_check);
    register_maker(FileNameCheck::identifier(), &MakeFileCheck<FileNameCheck>::make_file_check);
    register_maker(FilePermissionsCheck::identifier(), &MakeFileCheck<FilePermissionsCheck>::make_file_check);
    register_maker(FunctionCheck::identifier(), &MakeFileCheck<FunctionCheck>::make_file_check);
    register_maker(Glep31Check::identifier(), &MakeFileCheck<Glep31Check>::make_file_check);
    register_maker(HeaderCheck::identifier(), &MakeFileCheck<HeaderCheck>::make_file_check);
    register_maker(MetadataCheck::identifier(), &MakeFileCheck<MetadataCheck>::make_file_check);
    register_maker(RootCheck::identifier(), &MakeFileCheck<RootCheck>::make_file_check);
    register_maker(SubshellDieCheck::identifier(), &MakeFileCheck<SubshellDieCheck>::make_file_check);
    register_maker(VariableAssignsCheck::identifier(), &MakeFileCheck<VariableAssignsCheck>::make_file_check);
    register_maker(WhitespaceCheck::identifier(), &MakeFileCheck<WhitespaceCheck>::make_file_check);
}

