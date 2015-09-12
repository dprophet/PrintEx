/********************************************************************************
        Copyright 2015 Christopher Andrews.
        https://github.com/Chris--A/PrintEx

        Released under MIT licence.
********************************************************************************/

// Requires: 
// Globals.h
// TypeTraits.h
#include "NonStreamingIO.h"

#ifndef HEADER_ISTREAM
    #define HEADER_ISTREAM
	
	namespace ios{
	
		#ifndef ISCPP11
			template< typename derived > struct IStreamBase{};
		#else
		
		template<typename derived>
			struct IStream{
			
				IStream( derived &host ) : input(host) {}

				template< typename T >
					typename enable_if<is_integer<T>::value, IStream>::type operator>> ( T &t ){
						t = input.parseInt();
						return *this;
				}
				
				template< typename T >
					typename enable_if<is_float<T>::value, IStream>::type operator>> ( T &t ){
						t = input.parseFloat();
						return *this;
				}
				
				IStream operator>> ( String &string ){
						string = input.readString();
						return *this;
				}

				//Direct buffer reading, will stop on timeout.
				//NonStreamingIO::len is modified to reflect the number of characters read.
				template< typename T >
					typename enable_if<is_base_of<NonStreamingIO,T>::value, IStream>::type operator>> ( T &t ){
						int len = t.len;
						t.len = 0;
						char readBuff;
						for( int index = 0 ; index < len ; ++index ){
							if( !input.readBytes(&readBuff, 1) ) break;
							t.write(readBuff);
						}
						return *this;
				}
				
				derived &input;
		};
		//Entry point for an IStream chain.
		template< typename derived > 
			struct IStreamBase{
				template< typename T >
					IStream<derived> operator>> ( T &data ){
						IStream<derived> is( *static_cast<derived*>( this ) );
						return is >> data;
				}
		};
		#endif
	};
	
#endif