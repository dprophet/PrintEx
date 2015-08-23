/********************************************************************************
    Copyright 2015 Christopher Andrews.
    https://github.com/Chris--A/PrintEx

    Released under MIT licence.
********************************************************************************/
/********************************************************************************
    PrintEx feature inclusion constants.

    Define these constants below to disable certain features.

    #define PRINTEX_NO_WIDTH_PARAM
    #define PRINTEX_NO_PROGMEM
    #define PRINTEX_NO_EEPROM
    #define PRINTEX_NO_FLOATING_POINT
    #define PRINTEX_NO_REPEAT
    #define PRINTEX_NO_ERROR_CONDITION
    #define PRINTEX_NO_SPRINTF
	#define PRINTEX_LARGE_COUNTER
********************************************************************************/

#ifndef HEADER_PRINTEXTENSION
    #define HEADER_PRINTEXTENSION

    #include "Arduino.h"

    #ifndef _INLINE_
        #define _INLINE_  __attribute__( ( always_inline ) ) inline
    #endif

    typedef size_t pft;
	
	#ifdef PRINTEX_LARGE_COUNTER
		typedef uint16_t pfct;
	#else
		typedef uint8_t pfct;
	#endif

    class PrintExtension : public Print{

        public:

            template< typename T >
                _INLINE_ PrintExtension &operator +=( const T &t )
                    { return concat( t ); }

            template< typename T >
                _INLINE_ PrintExtension &concat( const T &t )
                    { return print( t ), *this; }

            template< typename T >
                _INLINE_ PrintExtension &concat( const T &t, const int i )
                    { return print( t, i ) *this; }

            template< typename T >
                _INLINE_ PrintExtension &concatln( const T &t )
                    { return println( t ), *this; }

            template< typename T >
                _INLINE_ PrintExtension &concatln( const T &t, const int i )
                    { return println( t, i ), *this; }

            pft printf( const char *format, ... );
            

			PrintExtension &repeat( const char *str, unsigned char repeatCount ){
				return *this;
			}
			
			PrintExtension &repeatln( const char *str, unsigned char repeatCount ){
				repeat( str, repeatCount );
				write("\r\n");
				return *this;
			}
			
			PrintExtension &repeat( const char &character, unsigned char count )
				{ _repeat(character, count); return *this; }

			PrintExtension &repeatln( const char &character, unsigned char count ){ 
				_repeat(character, count);
				write("\r\n"); //Shouldn't allocate RAM for string as there is a copy already used with println();
				return *this;
			}			
        protected:
		
			void _repeat( const char &character, unsigned char count );
            pft _printf( const char *format, const va_list &v_List );
			
			// Global sprintf replacement. To keep old version define PRINTEX_NO_SPRINTF (does not affect x.printf();)
            #ifndef PRINTEX_NO_SPRINTF
                friend int sprintf( char * str, const char * format, ... );
            #endif

        private:
            void cwrite( uint8_t data, pfct &counter );
    };


    struct PrintEx : PrintExtension{
        PrintEx( Print& in ) : print(in)    { return; }
        size_t write( uint8_t data )        { return print.write( data ); }
        Print &print;
    };

    struct StreamEx : Stream, PrintExtension{
        StreamEx( Stream &in ) : stream(in) {}
        int available( void )               { return stream.available(); }
        void flush( void )                  { stream.flush(); }
        int peek( void )                    { return stream.peek(); }
        int read( void )                    { return stream.read(); }
        size_t write( uint8_t data )        { return stream.write(data); }
        Stream &stream;
    };

#endif