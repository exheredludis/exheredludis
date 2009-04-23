/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2009 Ciaran McCreesh
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

#include <paludis/resolver/decision.hh>
#include <paludis/resolver/serialise-impl.hh>
#include <paludis/util/make_named_values.hh>
#include <sstream>

using namespace paludis;
using namespace paludis::resolver;

std::ostream &
paludis::resolver::operator<< (std::ostream & s, const Decision & d)
{
    std::stringstream ss;

    ss << "Decision(";

    if (d.if_package_id())
        ss << *d.if_package_id();
    else
        ss << "(nothing)";

    if (d.is_best())
        ss << ", is best";
    if (d.is_installed())
        ss << ", is installed";
    if (d.is_nothing())
        ss << ", is nothing";
    if (d.is_same())
        ss << ", is same";
    if (d.is_same_version())
        ss << ", is same version";

    ss << ")";

    s << ss.str();
    return s;
}

void
Decision::serialise(Serialiser & s) const
{
    s.object("Decision")
        .member(SerialiserFlags<serialise::might_be_null>(), "if_package_id", if_package_id())
        .member(SerialiserFlags<>(), "is_best", is_best())
        .member(SerialiserFlags<>(), "is_installed", is_installed())
        .member(SerialiserFlags<>(), "is_nothing", is_nothing())
        .member(SerialiserFlags<>(), "is_same", is_same())
        .member(SerialiserFlags<>(), "is_same_version", is_same_version())
        .member(SerialiserFlags<>(), "is_transient", is_transient())
        ;
}

const std::tr1::shared_ptr<Decision>
Decision::deserialise(Deserialisation & d)
{
    Deserialisator v(d, "Decision");
    return make_shared_ptr(new Decision(make_named_values<Decision>(
                    value_for<n::if_package_id>(v.member<std::tr1::shared_ptr<const PackageID> >("if_package_id")),
                    value_for<n::is_best>(v.member<bool>("is_best")),
                    value_for<n::is_installed>(v.member<bool>("is_installed")),
                    value_for<n::is_nothing>(v.member<bool>("is_nothing")),
                    value_for<n::is_same>(v.member<bool>("is_same")),
                    value_for<n::is_same_version>(v.member<bool>("is_same_version")),
                    value_for<n::is_transient>(v.member<bool>("is_transient"))
                    )));
}
