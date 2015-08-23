/********************************************************************************
    Copyright 2015 Christopher Andrews.
    https://github.com/Chris--A/PrintEx

    Released under MIT licence.
********************************************************************************/

    #include "MemoryPrinter.h"

    #define CHAR_PERCENT       '%'
    #define CHAR_NULL          '\0'
    #define CHAR_MINUS         '-'
    #define CHAR_ZERO          '0'
    #define CHAR_NINE          '9'
    #define CHAR_SPACE         ' '
    #define CHAR_STAR          '*'
    #define CHAR_HASH          '#'

    #define CHAR_s             's'
    #define CHAR_d             'd'
    #define CHAR_i             'i'
    #define CHAR_F             'F'
    #define CHAR_f             'f'
    #define CHAR_x             'x'
    #define CHAR_X             'X'
    #define CHAR_u             'u'
    #define CHAR_c             'c'
    #define CHAR_l             'l'
    #define CHAR_p             'p'
    #define CHAR_r             'r'
    #define CHAR_n             'n'

    #define PAD_RIGHT           1
    #define PAD_ZERO            2

    #define PRINTF_CONVERT_BUFFER_LEN   24
    #define PRINTF_ERROR_MESSAGE        "Error" //F("Error") may be used also.

    /****************************************************************
        sprintf function.
            This is a replacement for the globally accessible 
            sprintf. It incorporates the features found in 
            PrintExtension::printf().
    ****************************************************************/
    
    #ifndef PRINTEX_NO_SPRINTF
        int sprintf( char * str, const char * format, ... ){

            GString buf = str;

            va_list vList;
            va_start( vList, format );
            const pft p_Return = buf.printf( format, vList );
            va_end( vList );
            buf.end();
            return p_Return;
        }
    #endif
    
    /****************************************************************
        GetParam_XXX functions.
            The variations below proved to be far more efficient 
            than using va_arg() directly in printf() when compiled 
            on an AVR. 
    ****************************************************************/    

    uint32_t GetParam_uint( const va_list &vList, const bool b16 ){
        if( b16 )
            return va_arg( vList, unsigned int );
        else
            return va_arg( vList, uint32_t );
    }

    int GetParam_int( const va_list &vList ) __attribute__((noinline));
    int GetParam_int( const va_list &vList ){
        return va_arg( vList, int );
    }


    /****************************************************************
        cwrite function.
            Helper for printf.
    ****************************************************************/

    inline void PrintExtension::cwrite( uint8_t data, pfct &counter ){
        write( data );
        ++counter;
    }


    /****************************************************************
        sprintf, x.printf function.
            Prints formatted text.

            Formatting options use the following syntax:
                %[flags][width][length]specifier

            Flags:
                -:    Left-justify within the given field width; Right justification is the default.
                0:    When padding is specified, zeros are used instead of spaces.

            Width:
                (number):   Minimum number of characters to be printed.
                            If the value to be printed is shorter than this number,
                            the result is padded with blank spaces.
                            The value is not truncated even if the result is larger.

                *:  The width is not specified in the format string,
                    but as an additional integer value argument preceding
                    the argument that has to be formatted.

                Extra:

                    r:  When using 'r' to read the EEPROM the width is the number of
                        characters to read.

                    n:  Number of times to run repeat function.

            Length:
                l:  d, i use long instead of int.
                    u, x use unsigned long instead of unsigned int.
                    n    repeats a string.
            Specifier:
                s:    String ( null terminated ).
                p:    PROGMEM string. No formatting takes place, the string is printed directly.
                r:    EEPROM string. No formatting takes place, the string is printed directly.
                d:    Signed decimal integer ( 32bits max ).
                i:    Same as 'd'.
                u:    Unsigned decimal integer ( 32bits max ).
                f:    Decimal floating point number.
                x:    Unsigned decimal integer ( 32bits max ).
                c:    Character.
                n:    Repeat function ( default character, see length ).
                %:    Escape character for printing '%'
    ****************************************************************/

    pft PrintExtension::printf( const char *format, ... ){
        va_list vList;
        va_start( vList, format );
        const pft p_Return = _printf( format, vList );
        va_end( vList );
        return p_Return;
    }


    pft PrintExtension::_printf( const char *format, const va_list &vList ){
        unsigned int   width;
        pfct           pad;
        pfct           counter = 0x00;
        unsigned char  buffer[ PRINTF_CONVERT_BUFFER_LEN ];            //Buffer for conversions.

        for( ; *format ; ++format ){

            if( *format == CHAR_PERCENT ){

                ++format;
                width = pad = 0x00;

                bool largeType = false;

                if( *format == CHAR_NULL )
                    --format;
                else if( *format == CHAR_PERCENT )
                    cwrite( *format, counter );
                else{

                    //Check for left justify.
                    if( *format == CHAR_MINUS ){
                        ++format;
                        pad = PAD_RIGHT;
                    }

                    //Check for '0' pad specifier.
                    if( *format == CHAR_ZERO ){
                        ++format;
                        pad |= PAD_ZERO;
                    }

                    //Check for passed length.
                    #ifndef PRINTF_NO_WIDTH_PARAM
                        if( *format == CHAR_STAR ){
                            ++format;
                            width = ( unsigned int ) GetParam_int( vList );//va_arg( vList, int );

                        //Width provided in format specifier.
                        }else
                    #endif

                    //Calculate padding width.
                    for ( ; *format >= CHAR_ZERO && *format <= CHAR_NINE; ++format ){
                        width *= 10;
                        width += *format - CHAR_ZERO;
                    }

                    char    padChar = ( pad & PAD_ZERO ) ? CHAR_ZERO : CHAR_SPACE;
                    GString convertStr( buffer );

                    //Get length flag if for larger types.
                    if( *format == CHAR_l ){
                        largeType = true;
                        ++format;
                    }

                    //Convert argument into a string.
                    if( *format == CHAR_s ){                                                    //String.
                        char *c_Parameter = ( char* ) GetParam_int( vList );
                        convertStr.assign( c_Parameter, strlen( c_Parameter ) );

                    #ifndef PRINTF_NO_PROGMEM
                        }else if( *format == CHAR_p ){
                            print( PString( ( void* ) GetParam_int( vList ) ) );
                            continue;
                    #endif

                    #ifndef PRINTF_NO_EEPROM
                        }else if( *format == CHAR_r ){
                            print( EString( ( void* ) GetParam_int( vList ), width ) );
                            continue;
                    #endif

                    #ifndef PRINTF_NO_REPEAT
                        }else if( *format == CHAR_n ){
                            const int i_Data = GetParam_int( vList );
                            while( width-- ){
                                if( largeType ) print( ( char* ) i_Data );
                                else            print( ( char ) i_Data );
                            }
                            continue;
                    #endif

                    }else if( *format == CHAR_d ||  *format == CHAR_i ){                         //Signed decimal integer.
                        if( largeType ) convertStr.print( va_arg( vList, int32_t ), DEC );
                        else            convertStr.print( ( int32_t ) GetParam_int( vList ), DEC );

                    }else if( *format == CHAR_u ){                                                //Unsigned integer.
                        if( largeType ) convertStr.print( GetParam_uint( vList, false ), DEC );
                        else            convertStr.print( ( uint32_t ) GetParam_uint( vList, true ), DEC );

                    #ifndef PRINTF_NO_FLOATING_POINT
                        }else if( *format == CHAR_f ){                                            //Floating point data.
                            convertStr.print( va_arg( vList, double ) );
                    #endif

                    }else if( *format == CHAR_x ){                                                //Unsigned hexidecimal integer.
                        if( largeType ) convertStr.print( GetParam_uint( vList, false ), HEX );
                        else            convertStr.print( ( uint32_t ) GetParam_uint( vList, true ), HEX );

                    }else if(  *format == CHAR_c ){                                                //Character.
                        *buffer = ( char ) GetParam_int( vList );
                        convertStr.assign( buffer, 1 ); //This is cheaper than printing.

                    #if ( defined( PRINTF_NO_EEPROM ) || defined( PRINTF_NO_FLOATING_POINT ) || defined( PRINTF_NO_PROGMEM ) ) && !defined( PRINTF_NO_ERROR_CONDITION )
                        }else
                            convertStr.print( PRINTF_ERROR_MESSAGE );
                    #else
                        }
                    #endif

                    //Calculate remaining padding.
                    const pft len = convertStr.count();
                    if( len >= width )    width = 0x00;
                    else                width -= len;

                    //Pad left if right justified.
                    if( !( pad & PAD_RIGHT ) ){
                        _repeat( padChar, width );
                        counter += width;
                        width = 0;
                    }

                    counter += write( ( unsigned char* ) ( char* ) convertStr, len );

                    //Pad right if left justified.
                    _repeat( padChar, width );
                    counter += width;
                }
            }else //Standard character, write out.
                cwrite( *format, counter );
        }
        return counter;
    }

    /****************************************************************
        _repeat function.
            Prints character, repeating count times.
            The public interface is 'repeat'.
    ****************************************************************/

    void PrintExtension::_repeat( const char &character, unsigned char count ) { while( count-- ) write( character ); }