#ifndef VECTOR_H
#define VECTOR_H

#define vector Vector

class ArrayIndexOutOfBounds { };

template <class Object>
class vector
{
  public:
    explicit vector( int theSize = 0 ) : currentSize( theSize )
      { objects = new Object[ currentSize ]; }
    vector( const vector & rhs ) : objects( NULL )
      { operator=( rhs ); }
    ~vector( )
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


    const vector & operator = ( const vector & rhs );
    void resize( int newSize );
  private:
    int currentSize;
    Object * objects;
};

#ifndef VECTOR_CPP_
#define VECTOR_CPP_

#include "vector.h"

template <class Object>
const vector<Object> & vector<Object>::operator=( const vector<Object> & rhs )
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
void vector<Object>::resize( int newSize )
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


