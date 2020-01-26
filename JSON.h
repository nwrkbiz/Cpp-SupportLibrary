/**
 * @file JSON.h
 * @brief Lightweight JSON library for exporting/importing data in JSON format from/to C++.
 * @author nbsdx (Neil)
 * @author Daniel Giritzer
 * @copyright "THE BEER-WARE LICENSE" (Revision 42):
 * <giri@nwrk.biz> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return Daniel Giritzer
 * 
 * Further informations:
 * ---------------------
 * 
 * Based on: https://github.com/nbsdx/SimpleJSON/
 * 
 * This version includes new features, 
 * performancefixes, documentation and bugfixes added by <giri@nwrk.biz>.
 * 
 * ### Original License:
 * Do what the fuck you want public license
 */
#ifndef SUPPORTLIB_JSON_H
#define SUPPORTLIB_JSON_H
#include <cstdint>
#include <cmath>
#include <cctype>
#include <string>
#include <deque>
#include <map>
#include <type_traits>
#include <initializer_list>
#include <ostream>
#include <iostream>
#include <charconv>

#include "Object.h"

namespace giri {
    /**
     * @brief Namespace containing all JSON related stuff.
     */
    namespace json {

        using std::map;
        using std::deque;
        using std::string;
        using std::enable_if;
        using std::initializer_list;
        using std::is_same;
        using std::is_convertible;
        using std::is_integral;
        using std::is_floating_point;

        namespace {
            string json_escape( const string &str ) {
                string output;
                for( unsigned i = 0; i < str.length(); ++i )
                    switch( str[i] ) {
                        case '\"': output += "\\\""; break;
                        case '\\': output += "\\\\"; break;
                        case '\b': output += "\\b";  break;
                        case '\f': output += "\\f";  break;
                        case '\n': output += "\\n";  break;
                        case '\r': output += "\\r";  break;
                        case '\t': output += "\\t";  break;
                        default  : output += str[i]; break;
                    }
                return output;
            }
        }

