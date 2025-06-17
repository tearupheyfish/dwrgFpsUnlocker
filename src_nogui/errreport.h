//
// Created by Tofu on 2025/5/15.
//

#ifndef ERRREPORT_H
#define ERRREPORT_H
#include <iostream>
#include <string>
struct ErrorReporter
{
    static constexpr char 严重[] = "严重";
    static auto instance()
    {
        static struct
        {
            void receive(const std::string& level, const std::string& msg)
            {
                std::cerr<<level<<':'<<msg<<std::endl;

                    std::exit(1);
            }
        }s;
        return &s;
    }
};
#endif //ERRREPORT_H
