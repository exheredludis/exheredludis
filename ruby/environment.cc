/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007 Ciaran McCreesh <ciaranm@ciaranm.org>
 * Copyright (c) 2007 Richard Brown <rbrown@gentoo.org>
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

#include <paludis_ruby.hh>
#include <paludis/environments/paludis/paludis_environment.hh>
#include <paludis/environments/no_config/no_config_environment.hh>
#include <paludis/environments/adapted/adapted_environment.hh>
#include <paludis/environment_maker.hh>
#include <libwrapiter/libwrapiter_forward_iterator.hh>
#include <libwrapiter/libwrapiter_output_iterator.hh>
#include <paludis/util/set.hh>
#include <ruby.h>

using namespace paludis;
using namespace paludis::ruby;

#define RUBY_FUNC_CAST(x) reinterpret_cast<VALUE (*)(...)>(x)

namespace
{
    static VALUE c_environment;
    static VALUE c_paludis_environment;
    static VALUE c_no_config_environment;
    static VALUE c_adapted_environment;
    static VALUE c_environment_maker;

    tr1::shared_ptr<AdaptedEnvironment>
    value_to_adapted_environment(VALUE v)
    {
        if (rb_obj_is_kind_of(v, c_adapted_environment))
        {
            tr1::shared_ptr<AdaptedEnvironment> * v_ptr;
            Data_Get_Struct(v, tr1::shared_ptr<AdaptedEnvironment>, v_ptr);
            return *v_ptr;
        }
        else
        {
            rb_raise(rb_eTypeError, "Can't convert %s into AdaptedEnvironment", rb_obj_classname(v));
        }
    }

    /*
     * call-seq:
     *     query_use(use_flag, package_id) -> true or false
     *
     * Does the user want the specified USE flag set for a PackageID.
     */

    VALUE
    environment_query_use(VALUE self, VALUE flag, VALUE pid)
    {
        try
        {
            return value_to_environment(self)->query_use(UseFlagName(StringValuePtr(flag)), *(value_to_package_id(pid))) ? Qtrue : Qfalse;
        }
        catch (const std::exception & e)
        {
            exception_to_ruby_exception(e);
        }
    }

    /*
     * call-seq:
     *     package_database -> PackageDatabase
     *
     * Fetch our PackageDatabase.
     */
    VALUE
    environment_package_database(VALUE self)
    {
        try
        {
            return package_database_to_value(value_to_environment(self)->package_database());
        }
        catch (const std::exception & e)
        {
            exception_to_ruby_exception(e);
        }
    }

#if CIARANM_REMOVED_THIS
    /*
     * call-seq:
     *     set(set_name) -> DepSpec
     *
     * Fetch a named package set as a DepSpec.
     */
    VALUE
    environment_set(VALUE self, VALUE set_name)
    {
        try
        {
            return dep_spec_to_value(value_to_environment(self)->set(SetName(StringValuePtr(set_name))));
        }
        catch (const std::exception & e)
        {
            exception_to_ruby_exception(e);
        }
    }
#endif

    /*
     * call-seq:
     *     accept_license(license, package_id) -> true of false
     *
     * Do we accept a particular license for a particular package?
     */

    VALUE
    environment_accept_license(VALUE self, VALUE license, VALUE p)
    {
        try
        {
            return value_to_environment(self)->accept_license(std::string(StringValuePtr(license)), *(value_to_package_id(p))) ? Qtrue : Qfalse;
        }
        catch (const std::exception & e)
        {
            exception_to_ruby_exception(e);
        }
    }

    /*
     * call-seq:
     *     accept_keywords(keywords, package_id) -> true or false
     *
     * Do we accept any of the specified keywords for a particular package?
     */