        /**
         * @brief Class to represent and use JSON objects.
         * 
         * Example Usage:
         * --------------
         * 
         * ### Array Example ###
         * 
         * Example to show how to use Arrays.
         * 
         * @code{.cpp}
         * #include <JSON.h>
         * #include <iostream>
         * 
         * using giri::json::JSON;
         * using namespace std;
         * 
         * int main()
         * {
         *     JSON array;
         * 
         *     array[2] = "Test2";
         *     cout << array << endl;
         *     array[1] = "Test1";
         *     cout << array << endl;
         *     array[0] = "Test0";
         *     cout << array << endl;
         *     array[3] = "Test4";
         *     cout << array << endl;
         * 
         *     // Arrays can be nested:
         *     JSON Array2;
         * 
         *     Array2[2][0][1] = true;
         * 
         *     cout << Array2 << endl;
         * }
         * @endcode
         * 
         * ### Initialization Example ###
         * 
         * Simple example which shows how to directly load an object.
         * 
         * @code{.cpp}
         * #include <JSON.h>
         * #include <iostream>
         * #include <cstddef>
         * 
         * using giri::json::JSON;
         * using namespace std;
         * 
         * int main()
         * {
         *     JSON obj( {
         *         "Key", 1,
         *         "Key3", true,
         *         "Key4", nullptr,
         *         "Key2", {
         *             "Key4", "VALUE",
         *             "Arr", json::Array( 1, "Str", false )
         *         }
         *     } );
         * 
         *     cout << obj << endl;
         * }
         * @endcode
         * 
         * ### Iterator Example ###
         * 
         * This example shows how to iterate over stored arrays and objects.
         * 
         * @code{.cpp}
         * #include <JSON.h>
         * #include <iostream>
         * 
         * using giri::json::JSON;
         * using namespace std;
         * 
         * void dumpArrayConst( const JSON &array ) {
         *     for( auto &j : array.ArrayRange() )
         *         std::cout << "Value: " << j << "\n";
         * }
         * 
         * void dumpArray( JSON &array ) {
         *     for( auto &j : array.ArrayRange() )
         *         std::cout << "Value: " << j << "\n";
         * }
         * 
         * void dumpObjectConst( const JSON &object ) {
         *     for( auto &j : object.ObjectRange() )
         *         std::cout << "Object[ " << j.first << " ] = " << j.second << "\n";
         * }
         * 
         * void dumpObject( JSON &object ) {
         *     for( auto &j : object.ObjectRange() )
         *         std::cout << "Object[ " << j.first << " ] = " << j.second << "\n";
         * }
         * 
         * int main()
         * {
         *     JSON array = JSON::Make( JSON::Class::Array );
         *     JSON obj   = JSON::Make( JSON::Class::Object );
         * 
         *     array[0] = "Test0";
         *     array[1] = "Test1";
         *     array[2] = "Test2";
         *     array[3] = "Test3";
         * 
         *     obj[ "Key0" ] = "Value1";
         *     obj[ "Key1" ] = array;
         *     obj[ "Key2" ] = 123;
         * 
         * 
         *     std::cout << "=============== tests ================\n";
         *     dumpArray( array );
         *     dumpObject( obj );
         * 
         *     std::cout << "============ const tests =============\n";
         *     dumpArrayConst( array );
         *     dumpObjectConst( obj );
         * }
         * @endcode
         * 
         * ### Datatypes Example ###
         * 
         * In this example it is shown, how to store all supported datatypes within an JSON object.
         * 
         * @code{.cpp}
         * #include <JSON.h>
         * #include <iostream>
         * 
         * using giri::json::JSON;
         * using namespace std;
         * 
         * int main()
         * {
         *     // Example of creating each type
         *     // You can also do JSON::Make( JSON::Class )
         *     JSON null;
         *     JSON Bool( true );
         *     JSON Str( "RawString" );
         *     JSON Str2( string( "C++String" ) );
         *     JSON Int( 1 );
         *     JSON Float( 1.2 );
         *     JSON Arr = json::Array();
         *     JSON Obj = json::Object();
         * 
         *     // Types can be overwritten by assigning
         *     // to the object again.
         *     Bool = false;
         *     Bool = "rtew";
         *     Bool = 1;
         *     Bool = 1.1;
         *     Bool = string( "asd" );
         * 
         *     // Append to Arrays, appending to a non-array
         *     // will turn the object into an array with the
         *     // first element being the value that's being
         *     // appended.
         *     Arr.append( 1 );
         *     Arr.append( "test" );
         *     Arr.append( false );
         * 
         *     // Access Array elements with operator[]( unsigned ).
         *     // Note that this does not do bounds checking, and 
         *     // returns a reference to a JSON object.
         *     JSON& val = Arr[0];
         * 
         *     // Arrays can be intialized with any elements and
         *     // they are turned into JSON objects. Variadic 
         *     // Templates are pretty cool.
         *     JSON Arr2 = json::Array( 2, "Test", true );
         * 
         *     // Objects are accessed using operator[]( string ).
         *     // Will create new pairs on the fly, just as std::map
         *     // would.
         *     Obj["Key1"] = 1.0;
         *     Obj["Key2"] = "Value";
         * 
         *     JSON Obj2 = json::Object();
         *     Obj2["Key3"] = 1;
         *     Obj2["Key4"] = Arr;
         *     Obj2["Key5"] = Arr2;
         *     
         *     // Nested Object
         *     Obj["Key6"] = Obj2;
         * 
         *     // Dump Obj to a string.
         *     cout << Obj << endl;
         * 
         *     // We can also use a more JSON-like syntax to create 
         *     // JSON Objects.
         *     JSON Obj3 = {
         *         "Key1", "Value",
         *         "Key2", true,
         *         "Key3", {
         *             "Key4", json::Array( "This", "Is", "An", "Array" ),
         *             "Key5", {
         *                 "BooleanValue", true
         *             }
         *         }
         *     };
         * 
         *     cout << Obj3 << endl;
         * }
         * @endcode
         * 
         * ### Load string Example ###
         * 
         * This example shows how to load an object from string.
         * 
         * @code{.cpp}
         * #include <JSON.h>
         * #include <iostream>
         * 
         * using giri::namespace std;
         * using json::JSON;
         * 
         * int main()
         * {
         *     JSON Int = JSON::Load( " 123 " );
         *     JSON Float = JSON::Load( " 123.234 " );
         *     JSON Str = JSON::Load( "\"String\"" );
         *     JSON EscStr = JSON::Load( "\" \\\"Some\\/thing\\\" \"" );
         *     JSON Arr = JSON::Load( "[1,2, true, false,\"STRING\", 1.5]" );
         *     JSON Obj = JSON::Load( "{ \"Key\" : \"StringValue\","
         *                            "   \"Key2\" : true, "
         *                            "   \"Key3\" : 1234, "
         *                            "   \"Key4\" : null }" );
         *     
         *     cout << Int << endl;
         *     cout << Float << endl;
         *     cout << Str << endl;
         *     cout << EscStr << endl;
         *     cout << Arr << endl;
         *     cout << Obj << endl;
         * 
         * }
         * @endcode
         * 
         * ### Assignment of primitives Example ###
         * 
         * Assign and print primitives.
         * 
         * @code{.cpp}
         * #include <JSON.h>
         * #include <iostream>
         * #include <ios>
         * 
         * using giri::json::JSON;
         * using namespace std;
         * 
         * int main()
         * {
         *     JSON obj;
         * 
         *     obj = true;
         *     cout << "Value: " << boolalpha << obj.ToBool() << endl;
         * 
         *     obj = "Test String";
         *     cout << "Value: " << obj.ToString() << endl;
         * 
         *     obj = 2.2;
         *     cout << "Value: " << obj.ToFloat() << endl;
         * 
         *     obj = 3;
         *     cout << "Value: " << obj.ToInt() << endl;
         *     
         * }
         * @endcode
         */
        class JSON final : public Object<JSON>
        {
            union BackingData {
                BackingData( double d ) : Float( d ){}
                BackingData( long long   l ) : Int( l ){}
                BackingData( bool   b ) : Bool( b ){}
                BackingData( string s ) : String( new string( s ) ){}
                BackingData()           : Int( 0 ){}

