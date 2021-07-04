#include <cstdlib>
#include "gtest/gtest.h"
#include "env.hpp"

struct Env
{
    std::string EnvCPPName;
    std::string EnvCPPAddress;
    int EnvCPPAge;
    long EnvCPPHeight;
    float EnvCPPEnergy;
};

// This needs to be in global namespace
REFL_TYPE(Env, bases<>)
    REFL_FIELD(EnvCPPName)
    REFL_FIELD(EnvCPPAddress)
    REFL_FIELD(EnvCPPAge)
    REFL_FIELD(EnvCPPHeight)
    REFL_FIELD(EnvCPPEnergy)
REFL_END


struct Env2
{
    std::string EnvString;
};

REFL_TYPE(Env2)
    REFL_FIELD(EnvString, env::Name("EnvStringOverride"), env::Default("EnvStringDefaultValue"))
REFL_END

struct Env3
{
    std::string EnvString;
};

REFL_TYPE(Env3)
    REFL_FIELD(EnvString, env::Required())
REFL_END

namespace mytest{

TEST(Parse, Ok) {
    bool ret;

    std::string EnvCPPName = "EnvCPPNameVal";
    ret = env::SetEnvVar("EnvCPPName",EnvCPPName);
    EXPECT_EQ(ret, true);

    std::string EnvCPPAddress = "EnvCPPAddressVal";
    ret = env::SetEnvVar("EnvCPPAddress",EnvCPPAddress);
    EXPECT_EQ(ret, true);

    int EnvCPPAge = 10;
    ret = env::SetEnvVar("EnvCPPAge","10");
    EXPECT_EQ(ret, true);

    long EnvCPPHeight = 20;
    ret = env::SetEnvVar("EnvCPPHeight","20");
    EXPECT_EQ(ret, true);

    float EnvCPPEnergy = 10.1;
    ret = env::SetEnvVar("EnvCPPEnergy","10.1");
    EXPECT_EQ(ret, true);

    // parse
    Env e;
    env::Parse(e);

    EXPECT_EQ(e.EnvCPPName,EnvCPPName);
    EXPECT_EQ(e.EnvCPPAddress,EnvCPPAddress);
    EXPECT_EQ(e.EnvCPPAge,EnvCPPAge);
    EXPECT_EQ(e.EnvCPPHeight,EnvCPPHeight);
    EXPECT_EQ(e.EnvCPPEnergy,EnvCPPEnergy);
}

TEST(Parse, Exceptions) {
    int ret;
    ret = env::SetEnvVar("EnvCPPAge","bad-integer");
    EXPECT_EQ(ret, true);

    Env e;
    EXPECT_THROW(env::Parse(e), env::ParseException);
}

TEST(Parse, NameAttr) {
    int ret;
    std::string EnvStringOverride = "EnvStringOverrideValue";
    ret = env::SetEnvVar("EnvStringOverride", EnvStringOverride);
    EXPECT_EQ(ret, true);

    Env2 e;
    env::Parse(e);

    EXPECT_EQ(e.EnvString, EnvStringOverride);
}

TEST(Parse, DefaultAttr) {
    int ret;
    ret = env::UnsetEnvVar("EnvStringOverride");
    EXPECT_EQ(ret, true);

    Env2 e;
    env::Parse(e);

    EXPECT_EQ(e.EnvString, std::string("EnvStringDefaultValue"));
}

TEST(Parse, RequiredAttr) {
    int ret;
    ret = env::UnsetEnvVar("EnvString");
    EXPECT_EQ(ret, true);
    
    Env3 e;
    EXPECT_THROW(env::Parse(e), env::EnvNotFoundException);

    ret = env::SetEnvVar("EnvString", "dummy");
    EXPECT_EQ(ret, true);
    env::Parse(e);
}

}