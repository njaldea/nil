#include <boost/asio.hpp>
#include <experimental/source_location>
#include <iostream>
#include <thread>

#include <proto/message.pb.h>
#include <proto/request.pb.h>
#include <proto/type.pb.h>

int main()
{
    std::string message;
    {
        {
            nil::proto::Message m;
            nil::proto::Request r;
            r.set_content("hello world");
            m.set_data(r.SerializeAsString());
            m.set_type(nil::proto::type::Request);
            message = m.SerializeAsString();
        }

        {
            nil::proto::Message m;
            m.ParseFromString(message);
            std::cout << m.type() << std::endl;

            nil::proto::Request r;
            r.ParseFromString(m.data());
            std::cout << r.content() << std::endl;
        }
    }

    boost::asio::io_context context;
    const auto _ = boost::asio::make_work_guard(context);
    const auto thread = std::thread(
        [&context]()
        {
            while (true)
            {
                std::this_thread::sleep_for(std::chrono::seconds(1));
                context.post(
                    []()
                    {
                        const auto location = std::experimental::source_location::current();
                        std::cout << location.file_name() << ':'     //
                                  << location.function_name() << ':' //
                                  << location.line() << std::endl;
                    }
                );
            }
        }
    );
    context.run();
    // nil::misc::foo();
}