                deque<JSON>        *List;
                map<string,JSON>   *Map;
                string             *String;
                double              Float;
                long long           Int;
                bool                Bool;
            } Internal;

            public:
                enum class Class {
                    Null,
                    Object,
                    Array,
                    String,
                    Floating,
                    Integral,
                    Boolean
                };

                template <typename Container>
                class JSONWrapper {
                    Container *object;

                    public:
                        JSONWrapper( Container *val ) : object( val ) {}
                        JSONWrapper( std::nullptr_t )  : object( nullptr ) {}

                        typename Container::iterator begin() { return object ? object->begin() : typename Container::iterator(); }
                        typename Container::iterator end() { return object ? object->end() : typename Container::iterator(); }
                        typename Container::const_iterator begin() const { return object ? object->begin() : typename Container::iterator(); }
                        typename Container::const_iterator end() const { return object ? object->end() : typename Container::iterator(); }
                };

                template <typename Container>
                class JSONConstWrapper {
                    const Container *object;

                    public:
                        JSONConstWrapper( const Container *val ) : object( val ) {}
                        JSONConstWrapper( std::nullptr_t )  : object( nullptr ) {}

                        typename Container::const_iterator begin() const { return object ? object->begin() : typename Container::const_iterator(); }
                        typename Container::const_iterator end() const { return object ? object->end() : typename Container::const_iterator(); }
                };

                JSON() : Internal(), Type( Class::Null ){}

                explicit JSON(Class type): JSON() { SetType( type ); }

                JSON( initializer_list<JSON> list ) 
                    : JSON() 
                {
                    SetType( Class::Object );
                    for( auto i = list.begin(), e = list.end(); i != e; ++i, ++i )
                        operator[]( i->ToString() ) = *std::next( i );
                }

                JSON( JSON&& other )
                    : Internal( other.Internal )
                    , Type( other.Type )
                { other.Type = Class::Null; other.Internal.Map = nullptr; }

                JSON& operator=( JSON&& other ) {
                    ClearInternal();
                    Internal = other.Internal;
                    Type = other.Type;
                    other.Internal.Map = nullptr;
                    other.Type = Class::Null;
                    return *this;
                }

                JSON( const JSON &other ) {
                    switch( other.Type ) {
                    case Class::Object:
                        Internal.Map = 
                            new map<string,JSON>( other.Internal.Map->begin(),
                                                other.Internal.Map->end() );
                        break;
                    case Class::Array:
                        Internal.List = 
                            new deque<JSON>( other.Internal.List->begin(),
                                            other.Internal.List->end() );
                        break;
                    case Class::String:
                        Internal.String = 
                            new string( *other.Internal.String );
                        break;
                    default:
                        Internal = other.Internal;
                    }
                    Type = other.Type;
                }

                JSON& operator=( const JSON &other ) {
                    if (&other == this) return *this;
                    ClearInternal();
                    switch( other.Type ) {
                    case Class::Object:
                        Internal.Map = 
                            new map<string,JSON>( other.Internal.Map->begin(),
                                                other.Internal.Map->end() );
                        break;
                    case Class::Array:
                        Internal.List = 
                            new deque<JSON>( other.Internal.List->begin(),
                                            other.Internal.List->end() );
                        break;
                    case Class::String:
                        Internal.String = 
                            new string( *other.Internal.String );
                        break;
                    default:
                        Internal = other.Internal;
                    }
                    Type = other.Type;
                    return *this;
                }

                ~JSON() {
                    switch( Type ) {
                    case Class::Array:
                        delete Internal.List;
                        break;
                    case Class::Object:
                        delete Internal.Map;
                        break;
                    case Class::String:
                        delete Internal.String;
                        break;
                    default:;
                    }
                }

                template <typename T>
                JSON( T b, typename enable_if<is_same<T,bool>::value>::type* = 0 ) : Internal( b ), Type( Class::Boolean ){}

                template <typename T>
                JSON( T i, typename enable_if<is_integral<T>::value && !is_same<T,bool>::value>::type* = 0 ) : Internal( (long long)i ), Type( Class::Integral ){}

                template <typename T>
                JSON( T f, typename enable_if<is_floating_point<T>::value>::type* = 0 ) : Internal( (double)f ), Type( Class::Floating ){}

                template <typename T>
                JSON( T s, typename enable_if<is_convertible<T,string>::value>::type* = 0 ) : Internal( string( s ) ), Type( Class::String ){}

                JSON( std::nullptr_t ) : Internal(), Type( Class::Null ){}

                /**
                 * Creates a new JSON Object.
                 * @param type Class type to create.
                 * @returns JSON object of given class type.
                 */
                static JSON Make( Class type ) {
                    return JSON(type);
                }

