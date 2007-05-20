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

#include <paludis/repositories/virtuals/installed_virtuals_repository.hh>
#include <paludis/repositories/virtuals/vr_entry.hh>
#include <paludis/util/fast_unique_copy.hh>
#include <paludis/util/fs_entry.hh>
#include <paludis/environment.hh>
#include <paludis/package_database.hh>
#include <paludis/util/collection_concrete.hh>
#include <paludis/hook.hh>
#include <vector>
#include <algorithm>

using namespace paludis;

namespace paludis
{
    template<>
    struct Implementation<InstalledVirtualsRepository>
    {
        const Environment * const env;
        const FSEntry root;

        mutable std::vector<VREntry> entries;
        mutable bool has_entries;

        Implementation(const Environment * const e, const FSEntry & r) :
            env(e),
            root(r),
            has_entries(false)
        {
        }
    };
}

namespace
{
    struct MakeSafe
    {
        char operator() (const char & c) const
        {
            static const std::string allow(
                    "abcdefghijklmnopqrstuvwxyz"
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                    "0123456789_-");

            if (std::string::npos == allow.find(c))
                return '-';
            else
                return c;
        }
    };

    RepositoryName
    make_name(const FSEntry & r)
    {
        if (FSEntry("/") == r)
            return RepositoryName("installed_virtuals");
        else
        {
            std::string n("installed_virtuals-" + stringify(r)), result;
            std::transform(n.begin(), n.end(), std::back_inserter(result), MakeSafe());
            return RepositoryName(result);
        }
    }
}

InstalledVirtualsRepository::InstalledVirtualsRepository(const Environment * const env,
        const FSEntry & r) :
    Repository(RepositoryName(make_name(r)), RepositoryCapabilities::create()
            .installable_interface(0)
            .mask_interface(this)
            .installed_interface(this)
            .use_interface(0)
            .sets_interface(0)
            .syncable_interface(0)
            .uninstallable_interface(0)
            .mirrors_interface(0)
            .environment_variable_interface(0)
            .world_interface(0)
            .provides_interface(0)
            .virtuals_interface(0)
            .contents_interface(0)
            .config_interface(0)
            .destination_interface(0)
            .licenses_interface(0)
            .portage_interface(0)
            .pretend_interface(0)
            .hook_interface(this),
            "installed_virtuals"),
    PrivateImplementationPattern<InstalledVirtualsRepository>(
            new Implementation<InstalledVirtualsRepository>(env, r))
{
    tr1::shared_ptr<RepositoryInfoSection> config_info(new RepositoryInfoSection("Configuration information"));
    config_info->add_kv("format", "installed_virtuals");
    _info->add_section(config_info);
}

InstalledVirtualsRepository::~InstalledVirtualsRepository()
{
}

void
InstalledVirtualsRepository::need_entries() const
{
    if (_imp->has_entries)
        return;

    /* Populate our _imp->entries. We need to iterate over each repository in
     * our env's package database, see if it has a provides interface, and if it
     * does create an entry for each provided package. */
    for (PackageDatabase::RepositoryIterator r(_imp->env->package_database()->begin_repositories()),
            r_end(_imp->env->package_database()->end_repositories()) ; r != r_end ; ++r)
    {
        if (! (*r)->provides_interface)
            continue;

        tr1::shared_ptr<const RepositoryProvidesInterface::ProvidesCollection> pp(
                (*r)->provides_interface->provided_packages());

        for (RepositoryProvidesInterface::ProvidesCollection::Iterator p(
                    pp->begin()), p_end(pp->end()) ; p != p_end ; ++p)
        {
            _imp->entries.push_back(VREntry::create()
                    .virtual_name(p->virtual_name)
                    .version(p->version)
                    .provided_by_name(p->provided_by_name)
                    .provided_by_repository((*r)->name()));
        }
    }

    /* Our entries must be sorted, for fast lookups. But it's a bit trickier
     * than a straight operator< comparison, because the same virtual version
     * can be provided by more than one thing. For ease of coding elsewhere, we
     * want the 'best' entry to come earlier (sort lower). We define best as
     * being from the most important repository, and failing that from the
     * greater provided name (the second part purely for consistency between
     * invokations). */
    std::sort(_imp->entries.begin(), _imp->entries.end(),
            EntriesComparator(_imp->env->package_database()));

    _imp->has_entries = true;
}

tr1::shared_ptr<Repository>
InstalledVirtualsRepository::make_installed_virtuals_repository(
        Environment * const env,
        tr1::shared_ptr<const AssociativeCollection<std::string, std::string> > k)
{
    std::string root_str;

    if (k && (k->end() != k->find("root")))
        root_str = k->find("root")->second;

    if (root_str.empty())
        throw ConfigurationError("No root specified for InstalledVirtualsRepository");

    return tr1::shared_ptr<Repository>(new InstalledVirtualsRepository(env, FSEntry(root_str)));
}

bool
InstalledVirtualsRepository::do_query_repository_masks(const QualifiedPackageName &,
        const VersionSpec &) const
{
    return false;
}

bool
InstalledVirtualsRepository::do_query_profile_masks(const QualifiedPackageName &,
        const VersionSpec &) const
{
    return false;
}

