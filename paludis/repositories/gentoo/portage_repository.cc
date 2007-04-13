/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006, 2007 Ciaran McCreesh <ciaranm@ciaranm.org>
 * Copyright (c) 2006 Danny van Dyk <kugelfang@gentoo.org>
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

#include "config.h"

#include <paludis/repositories/gentoo/portage_repository.hh>
#include <paludis/repositories/gentoo/portage_repository_profile.hh>
#include <paludis/repositories/gentoo/portage_repository_news.hh>
#include <paludis/repositories/gentoo/portage_repository_sets.hh>
#include <paludis/repositories/gentoo/portage_repository_exceptions.hh>
#include <paludis/repositories/gentoo/portage_repository_entries.hh>
#include <paludis/repositories/gentoo/portage_virtual_version_metadata.hh>
#include <paludis/repositories/gentoo/use_desc.hh>

#include <paludis/config_file.hh>
#include <paludis/dep_spec.hh>
#include <paludis/dep_spec_flattener.hh>
#include <paludis/environment.hh>
#include <paludis/hashed_containers.hh>
#include <paludis/match_package.hh>
#include <paludis/package_database.hh>
#include <paludis/package_database_entry.hh>
#include <paludis/portage_dep_parser.hh>
#include <paludis/query.hh>
#include <paludis/repository_name_cache.hh>
#include <paludis/syncer.hh>
#include <paludis/util/collection_concrete.hh>
#include <paludis/util/dir_iterator.hh>
#include <paludis/util/fs_entry.hh>
#include <paludis/util/is_file_with_extension.hh>
#include <paludis/util/iterator.hh>
#include <paludis/util/log.hh>
#include <paludis/util/pstream.hh>
#include <paludis/util/random.hh>
#include <paludis/util/save.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/strip.hh>
#include <paludis/util/system.hh>
#include <paludis/util/tokeniser.hh>

#include <map>
#include <fstream>
#include <functional>
#include <algorithm>
#include <vector>
#include <limits>

#include <strings.h>
#include <ctype.h>

/** \file
 * Implementation of PortageRepository.
 *
 * \ingroup grpportagerepository
 */

using namespace paludis;

namespace paludis
{
    /// Map for versions.
    typedef MakeHashedMap<QualifiedPackageName, std::tr1::shared_ptr<VersionSpecCollection> >::Type VersionsMap;

    /// Map for repository masks.
    typedef MakeHashedMap<QualifiedPackageName, std::list<std::tr1::shared_ptr<const PackageDepSpec> > >::Type RepositoryMaskMap;

    /// Map for categories.
    typedef MakeHashedMap<CategoryNamePart, bool>::Type CategoryMap;

    /// Map for packages.
    typedef MakeHashedMap<QualifiedPackageName, bool>::Type PackagesMap;

    /// Map for mirrors.
    typedef MakeHashedMultiMap<std::string, std::string>::Type MirrorMap;

    /// Map for metadata.
    typedef MakeHashedMap<std::pair<QualifiedPackageName, VersionSpec>,
            std::tr1::shared_ptr<VersionMetadata> >::Type MetadataMap;

    /// Map for virtuals.
    typedef MakeHashedMap<QualifiedPackageName, std::tr1::shared_ptr<const PackageDepSpec> >::Type VirtualsMap;

    typedef std::list<RepositoryPortageInterface::ProfilesDescLine> ProfilesDesc;

    /**
     * Implementation data for a PortageRepository.
     *
     * \ingroup grpportagerepository
     */
    template <>
    struct Implementation<PortageRepository>
    {
        /// Our parameters
        const PortageRepositoryParams params;

        /// Have we loaded our category names?
        mutable bool has_category_names;

        /// Our category names, and whether we have a fully loaded list
        /// of package names for that category.
        mutable CategoryMap category_names;

        /// Our package names, and whether we have a fully loaded list of
        /// version specs for that category.
        mutable PackagesMap package_names;

        /// Our version specs for each package.
        mutable VersionsMap version_specs;

        /// Metadata cache.
        mutable MetadataMap metadata;

        /// Repository mask.
        mutable RepositoryMaskMap repo_mask;

        /// Have repository mask?
        mutable bool has_repo_mask;

        /// Have virtual names?
        mutable bool has_virtuals;

        /// Arch flags
        mutable std::tr1::shared_ptr<UseFlagNameCollection> arch_flags;

        /// Do we have mirrors?
        mutable bool has_mirrors;

        /// Mirrors.
        mutable MirrorMap mirrors;

        /// Constructor.
        Implementation(PortageRepository * const, const PortageRepositoryParams &);

        /// Destructor.
        ~Implementation();

        /// (Empty) provides map.
        const std::map<QualifiedPackageName, QualifiedPackageName> provide_map;

        /// Load profiles, if we haven't already.
        inline void need_profiles() const;

        void need_profiles_desc() const;

        /// Our profile handler.
        mutable std::tr1::shared_ptr<PortageRepositoryProfile> profile_ptr;

        std::list<FSEntry> profiles_dir_locations;

        /// Our news handler.
        mutable std::tr1::shared_ptr<PortageRepositoryNews> news_ptr;

        /// Our sets handler.
        mutable std::tr1::shared_ptr<PortageRepositorySets> sets_ptr;

        /// Our metadata handler.
        mutable std::tr1::shared_ptr<PortageRepositoryEntries> entries_ptr;