                /**
                 * Create a JSON object from string.
                 * @param str JSON string to parse and load.
                 * @returns New JSON object representing the json defined by the parsed string.
                 */
                static JSON Load( const string & );

                /**
                 * Create a JSON object from string.
                 * @param str JSON string to parse and load.
                 * @param ok [OUT] Output parameter giving feedback if parsing was successful.
                 * true on success, false otherwise.
                 * @returns New JSON object representing the json defined by the parsed string.
                 */
                static JSON Load( const string & , bool &);

                /**
                 * Allows appending items to array. Appending to a non-array will turn the object into an array with the
                 * first element being the value that's being appended.
                 * @param arg Item to append.
                 */
                template <typename T>
                void append( T arg ) {
                    SetType( Class::Array ); Internal.List->emplace_back( arg );
                }

                /**
                 * Allows appending items to array. Appending to a non-array will turn the object into an array with the
                 * first element being the value that's being appended.
                 * @param arg Item to append.
                 * @param args Further items to append.
                 */
                template <typename T, typename... U>
                void append( T arg, U... args ) {
                    append( arg ); append( args... );
                }

                template <typename T>
                    typename enable_if<is_same<T,bool>::value, JSON&>::type operator=( T b ) {
                        SetType( Class::Boolean ); Internal.Bool = b; return *this;
                    }

                template <typename T>
                    typename enable_if<is_integral<T>::value && !is_same<T,bool>::value, JSON&>::type operator=( T i ) {
                        SetType( Class::Integral ); Internal.Int = i; return *this;
                    }

                template <typename T>
                    typename enable_if<is_floating_point<T>::value, JSON&>::type operator=( T f ) {
                        SetType( Class::Floating ); Internal.Float = f; return *this;
                    }

                template <typename T>
                    typename enable_if<is_convertible<T,string>::value, JSON&>::type operator=( T s ) {
                        SetType( Class::String ); *Internal.String = string( s ); return *this;
                    }

                /**
                 * Allows accessing and creating object entries by key.
                 * @param key Key to access, will be created if not existent.
                 * @returns The object stored at key.
                 */
                JSON& operator[]( const string &key ) {
                    SetType( Class::Object ); return Internal.Map->operator[]( key );
                }

                /**
                 * Allows accessing and creating array entries by index.
                 * @param index Index to access, will be created if not existent.
                 * @returns The object stored at index.
                 */
                JSON& operator[]( unsigned index ) {
                    SetType( Class::Array );
                    if( index >= Internal.List->size() ) Internal.List->resize( index + 1 );
                    return Internal.List->operator[]( index );
                }

                /**
                 * Allows getting an object entry by key.
                 * @param key Key to access.
                 * @returns object entry by key.
                 */
                JSON &at( const string &key ) {
                    return operator[]( key );
                }

                /**
                 * Allows getting an object entry by key.
                 * @param key Key to access.
                 * @returns object entry by key.
                 */
                const JSON &at( const string &key ) const {
                    return Internal.Map->at( key );
                }

                /**
                 * Allows getting an array entry by index.
                 * @param index Index to access.
                 * @returns array entry by index.
                 */
                JSON &at( unsigned index ) {
                    return operator[]( index );
                }

                /**
                 * Allows getting an array entry by index.
                 * @param index Index to access.
                 * @returns array entry by index.
                 */
                const JSON &at( unsigned index ) const {
                    return Internal.List->at( index );
                }

                /**
                 * @returns The number of items stored within an Array. -1 if 
                 * class type is not Array.
                 */
                int length() const {
                    if( Type == Class::Array )
                        return static_cast<int>(Internal.List->size());
                    else
                        return -1;
                }

                /**
                 * @param key Key to check.
                 * @returns true if the object holds a item with the given key, false otherwise.
                 */
                bool hasKey( const string &key ) const {
                    if( Type == Class::Object )
                        return Internal.Map->find( key ) != Internal.Map->end();
                    return false;
                }

                /**
                 * @returns The number of items stored within an array or object. -1 if 
                 * class type is neither array nor object.
                 */
                int size() const {
                    if( Type == Class::Object )
                        return static_cast<int>(Internal.Map->size());
                    else if( Type == Class::Array )
                        return static_cast<int>(Internal.List->size());
                    else
                        return -1;
                }

                /**
                 * @returns Class type of the object.
                 */
                Class JSONType() const { return Type; }


                /**
                 * @returns true if the object is Null, false otherwise.
                 */
                bool IsNull() const { return Type == Class::Null; }

                /**
                 * @returns true if the object is Array, false otherwise.
                 */
                bool IsArray() const { return Type == Class::Array; }

                /**
                 * @returns true if the object is Boolean, false otherwise.
                 */
                bool IsBoolean() const { return Type == Class::Boolean; }

                /**
                 * @returns true if the object is Floating, false otherwise.
                 */
                bool IsFloating() const { return Type == Class::Floating; }