    VALUE
    environment_accept_keywords(VALUE self, VALUE keywords, VALUE p)
    {
        try
        {
            tr1::shared_ptr<KeywordNameSet> knc (new KeywordNameSet);
            long len = NUM2LONG(rb_funcall(keywords,rb_intern("length"),0));
            for (long i = 0; i < len; i++)
            {
                // Stupid macros won't let me do it on one line.
                VALUE kw = rb_ary_entry(keywords, i);
                knc->insert(KeywordName(StringValuePtr(kw)));
            }
            return value_to_environment(self)->accept_keywords(knc, *value_to_package_id(p)) ? Qtrue : Qfalse;
        }
        catch (const std::exception & e)
        {
            exception_to_ruby_exception(e);
        }
    }

    /*
     * call-seq:
     *     root -> String
     *
     * Default root location, default is /.
     */
    VALUE
    environment_root(VALUE self)
    {
        return rb_str_new2(stringify(value_to_environment(self)->root()).c_str());
    }

    /*
     * call-seq:
     *     default_destinations -> Array
     *
     * Default: All repositories that provide RepositoryDestinationInterface and mark themselves
     * as a default destination.
     */
    VALUE
    environment_default_destinations(VALUE self)
    {
        tr1::shared_ptr<const DestinationsSet> dc (value_to_environment(self)->default_destinations());
        VALUE result(rb_ary_new());
        for (DestinationsSet::ConstIterator i(dc->begin()), i_end(dc->end()) ; i != i_end ; ++i)
            rb_ary_push(result, repository_to_value(*i));

        return result;

    }

    tr1::shared_ptr<PaludisEnvironment>
    value_to_paludis_environment(VALUE v)
    {
        if (rb_obj_is_kind_of(v, c_paludis_environment))
        {
            tr1::shared_ptr<PaludisEnvironment> * v_ptr;
            Data_Get_Struct(v, tr1::shared_ptr<PaludisEnvironment>, v_ptr);
            return *v_ptr;
        }
        else
        {
            rb_raise(rb_eTypeError, "Can't convert %s into PaludisEnvironment", rb_obj_classname(v));
        }
    }

    VALUE
    paludis_environment_init(int, VALUE*, VALUE self)
    {
        return self;
    }

    /*
     * call-seq:
     *     PaludisEnvironment.new -> PaludisEnvironment
     *     PaludisEnvironment.new(config_suffix) -> PaludisEnvironment
     *
     * Create a new PaludisEnvironment, with the specified config suffix if any, otherwise the empty suffix.
     */
    VALUE
    paludis_environment_new(int argc, VALUE* argv, VALUE self)
    {
        try
        {
            std::string config_suffix;
            if (1 == argc)
                config_suffix = StringValuePtr(argv[0]);
            else if (0 != argc)
                rb_raise(rb_eArgError, "PaludisEnvironment.new expects one or zero arguments, but got %d", argc);

            tr1::shared_ptr<PaludisEnvironment> * e = new tr1::shared_ptr<PaludisEnvironment>(new PaludisEnvironment(config_suffix));
            VALUE tdata(Data_Wrap_Struct(self, 0, &Common<tr1::shared_ptr<PaludisEnvironment> >::free, e));
            rb_obj_call_init(tdata, argc, argv);
            return tdata;
        }
        catch (const std::exception & e)
        {
            exception_to_ruby_exception(e);
        }
    }

    /*
     * call-seq:
     *     config_dir -> String
     *
     * Configuration directory used by this PaludisEnvironment.
     */
    VALUE
    paludis_environment_config_dir(VALUE self)
    {
        return rb_str_new2(value_to_paludis_environment(self)->config_dir().c_str());
    }

    VALUE
    no_config_environment_init(int, VALUE*, VALUE self)
    {
        return self;
    }

