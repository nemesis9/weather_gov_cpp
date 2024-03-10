
#ifndef _WLOGGER_HPP_
#define _WLOGGER_HPP_

#include <iostream>
#include <sstream>
#include <map>

/*  From here:
    https://stackoverflow.com/questions/6168107/how-to-implement-a-good-debug-logging-feature-in-a-project
 */

/* consider adding boost thread id since we'll want to know whose writting and
 * won't want to repeat it for every single call */

/* consider adding policy class to allow users to redirect logging to specific
 * files via the command line
 */

enum loglevel_e
    {logERROR, logWARNING, logINFO, logDEBUG, logDEBUG1, logDEBUG2, logDEBUG3, logDEBUG4};



class logIt
{


public:
    // THIS WAS ORIGINAL
    //logIt(loglevel_e _loglevel = logERROR) {
    //    _buffer << _loglevel << " :"
    //        << std::string(
    //            _loglevel > logDEBUG
    //            ? (_loglevel - logDEBUG) * 4
    //            : 1
    //            , ' ');
    //}

    logIt(loglevel_e _loglevel = logERROR) {
        _buffer << wloglevel_strings[_loglevel] << ": ";
    }

    template <typename T>
    logIt & operator<<(T const & value)
    {
        _buffer << value;
        return *this;
    }

    ~logIt()
    {
        _buffer << std::endl;
        // This is atomic according to the POSIX standard
        // http://www.gnu.org/s/libc/manual/html_node/Streams-and-Threads.html
        std::cerr << _buffer.str();
    }

private:
    std::ostringstream _buffer;

    std::map<loglevel_e, std::string> wloglevel_strings{{logERROR, "ERROR ",}, {logWARNING, "WARNING "}, 
        {logINFO, "INFO"}, {logDEBUG, "DEBUG"}, {logDEBUG1, "DEBUG1"}, {logDEBUG2, "DEBUG2"}, 
        {logDEBUG3, "DEBUG3"}, {logDEBUG4, "DEBUG4"}};
};

extern loglevel_e wloglevel;

#define wlog(level) \
if (level > wloglevel) ; \
else logIt(level)

#endif


/*
Use it like this:

// define and turn off for the rest of the test suite
loglevel_e loglevel = logERROR;

void logTest(void) {
    loglevel_e loglevel_save = loglevel;

    loglevel = logDEBUG4;

    log(logINFO) << "foo " << "bar " << "baz";

    int count = 3;
    log(logDEBUG) << "A loop with "    << count << " iterations";
    for (int i = 0; i != count; ++i)
    {
        log(logDEBUG1) << "the counter i = " << i;
        log(logDEBUG2) << "the counter i = " << i;
    }

    loglevel = loglevel_save;
}
*/
