#include <iostream>
#include <cstdlib>
#include <string>
#include "refl.hpp"

namespace env {

// functions to handle envs with OS.

// return true if exist
bool GetEnvVar(std::string const & key, std::string & ret) noexcept {
    char * val = std::getenv( key.c_str() );
    if (val == NULL) {
        return false;
    }
    ret = std::string(val);
    return true;
}

// return true if success
bool SetEnvVar(std::string const & key , std::string const & val){
    // TODO: handle windows APIs
    int ret = setenv(key.c_str(),val.c_str(), true/*overwrite*/);
    return ret == 0;
}

// exceptions
class ParseException: public std::exception{
public:
    std::exception m_innerException;
    std::string m_fieldName;
    std::string m_fieldValue;
    std::string m_errStr;
    ParseException(std::exception const &inner, std::string const &fieldName, std::string const &fieldValue) 
        : m_innerException(inner), m_fieldName(fieldName),m_fieldValue(fieldValue),m_errStr() {
            m_errStr = "parsing field [" + m_fieldName + 
            "] with value [" + m_fieldValue + "]: " + m_innerException.what();
        };
    const char* what() const noexcept override{
        return m_errStr.c_str();
    }
};



// static assert for constexpr
template<bool flag = false> void static_type_not_supported() { static_assert(flag, "type in user struct is not supported"); }

struct serializable : refl::attr::usage::field
{
};

template <typename T>
void Parse(T&& value)
{
    // iterate over the members of T
    for_each(refl::reflect(value).members, [&](auto member)
    {
        // is_readable checks if the member is a non-const field
        // or a 0-arg const-qualified function marked with property attribute
        if constexpr (is_readable(member) && is_writable(member) && refl::descriptor::has_attribute<serializable>(member))
        {
            auto fieldName = get_display_name(member);
            std::string val;
            bool exists = GetEnvVar(fieldName, val);

            std::cout << "fieldName:" << fieldName << '\n';
            std::cout << "val:" << val << '\n';
            std::cout << "exist:" << exists << '\n';

            if (!exists){
                return;
            }

            // macro to check if the member is of type "a"
            #define ENV_IS_TYPE_EQ(a) std::is_same<std::remove_reference_t<decltype(member(value))>, a>::value

            try {
                if constexpr (ENV_IS_TYPE_EQ(std::string)){
                    // env is the same that we read
                    member(value) = val;
                } else if constexpr (ENV_IS_TYPE_EQ(int)){
                    member(value) = std::stoi(val);
                } else if constexpr (ENV_IS_TYPE_EQ(long)){
                    member(value) = std::stol(val);
                } else if constexpr (ENV_IS_TYPE_EQ(long long)){
                    member(value) = std::stoll(val);
                } else if constexpr (ENV_IS_TYPE_EQ(float)){
                    member(value) = std::stof(val);
                } else if constexpr (true){
                    // If compiler error out on this line it means that user struct field type is not supported
                    static_type_not_supported();
                }
            }catch (std::exception& e){
                // TODO: include type of member in error message
                ParseException pe(e,fieldName,val);
                throw pe;
            }
        }
    });
}

} // namespace env