    /*
     * call-seq:
     *     NoConfigEnvironment.new(environment_dir) -> NoConfigEnvironment
     *     NoConfigEnvironment.new(environment_dir, write_cache_dir) -> NoConfigEnvironment
     *     NoConfigEnvironment.new(environment_dir, write_cache_dir, master_repository_dir) -> NoConfigEnvironment
     *
     * Create a new NoConfigEnvironment from the specified directory. A write cache and master repository 
     * may also be specified.
     */
    VALUE
    no_config_environment_new(int argc, VALUE* argv, VALUE self)
    {
        try
        {
            std::string write_cache, master_repository_dir;
            if (1 == argc)
            {
                write_cache = "/var/empty/";
                master_repository_dir = "/var/empty/";
            }
            else if (2 == argc)
            {
                write_cache = StringValuePtr(argv[1]);
                master_repository_dir = "/var/empty/";
            }
            else if (3 == argc)
            {
                write_cache = StringValuePtr(argv[1]);
                master_repository_dir = StringValuePtr(argv[2]);
            }
            else
                rb_raise(rb_eArgError, "NoConfigEnvironment.new expects one to three arguments, but got %d", argc);

            std::string path;
            if (rb_obj_is_kind_of(argv[0], rb_cDir))
            {
                VALUE v = rb_funcall(argv[0], rb_intern("path"), 0);
                path = StringValuePtr(v);
            }
            else
                path = StringValuePtr(argv[0]);

            tr1::shared_ptr<NoConfigEnvironment> * e = new tr1::shared_ptr<NoConfigEnvironment>(new
                    NoConfigEnvironment(no_config_environment::Params::create()
                        .repository_dir(FSEntry(path))
                        .write_cache(write_cache)
                        .accept_unstable(false)
                        .disable_metadata_cache(false)
                        .repository_type(no_config_environment::ncer_auto)
                        .extra_params(tr1::shared_ptr<Map<std::string, std::string> >())
                        .master_repository_dir(FSEntry(master_repository_dir))));
            VALUE tdata(Data_Wrap_Struct(self, 0, &Common<tr1::shared_ptr<NoConfigEnvironment> >::free, e));
            rb_obj_call_init(tdata, argc, argv);
            return tdata;
        }
        catch (const std::exception & e)
        {
            exception_to_ruby_exception(e);
        }
    }

    /*
     * call-seq:
     *     main_repository -> Repository
     *
     * Return the main Repository in this environment
     */
    VALUE
    no_config_environment_main_repository(VALUE self)
    {
        try
        {
            return repository_to_value(value_to_no_config_environment(self)->main_repository());
        }
        catch (const std::exception & e)
        {
            exception_to_ruby_exception(e);
        }
    }

    /*
     * call-seq:
     *     master_repository -> PortageRepository
     *
     * Return the master repository in this environment
     */
    VALUE
    no_config_environment_master_repository(VALUE self)
    {
        try
        {
            return repository_to_value(value_to_no_config_environment(self)->master_repository());
        }
        catch (const std::exception & e)
        {
            exception_to_ruby_exception(e);
        }
    }

    /*
     * call-seq:
     *     accept_unstable=(true or false)
     *
     * Should we accept unstable keywords?
     */
    VALUE
    no_config_environment_set_accept_unstable(VALUE self, VALUE unstable)
    {
        value_to_no_config_environment(self)->set_accept_unstable(!(Qfalse == unstable || Qnil == unstable));
        return Qnil;
    }

    VALUE
    adapted_environment_init(int, VALUE *, VALUE self)
    {
        return self;
    }

    /*
     * call-seq:
     *     AdaptedEnvironment.new(environment) -> AdaptedEnvironment
     *
     * Return a new AdaptedEnvironment based on the given Environment.
     */
    VALUE
    adapted_environment_new(int argc, VALUE * argv, VALUE self)
    {
        if (1 != argc)
            rb_raise(rb_eArgError, "AdaptedEnvironment.new expects one argument, but got %d", argc);
        try
        {
            tr1::shared_ptr<AdaptedEnvironment> * e = new tr1::shared_ptr<AdaptedEnvironment>(new AdaptedEnvironment(value_to_environment(argv[0])));
            VALUE tdata(Data_Wrap_Struct(self, 0, &Common<tr1::shared_ptr<AdaptedEnvironment> >::free, e));
            rb_obj_call_init(tdata, argc, argv);
            return tdata;
        }
        catch (const std::exception & e)
        {
            exception_to_ruby_exception(e);
        }
    }

