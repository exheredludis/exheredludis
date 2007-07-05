/* vim: set sw=4 sts=4 et foldmethod=syntax : */

#include "version_info_model.hh"
#include "query_window.hh"
#include "versions_page.hh"
#include "markup.hh"
#include <paludis/util/iterator.hh>
#include <paludis/util/visitor-impl.hh>
#include <paludis/util/set.hh>
#include <paludis/environment.hh>
#include <paludis/package_database.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/query.hh>
#include <paludis/eapi.hh>
#include <paludis/metadata_key.hh>
#include <paludis/dep_spec_pretty_printer.hh>
#include <libwrapiter/libwrapiter_forward_iterator.hh>
#include <libwrapiter/libwrapiter_output_iterator.hh>
#include <list>

using namespace paludis;
using namespace gtkpaludis;

namespace paludis
{
    template<>
    struct Implementation<VersionInfoModel>
    {
        QueryWindow * const query_window;
        VersionsPage * const versions_page;
        VersionInfoModel::Columns columns;

        Implementation(QueryWindow * const m, VersionsPage * const p) :
            query_window(m),
            versions_page(p)
        {
        }
    };
}

namespace
{
    struct PopulateDataItem
    {
        std::string key;
        std::string value_markup;

        PopulateDataItem(const std::string & k, const std::string & m) :
            key(k),
            value_markup(m)
        {
        }
    };
}

namespace gtkpaludis
{
    struct VersionInfoModel::PopulateData
    {
        std::list<PopulateDataItem> items;
    };
}


VersionInfoModel::VersionInfoModel(QueryWindow * const m, VersionsPage * const p) :
    PrivateImplementationPattern<VersionInfoModel>(new Implementation<VersionInfoModel>(m, p)),
    Gtk::TreeStore(_imp->columns)
{
}

VersionInfoModel::~VersionInfoModel()
{
}

VersionInfoModel::Columns::Columns()
{
    add(col_key);
    add(col_value_markup);
}

VersionInfoModel::Columns::~Columns()
{
}

VersionInfoModel::Columns &
VersionInfoModel::columns()
{
    return _imp->columns;
}

void
VersionInfoModel::populate()
{
    _imp->query_window->paludis_thread_action(
            sigc::bind(sigc::mem_fun(this, &VersionInfoModel::populate_in_paludis_thread),
                _imp->versions_page->get_id()), "Populating version information model");
}

void
VersionInfoModel::populate_in_paludis_thread(tr1::shared_ptr<const PackageID> p)
{
    tr1::shared_ptr<PopulateData> data(new PopulateData);

    if (p)
    {
        if (p->eapi()->supported)
        {
            if (p->short_description_key())
                data->items.push_back(PopulateDataItem("Description", markup_escape(p->short_description_key()->value())));

            if (p->homepage_key())
            {
                DepSpecPrettyPrinter homepage_printer(0, false);
                p->homepage_key()->value()->accept(homepage_printer);
                if (! stringify(homepage_printer).empty())
                    data->items.push_back(PopulateDataItem("Homepage", markup_escape(stringify(homepage_printer))));
            }

            if (p->keywords_key())
            {
                std::string km;
                tr1::shared_ptr<const KeywordNameSet> keywords(p->keywords_key()->value());
                for (KeywordNameSet::Iterator k(keywords->begin()), k_end(keywords->end()) ;
                        k != k_end ; ++k)
                {
                    if (! km.empty())
                        km.append(" ");

                    tr1::shared_ptr<KeywordNameSet> kc(new KeywordNameSet);
                    kc->insert(*k);
                    if (_imp->query_window->environment()->accept_keywords(kc, *p))
                        km.append(markup_bold(markup_escape(stringify(*k))));
                    else
                        km.append(markup_italic(markup_escape(stringify(*k))));
                }

                data->items.push_back(PopulateDataItem("Keywords", km));
            }
        }
    }

    _imp->query_window->gui_thread_action(
            sigc::bind(sigc::mem_fun(this, &VersionInfoModel::populate_in_gui_thread), data));
}

void
VersionInfoModel::populate_in_gui_thread(tr1::shared_ptr<const VersionInfoModel::PopulateData> names)
{
    clear();

    for (std::list<PopulateDataItem>::const_iterator i(names->items.begin()), i_end(names->items.end()) ;
            i != i_end ; ++i)
    {
        iterator r(append());
        (*r)[_imp->columns.col_key] = i->key;
        (*r)[_imp->columns.col_value_markup] = i->value_markup;
    }
}


