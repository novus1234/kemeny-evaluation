/*   Kconsens -- Kemeny Consensus finder.
 *
 *   (C) 2008-2010 Robert Bredereck <RBredereck@web.de>
 *
 *   This file is part of Kconsens.
 *   It was taken from the free software Gram (C) 2008-2009 René van Bevern <m5bere2@uni-jena.de>
 *
 *   Kconsens is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   Kconsens is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with Kconsens.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _EFFICIENT_SET_H
#define _EFFICIENT_SET_H

#include <iostream>
#include <boost/unordered_set.hpp>
#include <boost/unordered_map.hpp>
#include <boost/functional/hash.hpp>

using namespace boost;

#define B2(n) n,     n+1,     n+1,     n+2
#define B4(n) B2(n), B2(n+1), B2(n+1), B2(n+2)
#define B6(n) B4(n), B4(n+1), B4(n+1), B4(n+2)
static const uint32_t b32[] = {0xAAAAAAAA, 0xCCCCCCCC, 0xF0F0F0F0, 0xFF00FF00, 0xFFFF0000};
static const uint64_t b64[] = {0xAAAAAAAA, 0xCCCCCCCC, 0xF0F0F0F0, 0xFF00FF00, 0xFFFF0000, 0xFFFFFFFF00000000LLU};
static const unsigned char BitsSetTable256[] = { B6 ( 0 ), B6 ( 1 ), B6 ( 1 ), B6 ( 2 ) };

template <class K, class R> class EfficientSet;

/** compute hash value of an ::EfficientSet */
template <class K, class R>
inline size_t hash_value ( const EfficientSet<K, R> &s )
{
    return hash_value ( typename EfficientSet<K, R>::repType ( s.rep ) );
}

/** function to check membership of elements in ::EfficientSet collections.
    @param c element to search for
    @param M collection to search in
    @return true or false, dependant of \f$c\in M\f$. */
template <class K, class R>
inline bool member ( typename EfficientSet<K, R>::key_type c, const EfficientSet<K, R> &M )
{
    return ( M.rep & ( typename EfficientSet<K, R>::repType ( 1 ) << c ) );
}

template <class K> class SubsetIterator;
template <class K> class RankedSubsetIterator;

/** container template with support for fast subset traversal.
    @tparam K element type
    @tparam R representation type for sets of K, defaults to long.

    @pre @a R should support a fast implementation of the C++ default
semantic of the integral @a ==, @a !=, @a << und @a <<= operators with type @a R on
the left and type @a K on the right side.
*/

template < class K, class R = uint32_t >
class EfficientSet
{
private:
    typedef R repType;

public:
    explicit EfficientSet ( repType r ) : rep ( r ) {}

    /// representation itself
    repType rep;

    /// element type
    typedef K key_type;

    /// construct an empty set
    /// @return the empty set
    EfficientSet() : rep ( 0 ) {}

    /// construct a singleton set
    /// @param e element to construct the singleton set with
    /// @return the set containing only the element @a e
    static EfficientSet singleton ( key_type e ) {
        return EfficientSet().insert ( e );
    }

    /// compare sets for mathematical equality
    /// @return true if two sets are equal, false otherwise
    bool operator== ( const EfficientSet& other ) const {
        return rep == other.rep;
    }

    /// compare sets for mathematical inequality
    /// @return false if two sets are equal, true otherwise
    bool operator!= ( const EfficientSet& other ) const {
        return rep != other.rep;
    }

    /** set difference of two ::EfficientSet containers.
        @param this base set
        @param other substraction set
        @return @a A with the elements in @a B removed. */
    EfficientSet operator- ( const EfficientSet& other ) const
    {
        return EfficientSet ( rep & ~other.rep );
    }

    /** set intersections of two ::EfficientSet containers. */
    EfficientSet operator& ( const EfficientSet& other ) const
    {
        return EfficientSet ( rep & other.rep );
    }

    void printSet()  const
    {
        int s = size();
        int count = 0;
        repType j = 0;
        while ( count < s )
        {
            if ( member ( j, *this ) ) {
                std::cout << j << " ";
                ++count;
            }
            ++j;
        }
    }

    /** union of to ::EfficientSet containers. */
    EfficientSet operator| ( const EfficientSet &other ) const
    {
        return EfficientSet ( rep | other.rep );
    }

    /// check for the empty set
    /// @return true if the set is empty, false otherwise
    bool empty() const {
        return rep == 0;
    }

    /// remove element from set
    /// @param c element to remove
    /// @return Nothing
    /// @post @code member(c, *this) == false @endcode
    void erase ( key_type c ) {
        rep &= ~ ( repType ( 1 ) << c );
    }

    /// insert element into set
    /// @param c element to insert
    /// @return set with @a c included
    /// @post @code member(c, *this) == true @endcode
    EfficientSet& insert ( key_type c ) {
        rep |= repType ( 1 ) << c;
        return *this;
    }

    /// construct set by copying another collection.
    /// @tparam T source collection type
    /// @param first start iterator of source collection
    /// @param last past-the-end iterator of source collection
    /// @return set containing only the elements from the source collection
    /// @remark elemenents that are present in the source collection
    /// multiple times are present in the returned set only once.

    template <class T>
    EfficientSet ( T first, T last ) : rep ( 0 )
    {
        for ( T i = first; i != last; ++i ) rep |= repType ( 1 ) << *i;
    }

    friend size_t hash_value<> ( const EfficientSet& );
    friend bool member<> ( key_type, const EfficientSet& );
    friend class SubsetIterator<EfficientSet<K, R> >;
    friend class RankedSubsetIterator<EfficientSet<K, R> >;

    int size() const
    {
        int uCount = 0 ;
        for ( repType u = rep; u; u &= ( u - 1 ) ) uCount++;
        return uCount ;
    }

    key_type getMemberOfSingleTon () const
    {
        register repType repC = rep;
        register unsigned int r = 0; // r will be lg(v)
        while ( repC >>= 1 )
        {
            r++;
        }
        return r;
    }
};

#endif
