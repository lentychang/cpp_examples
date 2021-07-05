#include <iostream>

int main(int argc, char **argv, char **envs)
{
    for (char **env = envs; *env != 0; env++)
    {
        char *thisEnv = *env;
        std::cout << thisEnv << std::endl;
    }
    return 0;
}