    /*
     * call-seq:
     *     adapt_use(package_dep_spec, use_flag_name, use_flag_state) -> Nil
     *
     * Set the state of a USE flag for the given PackageDepSpec, enable == true, disabled == false.
     */
    VALUE
    adapted_environment_adapt_use(VALUE self, VALUE package_dep_spec, VALUE use_flag_name, VALUE use_flag_state)
    {
        try
        {
            UseFlagState ufs;
            if (use_flag_state == Qtrue)
                ufs = use_enabled;
            else if (use_flag_state == Qfalse)
                ufs = use_disabled;
            else
                rb_raise(rb_eTypeError, "Can't convert %s into UseFlagState", rb_obj_classname(use_flag_state));

            value_to_adapted_environment(self)->adapt_use(value_to_package_dep_spec(package_dep_spec),
                                                            UseFlagName(StringValuePtr(use_flag_name)),
                                                            ufs);
            return Qnil;
        }
        catch (const std::exception & e)
        {
            exception_to_ruby_exception(e);
        }
    }

    /*
     * call-seq:
     *     clear_adaptions -> Nil
     *
     *  Clear all adaptions from this Environment.
     */
    VALUE
    adapted_environment_clear_adaptions(VALUE self)
    {
        value_to_adapted_environment(self)->clear_adaptions();
        return Qnil;
    }


    /*
     * call-seq:
     *     make_from_spec(spec) -> Environment
     *
     * Create an environment from the given spec.
     * A spec consisits of <b>class:suffix</b> both of which may be omitted. <b>class</b> is the environment class,
     * e.g. paludis or portage, <b>suffix</b> is the configuration directory suffix.
     *
     */
    VALUE
    environment_maker_make_from_spec(VALUE, VALUE spec)
    {
        try
        {
            tr1::shared_ptr<Environment> * e = new tr1::shared_ptr<Environment>(EnvironmentMaker::get_instance()->make_from_spec(
                        StringValuePtr(spec)));

            VALUE tdata(Data_Wrap_Struct(c_environment, 0, &Common<tr1::shared_ptr<Environment> >::free, e));
            return tdata;
        }
        catch (const std::exception & e)
        {
            exception_to_ruby_exception(e);
        }
    }

