//**********************************************************************
//
// Copyright (c) 2000-2005
// PathEngine
// Lyon, France
//
// All Rights Reserved
//
//**********************************************************************

//.... abstract digit string representation
//.... range checking on topdigit
//.... assumes new is checked
//.... exception on divide by 0?
//.. optimisation - maintain cDigitBlock ptr on traversals?

#include "common/interface/Assert.h"
#include "common/STL/algorithm.h"

class cLargeInteger2;

const int DIGITSPERBLOCK=8;
const int DIGITSPERBLOCKMASK=7;

class cDigitBlock
{
public:
	cDigitBlock()
	{
		int i;
		//.. memset
		for(i=0;i<DIGITSPERBLOCK;i++)
			digitarray[i]=0;
	}
	unsigned long digitarray[DIGITSPERBLOCK];
	cDigitBlock *next;
	cDigitBlock *prev;
};

class cDynamicInteger
{
	bool negativeFlag;
	cDigitBlock firstblock;
	cDigitBlock *lastblockptr;
	unsigned long topdigit;

	// remove any number of leading zeros
	void normalise();

//.. move public function definitions into cDynamicInteger.cpp

	cDigitBlock *adddigit(const cDigitBlock *const blockptr, unsigned long index)
	{
		cDigitBlock *newblockptr;
		topdigit++;
		assertD(index==(topdigit&DIGITSPERBLOCKMASK));
		if(blockptr)
		{
			assertD(index);
			assertD(blockptr==lastblockptr);
			return lastblockptr;
		}
		assertD(!index);
		newblockptr=new cDigitBlock;
		lastblockptr->next=newblockptr;
		newblockptr->prev=lastblockptr;
		newblockptr->next=0;
		lastblockptr=newblockptr;
		return newblockptr;
	}

	void deleteall()
	{
		cDigitBlock *ptr;
		cDigitBlock *nextptr;
		ptr=firstblock.next;
		while(ptr)
		{
			nextptr=ptr->next;
			delete ptr;
			ptr=nextptr;
		}
	}
	void initialiseFrom( const cDynamicInteger &di);
	void initialiseFrom( const unsigned long ulvalue)
	{
		int i;
		lastblockptr=&firstblock;
		firstblock.next=0;
		negativeFlag=0;
		for(i=1;i<DIGITSPERBLOCK;i++)
			firstblock.digitarray[i]=0;
		firstblock.digitarray[0]=ulvalue;
		topdigit=0;
	}
	void initialiseFrom( const long value)
	{
		unsigned long ulvalue;
		if(value<0)
		{
			if(value==0x80000000)
				ulvalue=(unsigned long)0x80000000;
			else
				ulvalue=(unsigned long)-value;
			initialiseFrom(ulvalue);
			negativeFlag=1;
			return;
		}
		ulvalue=(unsigned long)value;
		initialiseFrom(ulvalue);
	}
	void initialiseFrom( const unsigned long firstdigit, const unsigned long seconddigit, const unsigned long digits);

	unsigned long getseconddigit() const
	{
		int index;
		index=(topdigit&DIGITSPERBLOCKMASK);
		if(index)
			return lastblockptr->digitarray[index-1];
//		assertD(lastblockptr->prev);
		if((lastblockptr->prev)==0)
			return 0;
		return lastblockptr->prev->digitarray[DIGITSPERBLOCK-1];
	}

	bool cansubtractresidueproduct(const cDynamicInteger &divisor, unsigned long partialquotient, unsigned long partialremainder) const;
	void subtractresidueproduct(const cDynamicInteger &divisor, unsigned long partialquotient, unsigned long partialremainderhigh, unsigned long partialremainderlow);

	int comparebitstring(const cDigitBlock *endptr, const int endindex, const cDynamicInteger &test, const int testshift) const;
	void subtractbitstring(cDigitBlock *ptr, int index, const cDynamicInteger &src, cDigitBlock *srcptr, int srcindex, const int srcshift);

public:

	//.. public for hcfinfo
	unsigned long getfirstdigit() const
	{
		return lastblockptr->digitarray[topdigit&DIGITSPERBLOCKMASK];
	}
    unsigned long getTopDigitIndex() const
    {
        return topdigit;
    }

	cDynamicInteger()
	{
		firstblock.prev=0;
		initialiseFrom((unsigned long)0);
	}
	cDynamicInteger(const long value)
	{
		firstblock.prev=0;
		initialiseFrom(value);
	}
	cDynamicInteger(const unsigned long value)
	{
		firstblock.prev=0;
		initialiseFrom(value);
	}
	cDynamicInteger(const unsigned long * ptr, int digits)
	{
	//.. optimise - construct directly
		firstblock.prev=0;
		if(digits)
		{
			initialiseFrom(ptr[digits-1]);
			digits--;
		}
		else
			initialiseFrom((unsigned long)0);
		while(digits)
		{
			shiftdigitleft();
			*this+=ptr[digits-1];
			digits--;
		}
	}

    cDynamicInteger(const cLargeInteger2& value);