        /// Our virtuals
        mutable VirtualsMap our_virtuals;

        /// Have we loaded our virtuals?
        bool has_our_virtuals;

        mutable bool has_profiles_desc;

        mutable ProfilesDesc profiles_desc;

        std::tr1::shared_ptr<RepositoryNameCache> names_cache;

        mutable std::list<std::tr1::shared_ptr<UseDesc> > use_desc;

        PortageRepository * const repo;
    };

    Implementation<PortageRepository>::Implementation(PortageRepository * const r,
            const PortageRepositoryParams & p) :
        params(p),
        has_category_names(false),
        has_repo_mask(false),
        has_virtuals(false),
        has_mirrors(false),
        sets_ptr(new PortageRepositorySets(params.environment, r, p)),
        entries_ptr(PortageRepositoryEntriesMaker::get_instance()->find_maker(
                    params.entry_format)(params.environment, r, p)),
        has_our_virtuals(false),
        has_profiles_desc(false),
        names_cache(new RepositoryNameCache(p.names_cache, r)),
        repo(r)
    {
        if (params.master_repository)
            profiles_dir_locations.push_back(params.master_repository->params().location / "profiles");

        if ((params.location / "profiles").exists())
            profiles_dir_locations.push_back(params.location / "profiles");
    }

    Implementation<PortageRepository>::~Implementation()
    {
    }

    void
    Implementation<PortageRepository>::need_profiles() const
    {
        if (profile_ptr)
            return;

        profile_ptr.reset(new PortageRepositoryProfile(
                    params.environment, repo->name(), *params.profiles));
    }

    void
    Implementation<PortageRepository>::need_profiles_desc() const
    {
        if (has_profiles_desc)
            return;

        Context context("When loading profiles.desc:");

        bool found_one(false);
        for (std::list<FSEntry>::const_iterator p(profiles_dir_locations.begin()),
                p_end(profiles_dir_locations.end()) ; p != p_end ; ++p)
        {
            if (! (*p / "profiles.desc").exists())
                continue;

            found_one = true;

            LineConfigFile f(*p / "profiles.desc", LineConfigFileOptions());
            for (LineConfigFile::Iterator line(f.begin()), line_end(f.end()) ; line != line_end ; ++line)
            {
                std::vector<std::string> tokens;
                WhitespaceTokeniser::get_instance()->tokenise(*line,
                        std::back_inserter(tokens));
                if (tokens.size() < 3)
                    continue;

                FSEntryCollection::Concrete profiles;
                profiles.push_back(*p / tokens.at(1));
                profiles_desc.push_back(RepositoryPortageInterface::ProfilesDescLine::create()
                        .arch(tokens.at(0))
                        .path(*profiles.begin())
                        .status(tokens.at(2))
                        .profile(std::tr1::shared_ptr<PortageRepositoryProfile>(new PortageRepositoryProfile(
                                    params.environment, repo->name(), profiles))));
            }
        }

        if (! found_one)
            throw PortageRepositoryConfigurationError("No profiles.desc found");

        has_profiles_desc = true;
    }
}

PortageRepository::PortageRepository(const PortageRepositoryParams & p) :
    Repository(PortageRepository::fetch_repo_name(stringify(p.location)),
            RepositoryCapabilities::create()
            .mask_interface(this)
            .installable_interface(this)
            .installed_interface(0)
            .news_interface(this)
            .sets_interface(this)
            .syncable_interface(this)
            .uninstallable_interface(0)
            .use_interface(this)
            .world_interface(0)
            .environment_variable_interface(this)
            .mirrors_interface(this)
            .virtuals_interface(this)
            .provides_interface(0)
            .contents_interface(0)
            .config_interface(0)
            .destination_interface(p.enable_destinations ? this : 0)
            .licenses_interface(this)
            .portage_interface(this)
            .hook_interface(this),
            p.entry_format),
    PrivateImplementationPattern<PortageRepository>(new Implementation<PortageRepository>(this, p))
{
    // the info_vars and info_pkgs info is only added on demand, since it's
    // fairly slow to calculate.
    std::tr1::shared_ptr<RepositoryInfoSection> config_info(new RepositoryInfoSection("Configuration information"));

    config_info->add_kv("location", stringify(_imp->params.location));
    config_info->add_kv("profiles", join(_imp->params.profiles->begin(),
                _imp->params.profiles->end(), " "));
    config_info->add_kv("eclassdirs", join(_imp->params.eclassdirs->begin(),
                _imp->params.eclassdirs->end(), " "));
    config_info->add_kv("cache", stringify(_imp->params.cache));
    config_info->add_kv("write_cache", stringify(_imp->params.write_cache));
    config_info->add_kv("names_cache", stringify(_imp->params.names_cache));
    config_info->add_kv("distdir", stringify(_imp->params.distdir));
    config_info->add_kv("pkgdir", stringify(_imp->params.pkgdir));
    config_info->add_kv("securitydir", stringify(_imp->params.securitydir));
    config_info->add_kv("setsdir", stringify(_imp->params.setsdir));
    config_info->add_kv("newsdir", stringify(_imp->params.newsdir));
    config_info->add_kv("format", _imp->params.entry_format);
    config_info->add_kv("buildroot", stringify(_imp->params.buildroot));
    config_info->add_kv("sync", _imp->params.sync);
    config_info->add_kv("sync_options", _imp->params.sync_options);
    if (_imp->params.master_repository)
        config_info->add_kv("master_repository", stringify(_imp->params.master_repository->name()));

    _info->add_section(config_info);
}

