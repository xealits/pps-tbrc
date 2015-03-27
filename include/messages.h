#ifndef messages_h
#define messages_h

#include <string>
#include <vector>

/**
 * Convert a char/"char,char,..." pair into a <string,string,string,...>
 * vector
 * \param[in] a A word representing the first message type provided
 * \param[in] b A word representing a comma-separated list of message keys
 * (starting from the second in the user-defined list)
 * \return A well-defined, homogenized vector of strings to be propagated
 * through the message keys builder
 * \author Laurent Forthomme <laurent.forthomme@cern.ch>
 * \date 26 Mar 2015
 */
inline std::vector<std::string> sar(const char* a, const char* b)
{ 
  std::string o = b;
  std::vector<std::string> out;
  out.push_back(a);
  size_t pos; std::string token;
  while ((pos=o.find(","))!=std::string::npos) {
    token = o.substr(0, pos);
    out.push_back(token.c_str());
    o.erase(0, pos+2);
  }
  out.push_back(o.c_str());
  return out;
}

/**
 * Generate a list of message types (with a struct / string matching), given a
 * set of computer-readable names provided as an argument.
 * \brief Message keys builder
 * \note FIXME: is it sufficiently obfuscated ? I don't think so...
 * \author Laurent Forthomme <laurent.forthomme@cern.ch>
 * \date 26 Mar 2015
 */
#define ENUM(m1, ...)\
    enum MessageKey { m1=0, __VA_ARGS__  };\
    inline const char* MessageKeyToString(MessageKey value) {\
      return (sar(#m1, #__VA_ARGS__)[value]).c_str(); }\
    inline const MessageKey MessageKeyToObject(const char* value) {\
      for (size_t i=0; i<=sizeof(MessageKey); i++) {\
        if (sar(#m1, #__VA_ARGS__)[i]==std::string(value)) return (MessageKey)i;\
    } return (MessageKey)(-1); }
    
#endif