tr1::shared_ptr<const VersionMetadata>
InstalledVirtualsRepository::do_version_metadata(
        const QualifiedPackageName & q,
        const VersionSpec & v) const
{
    need_entries();

    std::pair<std::vector<VREntry>::const_iterator, std::vector<VREntry>::const_iterator> p(
            std::equal_range(_imp->entries.begin(), _imp->entries.end(),
                VREntry(q, v, QualifiedPackageName("dummy/package"), RepositoryName("dummy_repository"))));

    if (p.first == p.second)
        throw NoSuchPackageError(stringify(PackageDatabaseEntry(q, v, name())));

    return _imp->env->package_database()->fetch_repository(
            p.first->provided_by_repository)->provides_interface->provided_package_version_metadata(
                RepositoryProvidesEntry::create()
                .virtual_name(p.first->virtual_name)
                .version(p.first->version)
                .provided_by_name(p.first->provided_by_name));
}

bool
InstalledVirtualsRepository::do_has_version(const QualifiedPackageName & q,
        const VersionSpec & v) const
{
    if (q.category.data() != "virtual")
        return false;

    need_entries();

    std::pair<std::vector<VREntry>::const_iterator, std::vector<VREntry>::const_iterator> p(
            std::equal_range(_imp->entries.begin(), _imp->entries.end(),
                VREntry(q, v, QualifiedPackageName("dummy/package"), RepositoryName("dummy_repository"))));

    return p.first != p.second;
}

tr1::shared_ptr<const VersionSpecCollection>
InstalledVirtualsRepository::do_version_specs(const QualifiedPackageName & q) const
{
    if (q.category.data() != "virtual")
        return tr1::shared_ptr<VersionSpecCollection>(new VersionSpecCollection::Concrete);

    need_entries();

    std::pair<std::vector<VREntry>::const_iterator, std::vector<VREntry>::const_iterator> p(
            std::equal_range(_imp->entries.begin(), _imp->entries.end(),
                VREntry(q, VersionSpec("0"), QualifiedPackageName("dummy/package"),
                    RepositoryName("dummy_repository")), EntriesNameComparator()));

    tr1::shared_ptr<VersionSpecCollection> result(new VersionSpecCollection::Concrete);
    for ( ; p.first != p.second ; ++p.first)
        result->insert(p.first->version);

    return result;
}

tr1::shared_ptr<const QualifiedPackageNameCollection>
InstalledVirtualsRepository::do_package_names(const CategoryNamePart & c) const
{
    if (c.data() != "virtual")
        return tr1::shared_ptr<QualifiedPackageNameCollection>(new QualifiedPackageNameCollection::Concrete);

    need_entries();

    std::pair<std::vector<VREntry>::const_iterator, std::vector<VREntry>::const_iterator> p(
            std::equal_range(_imp->entries.begin(), _imp->entries.end(),
                VREntry(c + PackageNamePart("dummy"), VersionSpec("0"), QualifiedPackageName("dummy/package"),
                    RepositoryName("dummy_repository")), EntriesCategoryComparator()));


    tr1::shared_ptr<QualifiedPackageNameCollection> result(new QualifiedPackageNameCollection::Concrete);
    fast_unique_copy(p.first, p.second,
            transform_inserter(result->inserter(), EntriesNameExtractor()),
            EntriesNameComparator());

    return result;
}

tr1::shared_ptr<const CategoryNamePartCollection>
InstalledVirtualsRepository::do_category_names() const
{
    tr1::shared_ptr<CategoryNamePartCollection> result(new CategoryNamePartCollection::Concrete);
    result->insert(CategoryNamePart("virtual"));

    return result;
}

bool
InstalledVirtualsRepository::do_has_package_named(const QualifiedPackageName & q) const
{
    if (q.category.data() != "virtual")
        return false;

    need_entries();

    std::pair<std::vector<VREntry>::const_iterator, std::vector<VREntry>::const_iterator> p(
            std::equal_range(_imp->entries.begin(), _imp->entries.end(),
                VREntry(q, VersionSpec("0"), QualifiedPackageName("dummy/package"),
                    RepositoryName("dummy_repository")), EntriesNameComparator()));

    return p.first != p.second;
}

bool
InstalledVirtualsRepository::do_has_category_named(const CategoryNamePart & c) const
{
    if (c.data() != "virtual")
        return false;

    need_entries();

    std::pair<std::vector<VREntry>::const_iterator, std::vector<VREntry>::const_iterator> p(
            std::equal_range(_imp->entries.begin(), _imp->entries.end(),
                VREntry(c + PackageNamePart("dummy"), VersionSpec("0"), QualifiedPackageName("dummy/package"),
                    RepositoryName("dummy_repository")), EntriesCategoryComparator()));

    return p.first != p.second;
}

void
InstalledVirtualsRepository::invalidate()
{
    _imp.reset(new Implementation<InstalledVirtualsRepository>(_imp->env, _imp->root));
}

FSEntry
InstalledVirtualsRepository::root() const
{
    return _imp->root;
}

HookResult
InstalledVirtualsRepository::perform_hook(const Hook & hook) const
{
    Context context("When performing hook '" + stringify(hook.name()) + "' for repository '"
            + stringify(name()) + "':");

    return HookResult(0, "");
}