PortageRepository::~PortageRepository()
{
}

bool
PortageRepository::do_has_category_named(const CategoryNamePart & c) const
{
    Context context("When checking for category '" + stringify(c) +
            "' in " + stringify(name()) + ":");

    need_category_names();
    return _imp->category_names.end() !=
        _imp->category_names.find(c);
}

bool
PortageRepository::do_has_package_named(const QualifiedPackageName & q) const
{
    Context context("When checking for package '" + stringify(q) + "' in " +
            stringify(name()) + ":");

    need_category_names();

    CategoryMap::iterator cat_iter(_imp->category_names.find(q.category));

    if (_imp->category_names.end() == cat_iter)
        return false;

    if (cat_iter->second)
        return _imp->package_names.find(q) !=
            _imp->package_names.end();
    else
    {
        if (_imp->package_names.find(q) !=
                _imp->package_names.end())
            return true;

        FSEntry fs(_imp->params.location);
        fs /= stringify(q.category);
        fs /= stringify(q.package);
        if (! fs.is_directory())
            return false;
        _imp->package_names.insert(std::make_pair(q, false));
        return true;
    }
}

namespace
{
    /**
     * Filter QualifiedPackageName instances by category.
     *
     * \ingroup grpportagerepository
     */
    struct CategoryFilter :
        std::unary_function<bool, QualifiedPackageName>
    {
        /// Our category.
        CategoryNamePart category;

        /// Constructor.
        CategoryFilter(const CategoryNamePart & c) :
            category(c)
        {
        }

        /// Predicate.
        bool operator() (const QualifiedPackageName & a) const
        {
            return a.category == category;
        }
    };
}

std::tr1::shared_ptr<const CategoryNamePartCollection>
PortageRepository::do_category_names() const
{
    Context context("When fetching category names in " + stringify(name()) + ":");

    need_category_names();

    std::tr1::shared_ptr<CategoryNamePartCollection> result(new CategoryNamePartCollection::Concrete);
    CategoryMap::const_iterator i(_imp->category_names.begin()),
        i_end(_imp->category_names.end());
    for ( ; i != i_end ; ++i)
        result->insert(i->first);
    return result;
}

std::tr1::shared_ptr<const QualifiedPackageNameCollection>
PortageRepository::do_package_names(const CategoryNamePart & c) const
{
    /* this isn't particularly fast because it isn't called very often. avoid
     * changing the data structures used to make this faster at the expense of
     * slowing down single item queries. */

    Context context("When fetching package names in category '" + stringify(c)
            + "' in " + stringify(name()) + ":");

    need_category_names();

    if (_imp->category_names.end() == _imp->category_names.find(c))
        return std::tr1::shared_ptr<QualifiedPackageNameCollection>(new QualifiedPackageNameCollection::Concrete);

    if ((_imp->params.location / stringify(c)).is_directory())
        for (DirIterator d(_imp->params.location / stringify(c)), d_end ; d != d_end ; ++d)
        {
            if (! d->is_directory())
                continue;
            if (DirIterator() == std::find_if(DirIterator(*d), DirIterator(),
                        IsFileWithExtension(_imp->entries_ptr->file_extension())))
                continue;

            try
            {
                _imp->package_names.insert(std::make_pair(c + PackageNamePart(d->basename()), false));
            }
            catch (const NameError & e)
            {
                Log::get_instance()->message(ll_warning, lc_context, "Skipping entry '" +
                        d->basename() + "' in category '" + stringify(c) + "' in repository '"
                        + stringify(name()) + "' (" + e.message() + ")");
            }
        }

    _imp->category_names[c] = true;

    std::tr1::shared_ptr<QualifiedPackageNameCollection> result(new QualifiedPackageNameCollection::Concrete);

    std::copy(_imp->package_names.begin(), _imp->package_names.end(),
            transform_inserter(filter_inserter(result->inserter(), CategoryFilter(c)),
                    SelectFirst<const QualifiedPackageName, bool>()));

    return result;
}

std::tr1::shared_ptr<const VersionSpecCollection>
PortageRepository::do_version_specs(const QualifiedPackageName & n) const
{
    Context context("When fetching versions of '" + stringify(n) + "' in "
            + stringify(name()) + ":");

    if (has_package_named(n))
    {
        need_version_names(n);
        return _imp->version_specs.find(n)->second;
    }
    else
        return std::tr1::shared_ptr<VersionSpecCollection>(new VersionSpecCollection::Concrete);
}

bool
PortageRepository::do_has_version(const QualifiedPackageName & q,
        const VersionSpec & v) const
{
    Context context("When checking for version '" + stringify(v) + "' in '"
            + stringify(q) + "' in " + stringify(name()) + ":");

    if (has_package_named(q))
    {
        need_version_names(q);
        std::tr1::shared_ptr<VersionSpecCollection> vv(_imp->version_specs.find(q)->second);
        return vv->end() != vv->find(v);
    }
    else
        return false;
}