	~cDynamicInteger()
	{
		deleteall();
	}

	cDynamicInteger(const cDynamicInteger &di)
	{
	// copy constructor
		firstblock.prev=0;
		initialiseFrom(di);
	}
	cDynamicInteger& operator= (const cDynamicInteger &di)
	{
	// assignment constructor
		firstblock.prev=0;
		if(*this!=di)
		{
			//.. can be optimised
			deleteall();
			initialiseFrom(di);
		}
		return *this;
	}

    void
    swap(cDynamicInteger& rhs)
    {
        std::swap(negativeFlag, rhs.negativeFlag);
        std::swap(firstblock, rhs.firstblock);
        std::swap(lastblockptr, rhs.lastblockptr);
        if(firstblock.next)
        {
            assertD(firstblock.next->prev == &rhs.firstblock);
            firstblock.next->prev = &firstblock;
        }
        else
        {
            assertD(lastblockptr == &rhs.firstblock);
            lastblockptr = &firstblock;
        }
        if(rhs.firstblock.next)
        {
            assertD(rhs.firstblock.next->prev == &firstblock);
            rhs.firstblock.next->prev = &rhs.firstblock;
        }
        else
        {
            assertD(rhs.lastblockptr == &firstblock);
            rhs.lastblockptr = &rhs.firstblock;
        }
        std::swap(topdigit, rhs.topdigit);
    }

	// ======= derived operators ======

    cDynamicInteger operator+= (const cDynamicInteger &di2);
	cDynamicInteger operator-= (const cDynamicInteger &di2)
	{
		negate();
		*this+=di2;
		negate();
		return *this;
	}

	cDynamicInteger operator*= (const cDynamicInteger &di2);
	friend cDynamicInteger operator*(const cDynamicInteger &di1,const cDynamicInteger &di2);

	cDynamicInteger operator- ()
	{
		cDynamicInteger returnvalue=*this;
		returnvalue.negate();
		return returnvalue;
	}
	friend bool operator== (const cDynamicInteger &s1, const cDynamicInteger &s2);
	friend bool operator!= (const cDynamicInteger &s1, const cDynamicInteger &s2)
	{
		return !(s1==s2);
	}

    int
    compareAbsoluteValues(const cDynamicInteger& rhs) const;
    int
    compare(const cDynamicInteger& rhs) const;

	friend bool
    operator>(const cDynamicInteger &s1, const cDynamicInteger &s2)
    {
        return s1.compare(s2) == -1;
    }
	friend bool
    operator>=(const cDynamicInteger &s1, const cDynamicInteger &s2)
	{
//		return s1==s2 || s1>s2;
        return s1.compare(s2) != 1;
	}
	friend bool
    operator<(const cDynamicInteger &s1, const cDynamicInteger &s2)
	{
//		return s1!=s2 &! (s1>s2);
        return s1.compare(s2) == 1;
	}
	friend bool
    operator<=(const cDynamicInteger &s1, const cDynamicInteger &s2)
	{
//		return !(s1>s2);
        return s1.compare(s2) != -1;
	}
	// ===============================

	long asLong() const
	{
		unsigned long value;
		assertD(topdigit==0);
		value=firstblock.digitarray[0];
		if(negativeFlag)
		{
			assertD(value<=0x80000000);
			if(value==0x80000000)
				return 0x80000000;
			return -(long)value;
		}
		else
		{
			assertD(value<0x80000000);
			return (long)value;
		}
	}
	bool isValidLong() const
	{
		unsigned long value;
		if(topdigit)
			return false;
		value=firstblock.digitarray[0];
		if(negativeFlag)
			return(value<=0x80000000);
		else
			return(value<0x80000000);
	}

    float asFloat() const;

	bool isZero() const
	{
		return topdigit==0 && firstblock.digitarray[0]==0;
	}
	bool isNegative() const
	{
		return negativeFlag;
	}
	void negate()
	{
		if(!isZero())
			negativeFlag=!negativeFlag;
	}
	bool isEven() const
	{
		return (firstblock.digitarray[0]&1)==0;
	}
	bool isIdentity() const
	{
		return !negativeFlag && topdigit==0 && firstblock.digitarray[0]==1;
	}
	void shiftright();
	void shiftleft();
	void shiftbitsright(int bits);
	void shiftbitsleft(int bits);
	void shiftdigitright();
	void shiftdigitleft();

	friend bool absoluteValueGreaterThan (const cDynamicInteger &s1, const cDynamicInteger &s2);
	bool divideBy(const cDynamicInteger &di);
	friend cDynamicInteger highestcommonfactor(const cDynamicInteger &d1, const cDynamicInteger &d2);
};

cDynamicInteger operator+(const cDynamicInteger &di1,const cDynamicInteger &di2);
cDynamicInteger operator-(const cDynamicInteger &di1,const cDynamicInteger &di2);

void cofactors(const cDynamicInteger &di1, const cDynamicInteger &di2, cDynamicInteger &factor1, cDynamicInteger &factor2);

