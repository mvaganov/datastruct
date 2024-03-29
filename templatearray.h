#pragma once

#include "license.txt"

/** tests out of bounds before giving data from a requested index */
#define TEST_OOB

// a list of C++11 can be found here: http://gcc.gnu.org/projects/cxx0x.html
// if the compiler does not support C++11, comment the following 3 lines.
#define CPP11_HAS_MOVE_SEMANTICS
#define CPP11_HAS_LAMBDA_SEMANTICS
#define CPP11_HAS_INITIALIZER_LIST
/*
If programming in Eclipse CDT (Juno), do the following for C++11
[Properties]->[C/C++ Build]->[Settings]->[Tool Settings](tab)
->[GCC C++ Compiler]->[Miscellaneous]->[Other flags](text field)
->add "-std=c++0x", [Apply]

[Properties]->[C/C++ Build]->[Settings]->[Tool Settings](tab)
->[GCC C++ Compiler]->[Preprocessor]->[Defined symbols (-D)](text field)
->add (with +document icon) "__GXX_EXPERIMENTAL_CXX0X__"
*/

#ifdef CPP11_HAS_LAMBDA_SEMANTICS
#include <functional>
#endif

#ifdef CPP11_HAS_INITIALIZER_LIST
#include <initializer_list>
#endif

#include "mem.h"

/**
 * using malloc will not require a default constructor for the component type.
 * However, it will also fail to call a constructor, destructor, and fail to
 * initialize any virtual tables.
 */
//#define TEMPLATEARRAY_USES_MALLOC

/**
 * This data structure is ideal when the size is known at creation time, and
 * unlikely to change, though when it changes, it does so to large degrees
 */
template<typename DATA_TYPE>
class TemplateArray
{
protected:
	/** pointer to the allocated data for the vector */
	DATA_TYPE * m_data;

	/** actual number of allocated elements that we can use */
	int m_allocated;

public:
	/** @return true if the given index is safe to access in this array */
	inline bool isValidIndex(int const a_index) const
	{
		return a_index >= 0 && a_index < m_allocated;
	}

	/** @return value (by reference) from the list at given index */
	inline DATA_TYPE & get(const int a_index)
	{
#ifdef TEST_OOB
		if(a_index < 0 || a_index >= m_allocated)
		{
			int i=0;i=1/i;
		}
#endif
		return m_data[a_index];
	}

	/** @return value (by value) from the list at given index */
	inline DATA_TYPE getCONST(const int a_index) const {
#ifdef TEST_OOB
		if(a_index < 0 || a_index >= m_allocated)
		{
			int i=0;i=1/i;
		}
#endif
		return m_data[a_index];
	}

	/** @return value (by value) from the list at given index */
	inline const DATA_TYPE & getCONSTREF(const int a_index) const {
#ifdef TEST_OOB
		if(a_index < 0 || a_index >= m_allocated)
		{
			int i=0;i=1/i;
		}
#endif
		return m_data[a_index];
	}

	/** @return the size of the list (by reference) */
	inline const int & size() const
	{
		return m_allocated;
	}

	/** @return value (by reference) from the list at given index */
	inline DATA_TYPE & operator[](const int a_index)
	{
		return get(a_index);
	}

	/** simple mutator sets a value in the list */
	inline void set(const int a_index, DATA_TYPE const & a_value)
	{
#ifdef TEST_OOB
		if(a_index < 0 || a_index >= m_allocated)
		{
			int i=0;i=1/i;
		}
#endif
		// complex types must overload DATA_TYPE & operator=(const DATA_TYPE &)
		m_data[a_index] = a_value;
	}

	/** copy the given array data into this array */
	inline void setArray(const DATA_TYPE * const a_source, const int a_count)
	{
#ifdef TEST_OOB
		if(a_count > m_allocated)
		{
			int i=0;i=1/i;
		}
#endif
		for(int i = 0; i < a_count; ++i)
		{
			set(i, a_source[i]);
		}
	}

	/** essentially a move operation. takes the parameter array's data */
	void abduct(TemplateArray<DATA_TYPE> & a_array)
	{
		release();
		m_data = a_array.m_data;
		m_allocated = a_array.m_allocated;
		a_array.m_data = 0;
		a_array.m_allocated = 0;
	}