void
PortageRepository::need_category_names() const
{
    if (_imp->has_category_names)
        return;

    Context context("When loading category names for " + stringify(name()) + ":");

    Log::get_instance()->message(ll_debug, lc_context, "need_category_names");

    bool found_one(false);

    for (std::list<FSEntry>::const_iterator p(_imp->profiles_dir_locations.begin()),
            p_end(_imp->profiles_dir_locations.end()) ; p != p_end ; ++p)
    {
        if (! (*p / "categories").exists())
            continue;

        LineConfigFile cats(*p / "categories", LineConfigFileOptions());

        for (LineConfigFile::Iterator line(cats.begin()), line_end(cats.end()) ;
                line != line_end ; ++line)
        {
            try
            {
                _imp->category_names.insert(std::make_pair(CategoryNamePart(*line), false));
            }
            catch (const NameError & e)
            {
                Log::get_instance()->message(ll_warning, lc_context, "Skipping line '"
                        + *line + "' in '" + stringify(*p / "categories") + "' due to exception '"
                        + stringify(e.message()) + "' ('" + e.what() + ")");
            }
        }

        found_one = true;
    }

    if (! found_one)
    {
        Log::get_instance()->message(ll_qa, lc_context, "No categories file for repository at '"
                + stringify(_imp->params.location) + "', faking it");
        for (DirIterator d(_imp->params.location), d_end ; d != d_end ; ++d)
        {
            if (! d->is_directory())
                continue;

            std::string n(d->basename());
            if (n == "CVS" || n == "distfiles" || n == "scripts" || n == "eclass" || n == "licences"
                    || n == "packages")
                continue;

            try
            {
                _imp->category_names.insert(std::make_pair(CategoryNamePart(n), false));
            }
            catch (const NameError &)
            {
            }
        }
    }

    _imp->has_category_names = true;
}

void
PortageRepository::need_version_names(const QualifiedPackageName & n) const
{
    if (_imp->package_names[n])
        return;

    Context context("When loading versions for '" + stringify(n) + "' in "
            + stringify(name()) + ":");

    std::tr1::shared_ptr<VersionSpecCollection> v(new VersionSpecCollection::Concrete);

    FSEntry path(_imp->params.location / stringify(n.category) /
            stringify(n.package));

    for (DirIterator e(path), e_end ; e != e_end ; ++e)
    {
        if (! IsFileWithExtension(stringify(n.package) + "-",
                    _imp->entries_ptr->file_extension())(*e))
            continue;

        try
        {
            v->insert(VersionSpec(strip_leading_string(
                            strip_trailing_string(e->basename(), _imp->entries_ptr->file_extension()),
                            stringify(n.package) + "-")));
        }
        catch (const NameError & ee)
        {
            Log::get_instance()->message(ll_warning, lc_context, "Skipping entry '"
                    + stringify(*e) + "' for '" + stringify(n) + "' in repository '"
                    + stringify(name()) + "' due to exception '" + ee.message() + "' ("
                    + ee.what() + ")'");
        }
    }

    _imp->version_specs.insert(std::make_pair(n, v));
    _imp->package_names[n] = true;
}

RepositoryName
PortageRepository::fetch_repo_name(const std::string & location)
{
    try
    {
        do
        {
            FSEntry name_file(location);
            name_file /= "profiles";
            name_file /= "repo_name";

            if (! name_file.is_regular_file())
                break;

            LineConfigFile f(name_file, LineConfigFileOptions());
            if (f.begin() == f.end())
                break;
            return RepositoryName(*f.begin());

        } while (false);
    }
    catch (...)
    {
    }

    std::string modified_location(FSEntry(location).basename());
    std::replace(modified_location.begin(), modified_location.end(), '/', '-');

    Log::get_instance()->message(ll_qa, lc_no_context, "Couldn't open repo_name file in '"
            + location + "/profiles/', falling back to generated name 'x-" + modified_location +
            "' (ignore this message if you have yet to sync this repository).");

    return RepositoryName("x-" + modified_location);
}

std::tr1::shared_ptr<const VersionMetadata>
PortageRepository::do_version_metadata(
        const QualifiedPackageName & q, const VersionSpec & v) const
{
    if (_imp->metadata.end() != _imp->metadata.find(
                std::make_pair(q, v)))
            return _imp->metadata.find(std::make_pair(q, v))->second;

    Context context("When fetching metadata for '" + stringify(q) +
            "-" + stringify(v) + "':");

    if (! has_version(q, v))
        throw NoSuchPackageError(stringify(PackageDatabaseEntry(q, v, name())));

    std::tr1::shared_ptr<VersionMetadata> result(_imp->entries_ptr->generate_version_metadata(q, v));
    _imp->metadata.insert(std::make_pair(std::make_pair(q, v), result));
    return result;
}