    void do_register_environment()
    {
        rb_require("singleton");

        /*
         * Document-class: Paludis::Environment
         *
         * Represents a working environment, which contains an available packages database and provides 
         * various methods for querying package visibility and options.
         */
        c_environment = environment_class();
        rb_funcall(c_environment, rb_intern("private_class_method"), 1, rb_str_new2("new"));
        rb_define_method(c_environment, "query_use", RUBY_FUNC_CAST(&environment_query_use), 2);
        rb_define_method(c_environment, "package_database", RUBY_FUNC_CAST(&environment_package_database), 0);
#if CIARANM_REMOVED_THIS
        rb_define_method(c_environment, "set", RUBY_FUNC_CAST(&environment_set), 1);
#endif
        rb_define_method(c_environment, "root", RUBY_FUNC_CAST(&environment_root), 0);
        rb_define_method(c_environment, "default_destinations", RUBY_FUNC_CAST(&environment_default_destinations), 0);
        rb_define_method(c_environment, "accept_license", RUBY_FUNC_CAST(&environment_accept_license), 2);
        rb_define_method(c_environment, "accept_keywords", RUBY_FUNC_CAST(&environment_accept_keywords), 2);

        /*
         * Document-class: Paludis::PaludisEnvironment
         *
         * An Environment that corresponds to the normal operating evironment.
         */
        c_paludis_environment = rb_define_class_under(paludis_module(), "PaludisEnvironment", c_environment);
        rb_define_singleton_method(c_paludis_environment, "new", RUBY_FUNC_CAST(&paludis_environment_new), -1);
        rb_define_method(c_paludis_environment, "initialize", RUBY_FUNC_CAST(&paludis_environment_init), -1);
        rb_define_method(c_paludis_environment, "config_dir", RUBY_FUNC_CAST(&paludis_environment_config_dir), 0);

        /*
         * Document-class: Paludis::NoConfigEnvironment
         *
         * An environment that uses a single repository, with no user configuration.
         */
        c_no_config_environment = no_config_environment_class();
        rb_define_singleton_method(c_no_config_environment, "new", RUBY_FUNC_CAST(&no_config_environment_new), -1);
        rb_define_method(c_no_config_environment, "initialize", RUBY_FUNC_CAST(&no_config_environment_init), -1);
        rb_define_method(c_no_config_environment, "main_repository", RUBY_FUNC_CAST(&no_config_environment_main_repository), 0);
        rb_define_method(c_no_config_environment, "master_repository", RUBY_FUNC_CAST(&no_config_environment_master_repository), 0);
        rb_define_method(c_no_config_environment, "accept_unstable=", RUBY_FUNC_CAST(&no_config_environment_set_accept_unstable), 1);

        /*
         * Document-class: Paludis::AdaptedEnvironment
         *
         * An Environment that allows you to change aspects of an
         * existing Environment, e.g. the state of a USE flag for a
         * package.
         */

        c_adapted_environment = rb_define_class_under(paludis_module(), "AdaptedEnvironment", c_environment);
        rb_define_singleton_method(c_adapted_environment, "new", RUBY_FUNC_CAST(&adapted_environment_new), -1);
        rb_define_method(c_adapted_environment, "initialize", RUBY_FUNC_CAST(&adapted_environment_init), -1);
        rb_define_method(c_adapted_environment, "adapt_use", RUBY_FUNC_CAST(&adapted_environment_adapt_use), 3);
        rb_define_method(c_adapted_environment, "clear_adaptions", RUBY_FUNC_CAST(&adapted_environment_clear_adaptions), 0);

        /*
         * Document-class: Paludis::EnvironmentMaker
         *
         * A class that holds methods to create environments.
         *
         * To access the default environment use make_from_spec()
         */
        c_environment_maker = rb_define_class_under(paludis_module(), "EnvironmentMaker", rb_cObject);
        rb_funcall(rb_const_get(rb_cObject, rb_intern("Singleton")), rb_intern("included"), 1, c_environment_maker);
        rb_define_method(c_environment_maker, "make_from_spec", RUBY_FUNC_CAST(&environment_maker_make_from_spec), 1);
    }
}

tr1::shared_ptr<Environment>
paludis::ruby::value_to_environment(VALUE v)
{
    if (rb_obj_is_kind_of(v, c_environment))
    {
        tr1::shared_ptr<Environment> * v_ptr;
        Data_Get_Struct(v, tr1::shared_ptr<Environment>, v_ptr);
        return *v_ptr;
    }
    else
    {
        rb_raise(rb_eTypeError, "Can't convert %s into Environment", rb_obj_classname(v));
    }
}

tr1::shared_ptr<NoConfigEnvironment>
paludis::ruby::value_to_no_config_environment(VALUE v)
{
    if (rb_obj_is_kind_of(v, c_no_config_environment))
    {
        tr1::shared_ptr<NoConfigEnvironment> * v_ptr;
        Data_Get_Struct(v, tr1::shared_ptr<NoConfigEnvironment>, v_ptr);
        return *v_ptr;
    }
    else
    {
        rb_raise(rb_eTypeError, "Can't convert %s into NoConfigEnvironment", rb_obj_classname(v));
    }
}

RegisterRubyClass::Register paludis_ruby_register_environment PALUDIS_ATTRIBUTE((used))
    (&do_register_environment);

