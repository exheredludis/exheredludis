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

#include "repository_name_cache.hh"
#include <paludis/repository.hh>
#include <paludis/hashed_containers.hh>
#include <paludis/util/fs_entry.hh>
#include <paludis/util/collection_concrete.hh>
#include <paludis/util/log.hh>
#include <paludis/util/dir_iterator.hh>
#include <list>
#include <fstream>

using namespace paludis;

namespace paludis
{
    typedef MakeHashedMap<PackageNamePart, std::list<CategoryNamePart> >::Type NameCacheMap;

    template<>
    struct Implementation<RepositoryNameCache>
    {
        mutable FSEntry location;
        const Repository * const repo;

        mutable NameCacheMap name_cache_map;
        mutable bool checked_name_cache_map;

        Implementation(const FSEntry & l, const Repository * const r) :
            location(l == FSEntry("/var/empty") ? l : l / stringify(r->name())),
            repo(r),
            checked_name_cache_map(false)
        {
        }
    };
}

RepositoryNameCache::RepositoryNameCache(
        const FSEntry & location,
        const Repository * const repo) :
    PrivateImplementationPattern<RepositoryNameCache>(new Implementation<RepositoryNameCache>(
                location, repo)),
    _usable(_imp->location != FSEntry("/var/empty"))
{
}

RepositoryNameCache::~RepositoryNameCache()
{
}

tr1::shared_ptr<const CategoryNamePartCollection>
RepositoryNameCache::category_names_containing_package(const PackageNamePart & p) const
{
    if (! usable())
        return tr1::shared_ptr<const CategoryNamePartCollection>();

    Context context("When using name cache at '" + stringify(_imp->location) + "':");

    tr1::shared_ptr<CategoryNamePartCollection> result(new CategoryNamePartCollection::Concrete);
    NameCacheMap::iterator r(_imp->name_cache_map.find(p));

    _imp->location = FSEntry(stringify(_imp->location));

    if (_imp->name_cache_map.end() == r)
    {
        r = _imp->name_cache_map.insert(std::make_pair(p, std::list<CategoryNamePart>())).first;

        if (! _imp->checked_name_cache_map)
        {
            if (_imp->location.is_directory() && (_imp->location / "_VERSION_").exists())
            {
                std::ifstream vvf(stringify(_imp->location / "_VERSION_").c_str());
                std::string line;
                std::getline(vvf, line);
                if (line != "paludis-2")
                {
                    Log::get_instance()->message(ll_warning, lc_context, "Names cache for '" + stringify(_imp->repo->name())
                            + "' has version string '" + line + "', which is not supported. Was it generated using "
                            "a different Paludis version?");
                    _usable = false;
                    return tr1::shared_ptr<const CategoryNamePartCollection>();
                }
                std::getline(vvf, line);
                if (line != stringify(_imp->repo->name()))
                {
                    Log::get_instance()->message(ll_warning, lc_context, "Names cache for '" + stringify(_imp->repo->name())
                            + "' was generated for repository '" + line + "', so it cannot be used. You must not "
                            "have multiple name caches at the same location.");
                    _usable = false;
                    return tr1::shared_ptr<const CategoryNamePartCollection>();
                }
                _imp->checked_name_cache_map = true;
            }
            else if ((_imp->location.dirname() / "_VERSION_").exists())
            {
                Log::get_instance()->message(ll_warning, lc_context, "Names cache for '" + stringify(_imp->repo->name())
                        + "' does not exist at '" + stringify(_imp->location) + "', but a names cache exists at '" +
                        stringify(_imp->location.dirname()) + "'. This was probably generated by a Paludis version "
                        "older than 0.18.0. The names cache now automatically appends the repository name to the "
                        "directory. You probably want to manually remove '" + stringify(_imp->location.dirname()) +
                        "' and then regenerate the cache.");
                _usable = false;
                return tr1::shared_ptr<const CategoryNamePartCollection>();
            }
            else
            {
                Log::get_instance()->message(ll_warning, lc_context, "Names cache for '" + stringify(_imp->repo->name())
                        + "' has no version information, so cannot be used. Either it was generated using "
                        "an older Paludis version or it has not yet been generated.");
                _usable = false;
                return tr1::shared_ptr<const CategoryNamePartCollection>();
            }
        }

        FSEntry ff(_imp->location / stringify(p));
        if (ff.exists())
        {
            std::ifstream f(stringify(ff).c_str());
            if (! f)
                Log::get_instance()->message(ll_warning, lc_context, "Cannot read '" + stringify(ff) + "'");
            std::string line;
            while (std::getline(f, line))
                r->second.push_back(CategoryNamePart(line));
        }
    }

    std::copy(r->second.begin(), r->second.end(), result->inserter());

    return result;
}

void
RepositoryNameCache::regenerate_cache() const
{
    if (_imp->location == FSEntry("/var/empty"))
        return;

    Context context("When generating repository names cache at '"
            + stringify(_imp->location) + "':");

    if (_imp->location.is_directory())
        for (DirIterator i(_imp->location, true), i_end ; i != i_end ; ++i)
            FSEntry(*i).unlink();

    _imp->location.dirname().mkdir();
    if (_imp->location.exists() && ! _imp->location.is_directory())
        FSEntry(_imp->location).unlink();
    FSEntry(_imp->location).mkdir();

    MakeHashedMap<std::string, std::string>::Type m;

    tr1::shared_ptr<const CategoryNamePartCollection> cats(_imp->repo->category_names());
    for (CategoryNamePartCollection::Iterator c(cats->begin()), c_end(cats->end()) ;
            c != c_end ; ++c)
    {
        tr1::shared_ptr<const QualifiedPackageNameCollection> pkgs(_imp->repo->package_names(*c));
        for (QualifiedPackageNameCollection::Iterator p(pkgs->begin()), p_end(pkgs->end()) ;
                p != p_end ; ++p)
            m[stringify(p->package)].append(stringify(*c) + "\n");
    }

    for (MakeHashedMap<std::string, std::string>::Type::const_iterator e(m.begin()), e_end(m.end()) ;
            e != e_end ; ++e)
    {
        std::ofstream f(stringify(_imp->location / stringify(e->first)).c_str());
        if (! f)
        {
            Log::get_instance()->message(ll_warning, lc_context, "Cannot write to '"
                    + stringify(_imp->location) + "'");
            continue;
        }
        f << e->second;
    }

    std::ofstream f(stringify(_imp->location / "_VERSION_").c_str());
    if (f)
    {
        f << "paludis-2" << std::endl;
        f << _imp->repo->name() << std::endl;
    }
    else
        Log::get_instance()->message(ll_warning, lc_context, "Cannot write to '"
                + stringify(_imp->location) + "'");
}