bool
PortageRepository::do_query_repository_masks(const QualifiedPackageName & q, const VersionSpec & v) const
{
    if (! _imp->has_repo_mask)
    {
        Context context("When querying repository mask for '" + stringify(q) + "-"
                + stringify(v) + "':");

        for (std::list<FSEntry>::const_iterator p(_imp->profiles_dir_locations.begin()),
                p_end(_imp->profiles_dir_locations.end()) ; p != p_end ; ++p)
        {
            FSEntry fff(*p / "package.mask");
            Context context_local("When reading '" + stringify(fff) + "':");

            if (fff.exists())
            {
                LineConfigFile ff(fff, LineConfigFileOptions());
                for (LineConfigFile::Iterator line(ff.begin()), line_end(ff.end()) ;
                        line != line_end ; ++line)
                {
                    try
                    {
                        std::tr1::shared_ptr<const PackageDepSpec> a(new PackageDepSpec(*line, pds_pm_eapi_0));
                        if (a->package_ptr())
                            _imp->repo_mask[*a->package_ptr()].push_back(a);
                        else
                            Log::get_instance()->message(ll_warning, lc_context, "Loading package.mask spec '"
                                    + stringify(*line) + "' failed because specification does not restrict to a "
                                    "unique package");
                    }
                    catch (const Exception & e)
                    {
                        Log::get_instance()->message(ll_warning, lc_context, "Loading package.mask spec '"
                                + stringify(*line) + "' failed due to exception '" + e.message() + "' ("
                                + e.what() + ")");
                    }
                }
            }
        }

        _imp->has_repo_mask = true;
    }

    RepositoryMaskMap::iterator r(_imp->repo_mask.find(q));
    if (_imp->repo_mask.end() == r)
        return false;
    else
        for (IndirectIterator<std::list<std::tr1::shared_ptr<const PackageDepSpec> >::const_iterator, const PackageDepSpec>
                k(r->second.begin()), k_end(r->second.end()) ; k != k_end ; ++k)
            if (match_package(*_imp->params.environment, *k, PackageDatabaseEntry(q, v, name())))
                return true;

    return false;
}

bool
PortageRepository::do_query_profile_masks(const QualifiedPackageName & n,
        const VersionSpec & v) const
{
    _imp->need_profiles();
    return _imp->profile_ptr->profile_masked(n, v, name());
}

UseFlagState
PortageRepository::do_query_use(const UseFlagName & f, const PackageDatabaseEntry * e) const
{
    _imp->need_profiles();
    return _imp->profile_ptr->use_state_ignoring_masks(f, e);
}

bool
PortageRepository::do_query_use_mask(const UseFlagName & u, const PackageDatabaseEntry * e) const
{
    _imp->need_profiles();
    return _imp->profile_ptr->use_masked(u, e) ||
        (arch_flags()->end() != arch_flags()->find(u) && use_enabled != do_query_use(u, e));
}

bool
PortageRepository::do_query_use_force(const UseFlagName & u, const PackageDatabaseEntry * e) const
{
    _imp->need_profiles();
    return _imp->profile_ptr->use_forced(u, e);
}

std::tr1::shared_ptr<const UseFlagNameCollection>
PortageRepository::do_arch_flags() const
{
    if (! _imp->arch_flags)
    {
        Context context("When loading arch list:");
        _imp->arch_flags.reset(new UseFlagNameCollection::Concrete);

        bool found_one(false);
        for (std::list<FSEntry>::const_iterator p(_imp->profiles_dir_locations.begin()),
                p_end(_imp->profiles_dir_locations.end()) ; p != p_end ; ++p)
        {
            FSEntry a(*p / "arch.list");
            if (! a.exists())
                continue;

            LineConfigFile archs(a, LineConfigFileOptions());
            std::copy(archs.begin(), archs.end(), create_inserter<UseFlagName>(_imp->arch_flags->inserter()));
            found_one = true;
        }

        if (! found_one)
        {
            Log::get_instance()->message(ll_qa, lc_no_context, "Couldn't find arch.list file for repository '"
                    + stringify(name()) + "', arch flags may incorrectly show up as unmasked");
        }
    }

    return _imp->arch_flags;
}

std::tr1::shared_ptr<FSEntry>
PortageRepository::do_license_exists(const std::string & license) const
{
    std::tr1::shared_ptr<FSEntry> p;

    if (_imp->params.master_repository)
    {
        FSEntry l(_imp->params.master_repository->params().location /
                "licenses" / license);
        if (l.exists() && l.is_regular_file())
            p.reset(new FSEntry(l));
    }

    FSEntry l(_imp->params.location / "licenses" / license);
    if (l.exists() && l.is_regular_file())
        p.reset(new FSEntry(l));

    return p;
}

void
PortageRepository::need_mirrors() const
{
    if (! _imp->has_mirrors)
    {
        bool found_one(false);
        for (std::list<FSEntry>::const_iterator p(_imp->profiles_dir_locations.begin()),
                p_end(_imp->profiles_dir_locations.end()) ; p != p_end ; ++p)
        {
            if ((*p / "thirdpartymirrors").exists())
            {
                LineConfigFile mirrors(*p / "thirdpartymirrors", LineConfigFileOptions());
                for (LineConfigFile::Iterator line(mirrors.begin()) ; line != mirrors.end() ; ++line)
                {
                    std::vector<std::string> entries;
                    WhitespaceTokeniser::get_instance()->tokenise(*line, std::back_inserter(entries));
                    if (! entries.empty())
                    {
                        /* pick up to five random mirrors only */
                        static Random r;
                        std::random_shuffle(next(entries.begin()), entries.end(), r);
                        if (entries.size() > 6)
                            entries.resize(6);
                        for (std::vector<std::string>::const_iterator e(next(entries.begin())),
                                e_end(entries.end()) ; e != e_end ; ++e)
                            _imp->mirrors.insert(std::make_pair(entries.at(0), *e));
                    }
                }
            }

            found_one = true;
        }

        if (! found_one)
            Log::get_instance()->message(ll_warning, lc_no_context,
                    "No thirdpartymirrors file found in '"
                    + stringify(_imp->params.location / "profiles") + "', so mirror:// SRC_URI "
                    "components cannot be fetched");

        _imp->has_mirrors = true;
    }
}