                /**
                 * @returns true if the object is Integral, false otherwise.
                 */
                bool IsIntegral() const { return Type == Class::Integral; }

                /**
                 * @returns true if the object is String, false otherwise.
                 */
                bool IsString() const { return Type == Class::String; }

                /**
                 * @returns true if the object is Object, false otherwise.
                 */
                bool IsObject() const { return Type == Class::Object; }

                /**
                 * @returns If class type is String, the stored value. If class type is
                 * Null, Object, Array, Boolean, Floating or Integral a conversion will be tried. Empty string otherwise
                 * or on conversion error.
                 */
                string ToString() const { bool b; return ToString( b ); }

                /**
                 * @param ok [OUT] Output parameter giving feedback if the conversion was successful.
                 * @returns If class type is String, the stored value. If class type is
                 * Null, Object, Array, Boolean, Floating or Integral a conversion will be tried. Empty string otherwise
                 * or on conversion error.
                 */
                string ToString( bool &ok ) const {
                    ok = (Type == Class::String);
                    if(ok)
                        return json_escape( *Internal.String );

                    ok = (Type == Class::Object);
                    if(ok)
                        return dumpMinified();

                    ok = (Type == Class::Array);
                    if(ok)
                        return dumpMinified();

                    ok = (Type == Class::Boolean);
                    if(ok)
                        return Internal.Bool ? string("true") : string("false");
                    
                    ok = (Type == Class::Floating);
                    if(ok)
                        return std::to_string(Internal.Float);

                    ok = (Type == Class::Integral);
                    if(ok)
                        return std::to_string(Internal.Int);

                    ok = (Type == Class::Null);
                    if(ok)
                        return string("null");

                    return string("");
                }

                /**
                 * Useful if json objects are stored within the json as string.
                 * @returns If class type is String, the stored value without escaping. If class type is
                 * Null, Object, Array, Boolean, Floating or Integral a conversion will be tried. Empty string otherwise
                 * or on conversion error.
                 */
                string ToUnescapedString() const { bool b; return ToUnescapedString( b ); }

                /**
                 * Useful if json objects are stored within the json as string.
                 * @param ok [OUT] Output parameter giving feedback if the conversion was successful.
                 * @returns If class type is String, the stored value without escaping. If class type is
                 * Null, Object, Array, Boolean, Floating or Integral a conversion will be tried. Empty string otherwise
                 * or on conversion error.
                 */
                string ToUnescapedString( bool &ok ) const {
                    ok = (Type == Class::String);
                    if(ok)
                        return std::string( *Internal.String );
                    
                    ok = (Type == Class::Object);
                    if(ok)
                        return dumpMinified();

                    ok = (Type == Class::Array);
                    if(ok)
                        return dumpMinified();

                    ok = (Type == Class::Boolean);
                    if(ok)
                        return Internal.Bool ? string("true") : string("false");
                    
                    ok = (Type == Class::Floating);
                    if(ok)
                        return std::to_string(Internal.Float);

                    ok = (Type == Class::Integral);
                    if(ok)
                        return std::to_string(Internal.Int);

                    ok = (Type == Class::Null);
                    if(ok)
                        return string("null");

                    return string("");
                }

                /**
                 * @returns If class type is Integral, Floating or Boolean, the stored value. If the class type is
                 * String, an conversion will be tried. 0.0 otherwise or on conversion error.
                 */
                double ToFloat() const { bool b; return ToFloat( b ); }


                /**
                 * @param ok [OUT] Output parameter giving feedback if the conversion was successful.
                 * @returns If class type is Integral, Floating or Boolean, the stored value. If the class type is
                 * String, an conversion will be tried. 0.0 otherwise or on conversion error.
                 */
                double ToFloat( bool &ok ) const {
                    ok = (Type == Class::Floating);
                    if (ok)
                        return Internal.Float;

                    ok = (Type == Class::Boolean);
                    if(ok) 
                        return Internal.Bool;
                    
                    ok = (Type == Class::Integral);
                    if (ok)
                        return Internal.Int;

                    ok = (Type == Class::String);
                    if (ok)
                    {
                        double parsed;
                        try {
                            parsed = std::stod(*Internal.String);
                        }
                        catch(const std::invalid_argument &e) {
                            std::cerr << "Error: Parsing float failed: " << e.what() << std::endl;
                            ok = false;
                        }
                        catch(const std::out_of_range &e) {
                            std::cerr << "Error: Parsing float failed: " << e.what() << std::endl;
                            ok = false;
                        }
                        if(ok)
                            return parsed;
                    }
                    return 0.0;
                }

                /**
                 * @returns If class type is Integral, Floating or Boolean, the stored value. If the class type is
                 * String, an conversion will be tried. 0 otherwise or on conversion error.
                 */
                long long ToInt() const { bool b; return ToInt( b ); }

