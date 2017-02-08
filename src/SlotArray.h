#pragma once
#include "platform.h"
#include "Utils.h"
#include <queue>

using namespace std;

template <class T>
class CSlotArray
{
public:
	CSlotArray(void);
	~CSlotArray(void);

	void		Init(int num);
	void		Release();

	int			Add(T *element);
	void		Remove(int slot);

	T*	operator[](int index) { return pElements[index]; }

public:
	int			maxElements;
	int			count;

	queue<int>	emptySlots;
	vector<int>	slotsTaken;
	T			**pElements;
};



template <class T>
CSlotArray<T>::CSlotArray()
{
	pElements = NULL;
	count = 0;
	maxElements = 0;
}

template <class T>
CSlotArray<T>::~CSlotArray(void)
{
	Release();
}

template <class T>
void CSlotArray<T>::Release()
{
	if (pElements)
	{
		for (int i=0; i<maxElements; i++) 
			SAFE_DELETE(pElements[i]);
		SAFE_DELETE_ARRAY(pElements);
	}
	queue<int> empty;
	slotsTaken.clear();
}

template <class T>
void CSlotArray<T>::Init( int num )
{
	Release();
	pElements = new T*[num];
	maxElements = num;
	for (int i=0; i<maxElements; ++i) 
	{
		pElements[i] = NULL;
		emptySlots.push(i);
	}	
	count = 0;	
}

template <class T>
void CSlotArray<T>::Remove( int slot )
{
	pElements[slot] = NULL;
	emptySlots.push(slot);
	count--;
	for (vector<int>::iterator i=slotsTaken.begin(); i!=slotsTaken.end(); i++)
	{
		if (*i == slot)
		{
			slotsTaken.erase(i);
			break;
		}
	}
}

template <class T>
int CSlotArray<T>::Add( T *element )
{
	if (emptySlots.empty()) return -1;
	int slot = emptySlots.front();
	emptySlots.pop();
	pElements[slot] = element;
	count++;
	slotsTaken.push_back(slot);
	return slot;
}