void
PortageRepository::do_install(const QualifiedPackageName & q, const VersionSpec & v,
        const InstallOptions & o) const
{
    _imp->need_profiles();
    _imp->entries_ptr->install(q, v, o, _imp->profile_ptr);
}

std::tr1::shared_ptr<DepSpec>
PortageRepository::do_package_set(const SetName & s) const
{
    if (s.data() == "system")
    {
        _imp->need_profiles();
        return _imp->profile_ptr->system_packages();
    }

    return _imp->sets_ptr->package_set(s);
}

std::tr1::shared_ptr<const SetsCollection>
PortageRepository::sets_list() const
{
    return _imp->sets_ptr->sets_list();
}

bool
PortageRepository::do_sync() const
{
    Context context("When syncing repository '" + stringify(name()) + "':");

    if (_imp->params.sync.empty())
        return false;

    std::list<std::string> sync_list;
    WhitespaceTokeniser::get_instance()->tokenise(_imp->params.sync, std::back_inserter(sync_list));

    bool ok(false);
    for (std::list<std::string>::const_iterator s(sync_list.begin()),
            s_end(sync_list.end()) ; s != s_end ; ++s)
    {
        DefaultSyncer syncer(SyncerParams::create()
                                .environment(_imp->params.environment)
                                .local(stringify(_imp->params.location))
                                .remote(*s));
        SyncOptions opts(_imp->params.sync_options);
        try
        {
            syncer.sync(opts);
        }
        catch (const SyncFailedError & e)
        {
            continue;
        }

        ok=true;
        break;
    }

    if (! ok)
        throw SyncFailedError(stringify(_imp->params.location), _imp->params.sync);

    return true;
}

void
PortageRepository::invalidate()
{
    _imp.reset(new Implementation<PortageRepository>(this, _imp->params));
}

void
PortageRepository::update_news() const
{
    if (! _imp->news_ptr)
        _imp->news_ptr.reset(new PortageRepositoryNews(_imp->params.environment, this, _imp->params));

    _imp->news_ptr->update_news();
}

std::string
PortageRepository::get_environment_variable(
        const PackageDatabaseEntry & for_package,
        const std::string & var) const
{
    Context context("When fetching environment variable '" + var + "' from repository '"
            + stringify(name()) + "':");

    _imp->need_profiles();

    return _imp->entries_ptr->get_environment_variable(for_package.name,
            for_package.version, var, _imp->profile_ptr);
}

std::tr1::shared_ptr<const RepositoryInfo>
PortageRepository::info(bool verbose) const
{
    std::tr1::shared_ptr<const RepositoryInfo> result_non_verbose(Repository::info(verbose));
    if (! verbose)
        return result_non_verbose;

    std::tr1::shared_ptr<RepositoryInfo> result(new RepositoryInfo);

    for (RepositoryInfo::SectionIterator s(result_non_verbose->begin_sections()),
            s_end(result_non_verbose->end_sections()) ; s != s_end ; ++s)
        result->add_section(*s);

    // don't inherit from master_repository, just causes clutter
    std::set<std::string> info_pkgs;
    if ((_imp->params.location / "profiles" / "info_pkgs").exists())
    {
        LineConfigFile vars(_imp->params.location / "profiles" / "info_pkgs", LineConfigFileOptions());
        info_pkgs.insert(vars.begin(), vars.end());
    }

    if (! info_pkgs.empty())
    {
        std::tr1::shared_ptr<RepositoryInfoSection> package_info(new RepositoryInfoSection("Package information"));
        for (std::set<std::string>::const_iterator i(info_pkgs.begin()),
                i_end(info_pkgs.end()) ; i != i_end ; ++i)
        {
            std::tr1::shared_ptr<const PackageDatabaseEntryCollection> q(
                    _imp->params.environment->package_database()->query(
                        query::Matches(PackageDepSpec(*i, pds_pm_eapi_0)) &
                        query::InstalledAtRoot(_imp->params.environment->root()),
                        qo_order_by_version));
            if (q->empty())
                package_info->add_kv(*i, "(none)");
            else
            {
                std::set<VersionSpec> versions;

                for (PackageDatabaseEntryCollection::Iterator qq(q->begin()), qq_end(q->end()) ;
                        qq != qq_end ; ++qq)
                    versions.insert(qq->version);
                package_info->add_kv(*i, join(versions.begin(), versions.end(), ", "));
            }
        }

        result->add_section(package_info);
    }

    // don't inherit from master_repository, just causes clutter
    std::set<std::string> info_vars;
    if ((_imp->params.location / "profiles" / "info_vars").exists())
    {
        LineConfigFile vars(_imp->params.location / "profiles" / "info_vars", LineConfigFileOptions());
        info_vars.insert(vars.begin(), vars.end());
    }

    if (! info_vars.empty() && ! info_pkgs.empty() &&
            ! version_specs(QualifiedPackageName(*info_pkgs.begin()))->empty())
    {
        PackageDatabaseEntry e(QualifiedPackageName(*info_pkgs.begin()),
                *version_specs(QualifiedPackageName(*info_pkgs.begin()))->last(),
                name());
        std::tr1::shared_ptr<RepositoryInfoSection> variable_info(new RepositoryInfoSection("Variable information"));
        for (std::set<std::string>::const_iterator i(info_vars.begin()),
                i_end(info_vars.end()) ; i != i_end ; ++i)
            variable_info->add_kv(*i, get_environment_variable(e, *i));

        result->add_section(variable_info);
    }
    else if (! info_vars.empty())
        Log::get_instance()->message(ll_warning, lc_no_context,
                "Skipping info_vars for '" + stringify(name()) +
                "' because info_pkgs is not usable");

    return result;
}

