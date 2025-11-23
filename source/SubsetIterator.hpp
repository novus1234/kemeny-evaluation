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

#ifndef _SUBSET_ITERATOR_H
#define _SUBSET_ITERATOR_H

#include <iterator>
#include "EfficientSet.hpp"

using namespace boost;
using namespace std;

template<class T>
class _SSIterator
      : public std::iterator<input_iterator_tag, const T>
{
protected:
  T currentSet;
  const T baseSet;
  bool valid;

  _SSIterator() : currentSet(), baseSet(), valid ( false ) {}
  explicit _SSIterator ( const T& set ) : currentSet(), baseSet ( set ), valid ( true ) {}
public:
  /** compare two subset iterators for inequality. It's the opposite of
   * #operator==. */
  bool operator!= ( const _SSIterator &o ) const
  {
    if ( valid != o.valid )
      return true;
    if ( !valid and !o.valid )
      return false;
    return ( currentSet != o.currentSet ) or ( baseSet != o.baseSet );
  }

  /** compare two subset iterators for equality. Two subset iterators
   * are equal if they are either both invalid or iterate over the
   * same base set and are currently visiting the same subset.
   * @return true if two iterators are equal, false otherwise. */

  bool operator== ( const _SSIterator &o ) const
  {
    return ( valid == o.valid ) and ( currentSet == o.currentSet )
           and ( baseSet == o.baseSet );
  }

  /** get the current subset. @return the subset currently pointed to
   * by the iterator. */
  T operator*() const
  {
#ifdef DEBUG
    if ( valid )
      return currentSet;
    else
      throw ( out_of_range ( "Dereferenzierung eines ungültigen Iterators" ) );
#endif
    return currentSet;
  }

};

/** iterator over all subsets of a collection @tparam T type of the
    super set. Currently, onley instances of ::EfficientSet are
    supported. */
template <class T>
class SubsetIterator : public _SSIterator<T>
{
  using _SSIterator<T>::baseSet;
  using _SSIterator<T>::currentSet;
  using _SSIterator<T>::valid;
  const T baseSetC;
public:
  /** constructs the invalid or past-the-end iterator */
  SubsetIterator() : _SSIterator<T>() {}

  /** constructs an iterator over all subsets of @a set.
      @post The constructed iterator is valid, since every set at least has the empty subset.
  */
  explicit SubsetIterator ( const T& set ) : _SSIterator<T> ( set ), baseSetC ( ~ baseSet.rep ) {}

  /** advance to the next subset.
      @post If no further subsets are available,
   * the iterator becomes invalid and compares equal (@a ==) to the
   * invalid iterator, that can be constructed with #SubsetIterator().
   @return the iterator itself is returned after advancing to the next subset. */
  SubsetIterator<T>& operator++();
};

/** iterator over all subsets of certain size of a collection @tparam
    T type of the super set. Currently, only instances of
    ::EfficientSet are supported */
template <class T>
class RankedSubsetIterator : public _SSIterator<T>
{
  using _SSIterator<T>::baseSet;
  using _SSIterator<T>::currentSet;
  using _SSIterator<T>::valid;

  RankedSubsetIterator<T> *subsetsIncl, *subsetsExcl;
  typename T::repType rest;
public:
  /** constructs the invalid or past-the-end iterator */
  RankedSubsetIterator() : _SSIterator<T>(), subsetsIncl ( 0 ), subsetsExcl ( 0 ) {}
  ~RankedSubsetIterator();

  /** constructs an iterator over all size @a k subsets of @a set.
      @post The constructed iterator may be invalid in case there is
      no subset of size @a k. */

  RankedSubsetIterator ( const T& S, int k );

  /** advance to the next subset of size @a k as given to
        #RankedSubsetIterator(const T& S, int k).
        @post If no further subsets are
        available, the iterator becomes invalid and compares equal (@a
        ==) to the invalid iterator, that can be constructed with
        #RankedSubsetIterator().  @return the iterator itself is returned
        after advancing to the next subset. */

  RankedSubsetIterator<T>& operator++();
};

template <class T>
SubsetIterator<T>& SubsetIterator<T>::operator++()
{
  if ( currentSet.rep == baseSet.rep )
    valid = false;
  currentSet.rep = ( ( currentSet.rep | baseSetC.rep ) + typename T::repType ( 1 ) ) & baseSet.rep;
  return *this;
}

template <class T>
RankedSubsetIterator<T>::RankedSubsetIterator ( const T& set, int k )
    : _SSIterator<T> ( set ), subsetsIncl ( 0 ), subsetsExcl ( 0 )
{
  typename T::repType prev = set.rep & ( set.rep - typename T::repType ( 1 ) );
  if ( k == 0 )
    valid = true;
  else
    if ( set.rep == 0 )
      valid = false;
    else
      if ( ( k == 1 ) and ( prev == 0 ) )
        currentSet = baseSet;
      else
        if ( k >= 1 )
        {
          typedef RankedSubsetIterator<T> it;
          subsetsExcl = new it ( T ( prev ), k - 1 );
          subsetsIncl = new it ( T ( prev ), k );
          rest = baseSet.rep ^ prev;
          ++ ( *this );
        }
        else
          throw ( out_of_range ( "Kann nicht über Teilmengen mit weniger als 0 Elementen iterieren" ) );
}

template <class T>
RankedSubsetIterator<T>::~RankedSubsetIterator()
{
  if ( subsetsExcl )
    delete subsetsExcl;
  if ( subsetsIncl )
    delete subsetsIncl;
}

template <class T>
RankedSubsetIterator<T>& RankedSubsetIterator<T>::operator++()
{
  if ( subsetsExcl and subsetsExcl->valid )
  {
    currentSet = T ( ( **subsetsExcl ).rep | rest );
    ++ ( *subsetsExcl );
  }
  else
    if ( subsetsIncl and subsetsIncl->valid )
    {
      currentSet = T ( **subsetsIncl );
      ++ ( *subsetsIncl );
    }
    else
      valid = false;
  return *this;
}

#endif
