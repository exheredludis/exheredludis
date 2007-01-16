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

#ifndef PALUDIS_GUARD_SRC_COMMAND_LINE_HH
#define PALUDIS_GUARD_SRC_COMMAND_LINE_HH 1

#include <paludis/args/args.hh>
#include <paludis/util/instantiation_policy.hh>
#include <src/common_args/debug_build_arg.hh>
#include <src/common_args/log_level_arg.hh>
#include <src/common_args/deps_option_arg.hh>

/** \file
 * Declarations for the CommandLine class.
 */

/**
 * Our command line.
 */
class CommandLine :
    public paludis::args::ArgsHandler,
    public paludis::InstantiationPolicy<CommandLine, paludis::instantiation_method::SingletonAsNeededTag>
{
    friend class paludis::InstantiationPolicy<CommandLine, paludis::instantiation_method::SingletonAsNeededTag>;

    private:
        /// Constructor.
        CommandLine();

        /// Destructor.
        ~CommandLine();

    public:
        ///\name Program information
        ///\{

        virtual std::string app_name() const;
        virtual std::string app_synopsis() const;
        virtual std::string app_description() const;

        ///\}

        /// \name Action arguments
        ///{

        /// Action arguments.
        paludis::args::ArgsGroup action_args;

        /// --query
        paludis::args::SwitchArg a_query;

        /// --install
        paludis::args::SwitchArg a_install;

        /// --uninstall
        paludis::args::SwitchArg a_uninstall;

        /// --uninstall-unused
        paludis::args::SwitchArg a_uninstall_unused;

        /// --sync
        paludis::args::SwitchArg a_sync;

        /// --report
        paludis::args::SwitchArg a_report;

        /// --contents
        paludis::args::SwitchArg a_contents;

        /// --owner
        paludis::args::SwitchArg a_owner;

        /// --version
        paludis::args::SwitchArg a_version;

        /// --info
        paludis::args::SwitchArg a_info;

        /// --help
        paludis::args::SwitchArg a_help;

        /// Action arguments (internal).
        paludis::args::ArgsGroup action_args_internal;

        /// --has-version
        paludis::args::SwitchArg a_has_version;

        /// --best-version
        paludis::args::SwitchArg a_best_version;

        /// --environment-variable
        paludis::args::SwitchArg a_environment_variable;

        /// --configuration-variable
        paludis::args::SwitchArg a_configuration_variable;

        /// --list-repositories
        paludis::args::SwitchArg a_list_repositories;

        /// --list-categories
        paludis::args::SwitchArg a_list_categories;

        /// --list-packages
        paludis::args::SwitchArg a_list_packages;

        /// --list-sets
        paludis::args::SwitchArg a_list_sets;

        /// --list-sync-protocols
        paludis::args::SwitchArg a_list_sync_protocols;

        /// --list-repository-formats
        paludis::args::SwitchArg a_list_repository_formats;

        /// --list-dep-tag-categories
        paludis::args::SwitchArg a_list_dep_tag_categories;

        /// --update-news
        paludis::args::SwitchArg a_update_news;

        /// --regenerate-installed-cache
        paludis::args::SwitchArg a_regenerate_installed_cache;

        /// --regenerate-installable-cache
        paludis::args::SwitchArg a_regenerate_installable_cache;

        ///}

        /// \name General arguments
        ///{

        /// General arguments.
        paludis::args::ArgsGroup general_args;

        /// --log-level
        paludis::args::LogLevelArg a_log_level;

        /// --no-colour
        paludis::args::SwitchArg a_no_colour;

        /// --no-color
        paludis::args::AliasArg a_no_color;

        /// --config-suffix
        paludis::args::StringArg a_config_suffix;

        /// --resume-command-template
        paludis::args::StringArg a_resume_command_template;

        ///}

        /// \name Query arguments
        ///{

        /// Query arguments.
        paludis::args::ArgsGroup query_args;

        /// --show-deps
        paludis::args::SwitchArg a_show_deps;

        /// --show-metadata
        paludis::args::SwitchArg a_show_metadata;

        /// }

        /// \name (Un)Install arguments
        /// {

        /// Install arguments.
        paludis::args::ArgsGroup install_args;

        /// --pretend
        paludis::args::SwitchArg a_pretend;

        /// --preserve-world
        paludis::args::SwitchArg a_preserve_world;

        /// --add-to-world-atom
        paludis::args::StringArg a_add_to_world_atom;

        /// --no-config-protection
        paludis::args::SwitchArg a_no_config_protection;

        /// --debug-build
        paludis::args::DebugBuildArg a_debug_build;

        /// --fetch
        paludis::args::SwitchArg a_fetch;

        /// --safe-resume
        paludis::args::SwitchArg a_safe_resume;

        /// --show-reasons
        paludis::args::EnumArg a_show_reasons;

        /// --show-use-descriptions
        paludis::args::EnumArg a_show_use_descriptions;

        /// }

        /// \name Uninstall arguments
        ///\{

        paludis::args::ArgsGroup uninstall_args;

        /// --with-unused-dependencies
        paludis::args::SwitchArg a_with_unused_dependencies;

        /// --with-dependencies
        paludis::args::SwitchArg a_with_dependencies;

        /// --all-versions
        paludis::args::SwitchArg a_all_versions;

        ///\}

        /// \name DepList behaviour arguments
        /// {

        /// DepList behaviour arguments.
        paludis::args::ArgsGroup dl_args;

        paludis::args::EnumArg dl_reinstall;
        paludis::args::EnumArg dl_reinstall_scm;
        paludis::args::EnumArg dl_upgrade;
        paludis::args::EnumArg dl_new_slots;

        paludis::args::DepsOptionArg dl_deps_default;

        paludis::args::DepsOptionArg dl_installed_deps_pre;
        paludis::args::DepsOptionArg dl_installed_deps_runtime;
        paludis::args::DepsOptionArg dl_installed_deps_post;

        paludis::args::DepsOptionArg dl_uninstalled_deps_pre;
        paludis::args::DepsOptionArg dl_uninstalled_deps_runtime;
        paludis::args::DepsOptionArg dl_uninstalled_deps_post;
        paludis::args::DepsOptionArg dl_uninstalled_deps_suggested;

        paludis::args::EnumArg dl_suggested;
        paludis::args::EnumArg dl_circular;
        paludis::args::EnumArg dl_blocks;
        paludis::args::StringSetArg dl_override_masks;

        paludis::args::EnumArg dl_fall_back;

        /// }

        /// \name List arguments
        /// {

        /// List arguments.
        paludis::args::ArgsGroup list_args;

        /// --repository
        paludis::args::StringSetArg a_repository;

        /// --category
        paludis::args::StringSetArg a_category;

        /// --package
        paludis::args::StringSetArg a_package;

        /// --set
        paludis::args::StringSetArg a_set;

        /// }

        /// \name Owner arguments
        /// {

        /// Owner arguments.
        paludis::args::ArgsGroup owner_args;

        /// --full-match
        paludis::args::SwitchArg a_full_match;

        /// }


        /// \name Deprecated arguments
        ///\{

        paludis::args::ArgsGroup deprecated_args;
        paludis::args::SwitchArg a_dl_no_unnecessary_upgrades;
        paludis::args::SwitchArg a_dl_drop_all;
        paludis::args::SwitchArg a_dl_ignore_installed;
        paludis::args::StringArg a_show_install_reasons;

        ///\}
};

#endif