std::string
PortageRepository::profile_variable(const std::string & s) const
{
    _imp->need_profiles();

    return _imp->profile_ptr->environment_variable(s);
}

PortageRepository::MirrorsIterator
PortageRepository::begin_mirrors(const std::string & s) const
{
    need_mirrors();
    return MirrorsIterator(_imp->mirrors.equal_range(s).first);
}

PortageRepository::MirrorsIterator
PortageRepository::end_mirrors(const std::string & s) const
{
    need_mirrors();
    return MirrorsIterator(_imp->mirrors.equal_range(s).second);
}

std::tr1::shared_ptr<const PortageRepository::VirtualsCollection>
PortageRepository::virtual_packages() const
{
    Context context("When loading virtual packages for repository '" +
            stringify(name()) + "'");

    Log::get_instance()->message(ll_debug, lc_context, "Loading virtual packages for repository '"
            + stringify(name()) + "'");

    _imp->need_profiles();
    need_category_names();

    std::tr1::shared_ptr<VirtualsCollection> result(new VirtualsCollection::Concrete);

    for (PortageRepositoryProfile::VirtualsIterator i(_imp->profile_ptr->begin_virtuals()),
            i_end(_imp->profile_ptr->end_virtuals()) ; i != i_end ; ++i)
        result->insert(RepositoryVirtualsEntry::create()
                .provided_by_spec(i->second)
                .virtual_name(i->first));

    Log::get_instance()->message(ll_debug, lc_context, "Loaded " + stringify(result->size()) +
            " virtual packages for repository '" + stringify(name()) + "'");

    return result;
}

std::tr1::shared_ptr<const VersionMetadata>
PortageRepository::virtual_package_version_metadata(const RepositoryVirtualsEntry & p,
        const VersionSpec & v) const
{
    Context context("When fetching virtual package version metadata for '" + stringify(*p.provided_by_spec)
            + "' version '" + stringify(v) + "':");

    if (! p.provided_by_spec->package_ptr())
        throw ConfigurationError("Virtual provider atom does not specify an unambiguous package");

    std::tr1::shared_ptr<const VersionMetadata> m(version_metadata(*p.provided_by_spec->package_ptr(), v));
    std::tr1::shared_ptr<PortageVirtualVersionMetadata> result(new PortageVirtualVersionMetadata(
                m->slot, PackageDatabaseEntry(*p.provided_by_spec->package_ptr(), v, name())));

    result->eapi = m->eapi;
    result->build_depend_string = "=" + stringify(*p.provided_by_spec->package_ptr()) + "-" + stringify(v);
    result->run_depend_string = "=" + stringify(*p.provided_by_spec->package_ptr()) + "-" + stringify(v);

    return result;

}

std::tr1::shared_ptr<const UseFlagNameCollection>
PortageRepository::do_use_expand_flags() const
{
    _imp->need_profiles();

    std::tr1::shared_ptr<UseFlagNameCollection> result(new UseFlagNameCollection::Concrete);
    for (PortageRepositoryProfile::UseExpandIterator i(_imp->profile_ptr->begin_use_expand()),
            i_end(_imp->profile_ptr->end_use_expand()) ; i != i_end ; ++i)
    {
        std::list<std::string> values;
        WhitespaceTokeniser::get_instance()->tokenise(_imp->profile_ptr->environment_variable(
                    stringify(*i)), std::back_inserter(values));
        for (std::list<std::string>::const_iterator j(values.begin()), j_end(values.end()) ;
                j != j_end ; ++j)
        {
            std::string f(stringify(*i) + "_" + *j), lower_f;
            std::transform(f.begin(), f.end(), std::back_inserter(lower_f), &::tolower);
            result->insert(UseFlagName(lower_f));
        }
    }

    return result;
}

std::tr1::shared_ptr<const UseFlagNameCollection>
PortageRepository::do_use_expand_prefixes() const
{
    _imp->need_profiles();

    std::tr1::shared_ptr<UseFlagNameCollection> result(new UseFlagNameCollection::Concrete);
    for (PortageRepositoryProfile::UseExpandIterator i(_imp->profile_ptr->begin_use_expand()),
            i_end(_imp->profile_ptr->end_use_expand()) ; i != i_end ; ++i)
    {
        std::string lower_i;
        std::transform(i->data().begin(), i->data().end(), std::back_inserter(lower_i), &::tolower);
        result->insert(UseFlagName(lower_i));
    }

    return result;
}

std::tr1::shared_ptr<const UseFlagNameCollection>
PortageRepository::do_use_expand_hidden_prefixes() const
{
    _imp->need_profiles();

    std::tr1::shared_ptr<UseFlagNameCollection> result(new UseFlagNameCollection::Concrete);
    for (PortageRepositoryProfile::UseExpandIterator i(_imp->profile_ptr->begin_use_expand_hidden()),
            i_end(_imp->profile_ptr->end_use_expand_hidden()) ; i != i_end ; ++i)
    {
        std::string lower_i;
        std::transform(i->data().begin(), i->data().end(), std::back_inserter(lower_i), &::tolower);
        result->insert(UseFlagName(lower_i));
    }

    return result;
}