                /**
                 * @param ok [OUT] Output parameter giving feedback if the conversion was successful.
                 * @returns If class type is Integral, Floating or Boolean, the stored value. If the class type is
                 * String, an conversion will be tried. 0 otherwise or on conversion error.
                 */
                long long ToInt( bool &ok ) const {
                    ok = (Type == Class::Integral);
                    if (ok)
                        return Internal.Int;

                    ok = (Type == Class::Boolean);
                    if(ok) 
                        return Internal.Bool;
                        
                    ok = (Type == Class::Floating);
                    if (ok)
                        return Internal.Float;

                    ok = (Type == Class::String);
                    if (ok)
                    {
                        long long parsed;
                        std::from_chars_result result = std::from_chars(Internal.String->data(), Internal.String->data() + Internal.String->size(), parsed);
                        if(!(bool)result.ec)
                            return parsed;
                        ok = false;
                    }

                    return 0;
                }

                /**
                 * @returns If class type is Integral, Floating or Boolean, the stored value. If the class type is
                 * String, an conversion will be tried. false otherwise or on conversion error.
                 */
                bool ToBool() const { bool b; return ToBool( b ); }

                /**
                 * @param ok [OUT] Output parameter giving feedback if the conversion was successful.
                 * @returns If class type is Integral, Floating or Boolean, the stored value. If the class type is
                 * String, an conversion will be tried. false otherwise or on conversion error.
                 */
                bool ToBool( bool &ok ) const {
                    ok = (Type == Class::Boolean);
                    if(ok) 
                        return Internal.Bool;
                    
                    ok = (Type == Class::Integral);
                    if (ok)
                        return Internal.Int;

                    ok = (Type == Class::Floating);
                    if (ok)
                        return Internal.Float;

                    ok = (Type == Class::String);
                    if (ok)
                    {
                        if(Internal.String->find("true")!=std::string::npos)
                            return true;
                        if(Internal.String->find("false")!=std::string::npos)
                            return false;
                        int parsed;
                        std::from_chars_result result = std::from_chars(Internal.String->data(), Internal.String->data() + Internal.String->size(), parsed);
                        if(!(bool)result.ec)
                            return parsed;
                        ok = false;
                    }

                    return false;
                }

                /**
                 * Returns Object range which allows iterating over the object items.
                 * @returns Object range which allows iterating over the object items.
                 */
                JSONWrapper<map<string,JSON>> ObjectRange() {
                    if( Type == Class::Object )
                        return JSONWrapper<map<string,JSON>>( Internal.Map );
                    return JSONWrapper<map<string,JSON>>( nullptr );
                }

                /**
                 * Returns Array range which allows iterating over the array items.
                 * @returns Array range which allows iterating over the array items.
                 */
                JSONWrapper<deque<JSON>> ArrayRange() {
                    if( Type == Class::Array )
                        return JSONWrapper<deque<JSON>>( Internal.List );
                    return JSONWrapper<deque<JSON>>( nullptr );
                }

                /**
                 * Returns Object range which allows iterating over the object items.
                 * @returns Object range which allows iterating over the object items.
                 */
                JSONConstWrapper<map<string,JSON>> ObjectRange() const {
                    if( Type == Class::Object )
                        return JSONConstWrapper<map<string,JSON>>( Internal.Map );
                    return JSONConstWrapper<map<string,JSON>>( nullptr );
                }

                /**
                 * Returns Array range which allows iterating over the array items.
                 * @returns Array range which allows iterating over the array items.
                 */
                JSONConstWrapper<deque<JSON>> ArrayRange() const { 
                    if( Type == Class::Array )
                        return JSONConstWrapper<deque<JSON>>( Internal.List );
                    return JSONConstWrapper<deque<JSON>>( nullptr );
                }

                /**
                 * Returns the whole json object as formatted string.
                 * @param depth number of indentation per level (defaults to 1)
                 * @param tab indentation character(s) (defaults to two spaces)
                 * @returns json object as formatted string.
                 */ 
                string dump( int depth = 1, string tab = "  ") const {
                    switch( Type ) {
                        case Class::Null:
                            return "null";
                        case Class::Object: {
                            string pad = "";
                            for( int i = 0; i < depth; ++i, pad += tab );
                            string s = "{\n";
                            bool skip = true;
                            for( auto &p : *Internal.Map ) {
                                if( !skip ) s += ",\n";
                                s += ( pad + "\"" + p.first + "\" : " + p.second.dump( depth + 1, tab ) );
                                skip = false;
                            }
                            s += ( "\n" + pad.erase( 0, tab.size() ) + "}" ) ;
                            return s;
                        }
                        case Class::Array: {
                            string s = "[";
                            bool skip = true;
                            for( auto &p : *Internal.List ) {
                                if( !skip ) s += ", ";
                                s += p.dump( depth + 1, tab );
                                skip = false;
                            }
                            s += "]";
                            return s;
                        }
                        case Class::String:
                            return "\"" + json_escape( *Internal.String ) + "\"";
                        case Class::Floating:
                            return std::to_string( Internal.Float );
                        case Class::Integral:
                            return std::to_string( Internal.Int );
                        case Class::Boolean:
                            return Internal.Bool ? "true" : "false";
                        default:
                            return "";
                    }
                    return "";
                }

