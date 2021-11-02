#ifndef HEAPVECTOR_H
#define HEAPVECTOR_H

#define heapvector HeapVector

class ArrayIndexOutOfBounds { };

template <class Object>
class heapvector
{
  public:
    explicit heapvector( int theSize = 0 ) : currentSize( theSize )
      { objects = new Object[ currentSize ]; }
    heapvector( const heapvector & rhs ) : objects( NULL )
      { operator=( rhs ); }
    ~heapvector( )
#ifndef WIN32
      { delete [ ] objects; }
#else
      { if( currentSize != 0 ) delete [ ] objects; }
#endif

    int size( ) const
      { return currentSize; }

    Object & operator[]( int index )
    {
                                                     #ifndef NO_CHECK
        if( index < 0 || index >= currentSize )
            throw ArrayIndexOutOfBounds( );
                                                     #endif
        return objects[ index ];
    }

    const Object & operator[]( int index ) const
    {
                                                     #ifndef NO_CHECK
        if( index < 0 || index >= currentSize )
            throw ArrayIndexOutOfBounds( );
                                                     #endif
        return objects[ index ];
    }


    const heapvector & operator = ( const heapvector & rhs );
    void resize( int newSize );
  private:
    int currentSize;
    Object * objects;
};

#ifndef HEAPVECTOR_CPP_
#define HEAPVECTOR_CPP_

#include "heapvector.h"

template <class Object>
const heapvector<Object> & heapvector<Object>::operator=( const heapvector<Object> & rhs )
{
    if( this != &rhs )
    {
#ifdef WIN32
      if( currentSize != 0 )
#endif
        delete [ ] objects;
        currentSize = rhs.size( );
        objects = new Object[ currentSize ];
        for( int k = 0; k < currentSize; k++ )
            objects[ k ] = rhs.objects[ k ];
    }
    return *this;
}

template <class Object>
void heapvector<Object>::resize( int newSize )
{
    Object *oldArray = objects;
    int numToCopy = newSize < currentSize ? newSize : currentSize;

    objects = new Object[ newSize ];

    for( int k = 0; k < numToCopy; k++ )
        objects[ k ] = oldArray[ k ];

#ifdef WIN32
  if( currentSize != 0 )
#endif
    delete [ ] oldArray;
    currentSize = newSize;
}

#endif

#endif