UseFlagName
PortageRepository::do_use_expand_name(const UseFlagName & u) const
{
    for (PortageRepositoryProfile::UseExpandIterator i(_imp->profile_ptr->begin_use_expand()),
            i_end(_imp->profile_ptr->end_use_expand()) ; i != i_end ; ++i)
    {
        std::string lower_i;
        std::transform(i->data().begin(), i->data().end(), std::back_inserter(lower_i), ::tolower);
        if (0 == lower_i.compare(0, lower_i.length(), stringify(u), 0, lower_i.length()))
            return *i;
    }

    throw InternalError(PALUDIS_HERE, "Not a use expand name: '" + stringify(u) + "'");
}

UseFlagName
PortageRepository::do_use_expand_value(const UseFlagName & u) const
{
    for (PortageRepositoryProfile::UseExpandIterator i(_imp->profile_ptr->begin_use_expand()),
            i_end(_imp->profile_ptr->end_use_expand()) ; i != i_end ; ++i)
    {
        std::string lower_i;
        std::transform(i->data().begin(), i->data().end(), std::back_inserter(lower_i), ::tolower);
        if (0 == lower_i.compare(0, lower_i.length(), stringify(u), 0, lower_i.length()))
            return UseFlagName(stringify(u).substr(lower_i.length() + 1));
    }

    throw InternalError(PALUDIS_HERE, "Not a use expand name: '" + stringify(u) + "'");
}

void
PortageRepository::regenerate_cache() const
{
    _imp->names_cache->regenerate_cache();
}

std::tr1::shared_ptr<const CategoryNamePartCollection>
PortageRepository::do_category_names_containing_package(const PackageNamePart & p) const
{
    if (! _imp->names_cache->usable())
        return Repository::do_category_names_containing_package(p);

    std::tr1::shared_ptr<const CategoryNamePartCollection> result(
            _imp->names_cache->category_names_containing_package(p));

    return result ? result : Repository::do_category_names_containing_package(p);
}

PortageRepository::ProfilesIterator
PortageRepository::begin_profiles() const
{
    _imp->need_profiles_desc();
    return ProfilesIterator(_imp->profiles_desc.begin());
}

PortageRepository::ProfilesIterator
PortageRepository::end_profiles() const
{
    _imp->need_profiles_desc();
    return ProfilesIterator(_imp->profiles_desc.end());
}

PortageRepository::ProfilesIterator
PortageRepository::find_profile(const FSEntry & location) const
{
    _imp->need_profiles_desc();
    for (ProfilesDesc::const_iterator i(_imp->profiles_desc.begin()),
            i_end(_imp->profiles_desc.end()) ; i != i_end ; ++i)
        if (i->path == location)
            return ProfilesIterator(i);
    return ProfilesIterator(_imp->profiles_desc.end());
}

void
PortageRepository::set_profile(const ProfilesIterator & iter)
{
    Context context("When setting profile by iterator:");

    _imp->profile_ptr = iter->profile;

    if (_imp->params.environment->package_database()->has_repository_named(RepositoryName("virtuals")))
        _imp->params.environment->package_database()->fetch_repository(
                RepositoryName("virtuals"))->invalidate();
}

void
PortageRepository::set_profile_by_arch(const UseFlagName & arch)
{
    Context context("When setting profile by arch '" + stringify(arch) + "':");

    for (ProfilesIterator p(begin_profiles()), p_end(end_profiles()) ; p != p_end ; ++p)
        if (p->arch == stringify(arch) && p->status == "stable")
        {
            set_profile(p);
            return;
        }

    for (ProfilesIterator p(begin_profiles()), p_end(end_profiles()) ; p != p_end ; ++p)
        if (p->arch == stringify(arch))
        {
            set_profile(p);
            return;
        }

    throw ConfigurationError("Cannot find a profile appropriate for '" + stringify(arch) + "'");
}

std::string
PortageRepository::do_describe_use_flag(const UseFlagName & f,
        const PackageDatabaseEntry * const e) const
{
    if (_imp->use_desc.empty())
        for (std::list<FSEntry>::const_iterator p(_imp->profiles_dir_locations.begin()),
                p_end(_imp->profiles_dir_locations.end()) ; p != p_end ; ++p)
            _imp->use_desc.push_back(std::tr1::shared_ptr<UseDesc>(new UseDesc(*p)));

    std::string result;
    for (std::list<std::tr1::shared_ptr<UseDesc> >::const_iterator i(_imp->use_desc.begin()),
            i_end(_imp->use_desc.end()) ; i != i_end ; ++i)
    {
        std::string new_result((*i)->describe(f, e));
        if (! new_result.empty())
            result = new_result;
    }
    return result;
}

const PortageRepositoryParams &
PortageRepository::params() const
{
    return _imp->params;
}

bool
PortageRepository::is_suitable_destination_for(const PackageDatabaseEntry & e) const
{
    std::string f(_imp->params.environment->package_database()->fetch_repository(e.repository)->format());
    return f == "ebuild";
}

bool
PortageRepository::is_default_destination() const
{
    return false;
}

bool
PortageRepository::want_pre_post_phases() const
{
    return false;
}

void
PortageRepository::merge(const MergeOptions & o)
{
    _imp->entries_ptr->merge(o);
}

int
PortageRepository::perform_hook(const Hook & hook) const
{
    Context context("When performing hook '" + stringify(hook.name()) + "' for repository '"
            + stringify(name()) + "':");

    return 0;
}

