
set(Firebase_ESP_Client_SRCS 
    src/client/SSLClient/client/BSSL_CertStore.cpp
    src/client/SSLClient/client/BSSL_Helper.cpp
    src/client/SSLClient/client/BSSL_TCP_Client.cpp
    src/client/SSLClient/client/BSSL_SSL_Client.cpp

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
