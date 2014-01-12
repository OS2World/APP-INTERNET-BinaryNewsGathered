#ifndef PTC_REGEX_HPP
#define PTC_REGEX_HPP

#include <ocl.hpp>
#include <ostring.hpp>

extern "C" {
    #include <libc\wchar.h>
    #include <libc\locale.h>
    #include <libc\regex.h>
}

typedef class Regex *pRegex;

class Regex
{
private:
    
    regex_t
        reg;           // regular expression status buffer
    
    INT
        cflags,
        eflags,
        rc;            // return code
    
    OString
        pattern,
        string ;
    
public:
    
    Regex(BOOL icase_in = REG_ICASE)
      : cflags(REG_EXTENDED | icase_in),
        eflags(0),
        rc(0)
    {}
    
    Regex(PCSZ string_in, PCSZ pattern_in, BOOL icase_in = REG_ICASE)
      : string(string_in),
        pattern(pattern_in),
        cflags(REG_EXTENDED | icase_in),
        eflags(0),
        rc(0)
    {}
    
    ~Regex() {}
    
    ULONG  compare()
    {
        if(rc = regcomp(&reg, pattern, cflags)) {
            OString buffer(100);
            regerror(rc, &reg, buffer, 100);
            cerr << "regcomp() failed with " << buffer << endl;
            return rc;
        }
        else {
            rc = regexec(&reg, string, 0, NULL, eflags);
        }
        regfree(&reg);
        return rc;
    }

    VOID  set(PCSZ s, PCSZ p) { string  << s; pattern << p; }
    VOID  setString  (PCSZ s) { string  << s; }
    VOID  setString  (pOString s) { string  << s->getText(); }
    VOID  setPattern (PCSZ p) { pattern << p; }
};

#endif // PTC_REGEX_HPP

//   #define REG_EXTENDED    0x001   /* Use Extended RE syntax rules         */
//   #define REG_ICASE       0x002   /* Ignore case in match                 */
//   #define REG_NOSUB       0x008   /* regexec() not report subexpressions  */
//   #define REG_NEWLINE     0x004   /* Convert <backslash><n> to <newline>  */