	/**
	 * moves a_size elements starting from index a_from by a_offset
	 * @param a_from should be less than a_last, isValidIndex(a_from)
	 * @param a_offset should be less than 0
	 * @param a_last should be greater than than a_from, and 
	 * (isValidIndex(a_last)||a_last==m_allocated) should return true
	 */
	inline void moveDown(const int a_from, const int a_offset, const int a_last)
	{
		for(int i = a_from-a_offset; i < a_last; ++i)
		{
			set(i+a_offset, get(i));
		}
	}
	/**
	 * moves a_size elements starting from index a_from by a_offset
	 * @param a_from should be a_from 0 <= a_from < m_allocated (not checked)
	 * @param a_offset should be greater than zero (not checked)
	 * @param a_last should be 0 < a_last < (m_allocated) (not checked)
	 */
	inline void moveUp(const int a_from, const int a_offset, const int a_last)
	{
		for(int i = a_last-a_offset-1; i >= a_from; --i)
		{
			set(i+a_offset, get(i));
		}
	}

public:
	/** @param a_size reallocate the vector to this size */
	const bool allocateToSize(const int a_size)
	{
		// reallocate a new list with the given size
#ifndef TEMPLATEARRAY_USES_MALLOC
		DATA_TYPE * newList = NEWMEM_ARR(DATA_TYPE, a_size);
#else
		DATA_TYPE * newList = (DATA_TYPE *)malloc(sizeof(DATA_TYPE)*a_size);
#endif
		// if the list could not allocate, fail...
		if(!newList)	return false;
		// the temp list is the one we will keep, while the old list will be dropped.
		DATA_TYPE * oldList = m_data;
		// swap done here so set(index, value) can be called instead of the equals operator
		m_data = newList;
		// if there is old data
		if(oldList)
		{
			// when copying old data, make sure no over-writes happen.
			int smallestSize = m_allocated<a_size?m_allocated:a_size;
			// fill the new list with the old data
			for(int i = 0; i < smallestSize; ++i)
			{
				set(i, oldList[i]);
			}
			// get rid of the old list (so we can maybe use the memory later)
#ifndef TEMPLATEARRAY_USES_MALLOC
			DELMEM_ARR(oldList);
#else
			free(oldList);
#endif
		}
		// mark the new allocated size (held size of oldList)
		m_allocated = a_size;
		return true;
	}

	/** note: this method is memory intesive, and should not be in any inner loops... */
	void add(DATA_TYPE const & a_value)
	{
		NEWMEM_SOURCE_TRACE(allocateToSize(size()+1));
		set(size()-1, a_value);
	}

	/** note: this method is memory intesive, and should not be in any inner loops... */
	DATA_TYPE * add()
	{
		NEWMEM_SOURCE_TRACE(allocateToSize(size()+1));
		return &get(size()-1);
	}

	/** sets all fields to an initial data state. WARNING: can cause memory leaks if used without care */
	inline void init()
	{
		m_data = 0;
		m_allocated = 0;
	}

	/** cleans up memory */
	inline void release()
	{
		if(m_data)
		{
#ifndef TEMPLATEARRAY_USES_MALLOC
			DELMEM_ARR(m_data);
#else
			free(m_data);
#endif
			m_data = 0;
			m_allocated = 0;
		}
	}

	~TemplateArray(){release();}

	/** @return true if vector allocated this size */
	inline bool ensureCapacity(const int a_size)
	{
		if(a_size && m_allocated < a_size)
		{
			return allocateToSize(a_size);
		}
		return true;
	}

	/** @return true of the copy finished correctly */
	inline bool copy(TemplateArray<DATA_TYPE> const & a_array)
	{
		if(m_allocated != a_array.m_allocated)
		{
			release();
			bool allocated = false;
			NEWMEM_SOURCE_TRACE(allocated = allocateToSize(a_array.m_allocated));
			if(!allocated)
				return false;
		}
		for(int i = 0; i < a_array.m_allocated; ++i)
		{
			set(i, a_array.getCONST(i));
		}
		return true;
	}

	/** copy constructor */
	inline TemplateArray(TemplateArray<DATA_TYPE> const & a_array)
	{
		init();
		copy(a_array);
	}

#ifdef CPP11_HAS_MOVE_SEMANTICS
	/** will move the data from a_array to *this */
	inline void moveSemantic(TemplateArray & a_array)
	{
		m_allocated = a_array.m_allocated;
		a_array.m_allocated = 0;
		m_data = a_array.m_data;
		a_array.m_data = 0;
	}