                /**
                 * Returns the whole json object as minified string.
                 * @returns json object as minified string.
                 */
                string dumpMinified() const {
                    switch( Type ) {
                        case Class::Null:
                            return "null";
                        case Class::Object: {
                            string s = "{";
                            bool skip = true;
                            for( auto &p : *Internal.Map ) {
                                if( !skip ) s += ",";
                                s += ("\"" + p.first + "\":" + p.second.dumpMinified() );
                                skip = false;
                            }
                            s += "}";
                            return s;
                        }
                        case Class::Array: {
                            string s = "[";
                            bool skip = true;
                            for( auto &p : *Internal.List ) {
                                if( !skip ) s += ",";
                                s += p.dumpMinified();
                                skip = false;
                            }
                            s += "]";
                            return s;
                        }
                        case Class::String:
                            return "\"" + json_escape( *Internal.String ) + "\"";
                        case Class::Floating:
                            return std::to_string( Internal.Float );
                        case Class::Integral:
                            return std::to_string( Internal.Int );
                        case Class::Boolean:
                            return Internal.Bool ? "true" : "false";
                        default:
                            return "";
                    }
                    return "";
                }

                friend std::ostream& operator<<( std::ostream&, const JSON & );

            private:
                void SetType( Class type ) {
                    if( type == Type )
                        return;

                    ClearInternal();
                
                    switch( type ) {
                    case Class::Null:      Internal.Map    = nullptr;                break;
                    case Class::Object:    Internal.Map    = new map<string,JSON>(); break;
                    case Class::Array:     Internal.List   = new deque<JSON>();      break;
                    case Class::String:    Internal.String = new string();           break;
                    case Class::Floating:  Internal.Float  = 0.0;                    break;
                    case Class::Integral:  Internal.Int    = 0;                      break;
                    case Class::Boolean:   Internal.Bool   = false;                  break;
                    }

                    Type = type;
                }

            private:
            /* beware: only call if YOU know that Internal is allocated. No checks performed here. 
                This function should be called in a constructed JSON just before you are going to 
                overwrite Internal... 
            */
            void ClearInternal() {
                switch( Type ) {
                case Class::Object: delete Internal.Map;    break;
                case Class::Array:  delete Internal.List;   break;
                case Class::String: delete Internal.String; break;
                default:;
                }
            }

            private:

                Class Type = Class::Null;
        };

        inline JSON Array() {
            return JSON::Make( JSON::Class::Array );
        }

        template <typename... T>
        JSON Array( T... args ) {
            JSON arr = JSON::Make( JSON::Class::Array );
            arr.append( args... );
            return arr;
        }

        inline JSON Object() {
            return JSON::Make( JSON::Class::Object );
        }

        inline std::ostream& operator<<( std::ostream &os, const JSON &json ) {
            os << json.dump();
            return os;
        }

        namespace {
            JSON parse_next( const string &, size_t &, bool& );

            void consume_ws( const string &str, size_t &offset ) {
                while( isspace( str[offset] ) ) ++offset;
            }

            JSON parse_object( const string &str, size_t &offset, bool &ok ) {
                JSON Object = JSON::Make( JSON::Class::Object );

                ++offset;
                consume_ws( str, offset );
                if( str[offset] == '}' ) {
                    ++offset; return Object;
                }

                while( true ) {
                    JSON Key = parse_next( str, offset, ok );
                    consume_ws( str, offset );
                    if( str[offset] != ':' ) {
                        std::cerr << "Error: Object: Expected colon, found '" << str[offset] << "'";
                        ok = false;
                        break;
                    }
                    consume_ws( str, ++offset );
                    JSON Value = parse_next( str, offset, ok );
                    Object[Key.ToString()] = Value;
                    
                    consume_ws( str, offset );
                    if( str[offset] == ',' ) {
                        ++offset; continue;
                    }
                    else if( str[offset] == '}' ) {
                        ++offset; break;
                    }
                    else {
                        std::cerr << "Error: Object: Expected comma, found '" << str[offset] << "'";
                        ok = false;
                        break;
                    }
                }
                return Object;
            }

            JSON parse_array( const string &str, size_t &offset, bool &ok ) {
                JSON Array = JSON::Make( JSON::Class::Array );
                unsigned index = 0;
                
                ++offset;
                consume_ws( str, offset );
                if( str[offset] == ']' ) {
                    ++offset; return Array;
                }

                while( true ) {
                    Array[index++] = parse_next( str, offset, ok );
                    consume_ws( str, offset );

                    if( str[offset] == ',' ) {
                        ++offset; continue;
                    }
                    else if( str[offset] == ']' ) {
                        ++offset; break;
                    }
                    else {
                        std::cerr << "Error: Array: Expected ',' or ']', found '" << str[offset] << "'";
                        ok = false;
                        return JSON::Make( JSON::Class::Array );
                    }
                }
                return Array;
            }

