
set(Firebase_ESP_Client_SRCS 
#    src/client/SSLClient/client/BSSL_CertStore.cpp
#    src/client/SSLClient/client/BSSL_Helper.cpp
#    src/client/SSLClient/client/BSSL_TCP_Client.cpp
#    src/client/SSLClient/client/BSSL_SSL_Client.cpp

    src/core/Updater/OTAUpdater.cpp

    src/core/JWT/JWT.cpp
    )
set(Firebase_ESP_Client_INCL ${CMAKE_CURRENT_SOURCE_DIR}/src)

idf_component_register(
    INCLUDE_DIRS ${Firebase_ESP_Client_INCL} 
    SRCS ${Firebase_ESP_Client_SRCS}
    REQUIRES 
        arduino 
        ArduinoBearSSL
        ms_Logger
    )

# Various compiler warnings enabled
target_compile_options(${COMPONENT_LIB} PRIVATE
    -Wall
    -Wextra
    -Wformat
    -Wformat=2
#    -Wshadow
#    -Wconversion
#    -Wsign-conversion
    -Wpointer-arith
#    -Wcast-qual
    -Wmissing-declarations
#    -Wmissing-prototypes   # only C, not C++
#    -Wstrict-prototypes    # only C, not C++
)