	/**
	 * move constructor, for C++11, to make the following efficient
	 * <code>TemplateArray<int> list(TemplateArray<int>());</code>
	 */
	inline TemplateArray(TemplateArray<DATA_TYPE> && a_array)
	{
		moveSemantic(a_array);
	}
	/**
	 * move assignment, for C++11, to make the following efficient
	 * <code>TemplateArray<int> list = TemplateArray<int>();</code>
	 */
	inline TemplateArray & operator=(TemplateArray<DATA_TYPE> && a_array){
		release();
		moveSemantic(a_array);
		return *this;
	}
#endif

#ifdef CPP11_HAS_INITIALIZER_LIST
    TemplateArray( const std::initializer_list <DATA_TYPE> & ilist )
    	:m_data(0),m_allocated(0)
    {
        auto it = ilist.begin();
        allocateToSize(ilist.size());
        int index = 0;
        while( it != ilist.end() ) {
        	set(index++, *it);
            it++;
        }
    }
#endif

#ifdef CPP11_HAS_LAMBDA_SEMANTICS
	/** @param f execute this code for each element of this container */
	void for_each_full(std::function<void (DATA_TYPE & value, const int index)> f){
		for(int i = 0; i < size(); ++i)
			f(TemplateArray<DATA_TYPE>::get(i), i);
	}
	/** @param f execute this code for each element of this container */
	void for_each(std::function<void (DATA_TYPE & value)> f){
		for(int i = 0; i < size(); ++i)
			f(TemplateArray<DATA_TYPE>::get(i));
	}
#endif

	/** explicit copy operator overload */
	inline TemplateArray & operator=(TemplateArray<DATA_TYPE> const & a_array){
		release();
		copy(a_array);
		return *this;
	}

	/** default constructor allocates no list (zero size) */
	inline TemplateArray(){init();}


	/** size constructor */
	explicit inline TemplateArray(int const a_size)
	{
		init();
		NEWMEM_SOURCE_TRACE(ensureCapacity(a_size));
	}

	/** format constructor */
	inline TemplateArray(const int a_size, DATA_TYPE const & a_defaultValue)
	{
		init();
		NEWMEM_SOURCE_TRACE(ensureCapacity(a_size));
		for(int i = 0; i < a_size; ++i)
			set(i, a_defaultValue);
	}

	/** @return the last value in the list */
	inline DATA_TYPE & getLast()
	{
		return get(size()-1);
	}

	/**
	 * @return the raw pointer to the data... 
	 * @note dangerous accessor. use it only if you know what you're doing.
	 */
	inline DATA_TYPE * getRawList()
	{
		return m_data;
	}

	/** 
	 * @param a_index is overwritten by the next element, which is 
	 * overwritten by the next element, and so on, till the last element
	 * @note this operation could be memory intensive!
	 */
	void remove(const int a_index)
	{
		moveDown(a_index, -1, size());
		NEWMEM_SOURCE_TRACE(allocateToSize(m_allocated-1));
	}

	/** 
	 * @param a_index where to insert a_value. shifts elements in the vector.
	 * @note this operation could be memory intensive!
	 */
	void insert(const int a_index, DATA_TYPE const & a_value)
	{
		NEWMEM_SOURCE_TRACE(allocateToSize(m_allocated+1));
		moveUp(a_index, 1, size());
		set(a_index, a_value);
	}

	/** swaps to elements in the vector */
	inline void swap(const int a_index0, const int a_index1)
	{
		DATA_TYPE temp = get(a_index0);
		set(a_index0, get(a_index1));
		set(a_index1, temp);
	}

	/** @return index of 1st a_value at or after a_startingIndex. uses == */
	inline int indexOf(DATA_TYPE const & a_value, const int a_startingIndex, const int a_endingIndex) const
	{
		for(int i = a_startingIndex; i < a_endingIndex; ++i)
		{
			if(getCONST(i) == a_value)
				return i;
		}
		return -1;
	}

	/** @return index of 1st a_value at or after a_startingIndex. uses == */
	inline int indexOf(DATA_TYPE const & a_value) const
	{
		return indexOf(a_value, 0, size());
	}

	/**
	 * will only work correctly if the TemplateVector is sorted.
	 * @return the index of the given value, -1 if the value is not in the list
	 */
	int indexOfWithBinarySearch(DATA_TYPE const & a_value, int const a_first, int const a_limit) const
	{
		if(m_allocated)
		{
			int first = a_first, last = a_limit;
			while (first <= last)
			{
				int mid = (first + last) / 2;  // compute mid point.
				if (a_value > m_data[mid]) 
					first = mid + 1;  // repeat search in top half.
				else if (a_value < m_data[mid]) 
					last = mid - 1; // repeat search in bottom half.
				else
					return mid;     // found it. return position
			}
		}
		return -1;    // failed to find key
	}

	void setAll(DATA_TYPE const & a_value)
	{
		for(int i = 0; i < size(); ++i)
		{
			set(i, a_value);
		}
	}

	/** @return if linear pass proved all elements in order (uses less than) */
	bool isSorted() const
	{
		bool sorted = true;
		for(int i = 1; sorted && i < size(); ++i)
		{
			sorted = getCONST(i-1) < getCONST(i);
		}
		return sorted;
	}
};
