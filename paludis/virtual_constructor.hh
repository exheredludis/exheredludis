/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006 Ciaran McCreesh <ciaranm@gentoo.org>
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

#ifndef PALUDIS_GUARD_PALUDIS_VIRTUAL_CONSTRUCTOR_HH
#define PALUDIS_GUARD_PALUDIS_VIRTUAL_CONSTRUCTOR_HH 1

#include <paludis/exception.hh>
#include <paludis/instantiation_policy.hh>
#include <paludis/stringify.hh>

#include <algorithm>
#include <vector>

/** \file
 * Declarations for VirtualConstructor and related classes.
 *
 * \ingroup VirtualConstructor
 */

namespace paludis
{
    /**
     * Behaviour policy classes for what to do if an appropriate constructor
     * cannot be found for a VirtualConstructor::find_maker call.
     *
     * \ingroup VirtualConstructor
     */
    namespace virtual_constructor_not_found
    {
        /**
         * Throw an exception of type ExceptionType_, which should have a
         * constructor that takes a single parameter of KeyType_.
         *
         * \ingroup VirtualConstructor
         */
        template <typename ExceptionType_>
        struct ThrowException
        {
            /**
             * Internal use: provide handle_not_found.
             *
             * \ingroup VirtualConstructor
             */
            template <typename KeyType_, typename ValueType_>
            struct Parent
            {
                /**
                 * Internal use: called when we cannot find a key.
                 */
                ValueType_ handle_not_found(const KeyType_ & k) const
                {
                    throw ExceptionType_(k);
                }
            };
        };
    }

    /**
     * For internal use by VirtualConstructor.
     *
     * \ingroup VirtualConstructor
     */
    namespace virtual_constructor_internals
    {
        /**
         * Comparator class for our entries.
         *
         * \ingroup VirtualConstructor
         */
        template <typename First_, typename Second_>
        struct ComparePairByFirst
        {
            /**
             * Compare, with the entry on the LHS.
             */
            bool operator() (const std::pair<First_, Second_> & a, const First_ & b) const
            {
                return a.first < b;
            }

            /**
             * Compare, with the entry on the RHS.
             */
            bool operator() (const First_ & a, const std::pair<First_, Second_> & b) const
            {
                return a < b.first;
            }
        };
    }

    /**
     * Thrown if registering a key fails.
     *
     * \ingroup Exception
     * \ingroup VirtualConstructor
     */
    class VirtualConstructorRegisterFailure : public Exception
    {
        public:
            /**
             * Constructor.
             */
            template <typename T_>
            VirtualConstructorRegisterFailure(const T_ & k) throw () :
                Exception("Virtual constructor registration failed on key '" +
                        stringify(k) + "'")
            {
            }
    };

    /**
     * A VirtualConstructor can be used where a mapping between the value of
     * some key type (often a string) to the construction of some kind of
     * class (possibly via a functor) is required.
     *
     * \ingroup VirtualConstructor
     */
    template <typename KeyType_, typename ValueType_, typename NotFoundBehaviour_>
    class VirtualConstructor :
        public NotFoundBehaviour_::template Parent<KeyType_, ValueType_>,
        public InstantiationPolicy<VirtualConstructor<KeyType_, ValueType_, NotFoundBehaviour_>,
            instantiation_method::SingletonAsNeededTag>
    {
        friend class InstantiationPolicy<
            VirtualConstructor<KeyType_, ValueType_, NotFoundBehaviour_>,
            instantiation_method::SingletonAsNeededTag>;

        private:
            VirtualConstructor()
            {
            }

        protected:
            /**
             * Our entries, sorted.
             */
            std::vector<std::pair<KeyType_, ValueType_> > entries;

        public:
            /**
             * The type of our key.
             */
            typedef KeyType_ KeyType;

            /**
             * The type of our value.
             */
            typedef ValueType_ ValueType;

            /**
             * The behaviour policy for when a key is not found.
             */
            typedef NotFoundBehaviour_ NotFoundBehaviour;

            /**
             * Find a value for the specified key, or perform the appropriate
             * NotFoundBehaviour.
             */
            ValueType_ find_maker(const KeyType_ & k) const;

            /**
             * Convenience alias for find_maker.
             */
            ValueType_ operator[] (const KeyType_ & k) const
            {
                return find_maker(k);
            }

            /**
             * Register a new maker (should usually be called by the
             * RegisterMaker child class.
             */
            void register_maker(const KeyType_ & k, const ValueType_ & v);

            /**
             * An instance of this class registers a new maker with the
             * specified key.
             *
             * \ingroup VirtualConstructor
             */
            struct RegisterMaker
            {
                /**
                 * Constructor.
                 */
                RegisterMaker(const KeyType_ & k, const ValueType_ & v)
                {
                    VirtualConstructor<KeyType_, ValueType_, NotFoundBehaviour_>::get_instance()->
                        register_maker(k, v);
                }
            };
    };

    template <typename KeyType_, typename ValueType_, typename NotFoundBehaviour_>
    ValueType_
    VirtualConstructor<KeyType_, ValueType_, NotFoundBehaviour_>::find_maker(
            const KeyType_ & k) const
    {
        std::pair<
            typename std::vector<std::pair<KeyType_, ValueType_> >::const_iterator,
            typename std::vector<std::pair<KeyType_, ValueType_> >::const_iterator > m(
                    std::equal_range(entries.begin(), entries.end(), k,
                        virtual_constructor_internals::ComparePairByFirst<KeyType_, ValueType_>()));
        if (m.first == m.second)
            return this->handle_not_found(k);
        else
            return m.first->second;
    }

    template <typename KeyType_, typename ValueType_, typename NotFoundBehaviour_>
    void
    VirtualConstructor<KeyType_, ValueType_, NotFoundBehaviour_>::register_maker(
            const KeyType_ & k, const ValueType_ & v)
    {
        if (! entries.insert(std::lower_bound(entries.begin(), entries.end(), k,
                    virtual_constructor_internals::ComparePairByFirst<KeyType_, ValueType_>()),
                std::make_pair(k, v))->second)
            throw VirtualConstructorRegisterFailure(k);
    }
}

#endif
