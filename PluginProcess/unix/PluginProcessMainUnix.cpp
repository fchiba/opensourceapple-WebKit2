/*
 * Copyright (C) 2011, 2014 Igalia S.L.
 * Copyright (C) 2011 Apple Inc.
 * Copyright (C) 2012 Samsung Electronics
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "config.h"
#include "PluginProcessMainUnix.h"

#if ENABLE(PLUGIN_PROCESS)

#include "ChildProcessMain.h"
#include "Logging.h"
#include "NetscapePlugin.h"
#include "PluginProcess.h"
#include <WebCore/FileSystem.h>
#include <stdlib.h>

#if PLATFORM(GTK)
#include <gtk/gtk.h>
#endif

#if defined(XP_UNIX)
#include <WebCore/PlatformDisplayX11.h>
#include <WebCore/XErrorTrapper.h>
#endif

namespace WebKit {

#if defined(XP_UNIX)
static std::unique_ptr<WebCore::XErrorTrapper> xErrorTrapper;
#endif // XP_UNIX

class PluginProcessMain final: public ChildProcessMainBase {
public:
    bool platformInitialize() override
    {
#if PLATFORM(GTK)
        gtk_init(nullptr, nullptr);
#endif

        return true;
    }

    bool parseCommandLine(int argc, char** argv) override
    {
        ASSERT(argc == 3);
        if (argc != 3)
            return false;

        if (!strcmp(argv[1], "-scanPlugin"))
#if PLUGIN_ARCHITECTURE(X11)
            exit(NetscapePluginModule::scanPlugin(argv[2]) ? EXIT_SUCCESS : EXIT_FAILURE);
#else
            exit(EXIT_FAILURE);
#endif

#if defined(XP_UNIX)
        if (WebCore::PlatformDisplay::sharedDisplay().type() == WebCore::PlatformDisplay::Type::X11) {
            auto* display = downcast<WebCore::PlatformDisplayX11>(WebCore::PlatformDisplay::sharedDisplay()).native();
            xErrorTrapper = std::make_unique<WebCore::XErrorTrapper>(display, WebCore::XErrorTrapper::Policy::Warn);
        }
#endif

        m_parameters.extraInitializationData.add("plugin-path", argv[2]);
        return ChildProcessMainBase::parseCommandLine(argc, argv);
    }
};

int PluginProcessMainUnix(int argc, char** argv)
{
    return ChildProcessMain<PluginProcess, PluginProcessMain>(argc, argv);
}

} // namespace WebKit

#endif
