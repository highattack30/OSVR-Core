/** @file
    @brief Header

    @date 2014

    @author
    Kevin M. Godby
    <kevin@godby.org>
    <http://sensics.com>

*/

// Copyright 2014 Sensics, Inc.
//
// All rights reserved.
//
// (Final version intended to be licensed under
// the Apache License, Version 2.0)

#ifndef INCLUDED_Context_h_GUID_DD0155F5_61A4_4A76_8C2E_D9614C7A9EBD
#define INCLUDED_Context_h_GUID_DD0155F5_61A4_4A76_8C2E_D9614C7A9EBD

// Internal Includes
#include <osvr/ClientKit/ContextC.h>
#include <osvr/ClientKit/ParametersC.h>
#include <osvr/ClientKit/Interface.h>

// Library/third-party includes
#include <boost/scoped_array.hpp>

// Standard includes
#include <string>
#include <stdexcept>

namespace osvr {

namespace clientkit {

    class ClientContext {
      public:
        /// @brief Initialize the library.
        /// @param applicationIdentifier A string identifying your application.
        /// Reverse DNS format strongly suggested.
        /// @param flags initialization options (reserved) - pass 0 for now.
        ClientContext(const std::string &applicationIdentifier,
                      uint32_t flags = 0u);

        /// @brief Initialize the context with an existing context.
        /// @note The ClientContext class takes ownership of the
        /// OSVR_ClientContext pointer.
        ClientContext(OSVR_ClientContext context);

        /// @brief Shutdown the library.
        ~ClientContext();

        /// @brief Updates the state of the context - call regularly in your
        /// mainloop.
        void update();

        /// @brief Get the interface associated with the given path.
        /// @param path A resource path (null-terminated string)
        /// @param[out] iface The interface object. May be freed when no longer
        /// needed, otherwise it will be freed when the context is closed.
        InterfacePtr getInterface(const std::string &path);

        std::string getStringParameter(const std::string &path);

      private:
        OSVR_ClientContext m_context;
    };

    inline ClientContext::ClientContext(
        const std::string &applicationIdentifier, uint32_t flags) {
        m_context = osvrClientInit(applicationIdentifier.data(), flags);
    }

    inline ClientContext::ClientContext(OSVR_ClientContext context)
        : m_context(context) {
        // do nothing
    }

    inline ClientContext::~ClientContext() {
        // Can't do this unless we have the full class specification for
        // OSVR_ClientContext defined prior to defining the ClientContext class.
        // delete m_context;
        // m_context = NULL;
        // TODO should we leave m_context dangling or delete it ourselves?
    }

    inline void ClientContext::update() {
        OSVR_ReturnCode ret = osvrClientUpdate(m_context);
        if (OSVR_RETURN_SUCCESS != ret) {
            throw std::runtime_error("Error updating context.");
        }
    }

    inline InterfacePtr ClientContext::getInterface(const std::string &path) {
        OSVR_ClientInterface interface = NULL;
        OSVR_ReturnCode ret =
            osvrClientGetInterface(m_context, path.c_str(), &interface);
        if (OSVR_RETURN_SUCCESS != ret) {
            throw std::runtime_error(
                "Couldn't create interface because the path was invalid.");
        }

        InterfacePtr iface = make_shared<Interface>(interface);

        return iface;
    }

    inline std::string
    ClientContext::getStringParameter(const std::string &path) {
        size_t length = 0;
        OSVR_ReturnCode ret = osvrClientGetStringParameterLength(
            m_context, path.c_str(), &length);
        if (OSVR_RETURN_SUCCESS != ret) {
            throw std::runtime_error(
                "Invalid context or null reference to length variable.");
        }

        if (0 == length) {
            return std::string();
        }

        boost::scoped_array<char> buf(new char[length]);
        ret = osvrClientGetStringParameter(m_context, path.c_str(), buf.get(),
                                           length);
        if (OSVR_RETURN_SUCCESS != ret) {
            throw std::runtime_error("Invalid context, null reference to "
                                     "buffer, or buffer is too small.");
        }

        return std::string(buf.get(), length);
    }

} // end namespace clientkit

} // end namespace osvr

#endif // INCLUDED_Context_h_GUID_DD0155F5_61A4_4A76_8C2E_D9614C7A9EBD