            JSON parse_string( const string &str, size_t &offset, bool &ok ) {
                string val;
                for( char c = str[++offset]; c != '\"' ; c = str[++offset] ) {
                    if( c == '\\' ) {
                        switch( str[ ++offset ] ) {
                        case '\"': val += '\"'; break;
                        case '\\': val += '\\'; break;
                        case '/' : val += '/' ; break;
                        case 'b' : val += '\b'; break;
                        case 'f' : val += '\f'; break;
                        case 'n' : val += '\n'; break;
                        case 'r' : val += '\r'; break;
                        case 't' : val += '\t'; break;
                        case 'u' : {
                            val += "\\u" ;
                            for( unsigned i = 1; i <= 4; ++i ) {
                                c = str[offset+i];
                                if( (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F') )
                                    val += c;
                                else {
                                    std::cerr << "Error: String: Expected hex character in unicode escape, found '" << c << "'";
                                    ok = false;
                                    return JSON::Make( JSON::Class::String );
                                }
                            }
                            offset += 4;
                        } break;
                        default  : val += '\\'; break;
                        }
                    }
                    else
                        val += c;
                }
                ++offset;
                return JSON(val);
            }

            JSON parse_number( const string &str, size_t &offset, bool &ok ) {
                JSON Number;
                string val, exp_str;
                char c;
                bool isDouble = false;
                long long exp = 0;
                while( true ) {
                    c = str[offset++];
                    if( (c == '-') || (c >= '0' && c <= '9') )
                        val += c;
                    else if( c == '.' ) {
                        val += c; 
                        isDouble = true;
                    }
                    else
                        break;
                }
                if( c == 'E' || c == 'e' ) {
                    c = str[ offset ];
                    if( c == '-' ){ ++offset; exp_str += '-';}
                    if( c == '+' ){ ++offset;}
                    while( true ) {
                        c = str[ offset++ ];
                        if( c >= '0' && c <= '9' )
                            exp_str += c;
                        else if( !isspace( c ) && c != ',' && c != ']' && c != '}' ) {
                            std::cerr << "Error: Number: Expected a number for exponent, found '" << c << "'\n";
                            ok = false;
                            return JSON::Make( JSON::Class::Null );
                        }
                        else
                            break;
                    }
                    exp = std::stol( exp_str );
                }
                else if( !isspace( c ) && c != ',' && c != ']' && c != '}' ) {
                    std::cerr << "Error: Number: unexpected character '" << c << "'\n";
                    ok = false;
                    return JSON::Make( JSON::Class::Null );
                }
                --offset;
                
                if( isDouble )
                    Number = std::stod( val ) * std::pow( 10, exp );
                else {
                    if( !exp_str.empty() )
                        Number = std::stol( val ) * std::pow( 10, exp );
                    else
                        Number = std::stol( val );
                }
                return Number;
            }

            JSON parse_bool( const string &str, size_t &offset, bool &ok ) {
                JSON Bool;
                if( str.substr( offset, 4 ) == "true" )
                    Bool = true;
                else if( str.substr( offset, 5 ) == "false" )
                    Bool = false;
                else {
                    std::cerr << "Error: Bool: Expected 'true' or 'false', found '" << str.substr( offset, 5 ) << "'\n";
                    ok = false;
                    return JSON::Make( JSON::Class::Null );
                }
                offset += (Bool.ToBool() ? 4 : 5);
                return Bool;
            }

            JSON parse_null( const string &str, size_t &offset, bool &ok ) {
                if( str.substr( offset, 4 ) != "null" ) {
                    std::cerr << "Error: Null: Expected 'null', found '" << str.substr( offset, 4 ) << "'\n";
                    ok = false;
                    return JSON::Make( JSON::Class::Null );
                }
                offset += 4;
                return JSON();
            }

            JSON parse_next( const string &str, size_t &offset, bool &ok ) {
                char value;
                consume_ws( str, offset );
                value = str[offset];
                switch( value ) {
                    case '[' : return parse_array( str, offset, ok);
                    case '{' : return parse_object( str, offset, ok);
                    case '\"': return parse_string( str, offset, ok);
                    case 't' :
                    case 'f' : return parse_bool( str, offset, ok);
                    case 'n' : return parse_null( str, offset, ok);
                    default  : if( ( value <= '9' && value >= '0' ) || value == '-' )
                                return parse_number( str, offset, ok );
                }
                std::cerr << "Error: Parse: Unknown starting character '" << value << "'\n";
                ok = false;
                return JSON();
            }
        }
        inline JSON JSON::Load( const string &str, bool &ok ) {
            size_t offset = 0;
            ok = true;
            return parse_next( str, offset, ok );
        }
        inline JSON JSON::Load( const string &str ) {
            size_t offset = 0;
            bool ok = true;
            return parse_next( str, offset, ok );
        }
    } // End Namespace json
}
#endif //SUPPORTLIB_JSON_H