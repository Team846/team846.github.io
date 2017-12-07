// Perl must be installed for this to function - http://strawberryperl.com/
#ifndef BUILD_H_
#define BUILD_H_

#include <string>

/*!
 * \brief the class that contains the build time and build number.
 */
class Build
{
public:
	/*!
	 * \brief public accessor for the build number. 
	 */
    static int GetNumber()
    {
        return NUMBER;
    }

    /*!
     * \brief Public accessor for the time of the last build.
     */
    static std::string GetTime()
    {
        return TIME;
    }

private:
    const static int NUMBER;
    const static std::string TIME;
};


#endif

