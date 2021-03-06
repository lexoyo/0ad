/* Copyright (C) 2009 Wildfire Games.
 * This file is part of 0 A.D.
 *
 * 0 A.D. is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * 0 A.D. is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with 0 A.D.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <boost/preprocessor/punctuation/comma_if.hpp>
#include <boost/preprocessor/repetition/repeat.hpp>

// Define lots of useful macros:

// Varieties of comma-separated list to fit on the head/tail/whole of another comma-separated list
#define NUMBERED_LIST_HEAD(z, i, data) data##i,
#define NUMBERED_LIST_TAIL(z, i, data) ,data##i
#define NUMBERED_LIST_BALANCED(z, i, data) BOOST_PP_COMMA_IF(i) data##i
// Some other things
#define TYPED_ARGS(z, i, data) , T##i a##i
#define CONVERT_ARG(z, i, data) T##i a##i; if (! ScriptInterface::FromJSVal<T##i>(cx, i < argc ? JS_ARGV(cx, vp)[i] : JSVAL_VOID, a##i)) return JS_FALSE;

// List-generating macros, named roughly after their first list item
#define TYPENAME_T0_HEAD(z, i) BOOST_PP_REPEAT_##z (i, NUMBERED_LIST_HEAD, typename T) // "typename T0, typename T1, "
#define TYPENAME_T0_TAIL(z, i) BOOST_PP_REPEAT_##z (i, NUMBERED_LIST_TAIL, typename T) // ", typename T0, typename T1"
#define T0(z, i) BOOST_PP_REPEAT_##z (i, NUMBERED_LIST_BALANCED, T) // "T0, T1"
#define T0_HEAD(z, i) BOOST_PP_REPEAT_##z (i, NUMBERED_LIST_HEAD, T) // "T0, T1, "
#define T0_TAIL(z, i) BOOST_PP_REPEAT_##z (i, NUMBERED_LIST_TAIL, T) // ", T0, T1"
#define T0_A0(z, i) BOOST_PP_REPEAT_##z (i, TYPED_ARGS, ~) // "T0 a0, T1 a1"
#define A0(z, i) BOOST_PP_REPEAT_##z (i, NUMBERED_LIST_BALANCED, a) // "a0, a1"
#define A0_TAIL(z, i) BOOST_PP_REPEAT_##z (i, NUMBERED_LIST_TAIL, a) // ", a0, a1"

// Define RegisterFunction<TR, T0..., f>
#define OVERLOADS(z, i, data) \
	template <typename R, TYPENAME_T0_HEAD(z,i)  R (*fptr) ( void* T0_TAIL(z,i) )> \
	void RegisterFunction(const char* name) { \
		Register(name, call<R, T0_HEAD(z,i)  fptr>, nargs<0 T0_TAIL(z,i)>()); \
	}
BOOST_PP_REPEAT(SCRIPT_INTERFACE_MAX_ARGS, OVERLOADS, ~)
#undef OVERLOADS

// JSFastNative-compatible function that wraps the function identified in the template argument list
// (Definition comes later, since it depends on some things we haven't defined yet)
#define OVERLOADS(z, i, data) \
	template <typename R, TYPENAME_T0_HEAD(z,i)  R (*fptr) ( void* T0_TAIL(z,i) )> \
	static JSBool call(JSContext* cx, uintN argc, jsval* vp);
BOOST_PP_REPEAT(SCRIPT_INTERFACE_MAX_ARGS, OVERLOADS, ~)
#undef OVERLOADS

// Similar, for class methods
#define OVERLOADS(z, i, data) \
	template <typename R, TYPENAME_T0_HEAD(z,i)  JSClass* CLS, typename TC, R (TC::*fptr) ( T0(z,i) )> \
	static JSBool callMethod(JSContext* cx, uintN argc, jsval* vp);
BOOST_PP_REPEAT(SCRIPT_INTERFACE_MAX_ARGS, OVERLOADS, ~)
#undef OVERLOADS

// Argument-number counter
#define OVERLOADS(z, i, data) \
	template <int dummy TYPENAME_T0_TAIL(z,i)> /* add a dummy parameter so we still compile with 0 template args */ \
	static size_t nargs() { return i; }
BOOST_PP_REPEAT(SCRIPT_INTERFACE_MAX_ARGS, OVERLOADS, ~)
#undef OVERLOADS
