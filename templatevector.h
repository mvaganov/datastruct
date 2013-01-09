#pragma once

#include "license.txt"
#include "templatearray.h"

/**
 * Simple templated vector. Ideal for dynamic lists of primitive types and 
 * single dimensional pointers. Fast access.
 * @WARNING template with virtual types, or types that will be referenced by
 * other pointers to at your own risk! In those situations, templated lists of
 * pointers to those types are a much better idea. Or, TemplatedVectorList, 
 * which is memory stable, would be appropriate.
 * @author mvaganov@hotmail.com
 */
template<typename DATA_TYPE>
class TemplateVector : public TemplateArray<DATA_TYPE>
{
protected:
	/** the default size to allocate new vectors to */
	static const int DEFAULT_ALLOCATION_SIZE = 8;

	/** number of valid elements that the caller thinks we have.. */
	int m_size;
public:
	/** @return true if the given index is safe to access in this array */
	inline bool isValidIndex(int const a_index) const
	{
		return a_index >= 0 && a_index < m_size;
	}

	/** @return the size of the list */
	inline const int & size() const
	{
		return m_size;
	}

	/** @return how many elements are allocated to the vector in memory */
	inline const int & getAllocatedSize() const
	{
		return TemplateArray<DATA_TYPE>::size();
	}

	/** sets all fields to an initial data state. WARNING: can cause memory leaks if used without care */
	inline void init()
	{
		TemplateArray<DATA_TYPE>::init();
		m_size = 0;
	}

	/** cleans up memory */
	inline void release()
	{
		TemplateArray<DATA_TYPE>::release();
		m_size = 0;
	}

	/** @return true of the copy finished correctly */
	inline bool copy(TemplateVector<DATA_TYPE> const & a_vector)
	{
		bool allocated = false;
		NEWMEM_SOURCE_TRACE(allocated = TemplateArray<DATA_TYPE>::ensureCapacity(a_vector.m_size));
		if(allocated)
		{
			for(int i = 0; i < a_vector.m_size; ++i)
			{
				TemplateArray<DATA_TYPE>::set(i, a_vector.getCONSTREF(i));
			}
			m_size = a_vector.m_size;
			return true;
		}
		return false;
	}

	/** copy constructor */
	inline TemplateVector(TemplateVector<DATA_TYPE> const & a_vector)
	{
		init();
		NEWMEM_SOURCE_TRACE(copy(a_vector));
	}


	/** essentially a move operation. takes the parameter vector's data */
	void abduct(TemplateVector<DATA_TYPE> & a_vector)
	{
		TemplateArray<DATA_TYPE>::abduct(a_vector);
		m_size = a_array.m_size;
		a_array.m_size = 0;
	}

#ifdef CPP11_HAS_MOVE_SEMANTICS
	/** will move the data from a_array to *this */
	inline void moveSemantic(TemplateVector & a_vector)
	{
		TemplateArray<DATA_TYPE>::moveSemantic(a_vector);
		m_size = a_vector.m_size;
		a_vector.m_size = 0;
	}

	/**
	 * move constructor, for C++11, to make the following efficient
	 * <code>TemplateVector<int> list(TemplateVector<int>());</code>
	 */
	TemplateVector(TemplateVector<DATA_TYPE> && a_vector)
	{
		moveSemantic(a_vector);
	}

