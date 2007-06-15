/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007 Ciaran McCreesh <ciaranm@ciaranm.org>
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

#include <paludis/eapi.hh>
#include <paludis/name.hh>
#include <paludis/dep_spec.hh>
#include <paludis/hashed_containers.hh>
#include <paludis/util/dir_iterator.hh>
#include <paludis/util/is_file_with_extension.hh>
#include <paludis/util/system.hh>
#include <paludis/util/strip.hh>
#include <paludis/util/destringify.hh>
#include <paludis/util/make_shared_ptr.hh>
#include <paludis/util/collection_concrete.hh>
#include <paludis/util/tokeniser.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/instantiation_policy-impl.hh>
#include <paludis/config_file.hh>

using namespace paludis;

#include <paludis/eapi-sr.cc>

template class InstantiationPolicy<EAPIData, instantiation_method::SingletonTag>;

namespace paludis
{
    template<>
    struct Implementation<EAPIData>
    {
        MakeHashedMap<std::string, tr1::shared_ptr<const EAPI> >::Type values;

        Implementation()
        {
            Context c("When loading EAPI data:");

            for (DirIterator d(getenv_with_default("PALUDIS_EAPIS_DIR", DATADIR "/paludis/eapis")), d_end ;
                    d != d_end ; ++d)
            {
                if (! is_file_with_extension(*d, ".conf", IsFileWithOptions()))
                    continue;

                Context cc("When loading EAPI file '" + stringify(*d) + "':");

                KeyValueConfigFile k(*d, KeyValueConfigFileOptions());

                values.insert(std::make_pair(strip_trailing_string(d->basename(), ".conf"),
                            make_shared_ptr(new EAPI(strip_trailing_string(d->basename(), ".conf"), make_shared_ptr(new SupportedEAPI(
                                            SupportedEAPI::create()
                                            .package_dep_spec_parse_mode(destringify<PackageDepSpecParseMode>(
                                                    k.get("package_dep_spec_parse_mode")))
                                            .strict_package_dep_spec_parse_mode(destringify<PackageDepSpecParseMode>(
                                                    k.get("strict_package_dep_spec_parse_mode").empty() ?
                                                    k.get("package_dep_spec_parse_mode") :
                                                    k.get("strict_package_dep_spec_parse_mode")))
                                            .dependency_spec_tree_parse_mode(destringify<DependencySpecTreeParseMode>(
                                                    k.get("dependency_spec_tree_parse_mode")))
                                            .iuse_flag_parse_mode(destringify<IUseFlagParseMode>(
                                                    k.get("iuse_flag_parse_mode")))
                                            .strict_iuse_flag_parse_mode(destringify<IUseFlagParseMode>(
                                                    k.get("strict_iuse_flag_parse_mode").empty() ?
                                                    k.get("iuse_flag_parse_mode") :
                                                    k.get("strict_iuse_flag_parse_mode")))
                                            .breaks_portage(destringify<bool>(k.get("breaks_portage")))
                                            .uri_supports_arrow(destringify<bool>(k.get("uri_supports_arrow")))

                                            .ebuild_options(make_shared_ptr(new EAPIEbuildOptions(
                                                        EAPIEbuildOptions::create()
                                                        .want_aa_var(destringify<bool>(k.get("want_aa_var")))
                                                        .want_kv_var(destringify<bool>(k.get("want_kv_var")))
                                                        .want_arch_var(destringify<bool>(k.get("want_arch_var")))
                                                        .want_portage_emulation_vars(destringify<bool>(k.get("want_portage_emulation_vars")))
                                                        .require_use_expand_in_iuse(destringify<bool>(k.get("require_use_expand_in_iuse")))
                                                        .rdepend_defaults_to_depend(destringify<bool>(k.get("rdepend_defaults_to_depend")))
                                                        .non_empty_variables(k.get("non_empty_variables"))
                                                        .directory_variables(k.get("directory_variables"))
                                                        .directory_if_exists_variables(k.get("directory_if_exists_variables"))
                                                        .ebuild_must_not_set_variables(k.get("ebuild_must_not_set_variables"))
                                                        .source_merged_variables(k.get("source_merged_variables"))
                                                        .must_not_change_variables(k.get("must_not_change_variables"))
                                                        .support_eclasses(destringify<bool>(k.get("support_eclasses")))
                                                        .support_exlibs(destringify<bool>(k.get("support_exlibs")))
                                                        .utility_path_suffixes(k.get("utility_path_suffixes"))
                                                        .ebuild_module_suffixes(k.get("ebuild_module_suffixes"))
                                                        )))

                                            .ebuild_phases(make_shared_ptr(new EAPIEbuildPhases(
                                                            EAPIEbuildPhases::create()
                                                            .ebuild_install(k.get("ebuild_install"))
                                                            .ebuild_uninstall(k.get("ebuild_uninstall"))
                                                            .ebuild_pretend(k.get("ebuild_pretend"))
                                                            .ebuild_metadata(k.get("ebuild_metadata"))
                                                            .ebuild_fetch(k.get("ebuild_fetch"))
                                                            .ebuild_nofetch(k.get("ebuild_nofetch"))
                                                            .ebuild_variable(k.get("ebuild_variable"))
                                                            .ebuild_config(k.get("ebuild_config")))))

                                            .ebuild_metadata_variables(make_shared_ptr(new EAPIEbuildMetadataVariables(
                                                            EAPIEbuildMetadataVariables::create()
                                                            .metadata_build_depend(k.get("metadata_build_depend"))
                                                            .metadata_run_depend(k.get("metadata_run_depend"))
                                                            .metadata_slot(k.get("metadata_slot"))
                                                            .metadata_src_uri(k.get("metadata_src_uri"))
                                                            .metadata_restrict(k.get("metadata_restrict"))
                                                            .metadata_homepage(k.get("metadata_homepage"))
                                                            .metadata_license(k.get("metadata_license"))
                                                            .metadata_description(k.get("metadata_description"))
                                                            .metadata_keywords(k.get("metadata_keywords"))
                                                            .metadata_eclass_keywords(k.get("metadata_eclass_keywords"))
                                                            .metadata_inherited(k.get("metadata_inherited"))
                                                            .metadata_iuse(k.get("metadata_iuse"))
                                                            .metadata_pdepend(k.get("metadata_pdepend"))
                                                            .metadata_provide(k.get("metadata_provide"))
                                                            .metadata_eapi(k.get("metadata_eapi")))))
                                            ))))));
            }

            MakeHashedMap<std::string, tr1::shared_ptr<const EAPI> >::Type::const_iterator i(values.find("0"));
            if (i == values.end())
                throw EAPIConfigurationError("No EAPI configuration found for EAPI 0");
            else
                values.insert(std::make_pair("", i->second));

            values.insert(std::make_pair("CRAN-1",
                        make_shared_ptr(new EAPI("CRAN-1", make_shared_ptr(new SupportedEAPI(
                                        SupportedEAPI::create()
                                        .package_dep_spec_parse_mode(pds_pm_permissive)
                                        .strict_package_dep_spec_parse_mode(pds_pm_permissive)
                                        .dependency_spec_tree_parse_mode(dst_pm_eapi_0)
                                        .iuse_flag_parse_mode(iuse_pm_permissive)
                                        .strict_iuse_flag_parse_mode(iuse_pm_permissive)
                                        .breaks_portage(true)
                                        .uri_supports_arrow(false)
                                        .ebuild_options(tr1::shared_ptr<EAPIEbuildOptions>())
                                        .ebuild_metadata_variables(tr1::shared_ptr<EAPIEbuildMetadataVariables>())
                                        .ebuild_phases(tr1::shared_ptr<EAPIEbuildPhases>())
                                        ))))));

            values.insert(std::make_pair("gems-1",
                        make_shared_ptr(new EAPI("gems-1", make_shared_ptr(new SupportedEAPI(
                                        SupportedEAPI::create()
                                        .package_dep_spec_parse_mode(pds_pm_permissive)
                                        .strict_package_dep_spec_parse_mode(pds_pm_permissive)
                                        .dependency_spec_tree_parse_mode(dst_pm_eapi_0)
                                        .iuse_flag_parse_mode(iuse_pm_permissive)
                                        .strict_iuse_flag_parse_mode(iuse_pm_permissive)
                                        .breaks_portage(true)
                                        .uri_supports_arrow(false)
                                        .ebuild_options(tr1::shared_ptr<EAPIEbuildOptions>())
                                        .ebuild_metadata_variables(tr1::shared_ptr<EAPIEbuildMetadataVariables>())
                                        .ebuild_phases(tr1::shared_ptr<EAPIEbuildPhases>())
                                        ))))));
        }
    };
}

EAPIConfigurationError::EAPIConfigurationError(const std::string & s) throw () :
    ConfigurationError("EAPI configuration error: " + s)
{
}

EAPIData::EAPIData() :
    PrivateImplementationPattern<EAPIData>(new Implementation<EAPIData>)
{
}

EAPIData::~EAPIData()
{
}

tr1::shared_ptr<const EAPI>
EAPIData::eapi_from_string(const std::string & s) const
{
    MakeHashedMap<std::string, tr1::shared_ptr<const EAPI> >::Type::const_iterator i(_imp->values.find(s));
    if (i != _imp->values.end())
        return i->second;

    return make_shared_ptr(new EAPI(s, tr1::shared_ptr<SupportedEAPI>()));
}

tr1::shared_ptr<const EAPI>
EAPIData::unknown_eapi() const
{
    return make_shared_ptr(new EAPI("UNKNOWN", tr1::shared_ptr<SupportedEAPI>()));
}

