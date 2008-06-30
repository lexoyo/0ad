/**
 * =========================================================================
 * File        : wstartup.h
 * Project     : 0 A.D.
 * Description : windows-specific startup code
 * =========================================================================
 */

// license: GPL; see lib/license.txt

// linking with this component should automatically arrange for winit's
// functions to be called at the appropriate times.
//
// the current implementation manages to trigger initialization in-between
// calls to CRT init and the static C++ ctors. that means wpthread etc.
// APIs are safe to use from ctors, and winit initializers are allowed
// to use non-stateless CRT functions such as atexit.
//
// IMPORTANT NOTE: if compiling this into a static lib and not using VC8's
// "use library dependency inputs" linking mode, the object file will be
// discarded because it does not contain any symbols that resolve another
// module's undefined external(s). for a discussion of this topic, see:
// http://forums.microsoft.com/MSDN/ShowPost.aspx?PostID=144087
// workaround: in the main EXE project, reference a symbol from this module,
// thus forcing it to be linked in. example:
// #pragma comment(linker, "/include:_wstartup_InitAndRegisterShutdown")
// (doing that in this module isn't sufficient, because it would only
// affect the librarian and its generation of the static lib that holds
// this file. instead, the process of linking the main EXE must be fixed.)