	/**
	 * move assignment, for C++11, to make the following efficient
	 * <code>TemplateVector<int> list = TemplateVector<int>();</code>
	 */
	inline TemplateVector & operator=(TemplateVector<DATA_TYPE> && a_vector){
		release();
		moveSemantic(a_vector);
		return *this;
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

#ifdef CPP11_HAS_INITIALIZER_LIST
    TemplateVector( const std::initializer_list <DATA_TYPE> & ilist )
    {
    	init();
    	TemplateArray<DATA_TYPE>::setSize(ilist.size());
        auto it = ilist.begin();
        int index = 0;
        while( it != ilist.end() ) {
        	set(index++, *it);
            it++;
        }
    }
#endif

    /** explicit copy operator overload */
	inline TemplateVector & operator=(TemplateVector<DATA_TYPE> const & a_vector){
		release();
		NEWMEM_SOURCE_TRACE(copy(a_vector));
		return *this;
	}

	/** default constructor allocates no list (zero size) */
	inline TemplateVector(){init();}

	/** format constructor */
	TemplateVector(const int a_size, DATA_TYPE const & a_defaultValue)
	{
		init();
		NEWMEM_SOURCE_TRACE(TemplateArray<DATA_TYPE>::ensureCapacity(a_size));
		for(int i = 0; i < a_size; ++i)
			add(a_defaultValue);
	}

	/** @return the last value in the list */
	inline DATA_TYPE & getLast()
	{
		return this->m_data[m_size-1];
	}

	/** @return the last added value in the list, and lose that value */
	inline DATA_TYPE & pop()
	{
		return this->m_data[--m_size];
	}

	/**
	 * @param value to add to the list 
	 * @note adding a value may cause memory allocation
	 */
	void add(DATA_TYPE const & a_value)
	{
		// where am i storing these values?
		// if i don't have a place to store them, i better make one.
		if(this->m_data == 0)
		{
			// make a new list to store numbers in
			NEWMEM_SOURCE_TRACE(TemplateArray<DATA_TYPE>::allocateToSize(this->DEFAULT_ALLOCATION_SIZE));
		}
		// if we don't have enough memory allocated for this list
		if(m_size >= this->m_allocated)
		{
			// make a bigger list
			NEWMEM_SOURCE_TRACE(TemplateArray<DATA_TYPE>::allocateToSize(this->m_allocated*2));
		}
		TemplateArray<DATA_TYPE>::set(m_size++, a_value);
	}

	/**
	 * @param a_value to add to the list if it isnt in the list already
	 * @return the index where the element exists
	 */
	int addNoDuplicates(DATA_TYPE const & a_value)
	{
		int index = indexOf(a_value);
		if(index < 0)
		{
			index = m_size;
			add(a_value);
		}
		return index;
	}

	/** @param a_vector a vector to add all the elements from */
	inline void addVector(TemplateVector<DATA_TYPE> const & a_vector)
	{
		for(int i = 0; i < a_vector.size(); ++i)
		{
			NEWMEM_SOURCE_TRACE(add(a_vector.get(i)));
		}
	}

	/** @return the number of bytes allocated (likely larger than size()) */
	inline const int & allocatedCapacity() const {
		return this->m_allocated;
	}

	/** 
	 * @param size the user wants the vector to be (chopping off elements)
	 * @return false if could not allocate memory
	 * @note may cause memory allocation if size is bigger than current
	 */
	inline bool setSize(const int a_size)
	{
		bool allocated = false;
		NEWMEM_SOURCE_TRACE(allocated = TemplateArray<DATA_TYPE>::ensureCapacity(a_size));
		if(!allocated)
			return false;
		m_size = a_size;
		return true;
	}

	/** adds the given array */
	void add(DATA_TYPE * const & a_list, const int a_numElements){
		NEWMEM_SOURCE_TRACE(TemplateArray<DATA_TYPE>::ensureCapacity(a_numElements));
		for(int i = 0; i < a_numElements; ++i){
			add(a_list[i]);
		}
	}

	/** sets size to zero, but does not deallocate any memory */
	inline void clear()
	{
		setSize(0);
	}

	/** 
	 * @param a_index is overwritten by the next element, which is 
	 * overwritten by the next element, and so on, till the last element
	 */
	DATA_TYPE remove(const int a_index)
	{
		DATA_TYPE data = TemplateArray<DATA_TYPE>::get(a_index);
		TemplateArray<DATA_TYPE>::moveDown(a_index, -1, m_size);
		setSize(m_size-1);
		return data;
	}

	/** 
	 * @param a_index where to insert a_value. shifts elements in the vector.
	 */
	void insert(const int a_index, DATA_TYPE const & a_value)
	{
		NEWMEM_SOURCE_TRACE(setSize(m_size+1));
		TemplateArray<DATA_TYPE>::moveUp(a_index, 1, m_size);
		this->set(a_index, a_value);
	}

	/** 
	 * @return first element from the list and moves the rest up 
	 * @note removes the first element from the list
	 */
	inline const DATA_TYPE pull()
	{
		DATA_TYPE value = TemplateArray<DATA_TYPE>::get(0);
		remove(0);
		return value;
	}

	/**
	 * a remove function that does not maintain proper list order
	 * @param a_index is replaced by the last element, then size is reduced.
	 */
	inline void removeFast(const int a_index)
	{
		TemplateArray<DATA_TYPE>::swap(a_index, m_size-1);
		this->setSize(m_size-1);
	}

	/**
	 * @param removes all elements of this value in one O(N) process 
	 * @return number of elements removed
	 */
	int removeAll(DATA_TYPE const & a_value)
	{
		int removed = 0;
		for(int i = 0; i < m_size-removed; ++i)
		{
			while(i+removed < m_size
			&& TemplateArray<DATA_TYPE>::get(i+removed) == a_value){
				++removed;
			}
			if(removed > 0){
				this->set(i, TemplateArray<DATA_TYPE>::get(i+removed));
			}
		}
		setSize(m_size-removed);
		return removed;
	}

	/** @return the index of the first appearance of a_value in this vector. uses == */
	inline int indexOf(DATA_TYPE const & a_value) const
	{
		for(int i = 0; i < m_size; ++i)
		{
			if(this->getCONST(i) == a_value)
				return i;
		}
		return -1;
	}

	/** @return index of 1st a_value at or after a_startingIndex. uses == */
	inline int indexOf(DATA_TYPE const & a_value, const int a_startingIndex) const
	{
		return TemplateArray<DATA_TYPE>::indexOf(a_value, a_startingIndex, m_size);
	}

	/** @return index of 1st a_value at or after a_startingIndex. uses == */
	inline int indexOf(DATA_TYPE const & a_value, const int a_startingIndex, const int a_size) const
	{
		return TemplateArray<DATA_TYPE>::indexOf(a_value, a_startingIndex, a_size);
	}

	/**
	 * will only work correctly if the TemplateVector is sorted.
	 * @return the index of the given value, -1 if the value is not in the list
	 */
	inline int indexOfWithBinarySearch(DATA_TYPE const & a_value) const
	{
		if(m_size)
		{
			return TemplateArray<DATA_TYPE>::indexOfWithBinarySearch(a_value, 0, m_size);
		}
		return -1;    // failed to find key
	}

	/**
	 * uses binary sort to put values in the correct index. safe if soring is always used
	 * @param a_value value to insert in order
	 * @param a_allowDuplicates will not insert duplicates if set to false
	 * @return the index where a_value was inserted
	 */
	int insertSorted(DATA_TYPE const & a_value, const bool a_allowDuplicates)
	{
		int index = -1;
		if(!m_size || a_value < TemplateArray<DATA_TYPE>::get(0))
		{
			index = 0;
		}
		else if(!(a_value < TemplateArray<DATA_TYPE>::get(m_size-1)))
		{
			index = m_size;
		}
		else
		{
			int first = 0, last = m_size;
			while (first <= last)
			{
				index = (first + last) / 2;
				if (!(a_value < this->m_data[index]))
					first = index + 1;
				else if (a_value < this->m_data[index])
					last = index - 1;
			}
			if(!(a_value < this->m_data[index]))
				index++;
		}
		if(!m_size  || a_allowDuplicates || a_value != this->m_data[index])
			NEWMEM_SOURCE_TRACE(insert(index, a_value));
		return index;
	}

	/**
	 * a remove function that does not maintain proper list order
	 * @param a_value first appearance replaced by last element. breaks if not in list
	 */
	inline void removeDataFast(DATA_TYPE const & a_value)
	{
		removeFast(indexOf(a_value));
	}

	/**
	 * @param a_listToExclude removes these elements from *this list
	 * @return true if at least one element was removed
	 */
	inline bool removeListFast(TemplateVector<DATA_TYPE> const & a_listToExclude)
	{
		bool aTermWasRemoved = false;
		for(int e = 0; e < a_listToExclude.size(); ++e)
		{
			for(int i = 0; i < size(); ++i)
			{
				if(a_listToExclude.get(e) == TemplateArray<DATA_TYPE>::get(i))
				{
					removeFast(i);
					--i;
					aTermWasRemoved = true;
				}
			}
		}
		return aTermWasRemoved;
	}

	/** @param a_value first appearance is removed. breaks if not in list */
	inline void removeData(DATA_TYPE const & a_value)
	{
		remove(indexOf(a_value));
	}

	/** destructor */
	inline ~TemplateVector()
	{
		release();
	}

	void sort(){
		TemplateArray<DATA_TYPE>::sort(0, m_size);
	}

#ifdef CPP11_HAS_LAMBDA_SEMANTICS
	void sortFunction(std::function<bool(DATA_TYPE &, DATA_TYPE &)> aBeforeB){
		TemplateArray<DATA_TYPE>::sortFunction(0, m_size,aBeforeB);
	}
#endif
};
