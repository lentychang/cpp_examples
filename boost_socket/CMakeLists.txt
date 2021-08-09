find_package(Threads REQUIRED Threads)
find_package(Boost REQUIRED system serialization)

add_executable(serialization serialization.cpp)
target_include_directories(serialization PUBLIC /usr/local/include)
target_link_libraries(serialization PUBLIC Boost::serialization) 

add_executable(asio_synchro_server asio_synchro_server.cpp)
target_include_directories(asio_synchro_server PUBLIC /usr/local/include)
target_link_libraries(asio_synchro_server PUBLIC Boost::system Threads::Threads) 

add_executable(asio_synchron_client asio_synchron_client.cpp)
target_include_directories(asio_synchron_client PUBLIC /usr/local/include)
target_link_libraries(asio_synchron_client PUBLIC Boost::system Threads::Threads) 
